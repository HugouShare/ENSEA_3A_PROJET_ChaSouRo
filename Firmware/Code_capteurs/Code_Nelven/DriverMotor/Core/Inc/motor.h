/*
 * motor.h
 *
 *  Created on: Nov 19, 2025
 *      Author: nelven
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_
#include "tim.h"
#include "cmsis_os.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel_fwd;
    uint32_t channel_rev;
} MotorDriver;

extern MotorDriver motorL;
extern MotorDriver motorR;

typedef struct {
    int Motor_state;
} SystFlag;

extern SystFlag Flags;

void Motor_SetSpeed(MotorDriver *m, int speed);
void Motor_Run(MotorDriver *m, int speed, uint32_t duration_ms);

#endif /* INC_MOTOR_H_ */
