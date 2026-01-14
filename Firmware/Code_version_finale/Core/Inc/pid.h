/*
 * pid.h
 *
 *  Created on: Jan 12, 2026
 *      Author: hugoc
 */

#ifndef INC_PID_H_
#define INC_PID_H_

#include <stdint.h>
#include "freeRTOS_tasks_priority.h"

typedef struct {
    int32_t kp;
    int32_t ki;
    int32_t kd;

    int32_t integral;
    int32_t prev_error;

    int32_t out_min;
    int32_t out_max;
} PID_t;

void PID_Init(PID_t *pid,
              int32_t kp,
              int32_t ki,
              int32_t kd,
              int32_t out_min,
              int32_t out_max);

int32_t PID_Compute(PID_t *pid, int32_t error);


#endif /* INC_PID_H_ */
