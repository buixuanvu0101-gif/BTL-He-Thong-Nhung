
#include "bsp_oled.h"
#include "hal_i2c.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * DEV 2 - OLED SSD1306 TEST
 *
 * Kiem tra:
 * 1. Khoi tao OLED
 * 2. Truyen lenh I2C
 * 3. Ghi chuoi vao framebuffer
 * 4. Gui du lieu ra OLED
 */

#define OLED_TEST_ADDRESS 0x3CU
#define OLED_TEST_DATA_SIZE 129U

static uint32_t total_transfers = 0U;
static uint32_t command_transfers = 0U;
static uint32_t data_transfers = 0U;
static uint8_t saw_display_off = 0U;
static uint8_t saw_display_on = 0U;
static uint8_t saw_nonzero_pixel = 0U;

static Dev2I2cStatus_t mock_oled_write(
    uint8_t address,
    const uint8_t *data,
    size_t length,
    uint32_t timeout_ms)
{
    assert(address == OLED_TEST_ADDRESS);
    assert(data != NULL);
    assert(length >= 2U);
    assert(timeout_ms > 0U);

    total_transfers++;

    if (data[0] == 0x00U)
    {
        command_transfers++;

        assert(length == 2U);

        if (data[1] == 0xAEU)
        {
            saw_display_off = 1U;
        }

        if (data[1] == 0xAFU)
        {
            saw_display_on = 1U;
        }
    }
    else if (data[0] == 0x40U)
    {
        data_transfers++;

        assert(length == OLED_TEST_DATA_SIZE);

        for (size_t i = 1U; i < length; i++)
        {
            if (data[i] != 0U)
            {
                saw_nonzero_pixel = 1U;
            }
        }
    }
    else
    {
        assert(0 && "Invalid OLED control byte");
    }

    return DEV2_I2C_OK;
}

int main(void)
{
    puts("=== DEV 2 OLED TEST ===");

    assert(
        hal_i2c_bind(mock_oled_write)
        == DEV2_I2C_OK
    );

    /* Kiem tra khoi tao */
    assert(bsp_oled_init() == 1U);
    assert(saw_display_off == 1U);
    assert(saw_display_on == 1U);

    puts("[PASS] OLED initialization");

    /* Kiem tra framebuffer */
    bsp_oled_clear();

    bsp_oled_write_line(0U, "TEMP: 28.5 C");
    bsp_oled_write_line(1U, "HUMI: 65.0 %");
    bsp_oled_write_line(2U, "DIST: 15.2 CM");
    bsp_oled_write_line(3U, "LIMIT: 20.0 CM");
    bsp_oled_write_line(4U, "SOURCE: POT");
    bsp_oled_write_line(5U, "STATUS: ALARM");

    bsp_oled_update();

    assert(saw_nonzero_pixel == 1U);
    assert(data_transfers == 16U);

    puts("[PASS] OLED framebuffer transfer");

    /* Kiem tra xoa man hinh */
    saw_nonzero_pixel = 0U;

    bsp_oled_clear();
    bsp_oled_update();

    assert(saw_nonzero_pixel == 0U);
    assert(data_transfers == 24U);

    puts("[PASS] OLED clear");

    assert(total_transfers > 0U);
    assert(command_transfers > 0U);

    puts("All OLED tests passed!");

    return 0;
}
