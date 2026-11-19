#include "sensor_dht11.h"
#include "hal_gpio.h"
#include "tim.h"
#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#define DHT11_EDGE_TIMEOUT_US  120U
#define DHT11_BIT_ONE_US        48U

/* Doc bo dem TIM4, du kien 1 tick = 1 us */
static uint16_t prvNowUs(void)
{
    return (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
}

/* Tinh thoi gian co xu ly tran bo dem 16 bit */
static uint16_t prvElapsedUs(uint16_t start)
{
    return (uint16_t)(prvNowUs() - start);
}

/* Delay microsecond bang TIM4 */
static void prvDelayUs(uint16_t us)
{
    uint16_t start = prvNowUs();

    while (prvElapsedUs(start) < us)
    {
    }
}

/* Doi chan DATA dat muc yeu cau, co timeout */
static bool prvWaitLevel(uint8_t level, uint16_t timeout_us)
{
    uint16_t start = prvNowUs();

    while (APP_GPIO_DHT11_Read() != level)
    {
        if (prvElapsedUs(start) >= timeout_us)
        {
            return false;
        }
    }

    return true;
}

bool sensor_dht11_read(
    float *temperature_c,
    float *humidity_percent
)
{
    uint8_t bytes[5] = {0};
    bool success = false;

    if ((temperature_c == NULL) ||
        (humidity_percent == NULL))
    {
        return false;
    }

    /*
     * Yeu cau:
     * TIM4 clock 72 MHz, PSC = 71,
     * ARR = 65535, timer dang chay.
     */
    if ((htim4.Instance != TIM4) ||
        (TIM4->PSC != 71U) ||
        (TIM4->ARR != 65535U) ||
        ((TIM4->CR1 & TIM_CR1_CEN) == 0U))
    {
        return false;
    }

    /* MCU gui tin hieu START */
    APP_GPIO_DHT11_Output();
    APP_GPIO_DHT11_Write(0U);

    /* Giu DATA o muc LOW du lau, khong chan CPU */
    vTaskDelay(pdMS_TO_TICKS(30U) + 1U);

    /* Nha duong DATA */
    APP_GPIO_DHT11_Write(1U);
    APP_GPIO_DHT11_Input();

    prvDelayUs(25U);

    /* DHT11 phan hoi LOW - HIGH - LOW */
    if (!prvWaitLevel(0U, DHT11_EDGE_TIMEOUT_US))
        goto finish;

    if (!prvWaitLevel(1U, DHT11_EDGE_TIMEOUT_US))
        goto finish;

    if (!prvWaitLevel(0U, DHT11_EDGE_TIMEOUT_US))
        goto finish;

    /* Doc 40 bit, MSB truoc */
    for (uint32_t i = 0; i < 40U; i++)
    {
        if (!prvWaitLevel(1U, DHT11_EDGE_TIMEOUT_US))
            goto finish;

        uint16_t high_start = prvNowUs();

        if (!prvWaitLevel(0U, DHT11_EDGE_TIMEOUT_US))
            goto finish;

        uint16_t high_us = prvElapsedUs(high_start);

        if ((high_us < 10U) || (high_us > 100U))
            goto finish;

        uint8_t bit = (high_us > DHT11_BIT_ONE_US)
                    ? 1U : 0U;

        uint32_t index = i / 8U;

        bytes[index] =
            (uint8_t)((bytes[index] << 1U) | bit);
    }

    /* Kiem tra checksum */
    uint8_t checksum =
        (uint8_t)(bytes[0] + bytes[1] +
                  bytes[2] + bytes[3]);

    if (checksum != bytes[4])
        goto finish;

    /* Chi cap nhat output khi du lieu hop le */
    *humidity_percent =
        (float)bytes[0] + (float)bytes[1] / 10.0f;

    *temperature_c =
        (float)bytes[2] + (float)bytes[3] / 10.0f;

    success = true;

finish:
    /* Nha bus DATA sau moi lan doc */
    APP_GPIO_DHT11_Output();
    APP_GPIO_DHT11_Write(1U);

    return success;
}
