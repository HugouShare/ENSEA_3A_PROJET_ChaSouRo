/*
 * enc.c
 *
 *  Created on: Nov 9, 2025
 *      Author: hugoc
 */


#include "enc.h"


volatile int32_t last_position = 0;
volatile int32_t total_ticks = 0;
volatile float position_deg = 0;
volatile float velocity_deg_s = 0;
volatile int32_t FWD = 0;
volatile int32_t REV = 0;


// LED software timer
volatile int32_t led_timer = 0;


//FREERTOS
static TaskHandle_t htask_ENC_Update = NULL;

static void ENC_Tasks_Create(void);

////////////////////////////////////////////////////////////////////////
// INIT
////////////////////////////////////////////////////////////////////////
void ENC_Init(void){
	ENC_Tasks_Create();
	HAL_TIM_Base_Start_IT(&ENC_htimx);
	HAL_TIM_Base_Start_IT(&ENC_sampling_htimx);
}


////////////////////////////////////////////////////////////////////////
// CALLBACK
////////////////////////////////////////////////////////////////////////


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &ENC_sampling_htimx)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(htask_ENC_Update, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

////////////////////////////////////////////////////////////////////////
// FREERTOS
////////////////////////////////////////////////////////////////////////
//Définition des tasks

///////////////////////////NE PAS OUBLIER D'AJUSTER LA TAILLE DE LA PILE !!!

void task_ENC_Update(void *unused) {
	(void)unused;

	for (;;) {
		// Attente de la notification du timer
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		int32_t current = (int32_t)__HAL_TIM_GET_COUNTER(&ENC_htimx);
		int32_t delta = current - last_position;

		// Gestion du wrap-around 16 bits
		if(delta > 32767) delta -= 65536;
		if(delta < -32768) delta += 65536;

		total_ticks += delta;
		last_position = current;

		// LED
		if(delta != 0) led_timer = LED_ON_TIME_MS / TIMER_PERIOD_MS;

		// Conversion en degrés
		float raw_deg = ((float)total_ticks / TICKS_PER_REV) * 360.0f;

		if (raw_deg >= 360.0f) {
			FWD++;
			total_ticks -= TICKS_PER_REV;
		} else if (raw_deg <= -360.0f) {
			REV++;
			total_ticks += TICKS_PER_REV;
		}

		position_deg = ((float)total_ticks / TICKS_PER_REV) * 360.0f;
		velocity_deg_s = ((float)delta / TICKS_PER_REV) * (360.0f * (1000.0f / TIMER_PERIOD_MS));

		if(led_timer > 0) {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
			led_timer--;
		} else {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		}

		char buffer[120];
		int len = sprintf(buffer,
				"Pos: %7.2f deg | Vel: %7.2f deg/s | FWD: %ld | REV: %ld\r\n",
				position_deg, velocity_deg_s, FWD, REV);
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);
	}
}


//Création des tasks
static void ENC_Tasks_Create(void) {
	if(xTaskCreate(task_ENC_Update, "update de l'encodeur",1024 ,NULL, 5, &htask_ENC_Update) != pdPASS){
		printf("Error task_ENC_Update \r\n");
		Error_Handler();
	}
}

////////////////////////////////////////////////////////////////////////
// UPDATE
////////////////////////////////////////////////////////////////////////

void ENC_Update(void){
	int32_t current = (int32_t)__HAL_TIM_GET_COUNTER(&ENC_htimx);
	int32_t delta = current - last_position;

	// Gestion du wrap-around 16 bits
	if(delta > 32767) delta -= 65536;
	if(delta < -32768) delta += 65536;

	total_ticks += delta;
	last_position = current;

	// Détection du mouvement pour LED
	if(delta != 0)
	{
		led_timer = LED_ON_TIME_MS / TIMER_PERIOD_MS; // convert ms → ticks timer
	}

	// Conversion brute en degrés
	float raw_deg = ((float)total_ticks / TICKS_PER_REV) * 360.0f;

	// Détection du passage de +360° ou -360°
	if (raw_deg >= 360.0f)
	{
		FWD++;
		total_ticks -= TICKS_PER_REV; // remet la position dans la fenêtre [-360, +360]
	}
	else if (raw_deg <= -360.0f)
	{
		REV++;
		total_ticks += TICKS_PER_REV; // remet la position dans la fenêtre [-360, +360]
	}

	// Position instantanée modulo 360
	position_deg = ((float)total_ticks / TICKS_PER_REV) * 360.0f;

	// Vitesse (°/s)
	velocity_deg_s = ((float)delta / TICKS_PER_REV) * (360.0f * (1000.0f / TIMER_PERIOD_MS));

	// Gestion LED
	if(led_timer > 0)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		led_timer--;
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}

	// Affichage
	char buffer[120];
	int len = sprintf(buffer,
		"Pos: %7.2f deg | Vel: %7.2f deg/s | FWD: %ld | REV: %ld\r\n",
		position_deg, velocity_deg_s, FWD, REV);
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);

}

