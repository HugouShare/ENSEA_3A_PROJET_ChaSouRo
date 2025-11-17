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



#define ENC_htimx htim1
#define ENC_sampling_htimx htim6


#define PPR 224.5 //341.2           // PPR de l'encodeur
#define TICKS_PER_REV (PPR * 4)     // Quadrature → 4 ticks par impulsion
#define TIMER_PERIOD_MS 5           // période de sampling de TIM2 en ms
#define LED_ON_TIME_MS 5

extern TIM_HandleTypeDef ENC_htimx;
extern TIM_HandleTypeDef ENC_sampling_htimx;
extern UART_HandleTypeDef huart2;

extern void Error_Handler(void);

void ENC_Init(void);
void ENC_Update(void);


#endif /* INC_ENC_H_ */
