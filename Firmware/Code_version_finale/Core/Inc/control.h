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

#define KP_TURN 4.1
#define KI_TURN 0.95
#define KD_TURN 0.6

#define KP_TURN_SA 2.0
#define KI_TURN_SA 0.5
#define KD_TURN_SA 0.5

#define KP_MOVE 1.5
#define KI_MOVE 0.0
#define KD_MOVE 0.5

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

    int16_t start_theta;   /* angle initial [-180 ; +180] */
    int32_t start_dist;    /* distance initiale en mm (x_dist) */

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
bool Control_IsBusy(void);

/* Tâche RTOS */
void task_Control(void *arg);

#endif /* INC_CONTROL_H_ */
