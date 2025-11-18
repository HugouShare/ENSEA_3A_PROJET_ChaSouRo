/*
 * enc.c
 *
 *  Created on: Nov 9, 2025
 *      Author: hugoc
 */

#include "enc.h"

/* Instances globales */
Encoder_t ENC_D;
Encoder_t ENC_G;

/* Prototypes des tâches */
void task_ENC_D_Update(void *arg);
void task_ENC_G_Update(void *arg);

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
}

/* ---------------------------------------------------------------------
 * Tâche pour l'encodeur droit (ENC_D)
 * --------------------------------------------------------------------- */
void task_ENC_D_Update(void *arg)
{
    Encoder_t *E = (Encoder_t*)arg;

    for (;;)
    {
        /* Attente de la notification du timer d'échantillonnage */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        Encoder_Update_Generic(E);

        /* Affichage UART */
        char buffer[128];
        int len = sprintf(buffer,
            "[ENC_D] Pos:%7.2f deg | Vel:%7.2f deg/s | FWD:%ld | REV:%ld\r\n",
            E->position_deg, E->velocity_deg_s, E->FWD, E->REV);
        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);
    }
}

/* ---------------------------------------------------------------------
 * Tâche pour l'encodeur gauche (ENC_G)
 * --------------------------------------------------------------------- */
void task_ENC_G_Update(void *arg)
{
    Encoder_t *E = (Encoder_t*)arg;

    for (;;)
    {
        /* Attente de la notification du timer d'échantillonnage */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        Encoder_Update_Generic(E);

        /* Affichage UART */
        char buffer[128];
        int len = sprintf(buffer,
            "[ENC_G] Pos:%7.2f deg | Vel:%7.2f deg/s | FWD:%ld | REV:%ld\r\n",
            E->position_deg, E->velocity_deg_s, E->FWD, E->REV);
        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);
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

/* ---------------------------------------------------------------------
 * API publique
 * --------------------------------------------------------------------- */

///* Compatibilité : l'ancien ENC_Update() mettra à jour l'encodeur droit */
//void ENC_Update(void)
//{
//    Encoder_Update_Generic(&ENC_D);
//}

/* Mise à jour manuelle pour ENC_D */
void ENC_D_Update(void)
{
    Encoder_Update_Generic(&ENC_D);
}

/* Mise à jour manuelle pour ENC_G */
void ENC_G_Update(void)
{
    Encoder_Update_Generic(&ENC_G);
}

/* Initialisation : configure les structures, démarre timers/encoder et crée les tasks */
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

    if (xTaskCreate(task_ENC_D_Update, "ENC_D Task", 1024, &ENC_D, 5, &ENC_D.task_handle) != pdPASS) {
        Error_Handler();
    }

    /* Démarrage du timer encodeur et du timer d'échantillonnage */
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

    if (xTaskCreate(task_ENC_G_Update, "ENC_G Task", 1024, &ENC_G, 5, &ENC_G.task_handle) != pdPASS) {
        Error_Handler();
    }

    HAL_TIM_Encoder_Start(ENC_G.htim_enc, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(ENC_G.htim_sampling);
}






