#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stdbool.h>

/* Bat dau nhan UART bang ngat, goi mot lan */
bool APP_UART_StartRx(void);

/* Gui mot byte */
bool APP_UART_SendChar(uint8_t data, uint32_t timeout);

/* Gui chuoi */
bool APP_UART_SendString(const char *data, uint32_t timeout);

/* Doc mot byte tu bo dem, khong block */
bool APP_UART_ReceiveChar(uint8_t *data);

#endif
