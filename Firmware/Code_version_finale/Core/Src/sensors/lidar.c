#include "sensors/lidar.h"


////////////////////////////////////////////////////////////////////////PRIVATE VARIABLES
uint8_t LIDAR_dma_buf[LIDAR_DMA_BUF_SIZE];
volatile uint32_t LIDAR_dma_read_idx = 0;

LIDAR_Cluster LIDAR_clusters[LIDAR_MAX_CLUSTERS] = {0};
volatile uint8_t LIDAR_cluster_count = 0;
LIDAR_Sample LIDAR_view[LIDAR_N_ANGLES] = {0};

volatile uint32_t sample_cnt = 0;
volatile uint8_t buffer_fill_ratiox100 = 0;

LIDAR_Frame current_frame = {0};

TaskHandle_t htask_LIDAR_Update = NULL;
//static TaskHandle_t htask_test = NULL;

//Luts pour ne pas avoir à calculer cos et sin en float

const int16_t cos_lut[360] = {
32768, 32763, 32748, 32723, 32688, 32643, 32588, 32523, 32449, 32364, 32270, 32165, 32051, 31928, 31794, 31651, 31498, 31336, 31164, 30982, 30791, 30591, 30381, 30163, 29935, 29697, 29451, 29196, 28932, 28659, 28377, 28087, 27788, 27481, 27165, 26841, 26509, 26169, 25821, 25465, 25101, 24730, 24351, 23964, 23571, 23170, 22762, 22347, 21926, 21497, 21062, 20621, 20173, 19720, 19260, 18794, 18323, 17846, 17364, 16876, 16384, 15886, 15383, 14876, 14364, 13848, 13327, 12803, 12275, 11743, 11207, 10668, 10125, 9580, 9032, 8480, 7927, 7371, 6812, 6252, 5690, 5126, 4560, 3993, 3425, 2855, 2285, 1714, 1143, 571, 0, -571, -1143, -1714, -2285, -2855, -3425, -3993, -4560, -5126, -5690, -6252, -6812, -7371, -7927, -8480, -9032, -9580, -10125, -10668, -11207, -11743, -12275, -12803, -13327, -13848, -14364, -14876, -15383, -15886, -16383, -16876, -17364, -17846, -18323, -18794, -19260, -19720, -20173, -20621, -21062, -21497, -21926, -22347, -22762, -23170, -23571, -23964, -24351,
-24730, -25101, -25465, -25821, -26169, -26509, -26841, -27165, -27481, -27788, -28087, -28377, -28659, -28932, -29196, -29451, -29697, -29935, -30163, -30381, -30591, -30791, -30982, -31164, -31336, -31498, -31651, -31794, -31928, -32051, -32165, -32270, -32364, -32449, -32523, -32588, -32643, -32688, -32723, -32748, -32763, -32768, -32763, -32748, -32723, -32688, -32643, -32588, -32523, -32449, -32364, -32270, -32165, -32051, -31928, -31794, -31651, -31498, -31336, -31164, -30982, -30791, -30591, -30381, -30163, -29935, -29697, -29451, -29196, -28932, -28659, -28377, -28087, -27788, -27481, -27165, -26841, -26509, -26169, -25821, -25465, -25101, -24730, -24351, -23964, -23571, -23170, -22762, -22347, -21926, -21497, -21062, -20621, -20173, -19720, -19260, -18794, -18323, -17846, -17364, -16876, -16384, -15886, -15383, -14876, -14364, -13848, -13327, -12803,
-12275, -11743, -11207, -10668, -10125, -9580, -9032, -8480, -7927, -7371, -6812, -6252, -5690, -5126, -4560, -3993, -3425, -2855, -2285, -1714, -1143, -571, 0, 571, 1143, 1714, 2285, 2855, 3425, 3993, 4560, 5126, 5690, 6252, 6812, 7371, 7927, 8480, 9032, 9580, 10125, 10668, 11207, 11743, 12275, 12803, 13327, 13848, 14364, 14876, 15383, 15886, 16384, 16876, 17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621, 21062, 21497, 21926, 22347, 22762, 23170, 23571, 23964, 24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196, 29451, 29697, 29935, 30163, 30381, 30591, 30791, 30982, 31164, 31336,
31498, 31651, 31794, 31928, 32051, 32165, 32270, 32364, 32449, 32523, 32588, 32643, 32688, 32723, 32748, 32763
};


