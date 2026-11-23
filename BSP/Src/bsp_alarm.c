
#include "bsp_alarm.h"

/*
 * DEV 2 - ALARM LOGIC
 *
 * Khong phu thuoc STM32 HAL.
 * Co the test bang GCC tren Windows.
 */

BspAlarmOutput_t bsp_alarm_evaluate(
    uint32_t alarm_flags)
{
    BspAlarmOutput_t output = {0};

    if (alarm_flags == 0U)
    {
        /* Trang thai binh thuong */
        output.led_green = 1U;
        output.led_red = 0U;
        output.buzzer = 0U;
    }
    else
    {
        /* Trang thai canh bao */
        output.led_green = 0U;
        output.led_red = 1U;
        output.buzzer = 1U;
    }

    return output;
}
