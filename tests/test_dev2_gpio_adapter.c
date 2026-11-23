#include "bsp_led_adapter.h"
#include "bsp_button.h"
#include "bsp_alarm.h"
#include "app_cfg.h"
#include "stm32f1xx_hal.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

GPIO_TypeDef fake_gpio_c;
static int green, red, buzz;
static bool button_pressed;
void APP_GPIO_LED_On(void) { green = 1; }
void APP_GPIO_LED_Off(void) { green = 0; }
bool APP_GPIO_Button_Read(void) { return button_pressed; }
void Buzzer_Write(uint8_t on) { buzz = on; }
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    assert(port == GPIOC && pin == GPIO_PIN_14);
    red = (state == GPIO_PIN_SET);
}
int main(void)
{
    puts("=== DEV2: GPIO / PWM ADAPTER MOCK ===");
    button_pressed = true;
    assert(bsp_button_is_pressed() == 1U);
    button_pressed = false;
    assert(bsp_button_is_pressed() == 0U);
    bsp_led_write(1U, 0U);
    assert(green == 1 && red == 0);
    bsp_led_write(0U, 1U);
    assert(green == 0 && red == 1);
    BspAlarmOutput_t o = bsp_alarm_evaluate(ALARM_DISTANCE);
    bsp_alarm_apply(&o);
    assert(green == 0 && red == 1 && buzz == 1);
    o = bsp_alarm_evaluate(0U);
    bsp_alarm_apply(&o);
    assert(green == 1 && red == 0 && buzz == 0);
    puts("[PASS] Dev1 PB10/PC13 + PC14 + Dev3 buzzer callback");
    return 0;
}
