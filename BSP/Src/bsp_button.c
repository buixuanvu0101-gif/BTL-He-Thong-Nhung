
#include "bsp_button.h"

/*
 * Ban mock tam thoi.
 * Khi co STM32 HAL, thay bang HAL_GPIO_ReadPin().
 *
 * Gia tri 0: nut chua nhan.
 */
uint8_t bsp_button_is_pressed(void)
{
    return 0U;
}
