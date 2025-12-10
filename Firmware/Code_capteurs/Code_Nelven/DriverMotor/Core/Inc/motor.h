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
#include <stdio.h>

#define MOTOR_PWM_MAX   1000    // ARR = 1000 -> PWM max
#define MOTOR_PWM_MIN   0       // pas utile, mais propre

/* === Driver d'un moteur individuel === */
typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel_fwd;
    uint32_t channel_rev;
} MotorDriver;

extern MotorDriver motorL;
extern MotorDriver motorR;

/* === Indicateurs système  === */
typedef struct {
    int Motor_state;
} SystFlag;

extern SystFlag Flags;

/* === Structure de commande globale === */
typedef struct {
    int speedL;               // vitesse moteur gauche
    int speedR;               // vitesse moteur droit
    TickType_t end_time;      // moment d'arrêt (tick)
} MotorCommand;

extern MotorCommand motor_cmd;

/* === API publique === */
void Motor_SetSpeed(MotorDriver *m, int speed); //Sert à appliquer les PWM
void Motors_Set(int left, int right, uint32_t duration_ms);// Sert à modifier la structure utilisé par Motor_SetSpeed
// Architecture “commander via structure + appliquer via task” -> non bloquante

void Init_motors(void);
void CreateTaskMotors(void);

#endif /* INC_MOTOR_H_ */
