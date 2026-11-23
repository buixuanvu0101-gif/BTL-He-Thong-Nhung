#ifndef DEV2_MOCK_QUEUE_H
#define DEV2_MOCK_QUEUE_H
#include "FreeRTOS.h"
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t timeout);
BaseType_t xQueueSendFromISR(QueueHandle_t queue, const void *item, BaseType_t *woken);
#endif
