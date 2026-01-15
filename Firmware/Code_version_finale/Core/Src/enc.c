/*
 * enc.c
 *
 *  Created on: Nov 9, 2025
 *      Author: hugoc
 */

#include "enc.h"

/* Instances globales */
Encoder_t ENC_D;
Encoder_t ENC_G;

/* Pose globale du robot (mm, mm, degrés) */
robot_Pose_t robot_pose = {0, 0, 0};

/* Prototypes des tâches */
void task_ENC_D_Update(void *arg);
void task_ENC_G_Update(void *arg);
void task_Odom_Update(void *arg);

static TaskHandle_t odom_task_handle = NULL;

/* Fonction générique d'update d'un encodeur */
static void Encoder_Update_Generic(Encoder_t *E)
{
	int32_t current = (int32_t)__HAL_TIM_GET_COUNTER(E->htim_enc);
	int32_t delta   = current - E->last_position;

	/* Gestion wrap-around 16 bits */
	if (delta > 32767)  delta -= 65536;
	if (delta < -32768) delta += 65536;

	E->total_ticks += delta;
	E->last_position = current;

	/* Détection sens FWD/REV uniquement si un tour complet est passé */
	int32_t deg_raw = (E->total_ticks * 360) / (int32_t)TICKS_PER_REV;

	if (deg_raw >= 360) {
		E->FWD++;
		E->total_ticks -= (int32_t)TICKS_PER_REV;
	} else if (deg_raw <= -360) {
		E->REV++;
		E->total_ticks += (int32_t)TICKS_PER_REV;
	}

	/* Position en degrés modulo 360 (entier) */
	E->position_deg = (E->total_ticks * 360) / (int32_t)TICKS_PER_REV;

	/* Vitesse en deg/s */
	E->velocity_deg_s =
			(delta * 360 * (1000 / TIMER_PERIOD_MS)) /
			(int32_t)TICKS_PER_REV;

	/* Delta distance (mm) */
	E->delta_distance =
			(delta * (int32_t)E->wheel_circumference) /
			(int32_t)TICKS_PER_REV;
}

/* ---------------------------------------------------------------------
 * Tâche pour l'encodeur droit (ENC_D)
 * --------------------------------------------------------------------- */
