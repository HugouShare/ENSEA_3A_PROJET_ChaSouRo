////////////////////////////////////////////// INCLUDES
#include "adxl345.h"
#include <stdio.h>

////////////////////////////////////////////// VARIABLES
/* --------------------------------------------------------------------------
 * Variables internes
 * -------------------------------------------------------------------------- */
QueueHandle_t xADXL_Queue = NULL;

////////////////////////////////////////////// FONCTIONS
/* --------------------------------------------------------------------------
 * Fonctions internes de lecture/écriture registre
 * -------------------------------------------------------------------------- */
static HAL_StatusTypeDef adxl_write_reg(h_adxl345_t * h_adxl345, uint8_t reg, uint8_t value)
{
	return HAL_I2C_Mem_Write(h_adxl345->hi2c, h_adxl345->address, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef adxl_read_reg(h_adxl345_t * h_adxl345, uint8_t reg, uint8_t *value)
{
	return HAL_I2C_Mem_Read(h_adxl345->hi2c, h_adxl345->address, reg, I2C_MEMADD_SIZE_8BIT, value, 1, HAL_MAX_DELAY);
}

/* --------------------------------------------------------------------------
 * Lecture XYZ
 * -------------------------------------------------------------------------- */
HAL_StatusTypeDef ADXL345_ReadXYZ(h_adxl345_t * h_adxl345, int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6];

	if (HAL_I2C_Mem_Read(h_adxl345->hi2c, h_adxl345->address, ADXL345_REG_DATAX0,
			I2C_MEMADD_SIZE_8BIT, buf, 6, HAL_MAX_DELAY) != HAL_OK)
	{
		return HAL_ERROR;
	}

	*x = (int16_t)((buf[1] << 8) | buf[0]);
	*y = (int16_t)((buf[3] << 8) | buf[2]);
	*z = (int16_t)((buf[5] << 8) | buf[4]);

	return HAL_OK;
}


/* --------------------------------------------------------------------------
 * Tâche FreeRTOS : lecture ADXL345
 * -------------------------------------------------------------------------- */
static void ADXL_TaskRead(h_adxl345_t * h_adxl345)
{
	ADXL345_Data_t sample;

	for (;;)
	{
		if (ADXL345_ReadXYZ(h_adxl345, &sample.x, &sample.y, &sample.z) == HAL_OK)
		{
			xQueueSend(xADXL_Queue, &sample, 0);
		}
		vTaskDelay(pdMS_TO_TICKS(100));   // 10 Hz
	}
}


/* --------------------------------------------------------------------------
 * Tâche FreeRTOS : affichage UART
 * -------------------------------------------------------------------------- */
static void ADXL_TaskPrint(void * unused)
{
	ADXL345_Data_t data;
	for (;;)
	{
		if (xQueueReceive(xADXL_Queue, &data, portMAX_DELAY) == pdPASS)
		{
			printf("ADXL345 => X=%6d  Y=%6d  Z=%6d\r\n", data.x, data.y, data.z);
		}
	}
}

/* --------------------------------------------------------------------------
 * Initialisation du capteur
 * -------------------------------------------------------------------------- */
HAL_StatusTypeDef ADXL345_Init(h_adxl345_t * h_adxl345)
{
	printf("\r\n ================= ADXL345 ================= \r\n");

	h_adxl345->address = ADXL345_I2C_ADDR << 1;     // HAL = adresse 8 bits
	h_adxl345->scale_mg_lsb = 3.9f;                 // sensibilité typique

	uint8_t devid = 0;

	/* Lire l’ID de l’ADXL345 */
	if (adxl_read_reg(h_adxl345, ADXL345_REG_DEVID, &devid) != HAL_OK)
		return HAL_ERROR;

	if (devid != ADXL345_DEVID_EXPECTED)
		return HAL_ERROR;

	/* 100 Hz */
	if (adxl_write_reg(h_adxl345, ADXL345_REG_BW_RATE, 0x0A) != HAL_OK)
		return HAL_ERROR;

	/* Full resolution, ±2g */
	uint8_t format = ADXL345_FULL_RES | ADXL345_RANGE_2G;
	if (adxl_write_reg(h_adxl345, ADXL345_REG_DATA_FORMAT, format) != HAL_OK)
		return HAL_ERROR;

	/* Mode mesure */
	if (adxl_write_reg(h_adxl345, ADXL345_REG_POWER_CTL, ADXL345_MEASURE_MODE) != HAL_OK)
		return HAL_ERROR;

	HAL_Delay(10);

	return HAL_OK;

	printf("\n\r ADXL345 détecté et initialisé. \n\r");
}

void ADXL345_Tasks_Create (h_adxl345_t * h_adxl345)
{
	if (xADXL_Queue == NULL)
	{
		xADXL_Queue = xQueueCreate(10, sizeof(ADXL345_Data_t));
	}
	if (xTaskCreate(ADXL_TaskRead,  "ADXL_Read",  256, h_adxl345, 2, NULL) != pdPASS)
	{
		printf("\n\r Error creating ADXL read task \n\r");
		Error_Handler();
	}
	if (xTaskCreate(ADXL_TaskPrint, "ADXL_Print", 256, h_adxl345, 1, NULL) != pdPASS)
	{
		printf("\n\r Error creating ADXL print task \n\r");
		Error_Handler();
	}
}
