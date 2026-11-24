#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "app_cfg.h"
#include "app_ipc.h"
#include "task_display.h"
#include "display_outputs.h"
#include "bsp_alarm.h"
#include "bsp_oled.h"
#include "bsp_button.h"

#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

QueueHandle_t xSensorQueue = NULL;
QueueHandle_t xUiEventQueue = NULL;
SemaphoreHandle_t xSystemStateMutex = NULL;
EventGroupHandle_t xAlarmEventGroup = NULL;
SystemState_t gSystemState = {0};

static int fake_queue, fake_mutex, fake_group, fake_task;
static TickType_t fake_tick;
static void (*created_task)(void *);
static jmp_buf test_exit;
static unsigned loops, page_packets, mutex_reads, output_calls, queue_calls;
static unsigned oled_init_calls, oled_line_calls;
static UiEvent_t events[4];
static uint8_t led_green, led_red, buzzer_on;
static uint8_t last_page;

BaseType_t xTaskCreate(void (*fn)(void *), const char *name, uint32_t stack,
                       void *arg, UBaseType_t prio, TaskHandle_t *handle)
{
    (void)arg;
    assert(fn != NULL && name != NULL && handle != NULL);
    assert(strcmp(name, "Display") == 0);
    assert(stack == 512U && prio == tskIDLE_PRIORITY + 1U);
    created_task = fn;
    *handle = &fake_task;
    return pdPASS;
}
TickType_t xTaskGetTickCount(void) { return fake_tick; }
void vTaskDelayUntil(TickType_t *prev, TickType_t inc)
{
    assert(inc == 20U);
    *prev += inc;
    fake_tick = *prev;
    ++loops;
    if (fake_tick >= 3620U) { longjmp(test_exit, 1); }
}
BaseType_t xSemaphoreTake(SemaphoreHandle_t m, TickType_t timeout)
{
    assert(m == xSystemStateMutex && timeout > 0U);
    ++mutex_reads;
    /* Simulate an IPC fault after 3000 ms. */
    return (fake_tick >= 3000U) ? pdFALSE : pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t m)
{
    assert(m == xSystemStateMutex);
    return pdTRUE;
}
EventBits_t xEventGroupGetBits(EventGroupHandle_t group)
{
    assert(group == xAlarmEventGroup);
    return (fake_tick >= 2600U) ? ALARM_DISTANCE : 0U;
}
BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t timeout)
{
    assert(q == xUiEventQueue && item != NULL && timeout == 0U);
    assert(queue_calls < 4U);
    events[queue_calls++] = *(const UiEvent_t *)item;
    return pdPASS;
}
uint8_t bsp_button_is_pressed(void)
{
    /* 200ms short press, 1700ms long press. */
    return ((fake_tick >= 100U && fake_tick < 300U) ||
            (fake_tick >= 600U && fake_tick < 2300U)) ? 1U : 0U;
}
uint8_t bsp_oled_init(void) { ++oled_init_calls; return 1U; }
void bsp_oled_clear(void) { }
void bsp_oled_write_line(uint8_t row, const char *text)
{
    assert(row < 6U && text != NULL);
    ++oled_line_calls;
}
uint8_t bsp_oled_update_page(uint8_t page)
{
    assert(page < 8U);
    if (page_packets % 8U == 0U) { assert(page == 0U); }
    else { assert(page == last_page + 1U); }
    last_page = page;
    ++page_packets;
    return 1U;
}
uint8_t bsp_oled_update(void) { return 1U; }
static void led_callback(uint8_t green, uint8_t red)
{
    led_green = green;
    led_red = red;
    ++output_calls;
}
static void buzzer_callback(uint8_t on) { buzzer_on = on; }

int main(void)
{
    puts("=== DEV2: DISPLAY TASK / BUTTON / IPC ===");
    assert(task_display_create() == pdFAIL); /* Must have IPC first. */
    xUiEventQueue = &fake_queue;
    xSystemStateMutex = &fake_mutex;
    xAlarmEventGroup = &fake_group;
    gSystemState.sensor.temperature_c = 28.5f;
    gSystemState.sensor.humidity_percent = 65.0f;
    gSystemState.sensor.distance_cm = 15.2f;
    gSystemState.distance_threshold_cm = 20.0f;
    display_outputs_bind_led(led_callback);
    display_outputs_bind_buzzer(buzzer_callback);
    assert(task_display_create() == pdPASS);
    assert(task_display_create() == pdFAIL); /* No duplicate task. */
    assert(task_display_get_handle() != NULL);
    if (setjmp(test_exit) == 0) { created_task(NULL); assert(0); }

    assert(loops == 181U);
    assert(queue_calls == 2U);
    assert(events[0] == UI_EVENT_BUTTON_SHORT);
    assert(events[1] == UI_EVENT_BUTTON_LONG);
    assert(task_display_get_queue_drop_count() == 0U);
    assert(oled_init_calls == 1U);
    assert(page_packets >= 16U);
    assert(page_packets % 8U == 0U);
    assert(oled_line_calls >= 12U);
    assert(mutex_reads > 10U);
    assert(output_calls > 10U);
    /* After 3 consecutive mutex failures, outputs are in safe state. */
    assert(led_green == 0U && led_red == 1U && buzzer_on == 0U);
    assert(task_display_get_state_read_error_count() >= 3U);
    assert(task_display_get_oled_error_count() == 0U);
    puts("[PASS] IPC handles, periodic button polling, short/long events");
    puts("[PASS] OLED page transfer, alarm EventGroup, fail-safe outputs");
    return 0;
}