void task_ENC_D_Update(void *arg)
{
	Encoder_t *E = (Encoder_t*)arg;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	for (;;)
	{
		/* Attente de la notification du timer d'échantillonnage */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		Encoder_Update_Generic(E);

		ENC_D.has_been_updated = true;
		if (ENC_D.has_been_updated && ENC_G.has_been_updated){
			ENC_D.has_been_updated = false;
			ENC_G.has_been_updated = false;
			vTaskNotifyGiveFromISR(odom_task_handle, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}

		//        /* Affichage UART */
		//        char buffer[128];
		//        int len = sprintf(buffer,
		//            "[ENC_D] Pos:%7.2f deg | Vel:%7.2f deg/s | FWD:%ld | REV:%ld\r\n",
		//            E->position_deg, E->velocity_deg_s, E->FWD, E->REV);
		//        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);
	}
}

/* ---------------------------------------------------------------------
 * Tâche pour l'encodeur gauche (ENC_G)
 * --------------------------------------------------------------------- */
void task_ENC_G_Update(void *arg)
{
	Encoder_t *E = (Encoder_t*)arg;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	for (;;)
	{
		/* Attente de la notification du timer d'échantillonnage */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		Encoder_Update_Generic(E);

		ENC_G.has_been_updated = true;
		if (ENC_D.has_been_updated && ENC_G.has_been_updated){
			ENC_D.has_been_updated = false;
			ENC_G.has_been_updated = false;
			vTaskNotifyGiveFromISR(odom_task_handle, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}

		//        /* Affichage UART */
		//        char buffer[128];
		//        int len = sprintf(buffer,
		//            "[ENC_G] Pos:%7.2f deg | Vel:%7.2f deg/s | FWD:%ld | REV:%ld\r\n",
		//            E->position_deg, E->velocity_deg_s, E->FWD, E->REV);
		//        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, len, HAL_MAX_DELAY);
	}
}

/* ---------------------------------------------------------------------
 * Tâche d'odométrie (calcul de la position globale)
 * --------------------------------------------------------------------- */
void task_Odom_Update(void *arg)
{
	(void)arg;

	for (;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		/* Distances roues en mm */
		int32_t d_left  = ENC_G.delta_distance;
		int32_t d_right = ENC_D.delta_distance;

		/* Distance centre */
		int32_t d_center = (d_left + d_right) / 2;

		/* WHEEL_BASE en mm */
		const int32_t WHEEL_BASE_MM = (int32_t)(WHEEL_BASE * 1000.0f);

		/* d_theta en degrés (pas de float) :
           theta_deg = ( (R-L)/wheelbase ) * 180/pi
           180/pi ≈ 57.2958 ≈ 5730 / 100
		 */
		int32_t d_theta =
				((d_right - d_left) * 5730) /
				(WHEEL_BASE_MM * 100);

		/* Mise à jour orientation (-180..+179) */
		int32_t new_theta = (int32_t)robot_pose.theta + d_theta;

		while (new_theta > 180)  new_theta -= 360;
		while (new_theta <= -180) new_theta += 360;

		robot_pose.theta = (int16_t)new_theta;
		robot_pose.x_dist += (int32_t)d_center;

//		/* LUT Q15 */
//		/* Pour cos_lut et sin_lut, l'index doit être dans [0..359].
//		   On convertit theta en [0..359] pour la LUT */
//		uint16_t lut_index = (uint16_t)(new_theta < 0 ? (360 + new_theta) : new_theta);
//
//		int16_t c = cos_lut[lut_index];
//		int16_t s = sin_lut[lut_index];
//
//		/* Mise à jour (mm) : x += d_center * cos / 32768 */
//		robot_pose.x += X_COEFF *( (int32_t)d_center * c ) >> 15;
//		robot_pose.y += Y_COEFF *( (int32_t)d_center * s ) >> 15;
	}
}


/* ---------------------------------------------------------------------
 * Callback des timers d'échantillonnage : notifie la tâche correcte
 * --------------------------------------------------------------------- */


void enc_HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(ENC_G.task_handle != NULL && ENC_D.task_handle != NULL){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if (htim == ENC_D.htim_sampling) {
			vTaskNotifyGiveFromISR(ENC_D.task_handle, &xHigherPriorityTaskWoken);
		}

		if (htim == ENC_G.htim_sampling) {
			vTaskNotifyGiveFromISR(ENC_G.task_handle, &xHigherPriorityTaskWoken);
		}

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

/* Initialisation : configure les structures, démarre timers/encodeur et crée les tasks */
void ENC_Init(void)
{
	/* ---------- ENC_D (droite) ---------- */
	ENC_D.htim_enc      = &ENC_D_htimx;
	ENC_D.htim_sampling = &ENC_D_sampling_htimx;

	ENC_D.last_position = __HAL_TIM_GET_COUNTER(ENC_D.htim_enc);
	ENC_D.total_ticks   = 0;
	ENC_D.position_deg  = 0;
	ENC_D.velocity_deg_s = 0;
	ENC_D.FWD = 0;
	ENC_D.REV = 0;
	ENC_D.has_been_updated = false;

	ENC_D.task_handle = NULL;
	ENC_D.delta_distance = 0;

	ENC_D.wheel_circumference =
			(int32_t)(WHEEL_DIAMETER * 1000.0f * 3.1415926535f *
					WHEEL_DIAMETER_ERROR);


	HAL_TIM_Encoder_Start(ENC_D.htim_enc, TIM_CHANNEL_ALL);
	HAL_TIM_Base_Start_IT(ENC_D.htim_sampling);

	/* ---------- ENC_G (gauche) ---------- */
	ENC_G.htim_enc      = &ENC_G_htimx;
	ENC_G.htim_sampling = &ENC_G_sampling_htimx;

	ENC_G.last_position = __HAL_TIM_GET_COUNTER(ENC_G.htim_enc);
	ENC_G.total_ticks   = 0;
	ENC_G.position_deg  = 0;
	ENC_G.velocity_deg_s = 0;
	ENC_G.FWD = 0;
	ENC_G.REV = 0;
	ENC_G.has_been_updated = false;

	ENC_G.task_handle = NULL;
	ENC_G.delta_distance = 0;

	ENC_G.wheel_circumference =
			(int32_t)(WHEEL_DIAMETER * 1000.0f * 3.1415926535f);



	HAL_TIM_Encoder_Start(ENC_G.htim_enc, TIM_CHANNEL_ALL);
	HAL_TIM_Base_Start_IT(ENC_G.htim_sampling);


}

void ENC_Tasks_Create(void){
	if (xTaskCreate(task_ENC_D_Update, "ENC_D", ENC_STACK_SIZE, &ENC_D, task_ENC_D_Update_PRIORITY, &ENC_D.task_handle) != pdPASS){
		Error_Handler();
	}
	if (xTaskCreate(task_ENC_G_Update, "ENC_G", ENC_STACK_SIZE, &ENC_G, task_ENC_G_Update_PRIORITY, &ENC_G.task_handle) != pdPASS){
		Error_Handler();
	}
	/* ---------- Tâche odométrie ---------- */
	if (xTaskCreate(task_Odom_Update, "ODOM", ODOM_STACK_SIZE, NULL, task_Odom_Update_PRIORITY, &odom_task_handle) != pdPASS){
		Error_Handler();
	}
}
