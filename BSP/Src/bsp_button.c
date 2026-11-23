#include "bsp_button.h"
#include "hal_gpio.h"

/* Reuse Dev 1's PB10 active-low button mapping.
 * Do not read PB15: that pin is not configured as a button in .ioc.
 * PB11 is another input and requires a separately agreed UI mapping.
 */
uint8_t bsp_button_is_pressed(void)
{
    return APP_GPIO_Button_Read() ? 1U : 0U;
}
