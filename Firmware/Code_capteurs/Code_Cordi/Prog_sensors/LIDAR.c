/*
 * LIDAR.c
 *
 *  Created on: Sep 10, 2025
 *      Author: hugoc
 */
#include "LIDAR.h"

////////////////////////////////////////////////////////////////////////EXTERNAL VARIABLES
extern TIM_HandleTypeDef LID_htimx;
extern UART_HandleTypeDef LID_huartx;
extern UART_HandleTypeDef huart2; //Send data to pc for debug

////////////////////////////////////////////////////////////////////////PRIVATE VARIABLES
uint8_t lid_rx_buf[LID_RX_BUF_SIZE];
static LIDAR_Frame current_frame = {0};

LIDAR_Sample LID_list_of_samples[LID_SAMPLE_NUMBER];
uint16_t sample_cnt = 0;
static float buffer_fill_ratio = 0.0f;

LIDAR_Flags_system LID_Flags = {0};

LIDAR_Cluster clusters[MAX_CLUSTERS];
uint16_t cluster_count = 0;

////////////////////////////////////////////////////////////////////////
// INIT
////////////////////////////////////////////////////////////////////////
void LIDAR_Init(void)
{
	HAL_TIM_PWM_Start(&LID_htimx, LID_TIM_CHANNEL_X);
	HAL_UART_Receive_DMA(&LID_huartx, lid_rx_buf, LID_RX_BUF_SIZE);
	LID_TIMX_SetDuty(LID_SPEED);
}

////////////////////////////////////////////////////////////////////////
// WHILE(1)
////////////////////////////////////////////////////////////////////////
void LIDAR_While(void)
{
	if(LID_Flags.RxHalfCpltCallback){
		LID_ProcessDMA(lid_rx_buf, LID_RX_BUF_SIZE / 2, false);
		LID_Flags.RxHalfCpltCallback = false;
	}

	if(LID_Flags.RxCpltCallback){
		LID_ProcessDMA(&lid_rx_buf[LID_RX_BUF_SIZE / 2], LID_RX_BUF_SIZE / 2, true);
		LID_Flags.RxCpltCallback = false;
	}
	if (LID_Flags.send_frame_to_pc){		//debug
		LIDAR_SendBuffer_Frame(&huart2);
		LID_clear_sample_buffer();
		LID_Flags.send_frame_to_pc = false;
	}
	if (buffer_fill_ratio>SATISFYING_BUFFER_FILL_RATIO){ //traiter le buffer et le vider
		//		LID_Compute_Angular_sections_fill_rate();
		//		LIDAR_SendBuffer_Frame(&huart2);
		//		HAL_Delay(3000);

		LIDAR_ApplyMedianFilter(LID_list_of_samples, LID_SAMPLE_NUMBER);
		LIDAR_FindClusters();
		LID_clear_sample_buffer();


	}
}

////////////////////////////////////////////////////////////////////////
// SET DUTY
////////////////////////////////////////////////////////////////////////
void LID_TIMX_SetDuty(uint8_t duty_percent) {
	if (duty_percent > 100) duty_percent = 100;
	uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&LID_htimx);
	uint32_t ccr = (duty_percent * (arr + 1)) / 100;
	__HAL_TIM_SET_COMPARE(&LID_htimx, LID_TIM_CHANNEL_X, ccr);
}

////////////////////////////////////////////////////////////////////////
// CALLBACKS UART DMA
////////////////////////////////////////////////////////////////////////
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == LID_huartx.Instance)
		LID_Flags.RxHalfCpltCallback = true;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == LID_huartx.Instance)
		LID_Flags.RxCpltCallback = true;
}

////////////////////////////////////////////////////////////////////////
// CHECKSUM
////////////////////////////////////////////////////////////////////////
bool verify_checksum(const uint8_t *buf, uint16_t len)
{
	if (len % 2 != 0) return false;
	uint16_t acc = 0;
	for (uint16_t i = 0; i < len; i += 2) {
		uint16_t w = (uint16_t)buf[i] | ((uint16_t)buf[i + 1] << 8);
		acc ^= w;
	}
	return (acc == 0);
}

