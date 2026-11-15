
#ifndef DEV2_HAL_I2C_H
#define DEV2_HAL_I2C_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    DEV2_I2C_OK = 0,
    DEV2_I2C_ERROR,
    DEV2_I2C_TIMEOUT,
    DEV2_I2C_NOT_READY,
    DEV2_I2C_INVALID_ARG
} Dev2I2cStatus_t;

/* Backend nhan dia chi I2C 7-bit */
typedef Dev2I2cStatus_t (*Dev2I2cWriteFn_t)(
    uint8_t address_7bit,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms
);

/* Dang ky backend trong giai doan khoi tao */
Dev2I2cStatus_t hal_i2c_bind(
    Dev2I2cWriteFn_t write_fn
);

/* Gui du lieu, timeout tinh bang mili giay */
Dev2I2cStatus_t hal_i2c_write(
    uint8_t address_7bit,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms
);

#endif
