
#include "task_display.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "display_format.h"
#include "button_logic.h"

#include "bsp_oled.h"
#include "bsp_alarm.h"
#include "bsp_button.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ==========================================
 * CONFIG
 * ========================================== */

#define DISPLAY_TASK_STACK_SIZE 512U

#define DISPLAY_TASK_PRIORITY \
    (tskIDLE_PRIORITY + 1U)

#define BUTTON_POLL_MS 20U
#define MAX_STATE_READ_ERRORS 3U

static TaskHandle_t xDisplayTaskHandle = NULL;

/* Debug counters */
static volatile uint32_t ulQueueDropCount = 0U;
static volatile uint32_t ulMutexErrorCount = 0U;
static volatile uint32_t ulOledInitErrorCount = 0U;
static volatile uint32_t ulOledUpdateCount = 0U;
static volatile uint32_t ulOledSkippedCount = 0U;

/* ==========================================
 * READ SHARED STATE
 * ========================================== */

static BaseType_t prvReadSystemState(
    SystemState_t *pState)
{
    if ((pState == NULL) ||
        (xSystemStateMutex == NULL))
    {
        return pdFAIL;
    }

    if (xSemaphoreTake(
            xSystemStateMutex,
            pdMS_TO_TICKS(10U)) != pdTRUE)
    {
        return pdFAIL;
    }

    *pState = gSystemState;

    xSemaphoreGive(xSystemStateMutex);

    return pdPASS;
}

/* ==========================================
 * COMPARE DISPLAY CONTENT
 * ========================================== */

static uint8_t prvDisplayTextChanged(
    const DisplayText_t *current,
    const DisplayText_t *previous)
{
    if ((current == NULL) ||
        (previous == NULL))
    {
        return 1U;
    }

    if (strcmp(current->temperature,
               previous->temperature) != 0)
    {
        return 1U;
    }

    if (strcmp(current->humidity,
               previous->humidity) != 0)
    {
        return 1U;
    }

    if (strcmp(current->distance,
               previous->distance) != 0)
    {
        return 1U;
    }

    if (strcmp(current->threshold,
               previous->threshold) != 0)
    {
        return 1U;
    }

    if (strcmp(current->source,
               previous->source) != 0)
    {
        return 1U;
    }

    if (strcmp(current->alarm,
               previous->alarm) != 0)
    {
        return 1U;
    }

    return 0U;
}

/* ==========================================
 * UPDATE OLED
 * ========================================== */

static void prvUpdateOled(
    const DisplayText_t *pText)
{
    if (pText == NULL)
    {
        return;
    }

    bsp_oled_clear();

    bsp_oled_write_line(
        0U, pText->temperature);

    bsp_oled_write_line(
        1U, pText->humidity);

    bsp_oled_write_line(
        2U, pText->distance);

    bsp_oled_write_line(
        3U, pText->threshold);

    bsp_oled_write_line(
        4U, pText->source);

    bsp_oled_write_line(
        5U, pText->alarm);

    bsp_oled_update();
}

/* ==========================================
 * ALARM OUTPUT
 * ========================================== */

static void prvUpdateAlarm(
    const SystemState_t *pState)
{
    if (pState == NULL)
    {
        return;
    }

    BspAlarmOutput_t output =
        bsp_alarm_evaluate(
            pState->alarm_flags
        );

    bsp_alarm_apply(&output);
}

/* ==========================================
 * DATA ERROR OUTPUT
 * ========================================== */

static void prvSetDataErrorOutput(void)
{
    BspAlarmOutput_t output = {0};

    output.led_green = 0U;
    output.led_red = 1U;
    output.buzzer = 0U;

    bsp_alarm_apply(&output);
}

/* ==========================================
 * BUTTON QUEUE
 * ========================================== */

static void prvSendButtonEvent(
    UiEvent_t event)
{
    if (xUiEventQueue == NULL)
    {
        ulQueueDropCount++;
        return;
    }

    if (xQueueSend(
            xUiEventQueue,
            &event,
            0U) != pdPASS)
    {
        ulQueueDropCount++;
    }
}

/* ==========================================
 * BUTTON PROCESSING
 * ========================================== */

