
#include "task_sensor.h"

#include "app_cfg.h"
#include "app_ipc.h"
#include "sensor_dht11.h"
#include "sensor_hcsr04.h"
#include "tim.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdint.h>

#define HCSR04_PERIOD_MS         100U
#define DHT11_PERIOD_MS         2000U
#define SENSOR_TASK_STACK_WORDS  256U

static volatile SensorTaskStats_t s_stats = {0};

/* Kiem tra va khoi dong TIM4 */
static BaseType_t prvEnsureTim4Running(void)
{
    if ((htim4.Instance != TIM4) ||
        (htim4.Init.Prescaler != 71U) ||
        (htim4.Init.Period != 65535U))
    {
        return pdFAIL;
    }

    if ((TIM4->CR1 & TIM_CR1_CEN) != 0U)
    {
        return pdPASS;
    }

    return (HAL_TIM_Base_Start(&htim4) == HAL_OK)
           ? pdPASS : pdFAIL;
}

/* Sensor Task FreeRTOS */
static void prvSensorTask(void *argument)
{
    (void)argument;

    const TickType_t sensorPeriod =
        pdMS_TO_TICKS(HCSR04_PERIOD_MS);

    const TickType_t dhtPeriod =
        pdMS_TO_TICKS(DHT11_PERIOD_MS);

    TickType_t nextWake = xTaskGetTickCount();
    TickType_t lastDhtTick = 0U;

    BaseType_t firstDhtRead = pdTRUE;
    BaseType_t dhtValid = pdFALSE;
    BaseType_t hcsr04Ready = pdFALSE;

    SensorData_t data = {0};

    for (;;)
    {
        /* 1. Dam bao TIM4 da hoat dong */
        if (prvEnsureTim4Running() != pdPASS)
        {
            s_stats.tim4_start_fail++;

            vTaskDelay(sensorPeriod);
            nextWake = xTaskGetTickCount();
            continue;
        }

        /* 2. Doc DHT11 moi 2000 ms */
        TickType_t now = xTaskGetTickCount();

        if ((firstDhtRead == pdTRUE) ||
            ((TickType_t)(now - lastDhtTick) >= dhtPeriod))
        {
            firstDhtRead = pdFALSE;
            lastDhtTick = now;

            if (sensor_dht11_read(
                    &data.temperature_c,
                    &data.humidity_percent))
            {
                dhtValid = pdTRUE;
            }
            else
            {
                dhtValid = pdFALSE;
                s_stats.dht11_read_fail++;
            }
        }

        /* 3. Khoi tao HC-SR04 neu can */
        if (hcsr04Ready != pdTRUE)
        {
            hcsr04Ready = sensor_hcsr04_init()
                         ? pdTRUE : pdFALSE;

            if (hcsr04Ready != pdTRUE)
            {
                s_stats.hcsr04_init_fail++;
            }
        }

        /* 4. Doc HC-SR04 moi chu ky */
        BaseType_t distanceValid = pdFALSE;

        if (hcsr04Ready == pdTRUE)
        {
            if (sensor_hcsr04_read(&data.distance_cm))
            {
                distanceValid = pdTRUE;
            }
            else
            {
                s_stats.hcsr04_read_fail++;
            }
        }

        /* 5. Chi gui Queue khi du lieu hop le */
        if ((dhtValid == pdTRUE) &&
            (distanceValid == pdTRUE) &&
            (xSensorQueue != NULL))
        {
            if (xQueueSend(xSensorQueue, &data, 0U) == pdPASS)
            {
                s_stats.samples_sent++;
            }
            else
            {
                s_stats.queue_send_fail++;
            }
        }

        /*
         * 6. Canh chu ky 100 ms.
         * Dung vTaskDelay vi Leader dang tat
         * INCLUDE_vTaskDelayUntil.
         */
        nextWake += sensorPeriod;

        TickType_t endTick = xTaskGetTickCount();
        TickType_t remaining =
            (TickType_t)(nextWake - endTick);

        if ((remaining > 0U) &&
            (remaining <= sensorPeriod))
        {
            vTaskDelay(remaining);
        }
        else
        {
            /* Chu ky bi qua han */
            nextWake = endTick;
        }
    }
}

/* Tao Sensor Task */
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

/* Doc thong ke loi tu Task khac */
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
    out->tim4_start_fail = s_stats.tim4_start_fail;
    out->queue_send_fail = s_stats.queue_send_fail;
    out->samples_sent = s_stats.samples_sent;

    taskEXIT_CRITICAL();
}
