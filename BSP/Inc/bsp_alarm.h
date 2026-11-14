
#ifndef BSP_ALARM_H
#define BSP_ALARM_H

#include <stdint.h>

/* Trang thai cua LED va Buzzer */
typedef struct
{
    uint8_t led_green;
    uint8_t led_red;
    uint8_t buzzer;
} BspAlarmOutput_t;

/* Xac dinh trang thai canh bao */
BspAlarmOutput_t bsp_alarm_evaluate(
    uint32_t alarm_flags
);

/* Dieu khien LED va Buzzer */
void bsp_alarm_apply(
    const BspAlarmOutput_t *output
);

#endif
