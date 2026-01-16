/*
 * behavior.h
 *
 *  Created on: Jan 12, 2026
 *      Author: hugoc
 */

#ifndef INC_BEHAVIOR_H_
#define INC_BEHAVIOR_H_

#include "freeRTOS_tasks_priority.h"

#define ROOMBA_STACK_SIZE 128
#define CHAT_STACK_SIZE 128
#define SOURIS_STACK_SIZE 128
#define EDGE_STACK_SIZE 128


#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include <stdbool.h>
#include "sensors/lidar.h"

/* ================= EVENTS ================= */

#define EVT_CHAT_TRIGGERED    (1 << 0)
#define EVT_SOURIS_TRIGGERED  (1 << 1)
#define EVT_EDGE_TRIGGERED    (1 << 2)

/* ================= TASK HANDLES ================= */

extern TaskHandle_t htask_ROOMBA;
extern TaskHandle_t htask_CHAT;
extern TaskHandle_t htask_SOURIS;
extern TaskHandle_t htask_EDGE;

/* ================= API ================= */

void behavior_Tasks_Create(void);

/* ================= TASKS ================= */

void task_ROOMBA(void *unused);
void task_CHAT(void *unused);
void task_SOURIS(void *unused);
void task_EDGE(void *unused);


#endif /* INC_BEHAVIOR_H_ */
