/*
 * motor.c
 *
 *  Created on: Nov 19, 2025
 *      Author: nelven
 */

#include "motor.h"

SystFlag Flags = {0};
extern SemaphoreHandle_t xMotorSem;

MotorDriver motorL = {
    .htim = &htim2,
    .channel_fwd = TIM_CHANNEL_3,
    .channel_rev = TIM_CHANNEL_4
};

MotorDriver motorR = {
    .htim = &htim2,
    .channel_fwd = TIM_CHANNEL_1,
    .channel_rev = TIM_CHANNEL_2
};


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
    else {                                   // Stop
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_fwd, 0);
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_rev, 0);
    }
}

void Motor_Run(MotorDriver *m, int speed, uint32_t duration_ms)
{
    Motor_SetSpeed(m, speed);
    HAL_Delay(duration_ms);
    Motor_SetSpeed(m, 0);          // Stop
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == GPIO_PIN_1) {       // USER1
        Flags.Motor_state = 1;
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
        xSemaphoreGiveFromISR(xMotorSem, &xHigherPriorityTaskWoken);
    }

    else if (GPIO_Pin == GPIO_PIN_9) {       // USER2
        Flags.Motor_state = 2;
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        xSemaphoreGiveFromISR(xMotorSem, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}