////////////////////////////////////////////////////////////////////////
// TRAITEMENT DMA
////////////////////////////////////////////////////////////////////////
void LID_ProcessDMA(uint8_t *data, uint16_t len, bool RxCpltCallback)
{
	static uint16_t buff_idx = 0;
	static bool frame_detected = false;
	static uint16_t expected_len = 0;
	static uint8_t sample_count = 0;



	for (uint16_t i = 0; i < len; i++) {
		uint8_t b = data[i];

		// -----------------------------
		// Detect header 0xAA55
		// -----------------------------
		if (buff_idx == 0 && b == 0xAA) { current_frame.data[0] = b; buff_idx = 1; continue; }
		if (buff_idx == 1 && b == 0x55) { current_frame.data[1] = b; buff_idx = 2; continue; }

		// -----------------------------
		// Read CT
		// -----------------------------
		if (buff_idx == 2) { current_frame.data[2] = b; buff_idx++; continue; }

		// -----------------------------
		// Read LSN (sample count)
		// -----------------------------
		if (buff_idx == 3) {
			current_frame.data[3] = b; buff_idx++;
			sample_count = current_frame.data[3];

			if (sample_count > ((MAX_FRAME_SIZE - 10) / 2)) {
				buff_idx = 0;
				frame_detected = false;
				continue;
			}

			expected_len = 10 + 2 * sample_count;
			frame_detected = true;
			continue;
		}

		// -----------------------------
		// Accumulate remaining bytes
		// -----------------------------
		if (frame_detected && buff_idx < expected_len) {
			current_frame.data[buff_idx++] = b;
			continue;
		}

		// -----------------------------
		// Full frame received
		// -----------------------------
		if (frame_detected && buff_idx == expected_len) {
			if (!verify_checksum(current_frame.data, expected_len)) { buff_idx = 0; frame_detected = false; continue; }
			manage_current_frame(current_frame, sample_count);

			// Reset for next frame
			buff_idx = 0;
			frame_detected = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// MANAGE CURRENT FRAME
////////////////////////////////////////////////////////////////////////

void manage_current_frame(LIDAR_Frame current_frame, uint8_t sample_count){
	uint16_t FSA = (uint16_t)current_frame.data[4] | ((uint16_t)current_frame.data[5] << 8);
	uint16_t LSA = (uint16_t)current_frame.data[6] | ((uint16_t)current_frame.data[7] << 8);

	float angle_fsa = (float)(FSA >> 1) / 64.0f;
	float angle_lsa = (float)(LSA >> 1) / 64.0f;
	float diff_angle = angle_lsa - angle_fsa; if (diff_angle < 0) diff_angle += 360.0f;

	for (uint16_t s = 0; s < sample_count; s++) {
		uint16_t offset = 10 + 2 * s;
		uint16_t S_i = (uint16_t)current_frame.data[offset] | ((uint16_t)current_frame.data[offset + 1] << 8);

		float distance_mm_f = (float)(S_i >> 2);
		float angle_deg_f = (sample_count == 1) ? angle_fsa : angle_fsa + (diff_angle * s) / (sample_count - 1);

		if (distance_mm_f > 0.0f) {
			float ratio = 21.8f * (155.3f - distance_mm_f) / (155.3f * distance_mm_f);
			angle_deg_f += atanf(ratio) * RAD_TO_DEG;
		}

		if (angle_deg_f >= 360.0f) angle_deg_f -= 360.0f;
		if (angle_deg_f < 0.0f) angle_deg_f += 360.0f;

		LIDAR_Sample sample;
		sample.distance_mm = (uint16_t)distance_mm_f;
		sample.angle_deg_x10 = (uint16_t)(angle_deg_f * 10);
		sample.quality = (S_i & 0x0001) ? 0 : 1;

		LID_Store_Sample(sample);
	}
}


////////////////////////////////////////////////////////////////////////
// STOCKAGE DES SAMPLES
////////////////////////////////////////////////////////////////////////
void LID_Store_Sample(LIDAR_Sample sample) {
	uint16_t idx = sample.angle_deg_x10;
	if (idx >= LID_SAMPLE_NUMBER) return;

	if (LID_list_of_samples[idx].distance_mm == 0){
		sample_cnt++;								//si c'est un nouvel échantillon
	}

	LID_list_of_samples[idx].distance_mm = sample.distance_mm;
	LID_list_of_samples[idx].angle_deg_x10 = sample.angle_deg_x10;
	LID_list_of_samples[idx].quality = sample.quality;

	// Update buffer fill
	buffer_fill_ratio = (float)sample_cnt / (float)LID_SAMPLE_NUMBER;
}

////////////////////////////////////////////////////////////////////////
// COMMUNIQUER AVEC PC
////////////////////////////////////////////////////////////////////////
// Envoie tout le buffer LIDAR sur l'UART spécifié
// Chaque trame commence par '$', chaque échantillon sur une nouvelle ligne
void LIDAR_SendBuffer_Frame(UART_HandleTypeDef *huart) {
	char line[32];
	// Début de trame
	HAL_UART_Transmit(huart, (uint8_t*)"$\r\n", 2, 10);
	LIDAR_Sample chosen_s = {0};
	for (uint16_t i = 0; i < 360; i++) {
		for(uint16_t j = 0; j < 100; j++){
			LIDAR_Sample s = LID_list_of_samples[i*10 + j];		//debug
			if(s.angle_deg_x10 != 0 && s.distance_mm > LIDAR_MIN_DIST && s.distance_mm < LIDAR_MAX_DIST){
				chosen_s = s;
				break;
			}
		}

		uint16_t angle_x10 = chosen_s.angle_deg_x10;

		// Format "angle,distance\n"
		int len = snprintf(line, sizeof(line), "%u,%u\r\n", angle_x10, chosen_s.distance_mm);

		if (len > 0) {
			HAL_UART_Transmit(huart, (uint8_t*)line, len, 10);
		}
	}
}
////////////////////////////////////////////////////////////////////////
// CLEAR BUFFERS
////////////////////////////////////////////////////////////////////////

void LID_clear_sample_buffer(void) {
	sample_cnt = 0;
	buffer_fill_ratio = 0;
	memset(LID_list_of_samples, 0, sizeof(LID_list_of_samples));
}

void LID_clear_cluster_buffer(void) {
	cluster_count = 0;
	memset(clusters, 0, sizeof(clusters));
}
////////////////////////////////////////////////////////////////////////
// MEDIAN FILTER
////////////////////////////////////////////////////////////////////////

uint16_t median_filter(uint16_t *values, uint8_t n) {
	// Copie locale pour tri
	uint16_t temp[MEDIAN_KERNEL_SIZE];
	for (uint8_t i = 0; i < n; i++) temp[i] = values[i];

	// Tri simple (insertion sort)
	for (uint8_t i = 1; i < n; i++) {
		uint16_t key = temp[i];
		int j = i - 1;
		while (j >= 0 && temp[j] > key) {
			temp[j + 1] = temp[j];
			j--;
		}
		temp[j + 1] = key;
	}
	return temp[n / 2]; // valeur médiane
}

void LIDAR_ApplyMedianFilter(LIDAR_Sample* buffer, uint16_t sample_count) {
	if (sample_count < MEDIAN_KERNEL_SIZE) return;

	uint16_t window[MEDIAN_KERNEL_SIZE];
	for (uint16_t i = MEDIAN_KERNEL_SIZE / 2; i < sample_count - MEDIAN_KERNEL_SIZE / 2; i++) {
		// Remplir la fenêtre centrée sur i
		for (uint8_t k = 0; k < MEDIAN_KERNEL_SIZE; k++) {
			window[k] = buffer[i - MEDIAN_KERNEL_SIZE / 2 + k].distance_mm;
		}
		// Remplacer la distance par la valeur filtrée
		buffer[i].distance_mm = median_filter(window, MEDIAN_KERNEL_SIZE);
	}
}


////////////////////////////////////////////////////////////////////////
// FIND CLUSTERS
////////////////////////////////////////////////////////////////////////

void LIDAR_FindClusters(void) {
	cluster_count = 0;

	uint16_t i = 0;
	LID_clear_cluster_buffer();
	while (i < LID_SAMPLE_NUMBER) {
		// ignorer points invalides
		if (LID_list_of_samples[i].distance_mm == 0 ||
				LID_list_of_samples[i].distance_mm > (uint16_t)(LIDAR_MAX_DIST)||
				LID_list_of_samples[i].distance_mm < (uint16_t)(LIDAR_MIN_DIST)) {
			i++;
			continue;
		}

		// début de cluster
		uint16_t start_idx = i;
		uint16_t end_idx = i;
		LIDAR_Sample prev_sample = LID_list_of_samples[i];

		// recherche de la fin du cluster
		while (i < LID_SAMPLE_NUMBER) {
			LIDAR_Sample sample = LID_list_of_samples[i];
			if (find_Cluster_norm(sample, prev_sample) > (MAX_DISTANCE_GAP) && sample.distance_mm !=0) {
				break; // rupture -> fin de cluster
			}
			prev_sample = sample;
			end_idx = i;
			i++;
		}

		// nombre de points dans ce cluster
		uint16_t nPoints = end_idx - start_idx + 1;

		if (nPoints >= MIN_POINTS_CLUSTER && cluster_count < MAX_CLUSTERS) {
			// calcul centre du cluster
			float sumX = 0.0f;
			float sumY = 0.0f;
			float minD = 9999.0f;
			float maxD = 0.0f;
			float closestAngle_deg = 0.0f;

			for (uint16_t k = start_idx; k <= end_idx; k++) {
				if (LID_list_of_samples[k].distance_mm == 0){
					continue;
				}
				float dist_m = LID_list_of_samples[k].distance_mm / 1000.0f;
				float angle_deg = LID_list_of_samples[k].angle_deg_x10 / 10.0f;
				float angle_rad = angle_deg * (float)M_PI / 180.0f;

				float x = dist_m * cosf(angle_rad);
				float y = dist_m * sinf(angle_rad);

				sumX += x;
				sumY += y;

				if (dist_m < minD) {
					minD = dist_m;
					if (angle_deg!=0) closestAngle_deg = angle_deg;	//on prend l'angle du point le plus proche
				}
				if (dist_m > maxD) maxD = dist_m;
			}

			clusters[cluster_count].x = sumX / nPoints;
			clusters[cluster_count].y = sumY / nPoints;
			clusters[cluster_count].angle_deg = closestAngle_deg;
			clusters[cluster_count].nPoints = nPoints;
			clusters[cluster_count].minDist = minD;
			clusters[cluster_count].maxDist = maxD;

			cluster_count++;
		}

		i++;
	}
}


float find_Cluster_norm(LIDAR_Sample sample, LIDAR_Sample prev_sample){
	//	//Initial radial norm
	//	return fabsf((float)sample.distance_mm - (float)prev_sample.distance_mm);

	//Euclidian norm
	float angle1 = ((float)sample.angle_deg_x10) / 10.0f * (M_PI / 180.0f);
	float angle2 = ((float)prev_sample.angle_deg_x10) / 10.0f * (M_PI / 180.0f);

	float x1 = sample.distance_mm * cosf(angle1);
	float y1 = sample.distance_mm * sinf(angle1);

	float x2 = prev_sample.distance_mm * cosf(angle2);
	float y2 = prev_sample.distance_mm * sinf(angle2);

	return sqrtf((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

////////////////////////////////////////////////////////////////////////
// STATISTICS
////////////////////////////////////////////////////////////////////////

#define ANGULAR_SECTION_NUMBER 36
float LID_Angular_sections_fill_rate[ANGULAR_SECTION_NUMBER];

void LID_Compute_Angular_sections_fill_rate(void){
	uint16_t section_len = (uint16_t)LID_SAMPLE_NUMBER/ANGULAR_SECTION_NUMBER;
	for(int i=0; i<ANGULAR_SECTION_NUMBER; i++){
		LID_Angular_sections_fill_rate[i] = LID_FillRate_PerAngularSection(LID_list_of_samples, i*section_len, section_len);
	}
}

float LID_FillRate_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len){
	uint16_t cnt = 0;
	for(uint16_t i =start_angle_deg; i<start_angle_deg+section_len; i++){
		LIDAR_Sample s = buffer[i];
		if(s.quality == 0x1){
			cnt++;
		}
	}
	return((float)cnt/(float)LID_SAMPLE_NUMBER);
}


uint16_t LID_MinDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len) {
	uint16_t min_val = UINT16_MAX;

	for (uint16_t i = start_angle_deg; i < start_angle_deg + section_len; i++) {
		LIDAR_Sample s = buffer[i];
		if (s.quality == 0x1 && s.distance_mm < min_val) {
			min_val = s.distance_mm;
		}
	}

	return (min_val == UINT16_MAX) ? 0 : min_val;
}


uint16_t LID_MaxDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len) {
	uint16_t max_val = 0;

	for (uint16_t i = start_angle_deg; i < start_angle_deg + section_len; i++) {
		LIDAR_Sample s = buffer[i];
		if (s.quality == 0x1 && s.distance_mm > max_val) {
			max_val = s.distance_mm;
		}
	}

	return max_val;
}

float LID_MeanDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len) {
	uint32_t sum = 0;
	uint16_t cnt = 0;

	for (uint16_t i = start_angle_deg; i < start_angle_deg + section_len; i++) {
		LIDAR_Sample s = buffer[i];
		if (s.quality == 0x1) {
			sum += s.distance_mm;
			cnt++;
		}
	}

	return (cnt > 0) ? ((float)sum / (float)cnt) : 0.0f;
}


