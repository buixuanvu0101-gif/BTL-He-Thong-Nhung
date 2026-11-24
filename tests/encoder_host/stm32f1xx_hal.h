#ifndef STM32F1XX_HAL_H
#define STM32F1XX_HAL_H

#include <stdint.h>

typedef struct {
    uint32_t dummy;
} GPIO_TypeDef;

extern GPIO_TypeDef fake_gpioa;

#define GPIOA (&fake_gpioa)
#define GPIO_PIN_2 (1U << 2)
#define GPIO_PIN_3 (1U << 3)
#define GPIO_PIN_4 (1U << 4)

typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET = 1
} GPIO_PinState;

GPIO_PinState HAL_GPIO_ReadPin(
    GPIO_TypeDef *port, uint16_t pin);

uint32_t HAL_GetTick(void);

#endif
