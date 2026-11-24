#include "bsp_led_adapter.h"
#include "hal_gpio.h"
#include "stm32f1xx_hal.h"

#ifndef DEV2_RED_LED_ACTIVE_LOW
#define DEV2_RED_LED_ACTIVE_LOW 0U
#endif

void bsp_led_write(uint8_t green_on, uint8_t red_on)
{
    if (green_on != 0U)
    {
        APP_GPIO_LED_On(); /* PC13 is active-low on Blue Pill. */
    }
    else
    {
        APP_GPIO_LED_Off();
    }

    /* PC14 output already exists in the team's CubeMX pinout.
     * Only use a low-current external LED and verify wiring/polarity.
     */
    GPIO_PinState red_level;
    if (DEV2_RED_LED_ACTIVE_LOW != 0U)
    {
        red_level = (red_on != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    }
    else
    {
        red_level = (red_on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    }
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, red_level);
}
