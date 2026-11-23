#include "bsp_oled.h"
#include "hal_i2c.h"
#include "hal_spi.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static unsigned calls, data_packets;
static uint8_t fail_data;
static Dev2I2cStatus_t mock_write(uint8_t addr, const uint8_t *data,
                                  size_t n, uint32_t timeout)
{
    assert(addr == 0x3CU && data != NULL && timeout > 0U);
    assert((data[0] == 0x00U && n == 2U) ||
           (data[0] == 0x40U && n == 129U));
    ++calls;
    if (data[0] == 0x40U)
    {
        ++data_packets;
        if (fail_data != 0U) { return DEV2_I2C_TIMEOUT; }
    }
    return DEV2_I2C_OK;
}
int main(void)
{
    puts("=== DEV2: I2C / SSD1306 / SPI STUB ===");
    assert(hal_i2c_write(0x3CU, (const uint8_t *)"x", 1U, 10U)
           == DEV2_I2C_NOT_READY);
    assert(hal_i2c_bind(NULL) == DEV2_I2C_INVALID_ARG);
    assert(hal_i2c_bind(mock_write) == DEV2_I2C_OK);
    assert(bsp_oled_init() == 1U);
    assert(data_packets == 8U);
    bsp_oled_write_line(0U, "Temp: 28.5 C");
    bsp_oled_write_line(9U, "ignored");
    bsp_oled_write_line(0U, NULL);
    assert(bsp_oled_update_page(0U) == 1U);
    assert(data_packets == 9U);
    assert(bsp_oled_update_page(8U) == 0U);
    fail_data = 1U;
    assert(bsp_oled_update_page(1U) == 0U);
    assert(bsp_oled_update_page(2U) == 0U); /* Not ready after failure. */
    fail_data = 0U;
    assert(bsp_oled_init() == 1U); /* Recover via reinit. */
    assert(bsp_oled_update() == 1U);
    assert(hal_spi_init() == DEV2_SPI_NOT_SUPPORTED);
    assert(hal_spi_write(NULL, 1U, 10U) == DEV2_SPI_INVALID_ARG);
    assert(hal_spi_write((const uint8_t *)"x", 1U, 10U)
           == DEV2_SPI_NOT_SUPPORTED);
    printf("[PASS] %u I2C calls, %u data packets; timeout recovery\n",
           calls, data_packets);
    return 0;
}
