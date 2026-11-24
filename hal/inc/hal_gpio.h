
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>

/* Khoi tao GPIO */
void APP_GPIO_Init(void);

/* Dieu khien LED */
void APP_GPIO_LED_On(void);
void APP_GPIO_LED_Off(void);
void APP_GPIO_LED_Toggle(void);

/* Dieu khien chan TRIG cua HC-SR04 */
void APP_GPIO_Trig_Set(void);
void APP_GPIO_Trig_Reset(void);

/* Dieu khien chan DHT11 */
void APP_GPIO_DHT11_Output(void);
void APP_GPIO_DHT11_Input(void);
void APP_GPIO_DHT11_Write(uint8_t value);
uint8_t APP_GPIO_DHT11_Read(void);

/* Doc trang thai nut nhan */
bool APP_GPIO_Button_Read(void);

#endif
