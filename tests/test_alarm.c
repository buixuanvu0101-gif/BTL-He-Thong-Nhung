
#include <stdio.h>
#include <assert.h>

#include "bsp_alarm.h"

/* Test trang thai binh thuong */
static void test_normal(void)
{
    BspAlarmOutput_t output =
        bsp_alarm_evaluate(0U);

    assert(output.led_green == 1U);
    assert(output.led_red == 0U);
    assert(output.buzzer == 0U);

    printf("[PASS] Normal state\n");
}

/* Test trang thai canh bao */
static void test_alarm(void)
{
    BspAlarmOutput_t output =
        bsp_alarm_evaluate(1U);

    assert(output.led_green == 0U);
    assert(output.led_red == 1U);
    assert(output.buzzer == 1U);

    printf("[PASS] Alarm state\n");
}

/* Test nhieu bit canh bao */
static void test_multiple_alarm_bits(void)
{
    BspAlarmOutput_t output =
        bsp_alarm_evaluate(0x05U);

    assert(output.led_green == 0U);
    assert(output.led_red == 1U);
    assert(output.buzzer == 1U);

    printf("[PASS] Multiple alarm bits\n");
}

int main(void)
{
    printf("=== DEV 2 ALARM TEST ===\n");

    test_normal();
    test_alarm();
    test_multiple_alarm_bits();

    printf("All tests passed!\n");

    return 0;
}
