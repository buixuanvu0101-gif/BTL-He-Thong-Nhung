
#include "button_logic.h"
#include <stddef.h>

/* Khoi tao */
void button_logic_init(ButtonLogic_t *button)
{
    if (button == NULL)
    {
        return;
    }

    button->last_raw = 0U;
    button->stable_state = 0U;
    button->long_sent = 0U;

    button->last_change_ms = 0U;
    button->press_start_ms = 0U;
}

/* Xu ly nut nhan */
ButtonEvent_t button_logic_update(
    ButtonLogic_t *button,
    uint8_t pressed,
    uint32_t now_ms)
{
    if (button == NULL)
    {
        return BUTTON_EVENT_NONE;
    }

    pressed = (pressed != 0U) ? 1U : 0U;

    /* Phat hien muc dien thay doi */
    if (pressed != button->last_raw)
    {
        button->last_raw = pressed;
        button->last_change_ms = now_ms;
    }

    /* Xac nhan trang thai sau debounce */
    if ((uint32_t)(now_ms - button->last_change_ms)
            >= BUTTON_DEBOUNCE_MS)
    {
        if (pressed != button->stable_state)
        {
            button->stable_state = pressed;

            if (pressed != 0U)
            {
                button->press_start_ms = now_ms;
                button->long_sent = 0U;
            }
            else
            {
                if (button->long_sent == 0U)
                {
                    uint32_t held_ms =
                        now_ms - button->press_start_ms;

                    if (held_ms >= BUTTON_LONG_PRESS_MS)
                    {
                        button->long_sent = 1U;
                        return BUTTON_EVENT_LONG;
                    }

                    return BUTTON_EVENT_SHORT;
                }
            }
        }
    }

    /* Phat hien nhan giu */
    if ((button->stable_state != 0U) &&
        (button->long_sent == 0U) &&
        ((uint32_t)(now_ms - button->press_start_ms)
            >= BUTTON_LONG_PRESS_MS))
    {
        button->long_sent = 1U;
        return BUTTON_EVENT_LONG;
    }

    return BUTTON_EVENT_NONE;
}
