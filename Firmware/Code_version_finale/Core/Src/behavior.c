/*
 * behavior.c
 *
 *  Created on: Jan 12, 2026
 *      Author: hugoc
 */

#include "behavior.h"
#include "control.h"
#include "motor.h"
#include <stdlib.h>
#include <event_groups.h>

/* ================= TASK HANDLES ================= */

TaskHandle_t htask_ROOMBA = NULL;
TaskHandle_t htask_CHAT   = NULL;
TaskHandle_t htask_SOURIS = NULL;
TaskHandle_t htask_EDGE   = NULL;

/* ================= EVENTS ================= */

EventGroupHandle_t behaviorEvents;

/* ================= ROOMBA PARAMS ================= */

static int32_t angles[] = { -135, -90, -45, 45, 90, 135 };

/* ================= INIT ================= */

void behavior_Tasks_Create(void)
{
    behaviorEvents = xEventGroupCreate();
    configASSERT(behaviorEvents);

    if (xTaskCreate(task_ROOMBA, "TASK_ROOMBA",
                    ROOMBA_STACK_SIZE, NULL,
                    task_ROOMBA_PRIORITY, &htask_ROOMBA) != pdPASS)
        Error_Handler();

    if (xTaskCreate(task_CHAT, "TASK_CHAT",
                    CHAT_STACK_SIZE, NULL,
                    task_CHAT_PRIORITY, &htask_CHAT) != pdPASS)
        Error_Handler();

    if (xTaskCreate(task_SOURIS, "TASK_SOURIS",
                    SOURIS_STACK_SIZE, NULL,
                    task_SOURIS_PRIORITY, &htask_SOURIS) != pdPASS)
        Error_Handler();

    if (xTaskCreate(task_EDGE, "TASK_EDGE",
                    EDGE_STACK_SIZE, NULL,
                    task_EDGE_PRIORITY, &htask_EDGE) != pdPASS)
        Error_Handler();
}

/* ================= ROOMBA (IDLE BEHAVIOR) ================= */
void task_ROOMBA(void *unused)
{
    (void)unused;

    static bool first_iteration = true;

    srand(xTaskGetTickCount());

    for (;;)
    {
        // Attendre que tous les bits soient à 0 (aucun comportement prioritaire actif)
        while (xEventGroupGetBits(behaviorEvents) & (EVT_CHAT_TRIGGERED | EVT_SOURIS_TRIGGERED | EVT_EDGE_TRIGGERED))
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        if (first_iteration)
        {

            Control_MoveDistance(950);
            Control_WaitUntilNotBusy();
            vTaskDelay(pdMS_TO_TICKS(1000));
            Control_TurnAngle(95);
            Control_WaitUntilNotBusy();

            first_iteration = false;
        }
        else
        {
//            int idx = rand() % (sizeof(angles) / sizeof(angles[0]));
//            int32_t angle = angles[idx];
//
//            Control_TurnAngle(angle);
//            Control_WaitUntilNotBusy();
//
//            Control_MoveDistance(1000);
//            Control_WaitUntilNotBusy();
            vTaskDelay(500);
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}


/* ================= CHAT ================= */

void task_CHAT(void *unused)
{
    (void)unused;

    for (;;)
    {
        /* Bloquer la tâche jusqu'à ce que l'événement CHAT soit déclenché */
        xEventGroupWaitBits(
            behaviorEvents,
            EVT_CHAT_TRIGGERED,
            pdTRUE,      // Clear automatiquement le bit à la sortie
            pdFALSE,     // On ne bloque pas sur tous les bits, juste un
            portMAX_DELAY
        );

        /* === ICI : chat détecté === */
        Control_Stop();
        Control_TurnAngle(0);
        Control_WaitUntilNotBusy();
    }
}

/* ================= SOURIS ================= */

void task_SOURIS(void *unused)
{
    (void)unused;

    for (;;)
    {
        /* Bloquer la tâche jusqu'à détection souris */
        xEventGroupWaitBits(
            behaviorEvents,
            EVT_SOURIS_TRIGGERED,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        /* === ICI : souris détectée === */
        Control_Stop();
        Control_TurnAngle(90);
        Control_WaitUntilNotBusy();

        Control_MoveDistance(500);
        Control_WaitUntilNotBusy();
    }
}

/* ================= EDGE (PRIORITÉ MAX) ================= */

void task_EDGE(void *unused)
{
    (void)unused;

    for (;;)
    {
        /* Bloquer la tâche jusqu'à détection bord */
        xEventGroupWaitBits(
            behaviorEvents,
            EVT_EDGE_TRIGGERED,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        /* === ICI : bord détecté === */
        Control_Stop();
        Control_TurnAngle(180);
        Control_WaitUntilNotBusy();

        Control_MoveDistance(300);
        Control_WaitUntilNotBusy();
    }
}
