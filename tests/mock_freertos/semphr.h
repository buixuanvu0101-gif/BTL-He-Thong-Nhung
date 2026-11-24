#ifndef DEV2_MOCK_SEMPHR_H
#define DEV2_MOCK_SEMPHR_H

#include "FreeRTOS.h"

BaseType_t xSemaphoreTake(
    SemaphoreHandle_t handle,
    TickType_t timeout
);

BaseType_t xSemaphoreGive(
    SemaphoreHandle_t handle
);

#endif
