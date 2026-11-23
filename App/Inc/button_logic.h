
#ifndef BUTTON_LOGIC_H
#define BUTTON_LOGIC_H

#include <stdint.h>

/* Thoi gian xu ly nut nhan */
#define BUTTON_DEBOUNCE_MS   40U
#define BUTTON_LONG_PRESS_MS 1500U

/* Ket qua xu ly nut */
typedef enum
{
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT,
    BUTTON_EVENT_LONG
} ButtonEvent_t;

/* Trang thai bo xu ly nut */
typedef struct
{
    uint8_t last_raw;
    uint8_t stable_state;
    uint8_t long_sent;

    uint32_t last_change_ms;
    uint32_t press_start_ms;
} ButtonLogic_t;

/* Khoi tao trang thai */
void button_logic_init(ButtonLogic_t *button);

/* Xu ly mau nut tai thoi diem now_ms */
ButtonEvent_t button_logic_update(
    ButtonLogic_t *button,
    uint8_t pressed,
    uint32_t now_ms
);

#endif
