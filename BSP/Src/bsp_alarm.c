
#include "bsp_alarm.h"

#include <stddef.h>

/* ===================================
 * XAC DINH TRANG THAI CANH BAO
 * =================================== */

BspAlarmOutput_t bsp_alarm_evaluate(
    uint32_t alarm_flags)
{
    BspAlarmOutput_t output = {0};

    if (alarm_flags == 0U)
    {
        /* He thong binh thuong */
        output.led_green = 1U;
        output.led_red = 0U;
        output.buzzer = 0U;
    }
    else
    {
        /* He thong co canh bao */
        output.led_green = 0U;
        output.led_red = 1U;
        output.buzzer = 1U;
    }

    return output;
}

/* ===================================
 * CAP NHAT DAU RA PHAN CUNG
 * =================================== */

void bsp_alarm_apply(
    const BspAlarmOutput_t *output)
{
    if (output == NULL)
    {
        return;
    }

    /*
     * TODO: Tich hop STM32 HAL
     *
     * HAL_GPIO_WritePin() cho:
     * - LED xanh
     * - LED do
     * - Buzzer
     *
     * Hien tai chua dieu khien GPIO that.
     */
}
