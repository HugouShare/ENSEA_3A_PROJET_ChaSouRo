/*
 * control.c
 */

#include "control.h"
#include <math.h>
#include <stdlib.h>

/* ================= GLOBAL ================= */

static ctrl_cmd_t ctrl;
static SemaphoreHandle_t ctrl_mutex;
static QueueHandle_t ctrlQueue;

static PID_t pid_turn;
static PID_t pid_turn_small_angles;
static PID_t pid_move;

/* ================= INIT ================= */

void Control_Init(void)
{
    ctrl_mutex = xSemaphoreCreateMutex();
    configASSERT(ctrl_mutex != NULL);

    ctrlQueue = xQueueCreate(1, sizeof(ctrl_request_t));
    configASSERT(ctrlQueue != NULL);

    ctrl.mode = CTRL_IDLE;

    /* PID rotation */
    PID_Init(&pid_turn,
             KP_TURN, KI_TURN, KD_TURN,
             -MOTOR_PWM_MAX,
              MOTOR_PWM_MAX);

    /* PID rotation */
    PID_Init(&pid_turn_small_angles,
             KP_TURN_SA, KI_TURN_SA, KD_TURN_SA,
             -MOTOR_PWM_MAX,
              MOTOR_PWM_MAX);

    /* PID translation */
    PID_Init(&pid_move,
             KP_MOVE, KI_MOVE, KD_MOVE,
             -MOTOR_PWM_MAX,
              MOTOR_PWM_MAX);
}

void Control_Tasks_Create(void)
{
    if (xTaskCreate(task_Control,
                    "CTRL",
                    512,
                    NULL,
                    task_Control_PRIORITY,
                    NULL) != pdPASS)
    {
        Error_Handler();
    }
}

/* ================= API (TASK) ================= */

void Control_TurnAngle(int32_t angle_deg)
{
    if (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdTRUE)
    {
        ctrl.mode = CTRL_TURN;
        ctrl.target = angle_deg;
        ctrl.start_theta = robot_pose.theta;
        xSemaphoreGive(ctrl_mutex);
    }
}

void Control_MoveDistance(int32_t distance_mm)
{
    if (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdTRUE)
    {
        ctrl.mode = CTRL_MOVE;
        ctrl.target = distance_mm;
        ctrl.start_dist = robot_pose.x_dist;
        xSemaphoreGive(ctrl_mutex);
    }
}

void Control_Stop(void)
{
    if (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdTRUE)
    {
        ctrl.mode = CTRL_IDLE;
        robot_pose.theta = 0;
        robot_pose.x_dist = 0;
        xSemaphoreGive(ctrl_mutex);
    }

    Motors_SetPWM(0, 0);
}

/* ================= API (ISR) ================= */

void Control_TurnAngleFromISR(int32_t angle_deg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    ctrl_request_t req = {
        .mode = CTRL_TURN,
        .target = angle_deg
    };

    xQueueOverwriteFromISR(ctrlQueue, &req, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Control_MoveDistanceFromISR(int32_t distance_mm)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    ctrl_request_t req = {
        .mode = CTRL_MOVE,
        .target = distance_mm
    };

    xQueueOverwriteFromISR(ctrlQueue, &req, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Control_StopFromISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    ctrl_request_t req = {
        .mode = CTRL_IDLE,
        .target = 0
    };

    xQueueOverwriteFromISR(ctrlQueue, &req, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


bool Control_IsBusy(void)
{
    bool busy;

    if (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdTRUE)
    {
        busy = (ctrl.mode != CTRL_IDLE);
        xSemaphoreGive(ctrl_mutex);
    }

    return busy;
}

/* ================= TASK ================= */

void task_Control(void *arg)
{
    (void)arg;

    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        /* ===== Réception commandes ISR ===== */
        ctrl_request_t req;
        if (xQueueReceive(ctrlQueue, &req, 0) == pdPASS)
        {
            if (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdTRUE)
            {
                ctrl.mode = req.mode;
                ctrl.target = req.target;

                if (req.mode == CTRL_TURN)
                    ctrl.start_theta = robot_pose.theta;

                if (req.mode == CTRL_MOVE)
                    ctrl.start_dist = robot_pose.x_dist;

                xSemaphoreGive(ctrl_mutex);
            }
        }

        vTaskDelayUntil(&last, pdMS_TO_TICKS(20)); /* 50 Hz */

        ctrl_cmd_t local;
        if (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdTRUE)
        {
            local = ctrl;
            xSemaphoreGive(ctrl_mutex);
        }

        /* ===== IDLE ===== */
        if (local.mode == CTRL_IDLE)
        {
            Motors_SetPWM(0, 0);
            continue;
        }

        /* ===== ROTATION ===== */
        if (local.mode == CTRL_TURN)
        {
            int32_t delta = robot_pose.theta - local.start_theta;

            /* Normalisation [-180 ; +180] */
            if (delta > 180)  delta -= 360;
            if (delta < -180) delta += 360;

            int32_t error = local.target - delta;

            if (abs(error) < ANGLE_THRESHOLD_DEG)
            {
                Control_Stop();
                continue;
            }

            int32_t cmd = PID_Compute(&pid_turn, error);
            Motors_SetPWM(-cmd, cmd);
        }

        /* ===== TRANSLATION (POLAIRE) ===== */
        /* ===== TRANSLATION (POLAIRE + MAINTIEN DE CAP) ===== */
        if (local.mode == CTRL_MOVE)
        {
            /* --- Erreur de distance --- */
            int32_t dist = robot_pose.x_dist - local.start_dist;
            int32_t error_dist = local.target - dist;

            /* Arrêt si proche de la cible */
            if (abs(error_dist) < DIST_THRESHOLD_MM)
            {
                PID_Reset(&pid_move);
                PID_Reset(&pid_turn_small_angles);
                Control_Stop();
                continue;
            }

            /* --- Erreur d'angle (heading hold) --- */
            int32_t error_theta = local.start_theta - robot_pose.theta;

            /* Normalisation [-180 ; +180] */
            if (error_theta > 180)  error_theta -= 360;
            if (error_theta < -180) error_theta += 360;

            /* --- PID distance (vitesse moyenne) --- */
            int32_t cmd_dist = PID_Compute(&pid_move, error_dist);

            /* --- Reset intégrale PID angle si vitesse faible (évite windup) --- */
            if (abs(cmd_dist) < 5)
            {
                PID_ResetIntegrator(&pid_turn_small_angles);
            }

            /* --- PID angle (correction différentielle) --- */
            int32_t cmd_theta = PID_Compute(&pid_turn_small_angles, error_theta);

            /* --- Commande moteurs --- */
            int32_t left  = cmd_dist - cmd_theta;
            int32_t right = cmd_dist + cmd_theta;

            Motors_SetPWM(left, right);
        }

    }
}
