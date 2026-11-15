
#include "hal_i2c.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Mock backend gia lap I2C */
static uint32_t call_count = 0U;
static uint8_t last_address = 0U;
static uint8_t last_data[16];
static size_t last_length = 0U;
static uint32_t last_timeout = 0U;

static Dev2I2cStatus_t mock_write(
    uint8_t address,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms)
{
    if ((data == NULL) ||
        (length > sizeof(last_data)))
    {
        return DEV2_I2C_ERROR;
    }

    call_count++;
    last_address = address;
    last_length = length;
    last_timeout = timeout_ms;

    memcpy(last_data, data, length);

    return DEV2_I2C_OK;
}

static Dev2I2cStatus_t mock_timeout(
    uint8_t address,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms)
{
    (void)address;
    (void)data;
    (void)length;
    (void)timeout_ms;

    return DEV2_I2C_TIMEOUT;
}

int main(void)
{
    const uint8_t oled_command[] =
    {
        0x00U,
        0xAEU
    };

    puts("=== DEV 2 HAL I2C TEST ===");

    /* Chua dang ky backend */
    assert(
        hal_i2c_write(
            0x3CU,
            oled_command,
            sizeof(oled_command),
            50U
        ) == DEV2_I2C_NOT_READY
    );

    puts("[PASS] Backend not ready");

    /* Kiem tra tham so sai */
    assert(
        hal_i2c_bind(NULL)
        == DEV2_I2C_INVALID_ARG
    );

    assert(
        hal_i2c_write(
            0x80U,
            oled_command,
            sizeof(oled_command),
            50U
        ) == DEV2_I2C_INVALID_ARG
    );

    assert(
        hal_i2c_write(
            0x3CU,
            NULL,
            2U,
            50U
        ) == DEV2_I2C_INVALID_ARG
    );

    assert(
        hal_i2c_write(
            0x3CU,
            oled_command,
            sizeof(oled_command),
            0U
        ) == DEV2_I2C_INVALID_ARG
    );

    puts("[PASS] Invalid arguments");

    /* Dang ky backend */
    assert(
        hal_i2c_bind(mock_write)
        == DEV2_I2C_OK
    );

    /* Truyen lenh OLED */
    assert(
        hal_i2c_write(
            0x3CU,
            oled_command,
            sizeof(oled_command),
            50U
        ) == DEV2_I2C_OK
    );

    assert(call_count == 1U);
    assert(last_address == 0x3CU);
    assert(last_length == sizeof(oled_command));
    assert(last_timeout == 50U);

    assert(
        memcmp(
            last_data,
            oled_command,
            sizeof(oled_command)
        ) == 0
    );

    puts("[PASS] OLED I2C transfer");

    /* Gia lap timeout */
    assert(
        hal_i2c_bind(mock_timeout)
        == DEV2_I2C_OK
    );

    assert(
        hal_i2c_write(
            0x3CU,
            oled_command,
            sizeof(oled_command),
            50U
        ) == DEV2_I2C_TIMEOUT
    );

    puts("[PASS] Timeout propagation");

    puts("All HAL I2C tests passed!");

    return 0;
}
