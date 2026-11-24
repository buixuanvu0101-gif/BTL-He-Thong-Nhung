#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

/* FreeRTOS Display Task: OLED, alarm outputs and button event production. */
BaseType_t task_display_create(void);
TaskHandle_t task_display_get_handle(void);

/* Debugger diagnostics; counters reset only after power-on. */
uint32_t task_display_get_queue_drop_count(void);
uint32_t task_display_get_state_read_error_count(void);
uint32_t task_display_get_oled_error_count(void);

#endif
