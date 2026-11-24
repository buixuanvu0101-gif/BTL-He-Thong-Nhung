#include "task_display.h"

#include "app_cfg.h"
#include "app_ipc.h"
#include "button_logic.h"
#include "button_exti.h"
#include "display_format.h"
#include "display_outputs.h"
#include "bsp_alarm.h"
#include "bsp_button.h"
#include "bsp_oled.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* 0: polling (works with current .ioc); 1: PB10 both-edge EXTI mode.
 * EXTI mode needs a Leader-approved CubeMX change and uses button_exti.c.
 */
#ifndef DEV2_BUTTON_MODE_EXTI
#define DEV2_BUTTON_MODE_EXTI 0
#endif

/* DisplayTask handles UI only; sensor sampling and threshold decisions
 * belong to Dev 1 and Dev 3.  The 20 ms poll is intentional: the current
 * CubeMX PB10 EXTI is RISING-only, so it cannot time both button edges.
 * This mode works with the existing pinout without changing .ioc.
 */
#ifndef TASK_DISPLAY_PRIO
#define TASK_DISPLAY_PRIO (tskIDLE_PRIORITY + 1U)
#endif
#ifndef TASK_DISPLAY_STACK
#define TASK_DISPLAY_STACK 512U  /* FreeRTOS words, not bytes */
#endif
#define DEV2_BUTTON_POLL_MS       20U
#define DEV2_MUTEX_TIMEOUT_MS    10U
#define DEV2_OLED_RETRY_MS      1000U
#define DEV2_MAX_READ_ERRORS      3U
#define DEV2_OLED_PAGES          (OLED_HEIGHT / 8U)

static TaskHandle_t s_display_task_handle = NULL;
static volatile uint32_t s_queue_drop_count = 0U;
static volatile uint32_t s_state_read_errors = 0U;
static volatile uint32_t s_oled_errors = 0U;

static BaseType_t prvReadSystemState(SystemState_t *state)
{
    if ((state == NULL) || (xSystemStateMutex == NULL))
    {
        return pdFAIL;
    }
    TickType_t timeout = pdMS_TO_TICKS(DEV2_MUTEX_TIMEOUT_MS);
    if (timeout == 0U) { timeout = 1U; }
    if (xSemaphoreTake(xSystemStateMutex, timeout) != pdTRUE)
    {
        return pdFAIL;
    }
    *state = gSystemState;
    (void)xSemaphoreGive(xSystemStateMutex);
    return pdPASS;
}

static BaseType_t prvReadAlarmFlags(uint32_t *flags)
{
    if ((flags == NULL) || (xAlarmEventGroup == NULL))
    {
        return pdFAIL;
    }
    *flags = (uint32_t)(xEventGroupGetBits(xAlarmEventGroup) & ALARM_DISTANCE);
    return pdPASS;
}

static uint8_t prvTextChanged(const DisplayText_t *a, const DisplayText_t *b)
{
    return (uint8_t)(
        (strcmp(a->temperature, b->temperature) != 0) ||
        (strcmp(a->humidity, b->humidity) != 0) ||
        (strcmp(a->distance, b->distance) != 0) ||
        (strcmp(a->threshold, b->threshold) != 0) ||
        (strcmp(a->source, b->source) != 0) ||
        (strcmp(a->alarm, b->alarm) != 0));
}

static void prvPrepareOled(const DisplayText_t *text)
{
    bsp_oled_clear();
    bsp_oled_write_line(0U, text->temperature);
    bsp_oled_write_line(1U, text->humidity);
    bsp_oled_write_line(2U, text->distance);
    bsp_oled_write_line(3U, text->threshold);
    bsp_oled_write_line(4U, text->source);
    bsp_oled_write_line(5U, text->alarm);
}

static void prvApplyAlarm(uint32_t flags)
{
    const BspAlarmOutput_t output = bsp_alarm_evaluate(flags);
    display_outputs_apply(output.led_green, output.led_red, output.buzzer);
}

#if DEV2_BUTTON_MODE_EXTI == 0
static void prvProcessButton(ButtonLogic_t *button)
{
    /* The existing Dev 1 GPIO abstraction reads PB10, active-low. */
    const uint32_t now_ms = (uint32_t)xTaskGetTickCount() *
                            (uint32_t)portTICK_PERIOD_MS;
    const ButtonEvent_t button_event = button_logic_update(
        button, bsp_button_is_pressed(), now_ms);
    UiEvent_t event;
    if (button_event == BUTTON_EVENT_SHORT)
    {
        event = UI_EVENT_BUTTON_SHORT;
    }
    else if (button_event == BUTTON_EVENT_LONG)
    {
        event = UI_EVENT_BUTTON_LONG;
    }
    else
    {
        return;
    }

    if ((xUiEventQueue == NULL) ||
        (xQueueSend(xUiEventQueue, &event, 0U) != pdPASS))
    {
        ++s_queue_drop_count;
    }
}

