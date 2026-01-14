/*
 * pid.c
 *
 *  Created on: Jan 12, 2026
 *      Author: hugoc
 */


#include "pid.h"

void PID_Init(PID_t *pid,
              int32_t kp,
              int32_t ki,
              int32_t kd,
              int32_t out_min,
              int32_t out_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0;
    pid->prev_error = 0;
    pid->out_min = out_min;
    pid->out_max = out_max;
}

int32_t PID_Compute(PID_t *pid, int32_t error)
{
    pid->integral += error;
    int32_t derivative = error - pid->prev_error;
    pid->prev_error = error;

    int32_t output =
            pid->kp * error +
            pid->ki * pid->integral +
            pid->kd * derivative;

    if (output > pid->out_max) output = pid->out_max;
    if (output < pid->out_min) output = pid->out_min;

    return output;
}
