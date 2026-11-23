
#include "hal_i2c.h"

static Dev2I2cWriteFn_t s_write_fn = 0;

Dev2I2cStatus_t hal_i2c_bind(
    Dev2I2cWriteFn_t write_fn)
{
    if (write_fn == 0)
    {
        return DEV2_I2C_INVALID_ARG;
    }

    s_write_fn = write_fn;

    return DEV2_I2C_OK;
}

Dev2I2cStatus_t hal_i2c_write(
    uint8_t address_7bit,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms)
{
    if (address_7bit > 0x7FU ||
        data == 0 ||
        length == 0U ||
        timeout_ms == 0U)
    {
        return DEV2_I2C_INVALID_ARG;
    }

    if (s_write_fn == 0)
    {
        return DEV2_I2C_NOT_READY;
    }

    return s_write_fn(
        address_7bit,
        data,
        length,
        timeout_ms
    );
}
