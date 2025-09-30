/*
 * LIDAR.h
 *
 *  Created on: Sep 10, 2025
 *      Author: hugoc
 */

#ifndef INC_LIDAR_H_
#define INC_LIDAR_H_

////////////////////////////////////////////////////////////////////////INCLUDES
#include "stm32l4xx_hal.h"		//same as the one in main.h
#include "stdint.h"
#include "stdio.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>  // pour FLT_MAX

////////////////////////////////////////////////////////////////////////PARAMETERS
//Paramètres généraux
#define LID_htimx	htim2
#define LID_TIM_CHANNEL_X TIM_CHANNEL_1
#define LID_huartx huart4

//Filtrage et reconnaissance de cluster
#define LID_SPEED 50		//angular speed in %
#define SATISFYING_BUFFER_FILL_RATIO 0.5f			//pourcentage de remplissage du buffer non filtré satisfaisant

#define LIDAR_MIN_DIST  1    // mm
#define LIDAR_MAX_DIST  200    // mm
#define MAX_DISTANCE_GAP  200  // mm gap between consecutive points -> new cluster
#define MIN_POINTS_CLUSTER  3	//nombre de points minimum pour détecter un cluster
#define MAX_CLUSTERS 32

////////////////////////////////////////////////////////////////////////CONSTANTS
#define LID_RX_BUF_SIZE   256     // taille du buffer UART DMA (à ajuster)
#define LID_SAMPLE_NUMBER 360    // nombre de points par scan
#define MAX_FRAME_SIZE    120     // taille max d’une trame LIDAR (bytes)
#define RAD_TO_DEG        57.29577951308232f

#define MEDIAN_KERNEL_SIZE 5   // Taille de la fenêtre du filtre median(impair recommandé)


////////////////////////////////////////////////////////////////////////STRUCTURES
typedef struct {
	uint16_t distance_mm;
	uint16_t angle_x_deg;
	uint8_t quality;
} LIDAR_Sample;

typedef struct {
	uint8_t data[MAX_FRAME_SIZE];
	uint16_t length;
} LIDAR_Frame;

typedef struct {
	bool send_frame_to_pc;
	bool RxHalfCpltCallback;
	bool RxCpltCallback;
} LIDAR_Flags_system;

typedef struct {
	uint16_t start_angle_deg;
	uint16_t end_angle_deg;
} LIDAR_Gap;

typedef struct {
	float x;           // position moyenne X du cluster (m)
	float y;           // position moyenne Y du cluster (m)
	uint16_t nPoints;  // nombre de points dans le cluster
	float minDist;     // distance mini
	float maxDist;     // distance maxi
	float angle_deg;// angle du centre géométrique du cluster
} LIDAR_Cluster;

////////////////////////////////////////////////////////////////////////PUBLIC VARIABLES
extern uint8_t lid_rx_buf[LID_RX_BUF_SIZE];
extern LIDAR_Sample LID_list_of_samples[LID_SAMPLE_NUMBER];
extern LIDAR_Flags_system LID_Flags;
extern LIDAR_Cluster clusters[MAX_CLUSTERS];


////////////////////////////////////////////////////////////////////////DECLARATIONS
void LIDAR_Init(void);
void LIDAR_While(void);
void LID_TIMX_SetDuty(uint8_t duty_percent);

bool verify_checksum(const uint8_t *buf, uint16_t len);
void LID_ProcessDMA(uint8_t *data, uint16_t len, bool RxCpltCallback);
void manage_current_frame(LIDAR_Frame current_frame, uint8_t sample_count);
void LID_Store_Sample(LIDAR_Sample sample);

void LIDAR_SendBuffer_Frame(UART_HandleTypeDef *huart);

void LID_clear_sample_buffer(void);
void LID_clear_cluster_buffer(void);

uint16_t median_filter(uint16_t *values, uint8_t n);
void LIDAR_ApplyMedianFilter(LIDAR_Sample* buffer, uint16_t sample_count);
void LIDAR_FindClusters(void);
float find_Cluster_norm(LIDAR_Sample sample, LIDAR_Sample prev_sample);

float LID_FillRate_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len);
uint16_t LID_MinDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len);
uint16_t LID_MaxDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len);
float LID_MeanDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len);


#endif /* INC_LIDAR_H_ */
