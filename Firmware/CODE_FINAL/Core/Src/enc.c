/*
 * enc.c
 *
 *  Created on: Nov 9, 2025
 *      Author: hugoc
 */

#include "enc.h"
#include <math.h> // pour cosf, sinf

/* Instances globales */
Encoder_t ENC_D;
Encoder_t ENC_G;


robot_Pose_t robot_pose = {0.0f, 0.0f, 0.0f};

/* Prototypes des tâches */
void task_ENC_D_Update(void *arg);
void task_ENC_G_Update(void *arg);
void task_Odom_Update(void *arg);

static TaskHandle_t odom_task_handle = NULL;

/* Fonction générique d'update d'un encodeur */
static void Encoder_Update_Generic(Encoder_t *E)
{
    int32_t current = (int32_t)__HAL_TIM_GET_COUNTER(E->htim_enc);
    int32_t delta   = current - E->last_position;

    /* Gestion du wrap-around 16 bits */
    if (delta > 32767)  delta -= 65536;
    if (delta < -32768) delta += 65536;

    E->total_ticks += delta;
    E->last_position = current;

    /* LED : détecte mouvement */
    if (delta != 0) {
        E->led_timer = LED_ON_TIME_MS / TIMER_PERIOD_MS;
    }

    /* Conversion en degrés */
    float raw_deg = ((float)E->total_ticks / TICKS_PER_REV) * 360.0f;

    /* Détection du passage de +360° ou -360° */
    if (raw_deg >= 360.0f) {
        E->FWD++;
        E->total_ticks -= TICKS_PER_REV;
    } else if (raw_deg <= -360.0f) {
        E->REV++;
        E->total_ticks += TICKS_PER_REV;
    }

    /* Position modulo 360 */
    E->position_deg = ((float)E->total_ticks / TICKS_PER_REV) * 360.0f;

    /* Vitesse (°/s) - on suppose TIMER_PERIOD_MS en ms */
    E->velocity_deg_s = ((float)delta / TICKS_PER_REV) * (360.0f * (1000.0f / TIMER_PERIOD_MS));

    /* Gestion LED (diagnostic simple sur PA5) */
    if (E->led_timer > 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        E->led_timer--;
    } else {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }

    /* Calcul de la distance parcourue depuis la dernière mise à jour
     * à stocker dans la structure */
    E->delta_distance = ((float)delta / TICKS_PER_REV) * E->wheel_circumference; // en mètres ou unité choisie
}

/* ---------------------------------------------------------------------
 * Tâche pour l'encodeur droit (ENC_D)
 * --------------------------------------------------------------------- */
