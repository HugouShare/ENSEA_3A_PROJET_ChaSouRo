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
        ctrl.start_x = robot_pose.x;
        ctrl.start_y = robot_pose.y;
        xSemaphoreGive(ctrl_mutex);
    }
}

void Control_Stop(void)
{
    if (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdTRUE)
    {
        ctrl.mode = CTRL_IDLE;
        robot_pose.theta = 0;			//version simplifiée, on remet tout à 0
        robot_pose.x = 0;
        robot_pose.y = 0;
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
                {
                    ctrl.start_x = robot_pose.x;
                    ctrl.start_y = robot_pose.y;
                }

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

            while (delta > 180)  delta -= 360;
            while (delta < -180) delta += 360;

            int32_t error = local.target - delta;

            if (abs(error) < ANGLE_THRESHOLD_DEG)
            {
                Control_Stop();
                continue;
            }

            int32_t cmd = PID_Compute(&pid_turn, error);
            Motors_SetPWM(-cmd, cmd);
        }

        /* ===== TRANSLATION ===== */
        if (local.mode == CTRL_MOVE)
        {
            int32_t dx = robot_pose.x - local.start_x;
            int32_t dy = robot_pose.y - local.start_y;

            int32_t dist = (int32_t)sqrt((double)(dx * dx + dy * dy));
            int32_t error = local.target - dist;

            if (abs(error) < DIST_THRESHOLD_MM)
            {
                Control_Stop();
                continue;
            }

            int32_t cmd = PID_Compute(&pid_move, error);
            Motors_SetPWM(cmd, cmd);
        }
    }
}
