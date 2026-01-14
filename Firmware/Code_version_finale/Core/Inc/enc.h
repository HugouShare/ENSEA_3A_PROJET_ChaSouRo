/*
 * enc.h
 *
 *  Created on: Nov 9, 2025
 *      Author: hugoc
 */

#ifndef INC_ENC_H_
#define INC_ENC_H_

#include "stm32g4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "freeRTOS_tasks_priority.h"

// -----------------------------
// Défines d'origine (conservés)
// -----------------------------
#define ENC_D_htimx            htim3
#define ENC_D_sampling_htimx   htim17

#define PPR 224.5f
#define TICKS_PER_REV (PPR * 4.0f)
#define TIMER_PERIOD_MS 5
#define LED_ON_TIME_MS 5
#define WHEEL_BASE           0.15f   // distance entre les roues en mètres (ex : 15 cm)
#define WHEEL_DIAMETER       0.07f   // diamètre de la roue en mètres (ex : 7 cm) (roue gauche)
#define WHEEL_DIAMETER_ERROR 1.0f	 // Rapport diamètre roue droite sur diamètre roue gauche, à régler

#define ENC_STACK_SIZE 40
#define ODOM_STACK_SIZE 64

// -----------------------------
// Défines pour l'encodeur gauche
// (tu peux mapper vers d'autres timers si besoin)
// -----------------------------
#define ENC_G_htimx          htim8
#define ENC_G_sampling_htimx htim17

// Extern des timers (via defines ci-dessus)
extern TIM_HandleTypeDef ENC_D_htimx;
extern TIM_HandleTypeDef ENC_D_sampling_htimx;
extern TIM_HandleTypeDef ENC_G_htimx;
extern TIM_HandleTypeDef ENC_G_sampling_htimx;

extern UART_HandleTypeDef huart2;
extern void Error_Handler(void);

// on va utiliser les LUTS du fichier LIDAR.c
extern const int16_t cos_lut[360];
extern const int16_t sin_lut[360];

// -----------------------------
// Structure demandée (strictement conservée)
// -----------------------------
typedef struct {
    TIM_HandleTypeDef *htim_enc;
    TIM_HandleTypeDef *htim_sampling;
    TaskHandle_t task_handle;

    volatile uint32_t last_position;
    volatile int32_t total_ticks;
    volatile int32_t position_deg;
    volatile int32_t velocity_deg_s;

    volatile int8_t FWD;
    volatile int8_t REV;


    volatile bool has_been_updated;

    volatile int32_t delta_distance;	//en  mm
    volatile int32_t wheel_circumference;	//en  mm
} Encoder_t;

typedef struct {
    int32_t x; //en  mm
    int32_t y; //en  mm
    uint16_t theta; //en  deg
} robot_Pose_t;

// Instances externes
extern Encoder_t ENC_D; // encodeur droit
extern Encoder_t ENC_G; // encodeur gauche

extern robot_Pose_t robot_pose;

// API (noms conservés + additions pour les deux encodeurs)
void ENC_Init(void);      // initialise les 2 encodeurs, crée les tasks
void ENC_Update(void);    // compatibilité : met à jour ENC_D (ancien comportement)
void ENC_D_Update(void);
void ENC_G_Update(void);
void enc_HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void ENC_Tasks_Create(void);



#endif /* INC_ENC_H_ */
