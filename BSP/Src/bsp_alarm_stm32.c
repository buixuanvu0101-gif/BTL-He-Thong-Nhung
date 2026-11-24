#include "bsp_alarm.h"
#include "bsp_led_adapter.h"
#include "hal_buzzer.h"

#include <stddef.h>

/* Keep the legacy BSP API functional, but use the same actual hardware
 * mappings as the DisplayTask output adapter: PC13/PC14 and TIM3_CH1 PA6.
 * Do not drive PB12/PB13/PB14; those are not configured in this .ioc.
 */
void bsp_alarm_apply(const BspAlarmOutput_t *output)
{
    if (output == NULL) { return; }
    bsp_led_write(output->led_green, output->led_red);
    Buzzer_Write(output->buzzer);
}
