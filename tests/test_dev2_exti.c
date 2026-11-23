#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "stm32f1xx_hal.h"
#include "button_exti.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

GPIO_TypeDef fake_gpio_c;
GPIO_TypeDef fake_gpio_b;
QueueHandle_t xUiEventQueue = NULL;
static int queue_obj;
static TickType_t tick;
static GPIO_PinState pin_level = GPIO_PIN_SET;
static unsigned event_count, yields;
static UiEvent_t events[3];
TickType_t xTaskGetTickCountFromISR(void) { return tick; }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    assert(port == GPIOB && pin == GPIO_PIN_10);
    return pin_level;
}
BaseType_t xQueueSendFromISR(QueueHandle_t q, const void *item, BaseType_t *woken)
{
    assert(q == xUiEventQueue && woken != NULL);
    assert(event_count < 3U);
    events[event_count++] = *(const UiEvent_t *)item;
    *woken = pdTRUE;
    return pdPASS;
}
void fake_yield(BaseType_t woken) { if (woken == pdTRUE) { ++yields; } }
void HAL_GPIO_EXTI_Callback(uint16_t pin);
static void edge(TickType_t at, GPIO_PinState state)
{
    tick = at;
    pin_level = state;
    HAL_GPIO_EXTI_Callback(GPIO_PIN_10);
}
int main(void)
{
    puts("=== DEV2: OPTIONAL EXTI FROMISR ===");
    xUiEventQueue = &queue_obj;
    HAL_GPIO_EXTI_Callback(GPIO_PIN_14); /* Unrelated pin ignored. */
    edge(10U, GPIO_PIN_RESET); /* Before scheduler: ignored. */
    button_exti_enable();
    edge(100U, GPIO_PIN_RESET);
    edge(110U, GPIO_PIN_SET); /* Bounce: <40 ms ignored. */
    edge(200U, GPIO_PIN_RESET);
    edge(400U, GPIO_PIN_SET); /* 200 ms short */
    edge(600U, GPIO_PIN_RESET);
    edge(2300U, GPIO_PIN_SET); /* 1700 ms long */
    assert(event_count == 2U && yields == 2U);
    assert(events[0] == UI_EVENT_BUTTON_SHORT);
    assert(events[1] == UI_EVENT_BUTTON_LONG);
    assert(button_exti_get_queue_drop_count() == 0U);
    puts("[PASS] GPIO edge duration + xQueueSendFromISR + ISR yield");
    return 0;
}
