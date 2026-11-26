#include "task_control.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include <stdbool.h>

#define CONTROL_STACK_WORDS  256U
#define CONTROL_POLL_MS      20U
#define MUTEX_TIMEOUT_MS     20U
#define ALARM_HYST_CM        1.0f
#define ENC_STEP_CM          1.0f
#define DEFAULT_DIST_TH_CM   20.0f

static uint32_t prvEvalAlarmLocked(void)
{
    float dist = gSystemState.sensor.distance_cm;
    float th   = gSystemState.distance_threshold_cm;
    uint32_t flags = gSystemState.alarm_flags;

    if (dist >= (float)APP_DIST_MIN_CM && dist < th) {
        flags |= ALARM_DISTANCE;
    } else if (dist >= th + ALARM_HYST_CM) {
        flags &= ~ALARM_DISTANCE;
    }

    gSystemState.alarm_flags = flags;
    return flags;
}

static void prvApplyAlarmBits(uint32_t flags)
{
    if ((flags & ALARM_DISTANCE) != 0U) {
        (void)xEventGroupSetBits(xAlarmEventGroup, ALARM_DISTANCE);
    } else {
        (void)xEventGroupClearBits(xAlarmEventGroup, ALARM_DISTANCE);
    }
}

static bool prvHandleUiEventLocked(UiEvent_t ev)
{
    float th = gSystemState.distance_threshold_cm;

    switch (ev) {
    case UI_EVENT_ENCODER_PRESSED:
    case UI_EVENT_BUTTON_SHORT:
        gSystemState.threshold_source =
            (gSystemState.threshold_source == THRESHOLD_SOURCE_PC)
                ? THRESHOLD_SOURCE_ENC
                : THRESHOLD_SOURCE_PC;
        return true;

    case UI_EVENT_ENCODER_CW:
        if (gSystemState.threshold_source != THRESHOLD_SOURCE_ENC) {
            return false;
        }
        th += ENC_STEP_CM;
        if (th > (float)APP_DIST_MAX_CM) {
            th = (float)APP_DIST_MAX_CM;
        }
        gSystemState.distance_threshold_cm = th;
        return true;

    case UI_EVENT_ENCODER_CCW:
        if (gSystemState.threshold_source != THRESHOLD_SOURCE_ENC) {
            return false;
        }
        th -= ENC_STEP_CM;
        if (th < (float)APP_DIST_MIN_CM) {
            th = (float)APP_DIST_MIN_CM;
        }
        gSystemState.distance_threshold_cm = th;
        return true;

    case UI_EVENT_BUTTON_LONG:
        gSystemState.threshold_source = THRESHOLD_SOURCE_ENC;
        gSystemState.distance_threshold_cm = DEFAULT_DIST_TH_CM;
        return true;

    default:
        return false;
    }
}

static void prvControlTask(void *arg)
{
    SensorData_t d;
    UiEvent_t ev;
    uint32_t flags;
    (void)arg;

    for (;;) {
        /* 1. Nhan du lieu cam bien (cho toi da 20ms de van phan hoi Encoder muot) */
        if (xQueueReceive(xSensorQueue, &d, pdMS_TO_TICKS(CONTROL_POLL_MS)) == pdTRUE) {
            if (xSemaphoreTake(xSystemStateMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) == pdTRUE) {
                gSystemState.sensor = d;
                flags = prvEvalAlarmLocked();
                (void)xSemaphoreGive(xSystemStateMutex);
                prvApplyAlarmBits(flags);
            }
        }

        /* 2. Xu ly toan bo su kien xoay/nhan Encoder KY-040 tu xUiEventQueue */
        while (xUiEventQueue != NULL &&
               xQueueReceive(xUiEventQueue, &ev, 0) == pdTRUE) {
            if (xSemaphoreTake(xSystemStateMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) == pdTRUE) {
                if (prvHandleUiEventLocked(ev)) {
                    flags = prvEvalAlarmLocked();
                    (void)xSemaphoreGive(xSystemStateMutex);
                    prvApplyAlarmBits(flags);
                } else {
                    (void)xSemaphoreGive(xSystemStateMutex);
                }
            }
        }
    }
}

BaseType_t task_control_create(void)
{
    return xTaskCreate(prvControlTask, "Control", CONTROL_STACK_WORDS, NULL,
                       tskIDLE_PRIORITY + 2U, NULL);
}
