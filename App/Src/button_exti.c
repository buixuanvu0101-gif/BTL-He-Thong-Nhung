#include "button_exti.h"

#ifndef DEV2_BUTTON_MODE_EXTI
#define DEV2_BUTTON_MODE_EXTI 0
#endif

#if DEV2_BUTTON_MODE_EXTI == 1

#include "app_cfg.h"
#include "app_ipc.h"
#include "button_logic.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f1xx_hal.h"

/* Only PB10 is owned by this module. Other EXTI callbacks must be
 * forwarded to their owners if additional GPIO interrupts are added.
 * EXTI15_10_IRQn priority is 7 in current .ioc, valid for FreeRTOS
 * ...FromISR() with configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5.
 */
static volatile uint8_t s_ready = 0U;
static uint8_t s_pressed = 0U;
static TickType_t s_press_tick = 0U;
static volatile uint32_t s_queue_drops = 0U;

void button_exti_enable(void)
{
    s_pressed = 0U;
    s_ready = 1U; /* Called only from Display Task, after scheduler start. */
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ((s_ready == 0U) || (GPIO_Pin != GPIO_PIN_10)) { return; }
    const TickType_t now = xTaskGetTickCountFromISR();
    const uint8_t pressed = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)
                             == GPIO_PIN_RESET) ? 1U : 0U;
    if (pressed != 0U)
    {
        if (s_pressed == 0U)
        {
            s_pressed = 1U;
            s_press_tick = now;
        }
        return;
    }
    if (s_pressed == 0U) { return; }
    s_pressed = 0U;

    const TickType_t held = (TickType_t)(now - s_press_tick);
    const TickType_t debounce = pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS);
    const TickType_t long_press = pdMS_TO_TICKS(BUTTON_LONG_PRESS_MS);
    if (held < debounce) { return; }

    const UiEvent_t event = (held >= long_press)
                          ? UI_EVENT_BUTTON_LONG : UI_EVENT_BUTTON_SHORT;
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    if ((xUiEventQueue == NULL) ||
        (xQueueSendFromISR(xUiEventQueue, &event,
                           &higherPriorityTaskWoken) != pdPASS))
    {
        ++s_queue_drops;
    }
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

uint32_t button_exti_get_queue_drop_count(void)
{
    return s_queue_drops;
}

#else
typedef int dev2_button_exti_disabled_translation_unit;
#endif /* DEV2_BUTTON_MODE_EXTI */
