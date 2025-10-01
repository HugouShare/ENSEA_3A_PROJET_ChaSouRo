#include "LIDAR.h"

////////////////////////////////////////////////////////////////////////GLOBAL VARIABLES

////////////////////////////////////////////////////////////////////////PRIVATE VARIABLES
uint8_t LIDAR_dma_buf[LIDAR_DMA_BUF_SIZE];
volatile uint32_t LIDAR_dma_read_idx = 0;

LIDAR_Cluster LIDAR_clusters[LIDAR_MAX_CLUSTERS] = {0};
uint8_t LIDAR_cluster_count = 0;
LIDAR_Sample LIDAR_view[LIDAR_N_ANGLES] = {0};

static uint32_t sample_cnt = 0;
static float buffer_fill_ratio = 0;

LIDAR_Frame current_frame = {0};


////////////////////////////////////////////////////////////////////////
// INIT
////////////////////////////////////////////////////////////////////////
void LIDAR_Init(void) {
	HAL_UART_Receive_DMA(&LID_huartx, LIDAR_dma_buf, LIDAR_DMA_BUF_SIZE);
	LID_TIMX_SetDuty(LID_SPEED);
}

////////////////////////////////////////////////////////////////////////
// WHILE(1)
////////////////////////////////////////////////////////////////////////
void LIDAR_While(void) {
	LIDAR_ProcessDMA();
	if (buffer_fill_ratio>SATISFYING_BUFFER_FILL_RATIO){ //traiter le buffer et le vider
		LIDAR_ApplyMedianFilter(LIDAR_view, LIDAR_N_ANGLES);
		LIDAR_FindClusters();
		LIDAR_clear_view_buffer();
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
// CHECKSUM
////////////////////////////////////////////////////////////////////////
bool verify_checksum(const uint8_t *buf, uint16_t len) {
	if (len % 2 != 0) return false;
	uint16_t acc = 0;
	for (uint16_t i = 0; i < len; i += 2) {
		uint16_t w = (uint16_t)buf[i] | ((uint16_t)buf[i + 1] << 8);
		acc ^= w;
	}
	return (acc == 0);
}

////////////////////////////////////////////////////////////////////////
// PROCESS DMA
////////////////////////////////////////////////////////////////////////

static uint32_t get_dma_write_index(void) {
	return (LIDAR_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_uart4_rx)) % LIDAR_DMA_BUF_SIZE;
}

void LIDAR_ProcessDMA(void) {
	uint32_t write_idx = get_dma_write_index();

	while (LIDAR_dma_read_idx != write_idx) {
		uint32_t available = (write_idx + LIDAR_DMA_BUF_SIZE - LIDAR_dma_read_idx) % LIDAR_DMA_BUF_SIZE;
		if (available < 10) break;

		uint8_t b0 = LIDAR_dma_buf[LIDAR_dma_read_idx];
		uint8_t b1 = LIDAR_dma_buf[(LIDAR_dma_read_idx + 1) % LIDAR_DMA_BUF_SIZE];

		if (b0 != 0xAA || b1 != 0x55) {
			LIDAR_dma_read_idx = (LIDAR_dma_read_idx + 1) % LIDAR_DMA_BUF_SIZE;
			continue;
		}

		if (available < 4) break;

		uint8_t lsn = LIDAR_dma_buf[(LIDAR_dma_read_idx + 3) % LIDAR_DMA_BUF_SIZE];
		if (lsn == 0 || lsn > LIDAR_MAX_SAMPLES_PKT) {
			LIDAR_dma_read_idx = (LIDAR_dma_read_idx + 2) % LIDAR_DMA_BUF_SIZE;
			continue;
		}

		uint16_t expected_len = (uint16_t)(10 + lsn * 2);
		if (expected_len > sizeof(current_frame.data)) {
			LIDAR_dma_read_idx = (LIDAR_dma_read_idx + 2) % LIDAR_DMA_BUF_SIZE;
			continue;
		}

		if (available < expected_len) break;
		current_frame.length = expected_len;

		for (uint16_t i = 0; i < current_frame.length; i++) {
			current_frame.data[i] = LIDAR_dma_buf[(LIDAR_dma_read_idx + i) % LIDAR_DMA_BUF_SIZE];
		}

		if (!verify_checksum(current_frame.data, current_frame.length)) {
			LIDAR_dma_read_idx = (LIDAR_dma_read_idx + 1) % LIDAR_DMA_BUF_SIZE;
			continue;
		}

		LIDAR_ManageFrame(current_frame, lsn);
		LIDAR_dma_read_idx = (LIDAR_dma_read_idx + current_frame.length) % LIDAR_DMA_BUF_SIZE;
	}
}

////////////////////////////////////////////////////////////////////////
// MANAGE CURRENT FRAME
////////////////////////////////////////////////////////////////////////

void LIDAR_ManageFrame(LIDAR_Frame frame, uint16_t sample_count) {
	if (frame.length < 10) return;

	uint16_t FSA = (uint16_t)frame.data[4] | ((uint16_t)frame.data[5] << 8);
	uint16_t LSA = (uint16_t)frame.data[6] | ((uint16_t)frame.data[7] << 8);

	float angle_fsa = (float)(FSA >> 1) / 64.0f;
	float angle_lsa = (float)(LSA >> 1) / 64.0f;
	float diff_angle = angle_lsa - angle_fsa;
	if (diff_angle < 0.0f) diff_angle += 360.0f;

	for (uint16_t s = 0; s < sample_count; s++) {
		uint16_t offset = 10 + 2 * s;
		if ((offset + 1) >= frame.length) break;

		uint16_t S_i = (uint16_t)frame.data[offset] | ((uint16_t)frame.data[offset + 1] << 8);

		float dist = (float)(S_i >> 2);
		float angle = (sample_count == 1) ? angle_fsa : angle_fsa + diff_angle * s / (sample_count - 1);

		if (dist > 0.0f) {
			float ratio = 21.8f * (155.3f - dist) / (155.3f * dist);
			angle += atanf(ratio) * RAD_TO_DEG;
		}
		else{
			continue;
		}

		angle = fmodf(angle, 360.0f);
		if (angle < 0.0f) angle += 360.0f;
		if (angle < 0.0f) angle += 360.0f;

		int idx = (int)(angle * (float)LIDAR_N_ANGLES / 360.0f + 0.5f);
		if (idx < 0) idx = 0;
		if (idx >= LIDAR_N_ANGLES) idx = LIDAR_N_ANGLES - 1;

		LIDAR_Sample sample = {
				.X = 0.0f, // sera calculé plus tard si nécessaire
				.Y = 0.0f,
				.angle_deg = angle,
				.distance_mm = (uint16_t)dist,
				.quality = !(S_i & 0x0001)
		};
		sample.angle_deg = (uint16_t)idx;

		LIDAR_StoreSample(sample);
	}
}

////////////////////////////////////////////////////////////////////////
// STORE SAMPLES
////////////////////////////////////////////////////////////////////////

void LIDAR_StoreSample(LIDAR_Sample sample) {
	uint16_t idx = (uint16_t)sample.angle_deg;
	if (idx >= LIDAR_N_ANGLES) return;

	if (LIDAR_view[idx].distance_mm == 0) sample_cnt++;
	LIDAR_view[idx] = sample;

	buffer_fill_ratio = (float)sample_cnt / (float)LIDAR_N_ANGLES;
}


////////////////////////////////////////////////////////////////////////
// CLEAR BUFFERS
////////////////////////////////////////////////////////////////////////

void LIDAR_clear_cluster_buffer(void) {
	LIDAR_cluster_count = 0;
	memset(LIDAR_clusters, 0, sizeof(LIDAR_clusters));
}

void LIDAR_clear_view_buffer(void) {
	sample_cnt = 0;
	buffer_fill_ratio = 0;
	memset(LIDAR_view, 0, sizeof(LIDAR_view));
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
	LIDAR_clear_cluster_buffer();
	uint16_t i = 0;

	while (i < LIDAR_N_ANGLES) {
		// ignorer points invalides
		if (LIDAR_view[i].distance_mm == 0 ||
				LIDAR_view[i].distance_mm > LIDAR_MAX_RANGE ||
				LIDAR_view[i].distance_mm < LIDAR_MIN_VALID_DIST) {
			i++;
			continue;
		}

		// début de cluster
		uint16_t start_idx = i;
		uint16_t end_idx   = i;
		LIDAR_Sample prev_sample = LIDAR_view[i];

		// recherche de la fin du cluster
		while (++i < LIDAR_N_ANGLES) {
			LIDAR_Sample sample = LIDAR_view[i];
			if (sample.distance_mm == 0) break; // rupture si invalide
			if (LIDAR_findClusterNorm(sample, prev_sample) > LIDAR_MIN_CLUSTER_DIST) {
				break; // rupture -> fin de cluster
			}
			prev_sample = sample;
			end_idx = i;
		}

		// nombre de points dans ce cluster
		uint16_t nPoints = end_idx - start_idx + 1;

		if (nPoints >= MIN_POINTS_CLUSTER && LIDAR_cluster_count < LIDAR_MAX_CLUSTERS) {
			// calcul centre du cluster
			float sumX = 0.0f;
			float sumY = 0.0f;
			uint16_t validPoints = 0;

			for (uint16_t k = start_idx; k <= end_idx; k++) {
				if (LIDAR_view[k].distance_mm == 0) continue;

				float dist_m = LIDAR_view[k].distance_mm / 1000.0f;
				float angle_rad = (LIDAR_view[k].angle_deg * (float)M_PI) / 180.0f;

				float x = dist_m * cosf(angle_rad);
				float y = dist_m * sinf(angle_rad);

				sumX += x;
				sumY += y;
				validPoints++;
			}

			if (validPoints > 0) {
				LIDAR_clusters[LIDAR_cluster_count].x = sumX / validPoints;
				LIDAR_clusters[LIDAR_cluster_count].y = sumY / validPoints;
				LIDAR_clusters[LIDAR_cluster_count].start_idx = start_idx;
				LIDAR_clusters[LIDAR_cluster_count].end_idx   = end_idx;
				LIDAR_clusters[LIDAR_cluster_count].size      = validPoints;
				LIDAR_clusters[LIDAR_cluster_count].active    = true;

				LIDAR_cluster_count++;
			}
		}
	}
}

float LIDAR_findClusterNorm(LIDAR_Sample sample, LIDAR_Sample prev_sample) {
	// Norme euclidienne entre deux points polaires
	float angle1 = (sample.angle_deg * (float)M_PI) / 180.0f;
	float angle2 = (prev_sample.angle_deg * (float)M_PI) / 180.0f;

	float x1 = sample.distance_mm * cosf(angle1);
	float y1 = sample.distance_mm * sinf(angle1);

	float x2 = prev_sample.distance_mm * cosf(angle2);
	float y2 = prev_sample.distance_mm * sinf(angle2);

	return sqrtf((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}


////////////////////////////////////////////////////////////////////////
// COMMUNIQUER AVEC PC
////////////////////////////////////////////////////////////////////////
// Envoie tout le buffer LIDAR sur l'UART spécifié
// Chaque trame commence par '$', chaque échantillon sur une nouvelle ligne
//void LIDAR_SendBuffer_Frame(UART_HandleTypeDef *huart) {
//	char line[32];
//	// Début de trame
//	HAL_UART_Transmit(huart, (uint8_t*)"$\r\n", 2, 10);
//	LIDAR_Sample chosen_s = {0};
//	for (uint16_t i = 0; i < 360; i++) {
//		for(uint16_t j = 0; j < 100; j++){
//			LIDAR_Sample s = LID_list_of_samples[i*10 + j];		//debug
//			if(s.angle_x_deg != 0 && s.distance_mm > LIDAR_MIN_DIST && s.distance_mm < LIDAR_MAX_DIST){
//				chosen_s = s;
//				break;
//			}
//		}
//
//		uint16_t angle_x10 = chosen_s.angle_x_deg;
//
//		// Format "angle,distance\n"
//		int len = snprintf(line, sizeof(line), "%u,%u\r\n", angle_x10, chosen_s.distance_mm);
//
//		if (len > 0) {
//			HAL_UART_Transmit(huart, (uint8_t*)line, len, 10);
//		}
//	}
//}



////////////////////////////////////////////////////////////////////////
// STATISTICS
////////////////////////////////////////////////////////////////////////

//#define ANGULAR_SECTION_NUMBER 36
//float LID_Angular_sections_fill_rate[ANGULAR_SECTION_NUMBER];
//
//void LID_Compute_Angular_sections_fill_rate(void){
//	uint16_t section_len = (uint16_t)LID_SAMPLE_NUMBER/ANGULAR_SECTION_NUMBER;
//	for(int i=0; i<ANGULAR_SECTION_NUMBER; i++){
//		LID_Angular_sections_fill_rate[i] = LID_FillRate_PerAngularSection(LID_list_of_samples, i*section_len, section_len);
//	}
//}
//
//float LID_FillRate_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len){
//	uint16_t cnt = 0;
//	for(uint16_t i =start_angle_deg; i<start_angle_deg+section_len; i++){
//		LIDAR_Sample s = buffer[i];
//		if(s.quality == 0x1){
//			cnt++;
//		}
//	}
//	return (section_len > 0) ? ((float)cnt / (float)section_len) : 0.0f;
//}
//
//
//uint16_t LID_MinDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len) {
//	uint16_t min_val = UINT16_MAX;
//
//	for (uint16_t i = start_angle_deg; i < start_angle_deg + section_len; i++) {
//		LIDAR_Sample s = buffer[i];
//		if (s.quality == 0x1 && s.distance_mm < min_val) {
//			min_val = s.distance_mm;
//		}
//	}
//
//	return (min_val == UINT16_MAX) ? 0 : min_val;
//}
//
//
//uint16_t LID_MaxDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len) {
//	uint16_t max_val = 0;
//
//	for (uint16_t i = start_angle_deg; i < start_angle_deg + section_len; i++) {
//		LIDAR_Sample s = buffer[i];
//		if (s.quality == 0x1 && s.distance_mm > max_val) {
//			max_val = s.distance_mm;
//		}
//	}
//
//	return max_val;
//}
//
//float LID_MeanDistance_PerAngularSection(LIDAR_Sample* buffer, uint16_t start_angle_deg, uint16_t section_len) {
//	uint32_t sum = 0;
//	uint16_t cnt = 0;
//
//	for (uint16_t i = start_angle_deg; i < start_angle_deg + section_len; i++) {
//		LIDAR_Sample s = buffer[i];
//		if (s.quality == 0x1) {
//			sum += s.distance_mm;
//			cnt++;
//		}
//	}
//
//	return (cnt > 0) ? ((float)sum / (float)cnt) : 0.0f;
//}
