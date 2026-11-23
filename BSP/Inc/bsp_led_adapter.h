#ifndef BSP_LED_ADAPTER_H
#define BSP_LED_ADAPTER_H

#include <stdint.h>

/* Compatible with display_outputs_bind_led().
 * PC13: Dev 1's active-low green LED.
 * PC14: proposed active-high red LED, confirm physical wiring.
 */
void bsp_led_write(uint8_t green_on, uint8_t red_on);

#endif
