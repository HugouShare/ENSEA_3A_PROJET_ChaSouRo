/*
 * behavior.c – version avec sémaphore
 */

#include "actuators/behavior.h"
#include "actuators/control.h"
#include "actuators/motor.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdlib.h>

/* ===== TASK HANDLES ===== */
TaskHandle_t htask_ROOMBA = NULL;
TaskHandle_t htask_EDGE   = NULL;
TaskHandle_t htask_CHAT   = NULL;
TaskHandle_t htask_SOURIS   = NULL;

static void behavior_ROOMBA(void);
static void behavior_EDGE(void);
static void behavior_CHAT(void);
static void behavior_SOURIS(void);


LIDAR_Cluster target = {0};

/* ===== SEMAPHORE ===== */
SemaphoreHandle_t xEdgeSemaphore;

/* ===== PARAMS ===== */
static int32_t angles[] = { -135, -90, -45, 45, 90, 135 };
#define DISTANCE_THRESHOLD_MM 200

extern uint16_t dist1, dist2, dist3, dist4;

/* ===== INIT ===== */

void behavior_Tasks_Create(void)
{
	/* Création du sémaphore binaire */
	xEdgeSemaphore = xSemaphoreCreateBinary();
	if (xEdgeSemaphore == NULL)
		Error_Handler();

//	/* ROOMBA */
//	if (xTaskCreate(task_ROOMBA, "TASK_ROOMBA", ROOMBA_STACK_SIZE, NULL,
//			task_ROOMBA_PRIORITY, &htask_ROOMBA) != pdPASS)
//	{
//		Error_Handler();
//	}
//
	/* EDGE */
	if (xTaskCreate(task_EDGE, "TASK_EDGE", EDGE_STACK_SIZE, NULL,
			task_EDGE_PRIORITY, &htask_EDGE) != pdPASS)
	{
		Error_Handler();
	}

	/* CHAT */
	if (xTaskCreate(task_CHAT, "TASK_CHAT", CHAT_STACK_SIZE, NULL,
			task_CHAT_PRIORITY, &htask_CHAT) != pdPASS)
	{
		Error_Handler();
	}
	//
	//    /* SOURIS */
	//    if (xTaskCreate(task_SOURIS, "TASK_SOURIS", SOURIS_STACK_SIZE, NULL,
	//                    task_SOURIS_PRIORITY, &htask_SOURIS) != pdPASS)
	//    {
	//        Error_Handler();
	//    }
}

/* ===== ROOMBA (MODE LIBRE) ===== */

void task_ROOMBA(void *unused)
{
	(void)unused;
	srand(xTaskGetTickCount());

	for (;;)
	{
		/* ROOMBA exécute le mode libre */
		behavior_ROOMBA();

		/* Ensuite attend éventuellement l’ordre EDGE */
		if (xSemaphoreTake(xEdgeSemaphore, 0) == pdTRUE)
		{
			/* EDGE veut passer → ROOMBA cesse immédiatement */
			Control_Stop();
			Control_WaitUntilNotBusy();
		}

		vTaskDelay(pdMS_TO_TICKS(20));
	}
}

static void behavior_ROOMBA(void)
{
	/* Déplacement libre (aléatoire) */
	int idx = rand() % (sizeof(angles) / sizeof(angles[0]));
	int32_t angle = angles[idx];

	Control_TurnAngle(angle);
	Control_WaitUntilNotBusy();

	Control_MoveDistance(300);
	Control_WaitUntilNotBusy();
}

/* ===== EDGE (PRIORITÉ MAX) ===== */

void task_EDGE(void *unused)
{
	(void)unused;

	for (;;)
	{
		/* Vérifie les distances */
		if (dist1 > DISTANCE_THRESHOLD_MM ||
				dist2 > DISTANCE_THRESHOLD_MM ||
				dist3 > DISTANCE_THRESHOLD_MM ||
				dist4 > DISTANCE_THRESHOLD_MM)
		{
			/* Signale à ROOMBA d’arrêter immédiatement */
			xSemaphoreGive(xEdgeSemaphore);

			/* Appel du comportement avec les valeurs des capteurs */
			behavior_EDGE();

			/* EDGE terminé → ROOMBA reprendra son mode libre */
		}

		vTaskDelay(pdMS_TO_TICKS(5));
	}
}

