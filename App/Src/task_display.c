
#include "task_display.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "display_format.h"
#include "button_logic.h"
#include "bsp_alarm.h"
#include "bsp_button.h"

#include <stddef.h>
#include <stdint.h>

/* Task configuration */
#define DISPLAY_TASK_STACK_SIZE 384U
#define DISPLAY_TASK_PRIORITY \
    (tskIDLE_PRIORITY + 1U)

#define BUTTON_POLL_MS 20U

static TaskHandle_t xDisplayTaskHandle = NULL;

/* Read shared state safely */
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
            pdMS_TO_TICKS(10)) != pdTRUE)
    {
        return pdFAIL;
    }

    *pState = gSystemState;
    xSemaphoreGive(xSystemStateMutex);

    return pdPASS;
}

/* Update LED and buzzer */
static void prvUpdateAlarm(
    const SystemState_t *pState)
{
    if (pState == NULL)
    {
        return;
    }

    BspAlarmOutput_t output =
        bsp_alarm_evaluate(pState->alarm_flags);

    bsp_alarm_apply(&output);
}

/* Detect button event and send to Dev 3 */
static void prvProcessButton(
    ButtonLogic_t *button)
{
    uint32_t now_ms =
        (uint32_t)xTaskGetTickCount() *
        (uint32_t)portTICK_PERIOD_MS;

    ButtonEvent_t event = button_logic_update(
        button,
        bsp_button_is_pressed(),
        now_ms
    );

    UiEvent_t uiEvent;

    if (event == BUTTON_EVENT_SHORT)
    {
        uiEvent = UI_EVENT_BUTTON_SHORT;
    }
    else if (event == BUTTON_EVENT_LONG)
    {
        uiEvent = UI_EVENT_BUTTON_LONG;
    }
    else
    {
        return;
    }

    if (xUiEventQueue != NULL)
    {
        if (xQueueSend(
                xUiEventQueue,
                &uiEvent,
                0U) != pdPASS)
        {
            /* TODO: Record queue overflow */
        }
    }
}

/* Main display task */
static void prvDisplayTask(void *pvParameters)
{
    (void)pvParameters;

    TickType_t xLastWakeTime =
        xTaskGetTickCount();

    TickType_t pollTicks =
        pdMS_TO_TICKS(BUTTON_POLL_MS);

    TickType_t displayTicks =
        pdMS_TO_TICKS(DISPLAY_PERIOD_MS);

    TickType_t lastDisplayTick =
        xLastWakeTime;

    SystemState_t localState = {0};
    DisplayText_t displayText = {0};
    ButtonLogic_t button;

    button_logic_init(&button);

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
        prvProcessButton(&button);

        TickType_t now = xTaskGetTickCount();

        if ((now - lastDisplayTick) >= displayTicks)
        {
            lastDisplayTick = now;

            if (prvReadSystemState(&localState)
                == pdPASS)
            {
                display_format_data(
                    &localState,
                    &displayText
                );

                /*
                 * TODO:
                 * Draw displayText on SSD1306.
                 */

                prvUpdateAlarm(&localState);
            }
            else
            {
                /*
                 * TODO:
                 * Handle missing state.
                 */
            }
        }

        vTaskDelayUntil(
            &xLastWakeTime,
            pollTicks
        );
    }
}

/* Create FreeRTOS task */
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

/* Get handle for stack monitoring */
TaskHandle_t task_display_get_handle(void)
{
    return xDisplayTaskHandle;
}
