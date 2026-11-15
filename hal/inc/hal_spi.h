
#ifndef DEV2_HAL_SPI_H
#define DEV2_HAL_SPI_H

#include <stddef.h>
#include <stdint.h>

/*
 * DEV 2 - SPI INTERFACE
 *
 * Du an hien tai dung OLED I2C.
 * Chua co thiet bi SPI.
 *
 * File nay chi tao khung HAL SPI.
 */

typedef enum
{
    DEV2_SPI_OK = 0,
    DEV2_SPI_ERROR,
    DEV2_SPI_TIMEOUT,
    DEV2_SPI_NOT_SUPPORTED,
    DEV2_SPI_INVALID_ARG
} Dev2SpiStatus_t;

/*
 * Khoi tao SPI.
 * Hien tai chua duoc ho tro.
 */
Dev2SpiStatus_t hal_spi_init(void);

/*
 * Truyen du lieu SPI.
 */
Dev2SpiStatus_t hal_spi_write(
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms
);

#endif
