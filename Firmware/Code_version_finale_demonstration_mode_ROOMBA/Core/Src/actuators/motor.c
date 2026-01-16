/*
 * motor.c
 */

#include "actuators/motor.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"

/* ================= GLOBAL ================= */

static volatile int pwm_left_cmd  = 0;
static volatile int pwm_right_cmd = 0;

/* ================= DRIVERS ================= */

MotorDriver motorR = {
    .htim = &htim2,
    .channel_fwd = TIM_CHANNEL_2,
    .channel_rev = TIM_CHANNEL_1
};

MotorDriver motorL = {
    .htim = &htim2,
    .channel_fwd = TIM_CHANNEL_3,
    .channel_rev = TIM_CHANNEL_4
};

/* ================= API ================= */

void Motors_SetPWM(int left, int right)
{
    if (left > MOTOR_PWM_MAX)  left = MOTOR_PWM_MAX;
    if (left < -MOTOR_PWM_MAX) left = -MOTOR_PWM_MAX;

    if (right > MOTOR_PWM_MAX)  right = MOTOR_PWM_MAX;
    if (right < -MOTOR_PWM_MAX) right = -MOTOR_PWM_MAX;

    pwm_left_cmd  = left;
    pwm_right_cmd = right;
}

/* ================= TASK ================= */

void task_motor(void *arg)
{
    (void)arg;
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        Motor_SetSpeed(&motorL, pwm_left_cmd);
        Motor_SetSpeed(&motorR, pwm_right_cmd);

        /* 100 Hz */
        vTaskDelayUntil(&last, pdMS_TO_TICKS(10));
    }
}

/* ================= INIT ================= */

void Motors_Tasks_Create(void)
{
    if (xTaskCreate(task_motor,
                    "MOTOR",
                    MOTOR_STACK_SIZE,
                    NULL,
                    task_motor_PRIORITY,
                    NULL) != pdPASS)
    {
        Error_Handler();
    }
}

void Init_motors(void)
{
    HAL_TIM_PWM_Start(motorL.htim, motorL.channel_fwd);
    HAL_TIM_PWM_Start(motorL.htim, motorL.channel_rev);
    HAL_TIM_PWM_Start(motorR.htim, motorR.channel_fwd);
    HAL_TIM_PWM_Start(motorR.htim, motorR.channel_rev);




//    Motor_SetSpeed(&motorL, 300);
//    Motor_SetSpeed(&motorR, -300);
//
//    HAL_Delay(1000);
//
//    Motor_SetSpeed(&motorL, -300);
//    Motor_SetSpeed(&motorR, +300);
//
//    HAL_Delay(1000);
//
//    Motor_SetSpeed(&motorL, 0);
//    Motor_SetSpeed(&motorR, 0);

    Motors_SetPWM(0, 0);



}

/* ================= LOW LEVEL ================= */

void Motor_SetSpeed(MotorDriver *m, int speed)
{
    if (speed > 0) {
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_rev, 0);
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_fwd, speed);
    }
    else if (speed < 0) {
        speed = -speed;
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_fwd, 0);
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_rev, speed);
    }
    else {
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_fwd, 0);
        __HAL_TIM_SET_COMPARE(m->htim, m->channel_rev, 0);
    }
}
