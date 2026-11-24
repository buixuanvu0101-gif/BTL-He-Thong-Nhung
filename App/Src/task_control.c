#include "task_control.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "app_cfg.h"
#include "app_ipc.h"

#define CONTROL_STACK_WORDS  256U
#define CONTROL_WAIT_MS      100U
#define MUTEX_TIMEOUT_MS     20U
#define ALARM_HYST_CM        1.0f

static void prvControlTask(void *arg)
{
    SensorData_t d;
    uint32_t flags;
    (void)arg;

    for (;;) {
        if (xQueueReceive(xSensorQueue, &d, pdMS_TO_TICKS(CONTROL_WAIT_MS)) != pdTRUE) {
            continue;
        }
        if (xSemaphoreTake(xSystemStateMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
            continue;
        }

        gSystemState.sensor = d;
        flags = gSystemState.alarm_flags;

        if (d.distance_cm >= 2.0f && d.distance_cm < gSystemState.distance_threshold_cm) {
            flags |= ALARM_DISTANCE;
        } else if (d.distance_cm >= gSystemState.distance_threshold_cm + ALARM_HYST_CM) {
            flags &= ~ALARM_DISTANCE;
        }

        gSystemState.alarm_flags = flags;
        (void)xSemaphoreGive(xSystemStateMutex);

        if ((flags & ALARM_DISTANCE) != 0U) {
            (void)xEventGroupSetBits(xAlarmEventGroup, ALARM_DISTANCE);
        } else {
            (void)xEventGroupClearBits(xAlarmEventGroup, ALARM_DISTANCE);
        }
    }
}

BaseType_t task_control_create(void)
{
    return xTaskCreate(prvControlTask, "Control", CONTROL_STACK_WORDS, NULL,
                       tskIDLE_PRIORITY + 2U, NULL);
}