const int16_t sin_lut[360] = {
0, 571, 1143, 1714, 2285, 2855, 3425, 3993, 4560, 5126, 5690, 6252, 6812, 7371, 7927, 8480, 9032, 9580, 10125, 10668, 11207, 11743, 12275, 12803, 13327, 13848, 14364, 14876, 15383, 15886, 16383, 16876, 17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621, 21062, 21497, 21926, 22347, 22762, 23170, 23571, 23964, 24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196, 29451, 29697, 29935, 30163, 30381, 30591, 30791, 30982, 31164, 31336, 31498, 31651, 31794, 31928, 32051, 32165, 32270,
32364, 32449, 32523, 32588, 32643, 32688, 32723, 32748, 32763, 32768, 32763, 32748, 32723, 32688, 32643, 32588, 32523, 32449, 32364, 32270, 32165, 32051, 31928, 31794, 31651, 31498, 31336, 31164, 30982, 30791, 30591, 30381, 30163, 29935, 29697, 29451, 29196, 28932, 28659, 28377, 28087, 27788, 27481, 27165, 26841, 26509, 26169, 25821, 25465, 25101, 24730, 24351, 23964, 23571, 23170, 22762, 22347, 21926, 21497, 21062, 20621, 20173, 19720, 19260, 18794, 18323, 17846, 17364, 16876, 16383, 15886, 15383, 14876, 14364, 13848, 13327, 12803, 12275, 11743, 11207, 10668, 10125, 9580, 9032, 8480, 7927, 7371, 6812, 6252, 5690, 5126, 4560, 3993, 3425, 2855, 2285, 1714, 1143, 571, 0, -571, -1143, -1714, -2285, -2855, -3425, -3993, -4560, -5126, -5690, -6252, -6812, -7371, -7927, -8480, -9032, -9580, -10125, -10668, -11207, -11743, -12275, -12803, -13327, -13848, -14364, -14876, -15383, -15886, -16384, -16876, -17364, -17846, -18323, -18794, -19260, -19720, -20173, -20621, -21062, -21497, -21926, -22347, -22762, -23170, -23571, -23964, -24351, -24730, -25101, -25465, -25821, -26169, -26509, -26841, -27165, -27481, -27788, -28087, -28377, -28659, -28932, -29196, -29451, -29697, -29935, -30163, -30381, -30591, -30791, -30982, -31164, -31336, -31498, -31651, -31794, -31928, -32051, -32165, -32270, -32364,
-32449, -32523, -32588, -32643, -32688, -32723, -32748, -32763, -32768, -32763, -32748, -32723, -32688, -32643, -32588, -32523, -32449, -32364, -32270, -32165, -32051, -31928, -31794, -31651, -31498, -31336, -31164, -30982, -30791, -30591, -30381, -30163, -29935, -29697, -29451, -29196, -28932, -28659, -28377, -28087, -27788, -27481, -27165, -26841, -26509, -26169, -25821, -25465, -25101, -24730, -24351, -23964, -23571, -23170, -22762, -22347, -21926, -21497, -21062, -20621, -20173, -19720, -19260, -18794, -18323, -17846, -17364, -16876, -16384, -15886, -15383, -14876, -14364, -13848, -13327, -12803, -12275, -11743, -11207, -10668, -10125, -9580, -9032, -8480, -7927, -7371, -6812, -6252, -5690, -5126, -4560, -3993, -3425, -2855, -2285, -1714, -1143, -571
};
////////////////////////////////////////////////////////////////////////STATIC FUNCTIONS

static void LIDAR_ProcessDMA(void);
static void LIDAR_StoreSample(LIDAR_Sample sample);
static void LIDAR_ManageFrame(LIDAR_Frame frame, uint16_t sample_count);

static bool verify_checksum(const uint8_t *buf, uint16_t len);

static void LIDAR_FindClusters(void);
static uint16_t LIDAR_findClusterNorm(LIDAR_Sample s, LIDAR_Sample p);
static void LIDAR_clear_view_buffer(void);
static void LIDAR_ApplyMedianFilter(LIDAR_Sample* buffer, uint16_t sample_count);
static uint16_t median_filter(uint16_t *values, uint8_t n);
static void LID_TIMX_SetDuty(uint8_t duty_percent);

static inline uint8_t fast_round_ratio_100(uint32_t num, uint32_t den);

