#include "hal_uart.h"
#include "usart.h"
#include <stddef.h>

#define UART_RX_BUFFER_SIZE 64U

/* Bo dem vong: 1 producer ISR, 1 consumer task */
static volatile uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

/* Byte ma HAL dang nhan */
static uint8_t rx_it_byte;

/* Bat dau nhan bang ngat, sau MX_USART1_UART_Init */
bool APP_UART_StartRx(void)
{
    if (huart1.Instance != USART1)
    {
        return false;
    }

    return HAL_UART_Receive_IT(
        &huart1, &rx_it_byte, 1U
    ) == HAL_OK;
}

/* Gui mot byte */
bool APP_UART_SendChar(uint8_t data, uint32_t timeout)
{
    return HAL_UART_Transmit(
        &huart1, &data, 1U, timeout
    ) == HAL_OK;
}

/* Gui chuoi */
bool APP_UART_SendString(const char *data, uint32_t timeout)
{
    if (data == NULL)
    {
        return false;
    }

    while (*data != '\0')
    {
        if (!APP_UART_SendChar((uint8_t)*data, timeout))
        {
            return false;
        }

        data++;
    }

    return true;
}

/* Lay mot byte da nhan, khong cho doi */
bool APP_UART_ReceiveChar(uint8_t *data)
{
    if (data == NULL || rx_head == rx_tail)
    {
        return false;
    }

    *data = rx_buffer[rx_tail];

    rx_tail = (uint16_t)(
        (rx_tail + 1U) % UART_RX_BUFFER_SIZE
    );

    return true;
}

/* HAL goi callback sau khi nhan xong mot byte */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart1)
    {
        return;
    }

    uint16_t next = (uint16_t)(
        (rx_head + 1U) % UART_RX_BUFFER_SIZE
    );

    if (next != rx_tail)
    {
        rx_buffer[rx_head] = rx_it_byte;
        rx_head = next;
    }

    /* Tiep tuc nhan byte ke tiep */
    (void)HAL_UART_Receive_IT(
        &huart1, &rx_it_byte, 1U
    );
}

/* Khoi phuc nhan neu HAL da dung RX do loi */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1 &&
        huart1.RxState == HAL_UART_STATE_READY)
    {
        (void)HAL_UART_Receive_IT(
            &huart1, &rx_it_byte, 1U
        );
    }
}