static void behavior_EDGE(void)
{
	int angle = 180;
	int32_t distance = -1000;

	// TOF 1 et TOF2 => arrière
	// TOF 3 et TOF4 => arrière

	/* Choisit la rotation minimale en fonction du capteur qui détecte */
	if (	dist2 > DISTANCE_THRESHOLD_MM ||
			dist1 > DISTANCE_THRESHOLD_MM)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
		Control_Stop();
		Control_WaitUntilNotBusy();
		Control_MoveDistance(-distance);
		Control_WaitUntilNotBusy();
		Control_MoveDistance(-distance);
		Control_WaitUntilNotBusy();
		Control_MoveDistance(-distance);
		Control_WaitUntilNotBusy();
		Control_TurnAngle(angle);
		Control_WaitUntilNotBusy();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	}

	/* Choisit la rotation minimale en fonction du capteur qui détecte */
	if (dist4 > DISTANCE_THRESHOLD_MM ||
			dist3 > DISTANCE_THRESHOLD_MM)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
		Control_Stop();
		Control_WaitUntilNotBusy();
		Control_MoveDistance(distance);
		Control_WaitUntilNotBusy();
		Control_MoveDistance(distance);
		Control_WaitUntilNotBusy();
		Control_MoveDistance(distance);
		Control_WaitUntilNotBusy();
		Control_TurnAngle(angle);
		Control_WaitUntilNotBusy();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	}
}


/* ===== CHAT ===== */

void task_CHAT(void *unused)
{
	(void)unused;

	for (;;)
	{
		behavior_CHAT();
		vTaskDelay(pdMS_TO_TICKS(5));
	}
}


static void behavior_CHAT(void)
{
	// 1. Sélectionner le cluster le plus proche
	target = LIDAR_clusters[0];
	for (uint8_t k = 0; k < LIDAR_cluster_count; k++)
	{
		LIDAR_Cluster cluster = LIDAR_clusters[k];
		if (cluster.distance_mm < target.distance_mm)
		{
			target = cluster;
		}
	}

	if (target.distance_mm > LIDAR_MIN_CLUSTER_DIST){
		// 2. S'orienter vers la target
		//    On suppose que target.angle_deg est un angle relatif :
		//    - Positif = cible à droite
		//    - Négatif = cible à gauche
		Control_TurnAngle(target.angle_deg);
		Control_WaitUntilNotBusy();

		// 3. Foncer sur la target
		//    On avance exactement de la distance détectée
		Control_MoveDistance(target.distance_mm);
		Control_WaitUntilNotBusy();
	}


}

/* ===== CHAT ===== */

void task_SOURIS(void *unused)
{
	(void)unused;

	for (;;)
	{
		behavior_SOURIS();
		vTaskDelay(pdMS_TO_TICKS(5));
	}
}


static void behavior_SOURIS(void)
{
	// 1. Sélectionner le cluster le plus proche
	target = LIDAR_clusters[0];
	for (uint8_t k = 0; k < LIDAR_cluster_count; k++)
	{
		LIDAR_Cluster cluster = LIDAR_clusters[k];
		if (cluster.distance_mm < target.distance_mm)
		{
			target = cluster;
		}
	}

	// 2. S'orienter vers la target
	//    On suppose que target.angle_deg est un angle relatif :

	int16_t flee_angle = target.angle_deg + 180;
	flee_angle = ( (flee_angle + 180) % 360 ) - 180; //met dans l'intervalle -180, +180



	Control_TurnAngle(flee_angle);
	Control_WaitUntilNotBusy();


	// 3. Foncer sur la target
	//    On avance exactement de la distance détectée
	Control_MoveDistance(target.distance_mm);
	Control_WaitUntilNotBusy();
}


