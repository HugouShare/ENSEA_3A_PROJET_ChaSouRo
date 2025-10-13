#ifndef INC_LIDAR_H_
#define INC_LIDAR_H_

////////////////////////////////////////////////////////////////////////INCLUDES
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>  // pour abs()
#include "FreeRTOS.h"
#include "task.h"

////////////////////////////////////////////////////////////////////////PARAMETERS
//Paramètres généraux
#define LID_htimx htim2
#define LID_TIM_CHANNEL_X TIM_CHANNEL_1
#define LID_huartx huart4
#define LID_hdma_uartx_rx hdma_uart4_rx

//DMA
#define LIDAR_DMA_BUF_SIZE     1024
#define LIDAR_MAX_FRAME_SIZE   256
#define LIDAR_N_ANGLES         360
#define MEDIAN_KERNEL_SIZE 5

//Filtrage et reconnaissance de cluster
#define LID_SPEED 50		//angular speed in %
#define SATISFYING_BUFFER_FILL_RATIO 0.7f			//pourcentage de remplissage du buffer non filtré satisfaisant
#define MIN_POINTS_CLUSTER  5	//nombre de points minimum pour détecter un cluster

//Distances et seuils
#define LIDAR_MIN_VALID_DIST   10
#define LIDAR_MIN_CLUSTER_DIST 20.0f
#define LIDAR_DIST_THRESHOLD   300
#define LIDAR_MERGE_THRESHOLD  50.0f
#define LIDAR_MAX_RANGE        300

#define LIDAR_MAX_CLUSTERS     32
#define LIDAR_MAX_SAMPLES_PKT  200U


////////////////////////////////////////////////////////////////////////CONSTANTS


#define RAD_TO_DEG 57.29577951308232f

////////////////////////////////////////////////////////////////////////STRUCTURES
typedef struct {
	float X;
	float Y;
	float angle_deg;
	uint16_t distance_mm;
	bool quality;
} LIDAR_Sample;

typedef struct {
	uint8_t data[LIDAR_MAX_FRAME_SIZE];
	uint16_t length;
} LIDAR_Frame;

typedef struct {
	float x;
	float y;
	uint16_t start_idx;
	uint16_t end_idx;
	uint16_t size;
	bool active;
} LIDAR_Cluster;


////////////////////////////////////////////////////////////////////////STRUCTURES
extern LIDAR_Cluster LIDAR_clusters[LIDAR_MAX_CLUSTERS];
extern uint8_t LIDAR_cluster_count;

extern UART_HandleTypeDef LID_huartx;
extern DMA_HandleTypeDef LID_hdma_uartx_rx;
extern TIM_HandleTypeDef LID_htimx;


extern void Error_Handler(void);
extern int	printf (const char *__restrict, ...)
_ATTRIBUTE ((__format__ (__printf__, 1, 2)));

////////////////////////////////////////////////////////////////////////FONCTIONS
void LIDAR_Init(void);
void LIDAR_While(void);

//FREERTOS
void task_LIDAR_Update(void *unused);
void task_test(void*unused);


#endif /* INC_LIDAR_H_ */
