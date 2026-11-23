
#include "hal_spi.h"

#include <stddef.h>

/*
 * DEV 2 - SPI STUB
 *
 * Hien tai khong co thiet bi SPI
 * trong he thong giam sat moi truong.
 *
 * Khong truy cap phan cung.
 */

Dev2SpiStatus_t hal_spi_init(void)
{
    return DEV2_SPI_NOT_SUPPORTED;
}

Dev2SpiStatus_t hal_spi_write(
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms)
{
    if ((data == NULL) ||
        (length == 0U) ||
        (timeout_ms == 0U))
    {
        return DEV2_SPI_INVALID_ARG;
    }

    return DEV2_SPI_NOT_SUPPORTED;
}
