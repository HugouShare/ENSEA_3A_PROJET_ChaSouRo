/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_D2_Pin GPIO_PIN_13
#define LED_D2_GPIO_Port GPIOC
#define XShunt4_Pin GPIO_PIN_14
#define XShunt4_GPIO_Port GPIOC
#define LED_D1_Pin GPIO_PIN_15
#define LED_D1_GPIO_Port GPIOC
#define USER2_Pin GPIO_PIN_1
#define USER2_GPIO_Port GPIOA
#define USER2_EXTI_IRQn EXTI1_IRQn
#define ENC_PA_R_Pin GPIO_PIN_4
#define ENC_PA_R_GPIO_Port GPIOA
#define FWD_L_Pin GPIO_PIN_5
#define FWD_L_GPIO_Port GPIOA
#define ENC_PB_R_Pin GPIO_PIN_6
#define ENC_PB_R_GPIO_Port GPIOA
#define LED_STATE_Pin GPIO_PIN_7
#define LED_STATE_GPIO_Port GPIOA
#define XShunt3_Pin GPIO_PIN_0
#define XShunt3_GPIO_Port GPIOB
#define EN_BT_Pin GPIO_PIN_1
#define EN_BT_GPIO_Port GPIOB
#define XShunt2_Pin GPIO_PIN_13
#define XShunt2_GPIO_Port GPIOB
#define ENC_PA_L_Pin GPIO_PIN_6
#define ENC_PA_L_GPIO_Port GPIOC
#define FWD_R_Pin GPIO_PIN_9
#define FWD_R_GPIO_Port GPIOA
#define REV_R_Pin GPIO_PIN_10
#define REV_R_GPIO_Port GPIOA
#define INT_XL2_Pin GPIO_PIN_11
#define INT_XL2_GPIO_Port GPIOA
#define INT_XL1_Pin GPIO_PIN_12
#define INT_XL1_GPIO_Port GPIOA
#define REV_L_Pin GPIO_PIN_3
#define REV_L_GPIO_Port GPIOB
#define Buzzer_Pin GPIO_PIN_4
#define Buzzer_GPIO_Port GPIOB
#define XShunt1_Pin GPIO_PIN_6
#define XShunt1_GPIO_Port GPIOB
#define XShunt1_EXTI_IRQn EXTI9_5_IRQn
#define ENC_PB_L_Pin GPIO_PIN_8
#define ENC_PB_L_GPIO_Port GPIOB
#define USER1_Pin GPIO_PIN_9
#define USER1_GPIO_Port GPIOB
#define USER1_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
