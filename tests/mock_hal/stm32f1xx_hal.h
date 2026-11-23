#ifndef DEV2_MOCK_HAL_H
#define DEV2_MOCK_HAL_H
#include <stdint.h>
typedef enum {GPIO_PIN_RESET=0, GPIO_PIN_SET=1} GPIO_PinState;
typedef struct { uint32_t unused; } GPIO_TypeDef;
extern GPIO_TypeDef fake_gpio_c;
extern GPIO_TypeDef fake_gpio_b;
#define GPIOC (&fake_gpio_c)
#define GPIOB (&fake_gpio_b)
#define GPIO_PIN_10 (1U<<10)
#define GPIO_PIN_14 (1U<<14)
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
#endif
