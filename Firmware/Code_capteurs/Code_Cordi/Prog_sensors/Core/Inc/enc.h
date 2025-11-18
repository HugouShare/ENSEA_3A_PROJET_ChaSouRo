


/*
 * enc.h
 *
 *  Created on: Nov 9, 2025
 *      Author: hugoc
 */

#ifndef INC_ENC_H_
#define INC_ENC_H_


#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>  // pour abs()
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

// -----------------------------
// Défines d'origine (conservés)
// -----------------------------
#define ENC_D_htimx            htim1
#define ENC_D_sampling_htimx   htim6

#define PPR 224.5
#define TICKS_PER_REV (PPR * 4)
#define TIMER_PERIOD_MS 5
#define LED_ON_TIME_MS 5

// -----------------------------
// Défines pour l'encodeur gauche
// (tu peux mapper vers d'autres timers si besoin)
// -----------------------------
#define ENC_G_htimx          htim1
#define ENC_G_sampling_htimx htim6

// Extern des timers (via defines ci-dessus)
extern TIM_HandleTypeDef ENC_htimx;
extern TIM_HandleTypeDef ENC_sampling_htimx;
extern TIM_HandleTypeDef ENC_G_htimx;
extern TIM_HandleTypeDef ENC_G_sampling_htimx;

extern UART_HandleTypeDef huart2;
extern void Error_Handler(void);

// -----------------------------
// Structure demandée (strictement conservée)
// -----------------------------
typedef struct {
    TIM_HandleTypeDef *htim_enc;
    TIM_HandleTypeDef *htim_sampling;
    TaskHandle_t task_handle;

    volatile int32_t last_position;
    volatile int32_t total_ticks;
    volatile float position_deg;
    volatile float velocity_deg_s;

    volatile int32_t FWD;
    volatile int32_t REV;

    volatile int32_t led_timer;

} Encoder_t;

// Instances externes
extern Encoder_t ENC_D; // droit
extern Encoder_t ENC_G; // gauche

// API (noms conservés + additions pour les deux encodeurs)
void ENC_Init(void);      // initialise les 2 encodeurs, crée les tasks
void ENC_Update(void);    // compatibilité : met à jour ENC_D (ancien comportement)
void ENC_D_Update(void);
void ENC_G_Update(void);


#endif /* INC_ENC_H_ */
