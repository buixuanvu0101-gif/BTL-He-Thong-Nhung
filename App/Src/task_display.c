
#include "task_display.h"

#include "app_cfg.h"
#include "app_ipc.h"
#include "display_format.h"
#include "display_outputs.h"

#include "bsp_oled.h"
#include "bsp_alarm.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * DEV 2 - FREERTOS DISPLAY TASK
 *
 * Nhiem vu:
 * 1. Doc SystemState bang Mutex.
 * 2. Doc canh bao tu EventGroup.
 * 3. Hien thi OLED SSD1306.
 * 4. Gui trang thai LED/Buzzer qua adapter.
 *
 * Khong doc GPIO nut nhan.
 * Khong xu ly UART.
 * Khong doc cam bien.
 * Khong tinh nguong canh bao.
 */

#ifndef TASK_DISPLAY_PRIO
#define TASK_DISPLAY_PRIO \
    (tskIDLE_PRIORITY + 1U)
#endif

#ifndef TASK_DISPLAY_STACK
#define TASK_DISPLAY_STACK 512U
#endif

#define DISPLAY_MUTEX_TIMEOUT_MS 10U
#define DISPLAY_MAX_READ_ERRORS  3U

/* Dung DISPLAY_PERIOD_MS tu app_cfg.h */

static TaskHandle_t s_display_task_handle = NULL;

/* Bien theo doi trong debugger */
static volatile uint32_t s_mutex_errors = 0U;
static volatile uint32_t s_ipc_errors = 0U;
static volatile uint32_t s_oled_init_errors = 0U;
static volatile uint32_t s_oled_updates = 0U;
static volatile uint32_t s_oled_skipped = 0U;

/* ========================================
 * READ SYSTEM STATE
 * ======================================== */

static BaseType_t prvReadSystemState(
    SystemState_t *state)
{
    if ((state == NULL) ||
        (xSystemStateMutex == NULL))
    {
        return pdFAIL;
    }

    TickType_t timeout =
        pdMS_TO_TICKS(DISPLAY_MUTEX_TIMEOUT_MS);

    if (timeout == 0U)
    {
        timeout = 1U;
    }

    if (xSemaphoreTake(
            xSystemStateMutex,
            timeout) != pdTRUE)
    {
        return pdFAIL;
    }

    *state = gSystemState;

    (void)xSemaphoreGive(xSystemStateMutex);

    return pdPASS;
}

/* ========================================
 * READ ALARM EVENT GROUP
 * ======================================== */

static BaseType_t prvReadAlarmFlags(
    uint32_t *alarm_flags)
{
    if ((alarm_flags == NULL) ||
        (xAlarmEventGroup == NULL))
    {
        return pdFAIL;
    }

    EventBits_t bits =
        xEventGroupGetBits(xAlarmEventGroup);

    /*
     * Hien tai app_cfg.h dinh nghia:
     * ALARM_DISTANCE.
     *
     * Chi lay bit canh bao duoc thong nhat.
     */
    *alarm_flags =
        (uint32_t)(bits & ALARM_DISTANCE);

    return pdPASS;
}

/* ========================================
 * OLED CONTENT COMPARISON
 * ======================================== */

static uint8_t prvTextChanged(
    const DisplayText_t *current,
    const DisplayText_t *previous)
{
    if ((current == NULL) ||
        (previous == NULL))
    {
        return 1U;
    }

    if (strcmp(
            current->temperature,
            previous->temperature) != 0)
    {
        return 1U;
    }

    if (strcmp(
            current->humidity,
            previous->humidity) != 0)
    {
        return 1U;
    }

    if (strcmp(
            current->distance,
            previous->distance) != 0)
    {
        return 1U;
    }

    if (strcmp(
            current->threshold,
            previous->threshold) != 0)
    {
        return 1U;
    }

    if (strcmp(
            current->source,
            previous->source) != 0)
    {
        return 1U;
    }

    if (strcmp(
            current->alarm,
            previous->alarm) != 0)
    {
        return 1U;
    }

    return 0U;
}

/* ========================================
 * UPDATE OLED
 * ======================================== */