static void prvProcessButton(
    ButtonLogic_t *button)
{
    if (button == NULL)
    {
        return;
    }

    uint32_t now_ms =
        (uint32_t)xTaskGetTickCount() *
        (uint32_t)portTICK_PERIOD_MS;

    uint8_t pressed =
        bsp_button_is_pressed();

    ButtonEvent_t event =
        button_logic_update(
            button,
            pressed,
            now_ms
        );

    if (event == BUTTON_EVENT_SHORT)
    {
        prvSendButtonEvent(
            UI_EVENT_BUTTON_SHORT
        );
    }
    else if (event == BUTTON_EVENT_LONG)
    {
        prvSendButtonEvent(
            UI_EVENT_BUTTON_LONG
        );
    }
}

/* ==========================================
 * MAIN TASK
 * ========================================== */

static void prvDisplayTask(
    void *pvParameters)
{
    (void)pvParameters;

    TickType_t lastWakeTime =
        xTaskGetTickCount();

    TickType_t pollTicks =
        pdMS_TO_TICKS(BUTTON_POLL_MS);

    TickType_t displayTicks =
        pdMS_TO_TICKS(DISPLAY_PERIOD_MS);

    TickType_t lastDisplayTick =
        lastWakeTime;

    SystemState_t localState = {0};

    DisplayText_t currentText = {0};
    DisplayText_t previousText = {0};

    ButtonLogic_t button;

    uint8_t consecutiveErrors = 0U;
    uint8_t hasPreviousFrame = 0U;

    button_logic_init(&button);

    /* Khoi tao OLED */
    uint8_t oledReady = bsp_oled_init();

    if (oledReady == 0U)
    {
        ulOledInitErrorCount++;
    }

    if (pollTicks == 0U)
    {
        pollTicks = 1U;
    }

    if (displayTicks == 0U)
    {
        displayTicks = 1U;
    }

    for (;;)
    {
        /* Poll nut nhan */
        prvProcessButton(&button);

        TickType_t now =
            xTaskGetTickCount();

        if ((TickType_t)(now - lastDisplayTick)
            >= displayTicks)
        {
            lastDisplayTick = now;

            if (prvReadSystemState(&localState)
                == pdPASS)
            {
                consecutiveErrors = 0U;

                /* Cap nhat canh bao */
                prvUpdateAlarm(&localState);

                /* Dinh dang du lieu */
                display_format_data(
                    &localState,
                    &currentText
                );

                /* Kiem tra noi dung thay doi */
                uint8_t needsUpdate =
                    (hasPreviousFrame == 0U) ||
                    prvDisplayTextChanged(
                        &currentText,
                        &previousText
                    );

                if (needsUpdate != 0U)
                {
                    if (oledReady != 0U)
                    {
                        prvUpdateOled(
                            &currentText
                        );

                        /*
                         * Driver hien tai tra ve void,
                         * nen chua xac minh duoc
                         * viec truyen I2C thanh cong.
                         */
                        previousText = currentText;
                        hasPreviousFrame = 1U;

                        ulOledUpdateCount++;
                    }
                }
                else
                {
                    ulOledSkippedCount++;
                }
            }
            else
            {
                ulMutexErrorCount++;

                if (consecutiveErrors <
                    MAX_STATE_READ_ERRORS)
                {
                    consecutiveErrors++;
                }

                if (consecutiveErrors >=
                    MAX_STATE_READ_ERRORS)
                {
                    prvSetDataErrorOutput();
                }
            }
        }

        vTaskDelayUntil(
            &lastWakeTime,
            pollTicks
        );
    }
}

/* ==========================================
 * CREATE TASK
 * ========================================== */

BaseType_t task_display_create(void)
{
    if (xDisplayTaskHandle != NULL)
    {
        return pdFAIL;
    }

    return xTaskCreate(
        prvDisplayTask,
        "Display",
        DISPLAY_TASK_STACK_SIZE,
        NULL,
        DISPLAY_TASK_PRIORITY,
        &xDisplayTaskHandle
    );
}

/* ==========================================
 * GET TASK HANDLE
 * ========================================== */

TaskHandle_t task_display_get_handle(void)
{
    return xDisplayTaskHandle;
}
