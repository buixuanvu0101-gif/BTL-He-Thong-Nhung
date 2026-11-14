
#ifndef BSP_OLED_H
#define BSP_OLED_H

#include <stdint.h>

/*
 * OLED SSD1306
 * Giao tiep du kien: I2C
 * Kich thuoc du kien: 128x64
 */

#define OLED_WIDTH       128U
#define OLED_HEIGHT       64U
#define OLED_MAX_LINES     8U

/* Khoi tao man hinh OLED */
uint8_t bsp_oled_init(void);

/* Xoa bo dem man hinh */
void bsp_oled_clear(void);

/* Ghi chuoi vao dong chi dinh */
void bsp_oled_write_line(
    uint8_t row,
    const char *text
);

/* Cap nhat bo dem ra man hinh */
void bsp_oled_update(void);

#endif
