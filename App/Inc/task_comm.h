#ifndef TASK_COMM_H
#define TASK_COMM_H

#include "FreeRTOS.h"

BaseType_t Comm_TaskCreate(void);   /* Leader gọi trong main.c / freertos.c */

#endif