#ifndef INC_LIDAR_H_
#define INC_LIDAR_H_

////////////////////////////////////////////////////////////////////////INCLUDES
#include "stm32g4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>  // pour abs()
#include "FreeRTOS.h"
#include "task.h"

////////////////////////////////////////////////////////////////////////PARAMETERS
//Paramètres généraux
#define LID_htimx htim15
#define LID_TIM_CHANNEL_X TIM_CHANNEL_2
#define LID_huartx huart2
#define LID_hdma_usartx_rx hdma_usart2_rx

//DMA
#define LIDAR_DMA_BUF_SIZE     512
#define LIDAR_MAX_FRAME_SIZE   256
#define LIDAR_N_ANGLES         360
#define MEDIAN_KERNEL_SIZE 5

//Filtrage et reconnaissance de cluster
#define LID_SPEED 50		//angular speed in %
#define SATISFYING_BUFFER_FILL_RATIO 70			//pourcentage de remplissage du buffer non filtré satisfaisant
#define MIN_POINTS_CLUSTER  5	//nombre de points minimum pour détecter un cluster

//Distances et seuils
#define LIDAR_MIN_VALID_DIST   10
#define LIDAR_MIN_CLUSTER_DIST 20
#define LIDAR_DIST_THRESHOLD   300
#define LIDAR_MERGE_THRESHOLD  50
#define LIDAR_MAX_RANGE        300

#define LIDAR_MAX_CLUSTERS     32
#define LIDAR_MAX_SAMPLES_PKT  200U

// LUTS POUR COS ET SIN
#define DEG_COUNT 360
#define Q15_SCALE 32768
#define Q15_SHIFT 15


////////////////////////////////////////////////////////////////////////CONSTANTS


#define RAD_TO_DEG 57.29577951308232f

////////////////////////////////////////////////////////////////////////STRUCTURES
typedef struct {
	uint16_t angle_deg;
	uint16_t distance_mm;
	bool quality;
} LIDAR_Sample;

typedef struct {
	uint8_t data[LIDAR_MAX_FRAME_SIZE];
	uint16_t length;
} LIDAR_Frame;

typedef struct {
	int16_t x;
	int16_t y;
	uint16_t start_idx;
	uint16_t end_idx;
	uint16_t size;
	bool active;
} LIDAR_Cluster;


////////////////////////////////////////////////////////////////////////STRUCTURES
extern LIDAR_Cluster LIDAR_clusters[LIDAR_MAX_CLUSTERS];
extern volatile uint8_t LIDAR_cluster_count;

extern UART_HandleTypeDef LID_huartx;
extern DMA_HandleTypeDef LID_hdma_usartx_rx;
extern TIM_HandleTypeDef LID_htimx;


extern void Error_Handler(void);
////////////////////////////////////////////////////////////////////////FONCTIONS
void LIDAR_Init(void);
void LIDAR_While(void);

//FREERTOS
void task_LIDAR_Update(void *unused);
void task_test(void*unused);


#endif /* INC_LIDAR_H_ */
