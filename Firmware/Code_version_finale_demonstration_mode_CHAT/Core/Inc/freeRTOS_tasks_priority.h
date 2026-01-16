/*
 * freeRTOS_tasks_priority.h
 *
 *  Created on: Jan 13, 2026
 *      Author: hugoc
 */

#ifndef INC_FREERTOS_TASKS_PRIORITY_H_
#define INC_FREERTOS_TASKS_PRIORITY_H_

#define ADXL_TaskRead_PRIORITY 2
#define ADXL_TaskPrint_PRIORITY 1

#define task_ROOMBA_PRIORITY 6
#define task_CHAT_PRIORITY 2
#define task_SOURIS_PRIORITY 2
#define task_EDGE_PRIORITY 6

#define task_Control_PRIORITY 5

#define task_ENC_D_Update_PRIORITY 6
#define task_ENC_G_Update_PRIORITY 6
#define task_Odom_Update_PRIORITY 3

#define task_BLUETOOTH_TX 1
#define task_BLUETOOTH_RX 2

#define task_LIDAR_Update_PRIORITY 3

#define task_motor_PRIORITY 5

#define task_screen_PRIORITY 1



#endif /* INC_FREERTOS_TASKS_PRIORITY_H_ */
