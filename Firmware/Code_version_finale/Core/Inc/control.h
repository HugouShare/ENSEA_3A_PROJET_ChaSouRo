/*
 * control.h
 *
 *  Created on: Jan 12, 2026
 *      Author: hugoc
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "enc.h"
#include "motor.h"
#include "pid.h"
#include "freeRTOS_tasks_priority.h"

/* ================= CONFIG ================= */

#define DIST_THRESHOLD_MM     20
#define ANGLE_THRESHOLD_DEG   3

/* ================= TYPES ================= */

typedef enum
{
    CTRL_IDLE = 0,
    CTRL_TURN,
    CTRL_MOVE
} ctrl_mode_t;

/* Commande envoyée depuis ISR */
typedef struct
{
    ctrl_mode_t mode;
    int32_t target;        /* angle (deg) ou distance (mm) */
} ctrl_request_t;

/* Commande active interne */
typedef struct
{
    ctrl_mode_t mode;
    int32_t target;

    int32_t start_theta;
    int32_t start_x;
    int32_t start_y;

} ctrl_cmd_t;

/* ================= API ================= */

void Control_Init(void);
void Control_Tasks_Create(void);

/* === Task context === */
void Control_TurnAngle(int32_t angle_deg);
void Control_MoveDistance(int32_t distance_mm);

/* === ISR context === */
void Control_TurnAngleFromISR(int32_t angle_deg);
void Control_MoveDistanceFromISR(int32_t distance_mm);
void Control_StopFromISR(void);

/* Arrêt immédiat */
void Control_Stop(void);

/* Tâche RTOS */
void task_Control(void *arg);

#endif /* INC_CONTROL_H_ */
