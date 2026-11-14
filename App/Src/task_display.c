
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

/* ==========================================
 * CONFIG
 * ========================================== */

#define DISPLAY_TASK_STACK_SIZE 512U
#define DISPLAY_TASK_PRIORITY \
    (tskIDLE_PRIORITY + 1U)

#define BUTTON_POLL_MS 20U
#define MAX_STATE_READ_ERRORS 3U

static TaskHandle_t xDisplayTaskHandle = NULL;

/* Bien theo doi loi de xem trong Debugger */
static volatile uint32_t ulQueueDropCount = 0U;
static volatile uint32_t ulMutexErrorCount = 0U;
static volatile uint32_t ulOledInitErrorCount = 0U;

/* ==========================================
 * READ SYSTEM STATE
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
            pdMS_TO_TICKS(10)) != pdTRUE)
    {
        return pdFAIL;
    }

    /* Copy du lieu trong khi giu Mutex */
    *pState = gSystemState;

    xSemaphoreGive(xSystemStateMutex);

    return pdPASS;
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

    bsp_oled_write_line(0U, pText->temperature);
    bsp_oled_write_line(1U, pText->humidity);
    bsp_oled_write_line(2U, pText->distance);
    bsp_oled_write_line(3U, pText->threshold);
    bsp_oled_write_line(4U, pText->source);
    bsp_oled_write_line(5U, pText->alarm);

    bsp_oled_update();
}

/* ==========================================
 * UPDATE LED AND BUZZER
 * ========================================== */

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

/* ==========================================
 * SAFE STATE WHEN DATA IS UNAVAILABLE
 * ========================================== */

static void prvSetDataErrorOutput(void)
{
    BspAlarmOutput_t output = {0};

    /* Bao loi du lieu, khong coi la binh thuong */
    output.led_green = 0U;
    output.led_red = 1U;
    output.buzzer = 0U;

    bsp_alarm_apply(&output);
}

/* ==========================================
 * SEND BUTTON EVENT
 * ========================================== */

static void prvSendButtonEvent(UiEvent_t event)
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
        /* Queue day, su kien khong duoc gui */
        ulQueueDropCount++;
    }
}

/* ==========================================
 * PROCESS BUTTON
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

    ButtonEvent_t event = button_logic_update(
        button,
        bsp_button_is_pressed(),
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
 * MAIN DISPLAY TASK
 * ========================================== */

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

    uint8_t consecutiveErrors = 0U;

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
        /* Kiem tra nut nhan */
        prvProcessButton(&button);

        TickType_t now = xTaskGetTickCount();

        if ((now - lastDisplayTick) >= displayTicks)
        {
            lastDisplayTick = now;

            /* Doc SystemState */
            if (prvReadSystemState(&localState)
                == pdPASS)
            {
                /* Reset so lan loi lien tiep */
                consecutiveErrors = 0U;

                /* Chuyen du lieu thanh chuoi */
                display_format_data(
                    &localState,
                    &displayText
                );

                /* Cap nhat OLED neu san sang */
                if (oledReady != 0U)
                {
                    prvUpdateOled(&displayText);
                }

                /* Dieu khien LED va Buzzer */
                prvUpdateAlarm(&localState);
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
                    /* Bao loi du lieu */
                    prvSetDataErrorOutput();
                }
            }
        }

        /* Duy tri chu ky FreeRTOS */
        vTaskDelayUntil(
            &xLastWakeTime,
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
