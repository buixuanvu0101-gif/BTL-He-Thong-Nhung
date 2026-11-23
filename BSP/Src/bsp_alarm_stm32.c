
#include "bsp_alarm.h"
#include "stm32f1xx_hal.h"

#include <stddef.h>

/*
 * DEV 2 - STM32 GPIO OUTPUT
 *
 * Cau hinh chan du kien:
 * PB12: LED xanh
 * PB13: LED do
 * PB14: Buzzer
 *
 * Gia dinh cac dau ra active-high.
 */

#define LED_GREEN_PORT GPIOB
#define LED_GREEN_PIN  GPIO_PIN_12

#define LED_RED_PORT   GPIOB
#define LED_RED_PIN    GPIO_PIN_13

#define BUZZER_PORT    GPIOB
#define BUZZER_PIN     GPIO_PIN_14

/* Cap nhat dau ra phan cung */
void bsp_alarm_apply(
    const BspAlarmOutput_t *output)
{
    if (output == NULL)
    {
        return;
    }

    HAL_GPIO_WritePin(
        LED_GREEN_PORT,
        LED_GREEN_PIN,
        (output->led_green != 0U)
            ? GPIO_PIN_SET
            : GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        LED_RED_PORT,
        LED_RED_PIN,
        (output->led_red != 0U)
            ? GPIO_PIN_SET
            : GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        BUZZER_PORT,
        BUZZER_PIN,
        (output->buzzer != 0U)
            ? GPIO_PIN_SET
            : GPIO_PIN_RESET
    );
}
