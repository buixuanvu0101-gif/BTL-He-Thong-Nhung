
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "app_cfg.h"
#include "app_ipc.h"
#include "task_display.h"
#include "display_outputs.h"
#include "hal_i2c.h"

#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * DEV 2 - TASK DISPLAY INTEGRATION MOCK
 *
 * Mo phong:
 * - Tao FreeRTOS task
 * - Mutex bao ve SystemState
 * - EventGroup canh bao
 * - I2C OLED
 * - Callback LED / Buzzer
 *
 * Khong su dung STM32 that.
 */

/* Bien chung duoc khai bao trong app_ipc.h */
QueueHandle_t xSensorQueue = NULL;
QueueHandle_t xUiEventQueue = NULL;
SemaphoreHandle_t xSystemStateMutex = NULL;
EventGroupHandle_t xAlarmEventGroup = NULL;

SystemState_t gSystemState = {0};

/* Trang thai mock */
static int fake_mutex_object;
static int fake_event_object;
static int fake_task_object;

static TickType_t fake_tick = 0U;
static EventBits_t fake_alarm_bits = 0U;

static void (*created_task)(void *) = NULL;

static uint32_t task_loops = 0U;
static uint32_t mutex_reads = 0U;
static uint32_t oled_data_packets = 0U;
static uint32_t led_calls = 0U;
static uint32_t buzzer_calls = 0U;

static uint8_t green_state = 0U;
static uint8_t red_state = 0U;
static uint8_t buzzer_state = 0U;

static jmp_buf test_exit;

/* =====================================
 * FREERTOS TASK MOCK
 * ===================================== */

BaseType_t xTaskCreate(
    void (*task_fn)(void *),
    const char *name,
    uint32_t stack_depth,
    void *parameters,
    UBaseType_t priority,
    TaskHandle_t *handle)
{
    (void)parameters;
    (void)priority;

    assert(task_fn != NULL);
    assert(name != NULL);
    assert(stack_depth >= 256U);
    assert(handle != NULL);

    created_task = task_fn;
    *handle = &fake_task_object;

    return pdPASS;
}

TickType_t xTaskGetTickCount(void)
{
    return fake_tick;
}

void vTaskDelayUntil(
    TickType_t *previous_wake,
    TickType_t increment)
{
    assert(previous_wake != NULL);
    assert(increment > 0U);

    *previous_wake += increment;
    fake_tick = *previous_wake;

    task_loops++;

    /*
     * Lan 1: Normal
     * Lan 2: Normal, du lieu khong doi
     * Lan 3: Alarm
     */
    if (task_loops == 2U)
    {
        fake_alarm_bits = ALARM_DISTANCE;
    }

    if (task_loops >= 3U)
    {
        longjmp(test_exit, 1);
    }
}

/* =====================================
 * MUTEX MOCK
 * ===================================== */

BaseType_t xSemaphoreTake(
    SemaphoreHandle_t handle,
    TickType_t timeout)
{
    assert(handle == xSystemStateMutex);
    assert(timeout > 0U);

    mutex_reads++;

    return pdTRUE;
}

BaseType_t xSemaphoreGive(
    SemaphoreHandle_t handle)
{
    assert(handle == xSystemStateMutex);
    return pdTRUE;
}

/* =====================================
 * EVENT GROUP MOCK
 * ===================================== */

EventBits_t xEventGroupGetBits(
    EventGroupHandle_t group)
{
    assert(group == xAlarmEventGroup);
    return fake_alarm_bits;
}

/* =====================================
 * I2C MOCK
 * ===================================== */

static Dev2I2cStatus_t mock_i2c_write(
    uint8_t address,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms)
{
    assert(address == 0x3CU);
    assert(data != NULL);
    assert(length >= 2U);
    assert(timeout_ms > 0U);

    if (data[0] == 0x00U)
    {
        assert(length == 2U);
    }
    else if (data[0] == 0x40U)
    {
        assert(length == 129U);
        oled_data_packets++;
    }
    else
    {
        assert(0 && "Unexpected OLED packet");
    }

    return DEV2_I2C_OK;
}

/* =====================================
 * OUTPUT MOCK
 * ===================================== */

static void mock_led_write(
    uint8_t green,
    uint8_t red)
{
    green_state = green;
    red_state = red;
    led_calls++;
}

static void mock_buzzer_write(
    uint8_t on)
{
    buzzer_state = on;
    buzzer_calls++;
}

/* =====================================
 * MAIN TEST
 * ===================================== */

int main(void)
{
    puts("=== DEV 2 DISPLAY TASK TEST ===");

    /* Khoi tao IPC gia lap */
    xSystemStateMutex = &fake_mutex_object;
    xAlarmEventGroup = &fake_event_object;

    /* Du lieu cam bien gia lap */
    gSystemState.sensor.temperature_c = 28.5f;
    gSystemState.sensor.humidity_percent = 65.0f;
    gSystemState.sensor.distance_cm = 15.2f;
    gSystemState.sensor.potentiometer_raw = 2048U;

    gSystemState.distance_threshold_cm = 20.0f;
    gSystemState.threshold_source =
        THRESHOLD_SOURCE_POT;

    /* Dang ky I2C backend mock */
    assert(
        hal_i2c_bind(mock_i2c_write)
        == DEV2_I2C_OK
    );

    /* Dang ky output mock */
    display_outputs_bind_led(mock_led_write);
    display_outputs_bind_buzzer(mock_buzzer_write);

    /* Kiem tra tao task */
    assert(task_display_create() == pdPASS);
    assert(created_task != NULL);
    assert(task_display_get_handle() != NULL);

    puts("[PASS] Task creation");

    /*
     * Chay task gia lap.
     * Dung longjmp sau 3 vong lap de
     * thoat khoi FreeRTOS task vo han.
     */
    if (setjmp(test_exit) == 0)
    {
        created_task(NULL);

        /* Task khong duoc tu ket thuc */
        assert(0 && "Task returned unexpectedly");
    }

    assert(task_loops == 3U);
    assert(mutex_reads == 3U);

    puts("[PASS] Mutex and task scheduling");

    /*
     * OLED:
     * 8 goi khi init (clear)
     * 8 goi o vong 1 (Normal)
     * 0 goi o vong 2 (khong doi)
     * 8 goi o vong 3 (Alarm)
     */
    assert(oled_data_packets == 24U);

    puts("[PASS] OLED update on change");

    /* Trang thai cuoi la Alarm */
    assert(fake_alarm_bits == ALARM_DISTANCE);

    assert(green_state == 0U);
    assert(red_state == 1U);
    assert(buzzer_state == 1U);

    assert(led_calls == 3U);
    assert(buzzer_calls == 3U);

    puts("[PASS] Alarm EventGroup and outputs");

    puts("All display task tests passed!");

    return 0;
}