////////////////////////////////////////////////////////////////////////
// INIT
////////////////////////////////////////////////////////////////////////
void LIDAR_Init(void) {
	if(HAL_UART_Receive_DMA(&LID_huartx, LIDAR_dma_buf, LIDAR_DMA_BUF_SIZE) != HAL_OK){
		Error_Handler();
	}

	if(HAL_TIM_PWM_Start(&LID_htimx, LID_TIM_CHANNEL_X) != HAL_OK){
		Error_Handler();
	}
	LID_TIMX_SetDuty(LID_SPEED);
}

////////////////////////////////////////////////////////////////////////
// WHILE(1)
////////////////////////////////////////////////////////////////////////
void LIDAR_While(void) {
	LIDAR_ProcessDMA();
	if (buffer_fill_ratiox100>SATISFYING_BUFFER_FILL_RATIO){ //traiter le buffer et le vider
		LIDAR_ApplyMedianFilter(LIDAR_view, LIDAR_N_ANGLES);
		LIDAR_FindClusters();
		LIDAR_clear_view_buffer();
	}
}

////////////////////////////////////////////////////////////////////////
// FREERTOS
////////////////////////////////////////////////////////////////////////

///////////////////////////NE PAS OUBLIER D'AJUSTER LA TAILLE DE LA PILE !!!

void task_LIDAR_Update(void *unused)
{
    (void)unused;

    for (;;)
    {
        // dort jusqu'à ce que le DMA ou le timer le réveille
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // traiter les données fraîchement reçues
        LIDAR_ProcessDMA();

        if (buffer_fill_ratiox100 > SATISFYING_BUFFER_FILL_RATIO)
        {
        	taskENTER_CRITICAL();
            LIDAR_ApplyMedianFilter(LIDAR_view, LIDAR_N_ANGLES);
            LIDAR_FindClusters();
            LIDAR_clear_view_buffer();
            taskEXIT_CRITICAL();
        }
    }
}


//void task_test(void*unused){
//	(void)unused;
//	for(;;){
//		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//		vTaskDelay(pdMS_TO_TICKS(500));
//	}
//}

void LIDAR_Tasks_Create(void) {
	if(xTaskCreate(task_LIDAR_Update, "LIDAR UPDATE",LID_STACK_SIZE ,NULL, task_LIDAR_Update_PRIORITY, &htask_LIDAR_Update) != pdPASS){
		//		printf("Error task_LIDAR_Update \r\n");
		Error_Handler();
	}
	//	if(xTaskCreate(task_test, "test",512 ,NULL, 2, &htask_test) != pdPASS){
	//		printf("Error task_test \r\n");
	//		Error_Handler();
	//	}
}




////////////////////////////////////////////////////////////////////////
// SET DUTY
////////////////////////////////////////////////////////////////////////
static void LID_TIMX_SetDuty(uint8_t duty_percent) {
	if (duty_percent > 100) duty_percent = 100;
	uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&LID_htimx);
	uint32_t ccr = (duty_percent * (arr + 1)) / 100;
	__HAL_TIM_SET_COMPARE(&LID_htimx, LID_TIM_CHANNEL_X, ccr);
}


////////////////////////////////////////////////////////////////////////
// CHECKSUM
////////////////////////////////////////////////////////////////////////
static bool verify_checksum(const uint8_t *buf, uint16_t len) {
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
	return (LIDAR_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(&LID_hdma_uartx_rx)) % LIDAR_DMA_BUF_SIZE;
}

