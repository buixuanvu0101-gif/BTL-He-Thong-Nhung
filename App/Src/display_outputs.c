
#include "display_outputs.h"

#include <stddef.h>

/*
 * DEV 2 - OUTPUT ADAPTER
 *
 * Ham GPIO/PWM thuc te se duoc
 * Dev 1 va Dev 3 cung cap sau.
 *
 * Chi bind truoc khi task chay.
 */

static DisplayLedWriteFn_t s_led_write = NULL;

static DisplayBuzzerWriteFn_t
    s_buzzer_write = NULL;

void display_outputs_bind_led(
    DisplayLedWriteFn_t write_fn)
{
    s_led_write = write_fn;
}

void display_outputs_bind_buzzer(
    DisplayBuzzerWriteFn_t write_fn)
{
    s_buzzer_write = write_fn;
}

void display_outputs_apply(
    uint8_t green_on,
    uint8_t red_on,
    uint8_t buzzer_on)
{
    green_on = (green_on != 0U) ? 1U : 0U;
    red_on = (red_on != 0U) ? 1U : 0U;
    buzzer_on = (buzzer_on != 0U) ? 1U : 0U;

    if (s_led_write != NULL)
    {
        s_led_write(green_on, red_on);
    }

    if (s_buzzer_write != NULL)
    {
        s_buzzer_write(buzzer_on);
    }
}
