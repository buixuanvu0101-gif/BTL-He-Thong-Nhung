
#include "hal_i2c_stm32.h"

#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <stddef.h>
#include <stdint.h>

/*
 * DEV 2 - STM32F103C8T6 I2C1 BACKEND
 *
 * I2C1 do CubeMX khoi tao.
 * Mutex bao ve cac lan truyen qua
 * backend nay giua nhieu task.
 *
 * Can goi hal_i2c_stm32_init()
 * truoc khi su dung OLED.
 */

extern I2C_HandleTypeDef hi2c1;

static SemaphoreHandle_t s_i2c_mutex = NULL;

/* ==========================================
 * STM32 I2C WRITE BACKEND
 * ========================================== */

static Dev2I2cStatus_t prvStm32I2cWrite(
    uint8_t address_7bit,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms)
{
    if ((data == NULL) ||
        (length == 0U) ||
        (length > UINT16_MAX) ||
        (address_7bit > 0x7FU) ||
        (timeout_ms == 0U))
    {
        return DEV2_I2C_INVALID_ARG;
    }

    if (s_i2c_mutex == NULL)
    {
        return DEV2_I2C_NOT_READY;
    }

    /*
     * Chi su dung backend nay trong
     * task context, khong goi tu ISR.
     */
    TickType_t wait_ticks =
        pdMS_TO_TICKS(timeout_ms);

    if (wait_ticks == 0U)
    {
        wait_ticks = 1U;
    }

    if (xSemaphoreTake(
            s_i2c_mutex,
            wait_ticks) != pdTRUE)
    {
        return DEV2_I2C_TIMEOUT;
    }

    /*
     * STM32 HAL yeu cau dia chi
     * 7-bit duoc dich trai 1 bit.
     *
     * SSD1306: 0x3C -> 0x78.
     */
    uint16_t device_address =
        (uint16_t)address_7bit << 1U;

    HAL_StatusTypeDef result =
        HAL_I2C_Master_Transmit(
            &hi2c1,
            device_address,
            (uint8_t *)data,
            (uint16_t)length,
            timeout_ms
        );

    xSemaphoreGive(s_i2c_mutex);

    if (result == HAL_OK)
    {
        return DEV2_I2C_OK;
    }

    if (result == HAL_TIMEOUT)
    {
        return DEV2_I2C_TIMEOUT;
    }

    return DEV2_I2C_ERROR;
}

/* ==========================================
 * INITIALIZE BACKEND
 * ========================================== */

Dev2I2cStatus_t hal_i2c_stm32_init(void)
{
    if (s_i2c_mutex == NULL)
    {
        s_i2c_mutex = xSemaphoreCreateMutex();

        if (s_i2c_mutex == NULL)
        {
            return DEV2_I2C_ERROR;
        }
    }

    return hal_i2c_bind(prvStm32I2cWrite);
}
