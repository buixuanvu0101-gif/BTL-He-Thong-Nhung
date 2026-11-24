
#ifndef DISPLAY_OUTPUTS_H
#define DISPLAY_OUTPUTS_H

#include <stdint.h>

/*
 * DEV 2 - OUTPUT ADAPTER
 *
 * Ket noi voi:
 * - LED GPIO cua Dev 1
 * - Buzzer PWM cua Dev 3
 *
 * Dang ky cac callback truoc khi
 * FreeRTOS scheduler bat dau chay.
 */

typedef void (*DisplayLedWriteFn_t)(
    uint8_t green_on,
    uint8_t red_on
);

typedef void (*DisplayBuzzerWriteFn_t)(
    uint8_t buzzer_on
);

/* Gan ham dieu khien LED cua Dev 1 */
void display_outputs_bind_led(
    DisplayLedWriteFn_t write_fn
);

/* Gan ham dieu khien Buzzer cua Dev 3 */
void display_outputs_bind_buzzer(
    DisplayBuzzerWriteFn_t write_fn
);

/*
 * Gui trang thai ra thiet bi.
 * Chua bind thi khong truy cap GPIO/PWM.
 */
void display_outputs_apply(
    uint8_t green_on,
    uint8_t red_on,
    uint8_t buzzer_on
);

#endif