#endif /* polling mode */

static void prvDisplayTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t lastWake = xTaskGetTickCount();
    TickType_t lastFrameTick = lastWake;
    TickType_t lastOledAttempt = lastWake;
    TickType_t pollTicks = pdMS_TO_TICKS(DEV2_BUTTON_POLL_MS);
    TickType_t frameTicks = pdMS_TO_TICKS(DISPLAY_PERIOD_MS);
    TickType_t retryTicks = pdMS_TO_TICKS(DEV2_OLED_RETRY_MS);
    if (pollTicks == 0U) { pollTicks = 1U; }
    if (frameTicks == 0U) { frameTicks = 1U; }
    if (retryTicks == 0U) { retryTicks = 1U; }

#if DEV2_BUTTON_MODE_EXTI == 0
    ButtonLogic_t button;
    button_logic_init(&button);
#else
    button_exti_enable(); /* Scheduler is running; enable EXTI events. */
#endif
    SystemState_t localState = {0};
    DisplayText_t currentText = {0};
    DisplayText_t lastDrawn = {0};
    uint8_t firstFrame = 1U;
    uint8_t haveDrawn = 0U;
    uint8_t pendingPage = DEV2_OLED_PAGES;
    uint8_t consecutiveErrors = 0U;
    uint8_t oledReady = bsp_oled_init();
    if (oledReady == 0U) { ++s_oled_errors; }

    for (;;)
    {
        /* Short and long press detection needs a faster period than OLED. */
#if DEV2_BUTTON_MODE_EXTI == 0
        prvProcessButton(&button);
#endif
        const TickType_t now = xTaskGetTickCount();

        if ((firstFrame != 0U) ||
            ((TickType_t)(now - lastFrameTick) >= frameTicks))
        {
            firstFrame = 0U;
            lastFrameTick = now;
            uint32_t flags = 0U;
            const BaseType_t stateOk = prvReadSystemState(&localState);
            const BaseType_t alarmOk = prvReadAlarmFlags(&flags);
            if ((stateOk == pdPASS) && (alarmOk == pdPASS))
            {
                consecutiveErrors = 0U;
                localState.alarm_flags = flags; /* EventGroup is authoritative. */
                prvApplyAlarm(flags);
                display_format_data(&localState, &currentText);
                if ((oledReady != 0U) &&
                    ((haveDrawn == 0U) ||
                     (prvTextChanged(&currentText, &lastDrawn) != 0U)))
                {
                    prvPrepareOled(&currentText);
                    pendingPage = 0U;
                }
            }
            else
            {
                ++s_state_read_errors;
                if (consecutiveErrors < DEV2_MAX_READ_ERRORS)
                {
                    ++consecutiveErrors;
                }
                if (consecutiveErrors >= DEV2_MAX_READ_ERRORS)
                {
                    /* No buzzer on an IPC fault; red LED indicates fault. */
                    display_outputs_apply(0U, 1U, 0U);
                }
            }
        }

        /* Transfer one 128-byte SSD1306 page per 20 ms loop.  This keeps
         * button sampling responsive and bounds each I2C operation.
         */
        if ((oledReady != 0U) && (pendingPage < DEV2_OLED_PAGES))
        {
            if (bsp_oled_update_page(pendingPage) != 0U)
            {
                ++pendingPage;
                if (pendingPage == DEV2_OLED_PAGES)
                {
                    lastDrawn = currentText;
                    haveDrawn = 1U;
                }
            }
            else
            {
                ++s_oled_errors;
                oledReady = 0U;
                pendingPage = DEV2_OLED_PAGES;
                haveDrawn = 0U;
                lastOledAttempt = now;
            }
        }

        if ((oledReady == 0U) &&
            ((TickType_t)(now - lastOledAttempt) >= retryTicks))
        {
            lastOledAttempt = now;
            oledReady = bsp_oled_init();
            if (oledReady == 0U) { ++s_oled_errors; }
            haveDrawn = 0U; /* Force a redraw on the next display period. */
        }
        vTaskDelayUntil(&lastWake, pollTicks);
    }
}

BaseType_t task_display_create(void)
{
    if (s_display_task_handle != NULL) { return pdFAIL; }
    if ((xSystemStateMutex == NULL) || (xAlarmEventGroup == NULL) ||
        (xUiEventQueue == NULL))
    {
        return pdFAIL;
    }
    return xTaskCreate(prvDisplayTask, "Display", TASK_DISPLAY_STACK,
                       NULL, TASK_DISPLAY_PRIO, &s_display_task_handle);
}

TaskHandle_t task_display_get_handle(void)
{
    return s_display_task_handle;
}

uint32_t task_display_get_queue_drop_count(void)
{
    return s_queue_drop_count;
}

uint32_t task_display_get_state_read_error_count(void)
{
    return s_state_read_errors;
}

uint32_t task_display_get_oled_error_count(void)
{
    return s_oled_errors;
}