void task_ENC_D_Update(void *arg)
{
    Encoder_t *E = (Encoder_t*)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    for (;;)
    {
        /* Attente de la notification du timer d'échantillonnage */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        Encoder_Update_Generic(E);

        ENC_D.has_been_updated = true;
        if (ENC_D.has_been_updated && ENC_G.has_been_updated){
            ENC_D.has_been_updated = false;
            ENC_G.has_been_updated = false;
            vTaskNotifyGiveFromISR(odom_task_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

//        /* Affichage UART */
//        char buffer[128];
//        int len = sprintf(buffer,
//            "[ENC_D] Pos:%7.2f deg | Vel:%7.2f deg/s | FWD:%ld | REV:%ld\r\n",
//            E->position_deg, E->velocity_deg_s, E->FWD, E->REV);
//        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);
    }
}

/* ---------------------------------------------------------------------
 * Tâche pour l'encodeur gauche (ENC_G)
 * --------------------------------------------------------------------- */
void task_ENC_G_Update(void *arg)
{
    Encoder_t *E = (Encoder_t*)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    for (;;)
    {
        /* Attente de la notification du timer d'échantillonnage */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        Encoder_Update_Generic(E);

        ENC_G.has_been_updated = true;
        if (ENC_D.has_been_updated && ENC_G.has_been_updated){
            ENC_D.has_been_updated = false;
            ENC_G.has_been_updated = false;
            vTaskNotifyGiveFromISR(odom_task_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

//        /* Affichage UART */
//        char buffer[128];
//        int len = sprintf(buffer,
//            "[ENC_G] Pos:%7.2f deg | Vel:%7.2f deg/s | FWD:%ld | REV:%ld\r\n",
//            E->position_deg, E->velocity_deg_s, E->FWD, E->REV);
//        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);
    }
}

/* ---------------------------------------------------------------------
 * Tâche d'odométrie (calcul de la position globale)
 * --------------------------------------------------------------------- */
void task_Odom_Update(void *arg)
{
    (void)arg;

    for(;;)
    {
        /* Attente notification (donnée par les encodeurs) */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* Lecture des distances delta */
        float d_left = ENC_G.delta_distance;
        float d_right = ENC_D.delta_distance;

        /* Calcul de la distance moyenne */
        float d_center = (d_left + d_right) / 2.0f;

        /* Calcul du changement d'angle (en radians) */
        float d_theta = (d_right - d_left) / WHEEL_BASE;

        /* Mise à jour de la pose */
        robot_pose.theta += d_theta;

        /* Normalisation de theta entre -pi et pi */
        if (robot_pose.theta > M_PI)
            robot_pose.theta -= 2.0f * M_PI;
        else if (robot_pose.theta < -M_PI)
            robot_pose.theta += 2.0f * M_PI;

        /* Mise à jour x,y en fonction de la nouvelle orientation */
        robot_pose.x += d_center * cosf(robot_pose.theta);
        robot_pose.y += d_center * sinf(robot_pose.theta);

//        /* Optionnel : affichage sur UART */
//        char buf[128];
//        int len = sprintf(buf, "Pose: x=%.3f m, y=%.3f m, theta=%.3f rad\r\n",
//            robot_pose.x, robot_pose.y, robot_pose.theta);
//        HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, HAL_MAX_DELAY);
    }
}

/* ---------------------------------------------------------------------
 * Callback des timers d'échantillonnage : notifie la tâche correcte
 * --------------------------------------------------------------------- */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (htim == ENC_D.htim_sampling) {
        vTaskNotifyGiveFromISR(ENC_D.task_handle, &xHigherPriorityTaskWoken);
    }

    if (htim == ENC_G.htim_sampling) {
        vTaskNotifyGiveFromISR(ENC_G.task_handle, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Initialisation : configure les structures, démarre timers/encodeur et crée les tasks */
void ENC_Init(void)
{
    /* ----------------------------
     * ENC_D (encodeur droit)
     * ---------------------------- */
    ENC_D.htim_enc      = &ENC_D_htimx;
    ENC_D.htim_sampling = &ENC_D_sampling_htimx;

    ENC_D.last_position = (int32_t)__HAL_TIM_GET_COUNTER(ENC_D.htim_enc);
    ENC_D.total_ticks   = 0;
    ENC_D.position_deg  = 0.0f;
    ENC_D.velocity_deg_s = 0.0f;
    ENC_D.FWD = 0;
    ENC_D.REV = 0;
    ENC_D.led_timer = 0;
    ENC_D.task_handle = NULL;
    ENC_D.has_been_updated = false;
    ENC_D.delta_distance = 0.0f;
    ENC_D.wheel_circumference = WHEEL_DIAMETER * 3.14159265359f * WHEEL_DIAMETER_ERROR;

    if (xTaskCreate(task_ENC_D_Update, "ENC_D Task", 1024, &ENC_D, 5, &ENC_D.task_handle) != pdPASS) {
        Error_Handler();
    }

    HAL_TIM_Encoder_Start(ENC_D.htim_enc, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(ENC_D.htim_sampling);

    /* ----------------------------
     * ENC_G (encodeur gauche)
     * ---------------------------- */
    ENC_G.htim_enc      = &ENC_G_htimx;
    ENC_G.htim_sampling = &ENC_G_sampling_htimx;

    ENC_G.last_position = (int32_t)__HAL_TIM_GET_COUNTER(ENC_G.htim_enc);
    ENC_G.total_ticks   = 0;
    ENC_G.position_deg  = 0.0f;
    ENC_G.velocity_deg_s = 0.0f;
    ENC_G.FWD = 0;
    ENC_G.REV = 0;
    ENC_G.led_timer = 0;
    ENC_G.task_handle = NULL;
    ENC_G.has_been_updated = false;
    ENC_G.delta_distance = 0.0f;
    ENC_G.wheel_circumference = WHEEL_DIAMETER * 3.14159265359f;

    if (xTaskCreate(task_ENC_G_Update, "ENC_G Task", 1024, &ENC_G, 5, &ENC_G.task_handle) != pdPASS) {
        Error_Handler();
    }

    HAL_TIM_Encoder_Start(ENC_G.htim_enc, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(ENC_G.htim_sampling);

    /* Création de la tâche odométrie */
    if (xTaskCreate(task_Odom_Update, "Odom Task", 1024, NULL, 6, &odom_task_handle) != pdPASS) {
        Error_Handler();
    }
}
