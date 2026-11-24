
#include "app_ipc.h"

/* Queue du lieu cam bien */
QueueHandle_t xSensorQueue = NULL;

/* Queue su kien nut bam */
QueueHandle_t xUiEventQueue = NULL;

/* Mutex bao ve SystemState */
SemaphoreHandle_t xSystemStateMutex = NULL;

/* Event Group canh bao */
EventGroupHandle_t xAlarmEventGroup = NULL;

/* Trang thai ban dau */
SystemState_t gSystemState =
{
    .sensor = {0},
    .distance_threshold_cm = 20.0f,
    .threshold_source = THRESHOLD_SOURCE_POT,
    .alarm_flags = 0
};

/* Khoi tao cac doi tuong FreeRTOS */
BaseType_t app_ipc_init(void)
{
    /* Tao Queue cho du lieu cam bien */
    xSensorQueue = xQueueCreate(
        5,
        sizeof(SensorData_t)
    );

    /* Tao Queue cho su kien nut bam */
    xUiEventQueue = xQueueCreate(
        5,
        sizeof(UiEvent_t)
    );

    /* Tao Mutex bao ve du lieu */
    xSystemStateMutex = xSemaphoreCreateMutex();

    /* Tao Event Group cho canh bao */
    xAlarmEventGroup = xEventGroupCreate();

    /* Kiem tra khoi tao */
    if ((xSensorQueue == NULL) ||
        (xUiEventQueue == NULL) ||
        (xSystemStateMutex == NULL) ||
        (xAlarmEventGroup == NULL))
    {
        return pdFAIL;
    }

    return pdPASS;
}
