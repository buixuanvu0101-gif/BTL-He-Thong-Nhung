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

#define SENSOR_PERIOD_MS         3000U
#define SENSOR_TASK_STACK_WORDS  256U

static volatile SensorTaskStats_t s_stats = {0};

/* Sensor Task doc DHT11 va HC-SR04. */
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

    /* Encoder xu ly nguong rieng; khong doc ADC tai day. */

    return pdPASS;
}

/* FreeRTOS Sensor Task */
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

static void prvSensorTask(void *argument)
{
    (void)argument;

    BaseType_t hcsr04Ready = pdFALSE;


    for (;;)
    {
        if (prvEnsureTim4Running() != pdPASS)
        {
            s_stats.tim4_start_fail++;
            vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
            continue;
        }

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
    out->tim4_start_fail = s_stats.tim4_start_fail;
    out->queue_send_fail = s_stats.queue_send_fail;
    out->samples_sent = s_stats.samples_sent;

    taskEXIT_CRITICAL();
}
