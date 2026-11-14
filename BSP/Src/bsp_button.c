
#include "bsp_button.h"
#include "stm32f1xx_hal.h"

/* Nut nhan du kien ket noi PB15 */
#define BUTTON_PORT GPIOB
#define BUTTON_PIN  GPIO_PIN_15

/*
 * Nut dung pull-up:
 * LOW  = dang nhan
 * HIGH = da nha
 */
uint8_t bsp_button_is_pressed(void)
{
    GPIO_PinState state = HAL_GPIO_ReadPin(
        BUTTON_PORT,
        BUTTON_PIN
    );

    return (state == GPIO_PIN_RESET)
        ? 1U : 0U;
}
