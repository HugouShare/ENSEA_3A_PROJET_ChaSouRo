/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "VL53L0X.h"
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// === XSHUT PINS ===
#define XSHUT_TOF1_GPIO_Port GPIOB
#define XSHUT_TOF1_Pin       GPIO_PIN_6

#define XSHUT_TOF2_GPIO_Port GPIOB
#define XSHUT_TOF2_Pin       GPIO_PIN_13

#define XSHUT_TOF3_GPIO_Port GPIOB
#define XSHUT_TOF3_Pin       GPIO_PIN_0

#define XSHUT_TOF4_GPIO_Port GPIOC
#define XSHUT_TOF4_Pin       GPIO_PIN_14

// === I2C ADDRESSES (7 bits) ===
#define TOF1_ADDR 0x54
#define TOF2_ADDR 0x56
#define TOF3_ADDR 0x58
#define TOF4_ADDR 0x5A
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
VL53L0X_Dev_t tof1, tof2, tof3, tof4;
statInfo_t_VL53L0X measure1, measure2, measure3, measure4;
uint16_t dist1, dist2, dist3, dist4;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void init_tof(VL53L0X_Dev_t *tof,GPIO_TypeDef *port, uint16_t pin, uint8_t address);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int chr)
{
	HAL_UART_Transmit(&huart4, (uint8_t*) &chr, 1, HAL_MAX_DELAY);
	return chr;
}

void init_tof(VL53L0X_Dev_t *tof, GPIO_TypeDef *port, uint16_t pin, uint8_t address)
{
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	HAL_Delay(50);

	if (initVL53L0X(tof, 1, &hi2c3)!=1)
	{
		printf("VL53L0X init error\r\n");
		Error_Handler();
	}

	setAddress_VL53L0X(tof, address);
	setSignalRateLimit(tof, 0.1);
	setVcselPulsePeriod(tof, VcselPeriodPreRange, 18);
	setVcselPulsePeriod(tof, VcselPeriodFinalRange, 14);
	setMeasurementTimingBudget(tof, 200000);
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
	MX_I2C1_Init();
	MX_I2C3_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM8_Init();
	MX_UART4_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_TIM16_Init();
	MX_TIM7_Init();
	/* USER CODE BEGIN 2 */

	printf("\r\n ================= VL53L0X ================= \r\n");

	//	VL53L0X_Init(&h_vl53l0x);
	//	vTaskStartScheduler();


	// === RESET ALL SENSORS ===
	HAL_GPIO_WritePin(XSHUT_TOF1_GPIO_Port, XSHUT_TOF1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(XSHUT_TOF2_GPIO_Port, XSHUT_TOF2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(XSHUT_TOF3_GPIO_Port, XSHUT_TOF3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(XSHUT_TOF4_GPIO_Port, XSHUT_TOF4_Pin, GPIO_PIN_RESET);
	HAL_Delay(50);

	// === INIT SENSORS ONE BY ONE ===
	init_tof(&tof1, XSHUT_TOF1_GPIO_Port, XSHUT_TOF1_Pin, TOF1_ADDR);
	init_tof(&tof2, XSHUT_TOF2_GPIO_Port, XSHUT_TOF2_Pin, TOF2_ADDR);
	init_tof(&tof3, XSHUT_TOF3_GPIO_Port, XSHUT_TOF3_Pin, TOF3_ADDR);
	init_tof(&tof4, XSHUT_TOF4_GPIO_Port, XSHUT_TOF4_Pin, TOF4_ADDR);

	printf("All VL53L0X initialized successfully\r\n");


	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		dist1 = readRangeSingleMillimeters(&tof1, &measure1);
		dist2 = readRangeSingleMillimeters(&tof2, &measure2);
		dist3 = readRangeSingleMillimeters(&tof3, &measure3);
		dist4 = readRangeSingleMillimeters(&tof4, &measure4);

		printf("D1:%4d mm | D2:%4d mm | D3:%4d mm | D4:%4d mm\r\n",
				dist1, dist2, dist3, dist4);

		HAL_Delay(100);
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

/* USER CODE END 4 */

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
