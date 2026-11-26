#include "task_encoder.h"
#include "encoder_ky040.h"
#include "app_ipc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define ENCODER_TASK_STACK_WORDS 256U
#define ENCODER_POLL_PERIOD_MS 1U

/* Task doc Encoder KY-040 */
static void prvEncoderTask(void *pvParameters)
{
    (void)pvParameters;

    encoder_ky040_init();

    TickType_t pollTicks =
        pdMS_TO_TICKS(ENCODER_POLL_PERIOD_MS);

    if (pollTicks == 0U)
    {
        pollTicks = 1U;
    }

    for (;;)
    {
        EncoderEvent_t event =
            encoder_ky040_poll();

        UiEvent_t uiEvent;
        BaseType_t hasEvent = pdTRUE;

        switch (event)
        {
            case ENCODER_CW:
                uiEvent = UI_EVENT_ENCODER_CW;
                break;

            case ENCODER_CCW:
                uiEvent = UI_EVENT_ENCODER_CCW;
                break;

            case ENCODER_PRESSED:
                uiEvent = UI_EVENT_ENCODER_PRESSED;
                break;

            default:
                hasEvent = pdFALSE;
                break;
        }

        /* Gui su kien cho Dev 3 */
        if ((hasEvent == pdTRUE) &&
            (xUiEventQueue != NULL))
        {
            if (xQueueSend(
                    xUiEventQueue,
                    &uiEvent,
                    0U) != pdPASS)
            {
                /* TODO: Dem su kien bi mat */
            }
        }

        vTaskDelay(pollTicks);
    }
}

/* Tao FreeRTOS Encoder Task */
BaseType_t task_encoder_create(void)
{
    if (xUiEventQueue == NULL)
    {
        return pdFAIL;
    }

    return xTaskCreate(
        prvEncoderTask,
        "Encoder",
        ENCODER_TASK_STACK_WORDS,
        NULL,
        tskIDLE_PRIORITY + 2,
        NULL
    );
}
