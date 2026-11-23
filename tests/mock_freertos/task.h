#ifndef DEV2_MOCK_TASK_H
#define DEV2_MOCK_TASK_H

#include "FreeRTOS.h"

BaseType_t xTaskCreate(
    void (*task_fn)(void *),
    const char *name,
    uint32_t stack_depth,
    void *parameters,
    UBaseType_t priority,
    TaskHandle_t *handle
);

TickType_t xTaskGetTickCount(void);

void vTaskDelayUntil(
    TickType_t *previous_wake,
    TickType_t increment
);

#endif
