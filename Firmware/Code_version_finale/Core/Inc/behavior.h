/*
 * behavior.h
 *
 *  Created on: Jan 12, 2026
 *      Author: hugoc
 */

#ifndef INC_BEHAVIOR_H_
#define INC_BEHAVIOR_H_

#include "freeRTOS_tasks_priority.h"

#define ROOMBA_STACK_SIZE 512
#define CHAT_STACK_SIZE 512
#define SOURIS_STACK_SIZE 512


void task_ROOMBA(void *unused);
void task_CHAT(void *unused);
void task_SOURIS(void *unused);


#endif /* INC_BEHAVIOR_H_ */
