#include "encoder_ky040.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <stdlib.h>

GPIO_TypeDef fake_gpioa = {0};

static uint16_t pins =
    GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;

static uint32_t tick_ms = 0;

GPIO_PinState HAL_GPIO_ReadPin(
    GPIO_TypeDef *port, uint16_t pin)
{
    (void)port;
    return (pins & pin) ?
        GPIO_PIN_SET : GPIO_PIN_RESET;
}

uint32_t HAL_GetTick(void)
{
    return tick_ms;
}

static void set_ab(int clk, int dt)
{
    pins &= ~(GPIO_PIN_2 | GPIO_PIN_3);

    if (clk) pins |= GPIO_PIN_2;
    if (dt)  pins |= GPIO_PIN_3;
}

static void check(EncoderEvent_t expected,
                  const char *name)
{
    EncoderEvent_t actual = encoder_ky040_poll();

    if (actual != expected)
    {
        printf("FAIL: %s (expected=%d, got=%d)\n",
               name, expected, actual);
        exit(1);
    }
}

int main(void)
{
    encoder_ky040_init();

    /* Chieu duong cua bang giai ma */
    set_ab(0, 1); check(ENCODER_NONE, "CW 1");
    set_ab(0, 0); check(ENCODER_NONE, "CW 2");
    set_ab(1, 0); check(ENCODER_NONE, "CW 3");
    set_ab(1, 1); check(ENCODER_CW,   "CW 4");
    puts("PASS: clockwise");

    /* Chieu nguoc */
    set_ab(1, 0); check(ENCODER_NONE, "CCW 1");
    set_ab(0, 0); check(ENCODER_NONE, "CCW 2");
    set_ab(0, 1); check(ENCODER_NONE, "CCW 3");
    set_ab(1, 1); check(ENCODER_CCW,  "CCW 4");
    puts("PASS: counterclockwise");

    /* Chuyen trang thai khong hop le */
    set_ab(0, 0); check(ENCODER_NONE, "Invalid 1");
    set_ab(1, 1); check(ENCODER_NONE, "Invalid 2");
    puts("PASS: invalid transitions");

    /* Nhan nut va cho chong doi 30 ms */
    pins &= ~GPIO_PIN_4;
    check(ENCODER_NONE, "Press initial");

    tick_ms += 29;
    check(ENCODER_NONE, "Press before 30ms");

    tick_ms += 1;
    check(ENCODER_PRESSED, "Press after 30ms");
    puts("PASS: button debounce");

    /* Giu nut khong duoc phat nhieu lan */
    tick_ms += 100;
    check(ENCODER_NONE, "Hold button");
    puts("PASS: button hold");

    puts("ALL TESTS PASSED");
    return 0;
}
