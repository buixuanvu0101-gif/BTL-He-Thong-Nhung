
#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include <stdint.h>

/*
 * Tra ve:
 * 1 = nut dang duoc nhan
 * 0 = nut duoc nha
 *
 * Ham nay se ket noi GPIO khi tich hop STM32.
 */
uint8_t bsp_button_is_pressed(void);

#endif
