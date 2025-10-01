#ifndef INC_LIDAR_H_
#define INC_LIDAR_H_

////////////////////////////////////////////////////////////////////////INCLUDES
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>  // pour abs()

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
extern uint8_t LIDAR_dma_buf[LIDAR_DMA_BUF_SIZE];
extern volatile uint32_t LIDAR_dma_read_idx;
extern LIDAR_Sample LIDAR_view[LIDAR_N_ANGLES];
extern LIDAR_Cluster LIDAR_clusters[LIDAR_MAX_CLUSTERS];
extern uint8_t LIDAR_cluster_count;

extern UART_HandleTypeDef LID_huartx;
extern DMA_HandleTypeDef LID_hdma_uartx_rx;
extern TIM_HandleTypeDef LID_htimx;

/*------------------ FONCTIONS ------------------*/
void LIDAR_Init(void);
void LIDAR_While(void);
void LIDAR_ProcessDMA(void);
void LIDAR_StoreSample(LIDAR_Sample sample);
void LIDAR_ManageFrame(LIDAR_Frame frame, uint16_t sample_count);

int compute_cluster(LIDAR_Sample *points, uint16_t start, uint16_t end, uint8_t idx);
void segment_points(LIDAR_Sample *points, uint16_t n_points);
bool verify_checksum(const uint8_t *buf, uint16_t len);

void LIDAR_SegmentSamples(LIDAR_Sample *samples, uint16_t n_samples);
int LIDAR_ComputeCluster(LIDAR_Sample *samples, uint16_t start, uint16_t end, uint8_t idx);

void LIDAR_FindClusters(void);
float LIDAR_findClusterNorm(LIDAR_Sample sample, LIDAR_Sample prev_sample);
void LIDAR_clear_view_buffer(void);
void LIDAR_ApplyMedianFilter(LIDAR_Sample* buffer, uint16_t sample_count);
uint16_t median_filter(uint16_t *values, uint8_t n);
void LID_TIMX_SetDuty(uint8_t duty_percent);

#endif /* INC_LIDAR_H_ */