static void LIDAR_ProcessDMA(void) {
	static uint32_t last_write_idx = 0;
	static uint32_t stuck_cnt = 0;

	uint32_t write_idx = get_dma_write_index();

	/* ---- DMA WATCHDOG ---- */
	if (write_idx == last_write_idx) {
		if (++stuck_cnt > DMA_STUCK_THRESHOLD) {
			HAL_UART_AbortReceive(&LID_huartx);
			HAL_UART_Receive_DMA(&LID_huartx, LIDAR_dma_buf, LIDAR_DMA_BUF_SIZE);
			LIDAR_dma_read_idx = 0;
			stuck_cnt = 0;
		}
	} else {
		stuck_cnt = 0;
	}
	last_write_idx = write_idx;


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

static void LIDAR_ManageFrame(LIDAR_Frame frame, uint16_t sample_count)
{
	if (frame.length < 10) return;

	// Lecture angles de début et de fin (codés Q6.1)
	uint16_t FSA = (uint16_t)frame.data[4] | ((uint16_t)frame.data[5] << 8);
	uint16_t LSA = (uint16_t)frame.data[6] | ((uint16_t)frame.data[7] << 8);

	// Conversion -> angle x10 (entier, 1 = 0.1°)
	uint16_t angle_fsa_x10 = ((FSA >> 1) * 10) / 64;
	uint16_t angle_lsa_x10 = ((LSA >> 1) * 10) / 64;

	int16_t diff_angle_x10 = (int16_t)angle_lsa_x10 - (int16_t)angle_fsa_x10;
	if (diff_angle_x10 < 0) diff_angle_x10 += 3600;         // normalisation [0;3600]

	for (uint16_t s = 0; s < sample_count; s++)
	{
		uint16_t offset = 10 + 2 * s;
		if ((offset + 1) >= frame.length) break;

		uint16_t S_i = (uint16_t)frame.data[offset] | ((uint16_t)frame.data[offset + 1] << 8);

		// Distance en mm (S_i >> 2 est en dixièmes de mm, on convertit en mm)
		uint16_t distance_mm = S_i >> 2;

		if (distance_mm == 0) continue;

		// Angle interpolé en dixièmes de degrés
		uint16_t angle_x10;
		if (sample_count == 1) angle_x10 = angle_fsa_x10;
		else angle_x10 = angle_fsa_x10 + (diff_angle_x10 * s) / (sample_count - 1);

		// Normalisation [0..3600[
		if (angle_x10 >= 3600) angle_x10 %= 3600;

		// Conversion angle_x10 (0.1°) en angle entier en degrés [0..359]
		uint16_t angle_deg = (angle_x10 + 5) / 10; // arrondi à l'entier le plus proche

		if (angle_deg >= 360) angle_deg %= 360;

		// Index dans tableau [0..LIDAR_N_ANGLES-1]
		uint16_t idx = (uint16_t)((angle_deg * LIDAR_N_ANGLES + 180) / 360);
		if (idx >= LIDAR_N_ANGLES) idx = LIDAR_N_ANGLES - 1;

		// Stockage
		LIDAR_Sample sample = {
				.angle_deg = angle_deg,          // angle entier en degrés
				.distance_mm = distance_mm,
				.quality = !(S_i & 0x0001)
		};

		LIDAR_StoreSample(sample);
	}
}



////////////////////////////////////////////////////////////////////////
// STORE SAMPLES
////////////////////////////////////////////////////////////////////////

static void LIDAR_StoreSample(LIDAR_Sample sample) {
	uint16_t idx = sample.angle_deg;
	if (idx >= LIDAR_N_ANGLES) return;

	if (LIDAR_view[idx].distance_mm == 0) sample_cnt++;
	LIDAR_view[idx] = sample;

	buffer_fill_ratiox100 = fast_round_ratio_100(sample_cnt, LIDAR_N_ANGLES);
}


////////////////////////////////////////////////////////////////////////
// CLEAR BUFFERS
////////////////////////////////////////////////////////////////////////

static void LIDAR_clear_cluster_buffer(void) {
	LIDAR_cluster_count = 0;
	memset(LIDAR_clusters, 0, sizeof(LIDAR_clusters));
}

static void LIDAR_clear_view_buffer(void) {
	sample_cnt = 0;
	buffer_fill_ratiox100 = 0;
	memset(LIDAR_view, 0, sizeof(LIDAR_view));
}

////////////////////////////////////////////////////////////////////////
// MEDIAN FILTER
////////////////////////////////////////////////////////////////////////
static uint16_t median_filter(uint16_t *values, uint8_t n) {
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

static void LIDAR_ApplyMedianFilter(LIDAR_Sample* buffer, uint16_t sample_count) {
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
// FIXED POINT HELPER
////////////////////////////////////////////////////////////////////////
static inline uint8_t fast_round_ratio_100(uint32_t num, uint32_t den) {
	return (uint8_t)((100 * num + (den >> 1)) / den);
}


////////////////////////////////////////////////////////////////////////
// FIXED POINT CLUSTER DISTANCE
////////////////////////////////////////////////////////////////////////
static uint16_t LIDAR_findClusterNorm(LIDAR_Sample s, LIDAR_Sample p) {
    uint16_t r1 = s.distance_mm;
    uint16_t r2 = p.distance_mm;
    uint16_t a1 = s.angle_deg;  // angle en degrés entiers [0..359]
    uint16_t a2 = p.angle_deg;

    // Sécurité : s'assurer que l'angle est dans [0..359]
    a1 %= 360;
    a2 %= 360;

    int32_t x1 = ((int32_t)r1 * cos_lut[a1]) >> Q15_SHIFT;
    int32_t y1 = ((int32_t)r1 * sin_lut[a1]) >> Q15_SHIFT;

    int32_t x2 = ((int32_t)r2 * cos_lut[a2]) >> Q15_SHIFT;
    int32_t y2 = ((int32_t)r2 * sin_lut[a2]) >> Q15_SHIFT;

    int32_t dx = x1 - x2;
    int32_t dy = y1 - y2;

    // Calculer dx² + dy² (64 bits pour éviter overflow)
    uint64_t dist_squared = (uint64_t)dx * dx + (uint64_t)dy * dy;

    // Racine carrée entière rapide (approximation)
    uint32_t d2 = (uint32_t)dist_squared;
    uint32_t r = 0;
    uint32_t bit = 1UL << 30;
    while (bit > d2) bit >>= 2;
    while (bit != 0) {
        if (d2 >= r + bit) {
            d2 -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (uint16_t)r;
}


////////////////////////////////////////////////////////////////////////
// FIND CLUSTERS (fixed‑point centers)
////////////////////////////////////////////////////////////////////////
static void LIDAR_FindClusters(void) {
    LIDAR_clear_cluster_buffer();
    uint16_t i = 0;

    while (i < LIDAR_N_ANGLES) {
        if (LIDAR_view[i].distance_mm == 0 ||
            LIDAR_view[i].distance_mm > LIDAR_MAX_RANGE ||
            LIDAR_view[i].distance_mm < LIDAR_MIN_VALID_DIST) {
            i++;
            continue;
        }

        uint16_t start_idx = i;
        uint16_t end_idx = i;
        LIDAR_Sample prev = LIDAR_view[i];

        while (++i < LIDAR_N_ANGLES) {
            LIDAR_Sample s = LIDAR_view[i];
            if (s.distance_mm == 0) break;
            if (LIDAR_findClusterNorm(s, prev) > LIDAR_MIN_CLUSTER_DIST) break;
            prev = s;
            end_idx = i;
        }

        uint16_t nPoints = end_idx - start_idx + 1;

        if (nPoints >= MIN_POINTS_CLUSTER && LIDAR_cluster_count < LIDAR_MAX_CLUSTERS) {
            int32_t sumX = 0;  // somme en int32_t pour éviter overflow
            int32_t sumY = 0;
            uint16_t valid = 0;
            int16_t angle180;

            for (uint16_t k = start_idx; k <= end_idx; k++) {
                LIDAR_Sample s = LIDAR_view[k];
                if (s.distance_mm == 0) continue;

                uint16_t a = s.angle_deg;  // Angle entier [0..359]
                uint16_t r = s.distance_mm;

                angle180 = (s.angle_deg > 180) ? s.angle_deg - 360 : s.angle_deg;


                // Calcul en Q15 fixed-point : (r * cos(angle)) >> 15
                int16_t x = (int16_t)(((int32_t)r * cos_lut[a]) >> Q15_SHIFT);
                int16_t y = (int16_t)(((int32_t)r * sin_lut[a]) >> Q15_SHIFT);

                sumX += x;
                sumY += y;
                valid++;
            }

            if (valid > 0) {
                // moyenne en int16_t, en s'assurant que la moyenne rentre dans 16 bits
                LIDAR_clusters[LIDAR_cluster_count].x = (int16_t)(sumX / valid);
                LIDAR_clusters[LIDAR_cluster_count].y = (int16_t)(sumY / valid);
                LIDAR_clusters[LIDAR_cluster_count].angle_deg = angle180;
                LIDAR_clusters[LIDAR_cluster_count].start_idx = start_idx;
                LIDAR_clusters[LIDAR_cluster_count].end_idx = end_idx;
                LIDAR_clusters[LIDAR_cluster_count].size = valid;
                LIDAR_clusters[LIDAR_cluster_count].active = true;

                LIDAR_cluster_count++;
            }
        }
    }
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
