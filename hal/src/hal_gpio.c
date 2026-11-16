#include "hal_gpio.h"
#include "stm32f1xx_hal.h"

#define LED_PORT GPIOC
#define LED_PIN GPIO_PIN_13

#define TRIG_PORT GPIOA
#define TRIG_PIN GPIO_PIN_1

#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_0

#define BUTTON_PORT GPIOB
#define BUTTON_PIN GPIO_PIN_10

void APP_GPIO_Init(void)
{
    APP_GPIO_LED_Off();
    APP_GPIO_Trig_Reset();
    APP_GPIO_DHT11_Write(1);
}

void APP_GPIO_LED_On(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

void APP_GPIO_LED_Off(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void APP_GPIO_LED_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

void APP_GPIO_Trig_Set(void)
{
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
}

void APP_GPIO_Trig_Reset(void)
{
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
}

void APP_GPIO_DHT11_Output(void)
{
    GPIO_InitTypeDef gpio = {0};

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);

    gpio.Pin = DHT11_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(DHT11_PORT, &gpio);
}

void APP_GPIO_DHT11_Input(void)
{
    GPIO_InitTypeDef gpio = {0};

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);

    gpio.Pin = DHT11_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(DHT11_PORT, &gpio);
}

void APP_GPIO_DHT11_Write(uint8_t value)
{
    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        value ? GPIO_PIN_SET : GPIO_PIN_RESET
    );
}

uint8_t APP_GPIO_DHT11_Read(void)
{
    return HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)
           == GPIO_PIN_SET ? 1U : 0U;
}

bool APP_GPIO_Button_Read(void)
{
    return HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN)
           == GPIO_PIN_RESET;
}
