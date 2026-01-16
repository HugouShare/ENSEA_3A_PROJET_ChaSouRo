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

/* ===== SEMAPHORE ===== */
SemaphoreHandle_t xEdgeSemaphore;

/* ===== PARAMS ===== */
static int32_t angles[] = { -135, -90, -45, 45, 90, 135 };
#define DISTANCE_THRESHOLD_MM 1000

extern uint16_t dist1, dist2, dist3, dist4;

/* ===== INIT ===== */

void behavior_Tasks_Create(void)
{
    /* Création du sémaphore binaire */
    xEdgeSemaphore = xSemaphoreCreateBinary();
    if (xEdgeSemaphore == NULL)
        Error_Handler();

    /* ROOMBA */
    if (xTaskCreate(task_ROOMBA, "TASK_ROOMBA", ROOMBA_STACK_SIZE, NULL,
                    task_ROOMBA_PRIORITY, &htask_ROOMBA) != pdPASS)
    {
        Error_Handler();
    }

    /* EDGE */
    if (xTaskCreate(task_EDGE, "TASK_EDGE", EDGE_STACK_SIZE, NULL,
                    task_EDGE_PRIORITY, &htask_EDGE) != pdPASS)
    {
        Error_Handler();
    }
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

void behavior_ROOMBA(void)
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

            behavior_EDGE();
            /* EDGE terminé → ROOMBA reprendra son mode libre */
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void behavior_EDGE (void)
{
	/* Exécute le comportement d’évitement */
	  Control_Stop();
	  Control_WaitUntilNotBusy();

	  /* Recul de 100 mm */
	  Control_MoveDistance(-100);
	  Control_WaitUntilNotBusy();

	  /* Tourne de 90° */
	  Control_TurnAngle(90);
	  Control_WaitUntilNotBusy();
}
