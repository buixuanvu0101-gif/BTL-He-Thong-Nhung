
#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include "FreeRTOS.h"
#include "task.h"

/* Tao task hien thi */
BaseType_t task_display_create(void);

/* Lay handle de kiem tra stack */
TaskHandle_t task_display_get_handle(void);

#endif
