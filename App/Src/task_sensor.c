#include "task_sensor.h"

#include "app_cfg.h"
#include "app_ipc.h"
#include "sensor_dht11.h"
#include "sensor_hcsr04.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdint.h>

#define SENSOR_PERIOD_MS         3000U
#define SENSOR_TASK_STACK_WORDS  256U

static volatile SensorTaskStats_t s_stats = {0};

/*
 * ADC do Leader phu trach.
 * Chua co API chinh thuc nen tam thoi chua gui Queue.
 */
static BaseType_t prvReadPotentiometer(uint16_t *raw)
{
    (void)raw;
    return pdFAIL;
}

/* Doc va dong goi du lieu cam bien */
static BaseType_t prvReadSensors(SensorData_t *data)
{
    if (data == NULL)
    {
        return pdFAIL;
    }

    /* Kiem tra hai cam bien doc lap */
    BaseType_t sensorsOk = pdPASS;

    /* Doc DHT11 */
    if (!sensor_dht11_read(
            &data->temperature_c,
            &data->humidity_percent))
    {
        s_stats.dht11_read_fail++;
        sensorsOk = pdFAIL;
    }

    /* Doc HC-SR04 */
    if (!sensor_hcsr04_read(&data->distance_cm))
    {
        s_stats.hcsr04_read_fail++;
        sensorsOk = pdFAIL;
    }

    /* Khong su dung mau neu co cam bien loi */
    if (sensorsOk != pdPASS)
    {
        return pdFAIL;
    }

    /* Cho Leader thong nhat API ADC */
    if (prvReadPotentiometer(
            &data->potentiometer_raw) != pdPASS)
    {
        s_stats.adc_pending++;
        return pdFAIL;
    }

    return pdPASS;
}

/* FreeRTOS Sensor Task */
static void prvSensorTask(void *argument)
{
    (void)argument;

    BaseType_t hcsr04Ready = pdFALSE;


    for (;;)
    {
        /* Thu khoi tao HC-SR04 neu chua san sang */
        if (hcsr04Ready != pdTRUE)
        {
            hcsr04Ready = sensor_hcsr04_init()
                       ? pdTRUE : pdFALSE;

            if (hcsr04Ready != pdTRUE)
            {
                s_stats.hcsr04_init_fail++;
            }
        }

        if ((hcsr04Ready == pdTRUE) &&
            (xSensorQueue != NULL))
        {
            SensorData_t data = {0};

            if (prvReadSensors(&data) == pdPASS)
            {
                if (xQueueSend(
                        xSensorQueue,
                        &data,
                        0U
                    ) == pdPASS)
                {
                    s_stats.samples_sent++;
                }
                else
                {
                    s_stats.queue_send_fail++;
                }
            }
        }

        vTaskDelay(
            pdMS_TO_TICKS(SENSOR_PERIOD_MS)
        );
    }
}

/* Tao task sau app_ipc_init() */
BaseType_t task_sensor_create(void)
{
    if (xSensorQueue == NULL)
    {
        return pdFAIL;
    }

    return xTaskCreate(
        prvSensorTask,
        "Sensor",
        SENSOR_TASK_STACK_WORDS,
        NULL,
        tskIDLE_PRIORITY + 2U,
        NULL
    );
}

/* Doc thong ke an toan trong Task context */
void task_sensor_get_stats(SensorTaskStats_t *out)
{
    if (out == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();

    out->hcsr04_init_fail = s_stats.hcsr04_init_fail;
    out->dht11_read_fail = s_stats.dht11_read_fail;
    out->hcsr04_read_fail = s_stats.hcsr04_read_fail;
    out->adc_pending = s_stats.adc_pending;
    out->queue_send_fail = s_stats.queue_send_fail;
    out->samples_sent = s_stats.samples_sent;

    taskEXIT_CRITICAL();
}
