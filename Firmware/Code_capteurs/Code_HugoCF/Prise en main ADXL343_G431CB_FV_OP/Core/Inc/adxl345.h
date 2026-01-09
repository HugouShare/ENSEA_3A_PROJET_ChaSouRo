#ifndef ADXL345_H
#define ADXL345_H

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////// INCLUDES
#include "stm32g4xx_hal.h"
#include "cmsis_os.h"
#include <stdint.h>

////////////////////////////////////////////// DEFINES
/* --------------------------------------------------------------------------
 *  ADXL345 — Constantes matérielles
 * -------------------------------------------------------------------------- */
#define ADXL345_I2C_ADDR            0x53U      // adresse 7 bits
#define ADXL345_DEVID_EXPECTED      0xE5

/* Registres ADXL345 */
#define ADXL345_REG_DEVID           0x00
#define ADXL345_REG_BW_RATE         0x2C
#define ADXL345_REG_POWER_CTL       0x2D
#define ADXL345_REG_DATA_FORMAT     0x31
#define ADXL345_REG_DATAX0          0x32

/* Configuration */
#define ADXL345_MEASURE_MODE        0x08
#define ADXL345_FULL_RES            0x08
#define ADXL345_RANGE_2G            0x00

////////////////////////////////////////////// STRUCTURES
/* --------------------------------------------------------------------------
 *  Structure du driver
 * -------------------------------------------------------------------------- */
typedef struct {
	I2C_HandleTypeDef *hi2c;
	uint8_t address;          // adresse 8 bits pour HAL (<<1)
	float scale_mg_lsb;       // sensibilité typique 3.9 mg/LSB
} h_adxl345_t;


/* Structure envoyée via queue FreeRTOS */
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} ADXL345_Data_t;

////////////////////////////////////////////// FONCTIONS DRIVER
/* --------------------------------------------------------------------------
 *  API Driver
 * -------------------------------------------------------------------------- */
HAL_StatusTypeDef ADXL345_Init(h_adxl345_t * h_adxl345);
HAL_StatusTypeDef ADXL345_ReadXYZ(h_adxl345_t * h_adxl345, int16_t *x, int16_t *y, int16_t *z);

/* Si besoin d'accéder à la queue depuis autre fichier */
extern QueueHandle_t xADXL_Queue;

#ifdef __cplusplus
}
#endif

#endif // ADXL345_H
