/*
 * motor.c
 *
 *  Created on: Nov 19, 2025
 *      Author: nelven
 */

#include "motor.h"
#include "gpio.h"

int count = 0;
SystFlag Flags = {0};
extern SemaphoreHandle_t xMotorSem;
MotorCommand motor_cmd = {0, 0, 0};


MotorDriver motorL = {
    .htim = &htim2,
    .channel_fwd = TIM_CHANNEL_2,
    .channel_rev = TIM_CHANNEL_1
};

MotorDriver motorR = {
    .htim = &htim2,
    .channel_fwd = TIM_CHANNEL_3,
    .channel_rev = TIM_CHANNEL_4
};

void Init_motors(void){

    // --- 1. Démarrer tous les PWM ---
    HAL_TIM_PWM_Start(motorL.htim, motorL.channel_fwd);
    HAL_TIM_PWM_Start(motorL.htim, motorL.channel_rev);
    HAL_TIM_PWM_Start(motorR.htim, motorR.channel_fwd);
    HAL_TIM_PWM_Start(motorR.htim, motorR.channel_rev);

    // --- 2. S'assurer que tout est à l'arrêt ---
    Motor_SetSpeed(&motorL, 0);
    Motor_SetSpeed(&motorR, 0);
    HAL_Delay(500);

    // --- 3. Faire tourner le robot sur lui-même à droite ---
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
    Motor_SetSpeed(&motorL, 300);   // gauche avant
    Motor_SetSpeed(&motorR, -300);  // droite arrière
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);

    // --- 4. Stop ---
    Motor_SetSpeed(&motorL, 0);
    Motor_SetSpeed(&motorR, 0);
    HAL_Delay(500);

    // --- 5. Faire tourner le robot sur lui-même à gauche ---
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    Motor_SetSpeed(&motorL, -300);  // gauche arrière
    Motor_SetSpeed(&motorR, 300);   // droite avant
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

    // --- 6. Stop final ---
    Motor_SetSpeed(&motorL, 0);
    Motor_SetSpeed(&motorR, 0);
    HAL_Delay(500);

}

void Motor_SetSpeed(MotorDriver *m, int speed)
{
    if (speed > 0) {                         // Marche avant
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_rev, 0);
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_fwd, speed);
    }
    else if (speed < 0) {                    // Marche arrière
        speed = -speed;
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_fwd, 0);
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_rev, speed);
    }
    else{
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_fwd, 0);
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_rev, 0);
    }
}


void Motors_Set(int left, int right, uint32_t duration_ms)
{
	// Met à jour la structure motor_cmd qui ensuite est utilisée dans la tâche
	// pour modifier les PWM via Motor_SetSpeed

    // --- 1. Saturation des vitesses ---
    if (left > MOTOR_PWM_MAX)  left = MOTOR_PWM_MAX;
    if (left < -MOTOR_PWM_MAX) left = -MOTOR_PWM_MAX;

    if (right > MOTOR_PWM_MAX)  right = MOTOR_PWM_MAX;
    if (right < -MOTOR_PWM_MAX) right = -MOTOR_PWM_MAX;

    // --- 2. Durée minimale ---
    if (duration_ms == 0)
        duration_ms = 1;  // éviter commande instantanée impossible

    // --- 3. Charger la commande ---
    motor_cmd.speedL = left;
    motor_cmd.speedR = right;

    // --- 4. Calcul de la date d'arrêt ---
    motor_cmd.end_time = xTaskGetTickCount() + pdMS_TO_TICKS(duration_ms);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == GPIO_PIN_1) {
        Motors_Set(300, 300, 5000);
    }

    else if (GPIO_Pin == GPIO_PIN_9) {
    	if (count == 2){
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
        Motors_Set(500, -500, 500);
        count = 0;
    	}
    	if (count == 1){
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		Motors_Set(500, -500, 250);
		count++;
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
		}
    	if (count == 0){
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		Motors_Set(500, -500, 100);
		count++;
		}
    }

    xSemaphoreGiveFromISR(xMotorSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}
