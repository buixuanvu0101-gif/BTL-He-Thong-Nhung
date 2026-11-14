
#ifndef BSP_OLED_H
#define BSP_OLED_H

#include <stdint.h>

/* OLED SSD1306 I2C 128x64 */
#define OLED_WIDTH       128U
#define OLED_HEIGHT       64U
#define OLED_MAX_LINES     8U

#define OLED_I2C_ADDRESS  0x3CU

/* 1: thanh cong, 0: that bai */
uint8_t bsp_oled_init(void);

/* Xoa bo dem OLED trong RAM */
void bsp_oled_clear(void);

/* Ghi chuoi vao dong 0..7 */
void bsp_oled_write_line(
    uint8_t row,
    const char *text
);

/* Gui bo dem RAM ra OLED */
void bsp_oled_update(void);

#endif
