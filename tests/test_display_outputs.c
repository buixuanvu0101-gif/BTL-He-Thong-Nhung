
#include "display_outputs.h"
#include "bsp_alarm.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t green_state = 0U;
static uint8_t red_state = 0U;
static uint8_t buzzer_state = 0U;

static uint32_t led_calls = 0U;
static uint32_t buzzer_calls = 0U;

/* Mock LED cua Dev 1 */
static void mock_led_write(
    uint8_t green_on,
    uint8_t red_on)
{
    green_state = green_on;
    red_state = red_on;
    led_calls++;
}

/* Mock PWM Buzzer cua Dev 3 */
static void mock_buzzer_write(
    uint8_t buzzer_on)
{
    buzzer_state = buzzer_on;
    buzzer_calls++;
}

static void apply_alarm(uint32_t flags)
{
    BspAlarmOutput_t output =
        bsp_alarm_evaluate(flags);

    display_outputs_apply(
        output.led_green,
        output.led_red,
        output.buzzer
    );
}

int main(void)
{
    puts("=== DEV 2 OUTPUT TEST ===");

    display_outputs_bind_led(mock_led_write);
    display_outputs_bind_buzzer(mock_buzzer_write);

    /* Binh thuong */
    apply_alarm(0U);

    assert(green_state == 1U);
    assert(red_state == 0U);
    assert(buzzer_state == 0U);

    puts("[PASS] Normal outputs");

    /* Co canh bao */
    apply_alarm(1U);

    assert(green_state == 0U);
    assert(red_state == 1U);
    assert(buzzer_state == 1U);

    puts("[PASS] Alarm outputs");

    /* Loi du lieu */
    display_outputs_apply(0U, 1U, 0U);

    assert(green_state == 0U);
    assert(red_state == 1U);
    assert(buzzer_state == 0U);

    puts("[PASS] Data error outputs");

    assert(led_calls == 3U);
    assert(buzzer_calls == 3U);

    puts("[PASS] Callback invocation");

    puts("All display output tests passed!");

    return 0;
}
