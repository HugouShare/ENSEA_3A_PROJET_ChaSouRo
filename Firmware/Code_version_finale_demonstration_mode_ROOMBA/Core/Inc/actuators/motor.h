/*
 * motor.h
 *
 *  Created on: Nov 19, 2025
 *      Author: nelven
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "freeRTOS_tasks_priority.h"

/* ================= CONFIG ================= */

#define MOTOR_PWM_MAX     250    // ARR = 1000 -> PWM max
#define MOTOR_STACK_SIZE  64      // mots (≈ 256 octets)

/* ================= DRIVER ================= */

/* Driver d'un moteur individuel */
typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel_fwd;
    uint32_t channel_rev;
} MotorDriver;

/* Moteurs globaux */
extern MotorDriver motorL;
extern MotorDriver motorR;

/* ================= API ================= */

/* Applique directement une PWM à un moteur */
void Motor_SetSpeed(MotorDriver *m, int speed);

/* Définit la consigne globale (appelée par Control) */
void Motors_SetPWM(int left, int right);

/* Initialisation hardware PWM */
void Init_motors(void);

/* Création de la tâche moteur */
void Motors_Tasks_Create(void);

/* Tâche RTOS moteur */
void task_motor(void *arg);


#endif /* INC_MOTOR_H_ */
