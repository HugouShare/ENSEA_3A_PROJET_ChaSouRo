/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
**/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "actuators/motor.h"
#include "actuators/encoder.h"
// #include "ssd1306.h" Finalement pas utilisee
// #include "ssd1306_fonts.h" Finalement pas utilisee
#include "sensors/accelerometer.h"
#include "actuators/control.h"
#include "actuators/behavior.h"
#include "sensors/tofs.h"
#include "bluetooth/bluetooth.h"
#include "sensors/lidar.h"
//#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
ADXL345_HandleTypeDef hadxl;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
h_hc05_t h_hc05 =
{
		.huart = &huart3
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//TERMINAL
int __io_putchar(int chr)
{
	HAL_UART_Transmit(&huart4, (uint8_t*) &chr, 1, HAL_MAX_DELAY);
	return chr;
}

//ANTI-REBOND
bool debounce_check(uint32_t *lastTick, TickType_t delay_ms)
{
	TickType_t now = xTaskGetTickCountFromISR();

	if ((now - *lastTick) < pdMS_TO_TICKS(delay_ms))
	{
		return false; // rebond, ignorer
	}
	*lastTick = now;
	return true; // action validée
}

//PROTECTION MEMOIRE
void vApplicationMallocFailedHook(void)
{
	printf("Malloc failed !\r\n"); //n'arrive pas à créé une task (pas de place)
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

//PROTECTION MEMOIRE
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("Stack overflow in task %s\r\n", pcTaskName); //Stack overflow
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM8_Init();
  MX_UART4_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM16_Init();
  MX_TIM15_Init();
  MX_TIM17_Init();
  /* USER CODE BEGIN 2 */

	/* INITS */
	Init_motors();
	//LIDAR_Init();
	TOFs_Init();
	ENC_Init();
	ADXL345_Init(&hadxl, &hi2c1);
	Control_Init();
	//OLED_Init();

	/* CREATION DES TASKS */
	TOFs_Tasks_Create();
	Control_Tasks_Create();
	Motors_Tasks_Create();
	LIDAR_Tasks_Create();
	ENC_Tasks_Create();
	ADXL345_StartTasks(&hadxl);
	behavior_Tasks_Create();
	HC05_Tasks_Create(&h_hc05);
	//OLED_Tasks_Create();

	/* LANCEMENT DU SCHEDULER */
	vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// Finalement pas utilisé
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//    static TickType_t lastTickUser1 = 0;
//    static uint8_t toggle = 1;
//
//    if (GPIO_Pin == USER1_Pin)
//    {
//        if (!debounce_check(&lastTickUser1, 200))
//            return; // Ignorer rebond
//        if(toggle){
//        	Control_MoveDistanceFromISR(1000);
//			toggle = 0;
//        }
//    }
//}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (huart == &LID_huartx)
	{
		vTaskNotifyGiveFromISR(htask_LIDAR_Update, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (huart == &LID_huartx)
	{
		vTaskNotifyGiveFromISR(htask_LIDAR_Update, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	if (huart->Instance==USART3)
	{
		hc05_RX_callback(&h_hc05);
	}
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

	//CODE DES CALLBACKS
	enc_HAL_TIM_PeriodElapsedCallback(htim);
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
		HAL_GPIO_WritePin(GPIOC, LED_D1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, LED_D2_Pin, GPIO_PIN_SET);
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
