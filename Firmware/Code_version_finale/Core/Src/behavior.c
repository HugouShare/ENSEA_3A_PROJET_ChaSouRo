/*
 * behavior.c
 *
 *  Created on: Jan 12, 2026
 *      Author: hugoc
 */

#include "behavior.h"
#include "motor.h"
#include "control.h"

TaskHandle_t htask_ROOMBA = NULL;
TaskHandle_t htask_CHAT = NULL;
TaskHandle_t htask_SOURIS = NULL;
TaskHandle_t htask_EDGE = NULL;


void behavior_Tasks_Create(void) {

	if(xTaskCreate(task_ROOMBA, "TASK ROOMBA",ROOMBA_STACK_SIZE ,NULL, task_ROOMBA_PRIORITY, &htask_ROOMBA) != pdPASS){
		Error_Handler();
	}
	if(xTaskCreate(task_CHAT, "TASK CHAT",CHAT_STACK_SIZE ,NULL, task_CHAT_PRIORITY, &htask_CHAT) != pdPASS){
		Error_Handler();
	}
	if(xTaskCreate(task_SOURIS, "TASK SOURIS",SOURIS_STACK_SIZE ,NULL, task_SOURIS_PRIORITY, &htask_SOURIS) != pdPASS){
		Error_Handler();
	}
	if(xTaskCreate(task_EDGE, "TASK EDGE",EDGE_STACK_SIZE ,NULL, task_EDGE_PRIORITY, &htask_EDGE) != pdPASS){
		Error_Handler();
	}
}



void task_ROOMBA(void *unused) {
	(void)unused;
	static bool first_iteration = true;
	for (;;) {
//		if (first_iteration){
//			Control_TurnAngle(-45);
//			Control_MoveDistance(1000);
//			first_iteration = false;
//		}




		vTaskDelay(pdMS_TO_TICKS(5));
	}
}

void task_CHAT(void *unused) {
	(void)unused;
	for (;;) {

		vTaskDelay(pdMS_TO_TICKS(5));
	}
}

void task_SOURIS(void *unused) {
	(void)unused;
	for (;;) {

		vTaskDelay(pdMS_TO_TICKS(5));
	}
}





