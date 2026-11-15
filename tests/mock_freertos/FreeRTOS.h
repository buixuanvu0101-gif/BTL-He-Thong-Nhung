
#ifndef DEV2_MOCK_FREERTOS_H
#define DEV2_MOCK_FREERTOS_H

#include <stdint.h>
#include <stddef.h>

/*
 * MOCK FREERTOS FOR WINDOWS TESTING
 *
 * Chi dung trong thu muc tests.
 * KHONG dung de thay the FreeRTOS
 * chinh thuc khi build STM32.
 */

/* FreeRTOS basic types */
typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef uint32_t TickType_t;
typedef uint32_t EventBits_t;

/* FreeRTOS handles */
typedef void *TaskHandle_t;
typedef void *SemaphoreHandle_t;
typedef void *QueueHandle_t;
typedef void *EventGroupHandle_t;

/* Return values */
#define pdTRUE   1
#define pdFALSE  0

#define pdPASS   1
#define pdFAIL   0

/* Task priority */
#define tskIDLE_PRIORITY 0U

/*
 * Mock tick period: 1 ms
 * Phu hop bai test Windows.
 */
#define portTICK_PERIOD_MS 1U

#define pdMS_TO_TICKS(ms) \
    ((TickType_t)(ms))

#endif /* DEV2_MOCK_FREERTOS_H */