static void prvUpdateOled(
    const DisplayText_t *text)
{
    if (text == NULL)
    {
        return;
    }

    bsp_oled_clear();

    bsp_oled_write_line(
        0U, text->temperature);

    bsp_oled_write_line(
        1U, text->humidity);

    bsp_oled_write_line(
        2U, text->distance);

    bsp_oled_write_line(
        3U, text->threshold);

    bsp_oled_write_line(
        4U, text->source);

    bsp_oled_write_line(
        5U, text->alarm);

    bsp_oled_update();
}

/* ========================================
 * LED AND BUZZER
 * ======================================== */

static void prvApplyAlarm(
    uint32_t alarm_flags)
{
    BspAlarmOutput_t output =
        bsp_alarm_evaluate(alarm_flags);

    display_outputs_apply(
        output.led_green,
        output.led_red,
        output.buzzer
    );
}

/* ========================================
 * SAFE STATE ON IPC ERROR
 * ======================================== */

static void prvApplyDataError(void)
{
    /*
     * LED xanh OFF
     * LED do ON
     * Buzzer OFF
     *
     * Chinh sach loi de xuat.
     */
    display_outputs_apply(
        0U,
        1U,
        0U
    );
}

/* ========================================
 * MAIN DISPLAY TASK
 * ======================================== */

static void prvDisplayTask(
    void *pvParameters)
{
    (void)pvParameters;

    TickType_t lastWakeTime =
        xTaskGetTickCount();

    TickType_t periodTicks =
        pdMS_TO_TICKS(DISPLAY_PERIOD_MS);

    if (periodTicks == 0U)
    {
        periodTicks = 1U;
    }

    SystemState_t localState = {0};

    DisplayText_t currentText = {0};
    DisplayText_t previousText = {0};

    uint32_t alarmFlags = 0U;

    uint8_t consecutiveErrors = 0U;
    uint8_t hasPreviousFrame = 0U;

    /*
     * Backend I2C phai duoc bind
     * truoc khi task bat dau.
     */
    uint8_t oledReady = bsp_oled_init();

    if (oledReady == 0U)
    {
        s_oled_init_errors++;
    }

    for (;;)
    {
        BaseType_t stateOk =
            prvReadSystemState(&localState);

        BaseType_t alarmOk =
            prvReadAlarmFlags(&alarmFlags);

        if ((stateOk == pdPASS) &&
            (alarmOk == pdPASS))
        {
            consecutiveErrors = 0U;

            /*
             * EventGroup la nguon
             * trang thai canh bao.
             */
            localState.alarm_flags =
                alarmFlags;

            /* Cap nhat LED/Buzzer */
            prvApplyAlarm(alarmFlags);

            /* Chuyen du lieu thanh chuoi */
            display_format_data(
                &localState,
                &currentText
            );

            /* Chi ve lai neu co thay doi */
            uint8_t needsUpdate =
                (hasPreviousFrame == 0U) ||
                prvTextChanged(
                    &currentText,
                    &previousText
                );

            if ((needsUpdate != 0U) &&
                (oledReady != 0U))
            {
                prvUpdateOled(&currentText);

                /*
                 * API OLED update hien
                 * tra ve void.
                 *
                 * Chua xac nhan I2C
                 * da truyen thanh cong.
                 */
                previousText = currentText;
                hasPreviousFrame = 1U;

                s_oled_updates++;
            }
            else if (needsUpdate == 0U)
            {
                s_oled_skipped++;
            }
        }
        else
        {
            if (stateOk != pdPASS)
            {
                s_mutex_errors++;
            }

            if (alarmOk != pdPASS)
            {
                s_ipc_errors++;
            }

            if (consecutiveErrors <
                DISPLAY_MAX_READ_ERRORS)
            {
                consecutiveErrors++;
            }

            if (consecutiveErrors >=
                DISPLAY_MAX_READ_ERRORS)
            {
                prvApplyDataError();
            }
        }

        vTaskDelayUntil(
            &lastWakeTime,
            periodTicks
        );
    }
}

/* ========================================
 * CREATE TASK
 * ======================================== */

BaseType_t task_display_create(void)
{
    if (s_display_task_handle != NULL)
    {
        return pdFAIL;
    }

    return xTaskCreate(
        prvDisplayTask,
        "Display",
        TASK_DISPLAY_STACK,
        NULL,
        TASK_DISPLAY_PRIO,
        &s_display_task_handle
    );
}

/* ========================================
 * GET TASK HANDLE
 * ======================================== */

TaskHandle_t task_display_get_handle(void)
{
    return s_display_task_handle;
}
