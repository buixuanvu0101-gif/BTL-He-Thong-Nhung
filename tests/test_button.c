
#include <stdio.h>
#include <assert.h>

#include "button_logic.h"

/* Dem so su kien */
static unsigned int short_count = 0U;
static unsigned int long_count = 0U;

/* Che do nguong gia lap cua Dev 3 */
static unsigned int pc_mode = 0U;

/* Dev 3 gia lap xu ly su kien */
static void handle_event(ButtonEvent_t event)
{
    if (event == BUTTON_EVENT_SHORT)
    {
        short_count++;

        /* Gia lap chuyen POT / PC */
        pc_mode = (pc_mode == 0U) ? 1U : 0U;
    }
    else if (event == BUTTON_EVENT_LONG)
    {
        long_count++;
    }
}

/* Kiem tra nhan ngan */
static void test_short_press(void)
{
    ButtonLogic_t button;
    button_logic_init(&button);

    handle_event(button_logic_update(&button, 1U, 0U));
    handle_event(button_logic_update(&button, 1U, 20U));
    handle_event(button_logic_update(&button, 1U, 40U));

    handle_event(button_logic_update(&button, 0U, 200U));
    handle_event(button_logic_update(&button, 0U, 240U));

    assert(short_count == 1U);
    assert(long_count == 0U);
    assert(pc_mode == 1U);

    printf("[PASS] Short press: POT -> PC\n");
}

/* Kiem tra nhan giu */
static void test_long_press(void)
{
    ButtonLogic_t button;
    button_logic_init(&button);

    handle_event(button_logic_update(&button, 1U, 1000U));
    handle_event(button_logic_update(&button, 1U, 1040U));

    handle_event(button_logic_update(&button, 1U, 2540U));

    handle_event(button_logic_update(&button, 0U, 2600U));
    handle_event(button_logic_update(&button, 0U, 2640U));

    assert(long_count == 1U);
    assert(short_count == 1U);
    assert(pc_mode == 1U);

    printf("[PASS] Long press detected once\n");
}

/* Kiem tra chong doi */
static void test_debounce(void)
{
    ButtonLogic_t button;
    button_logic_init(&button);

    unsigned int before = short_count;

    handle_event(button_logic_update(&button, 1U, 3000U));
    handle_event(button_logic_update(&button, 0U, 3010U));
    handle_event(button_logic_update(&button, 1U, 3020U));
    handle_event(button_logic_update(&button, 0U, 3030U));
    handle_event(button_logic_update(&button, 0U, 3070U));

    assert(short_count == before);

    printf("[PASS] Button debounce\n");
}

/* Kiem tra chuyen PC ve POT */
static void test_mode_switch_back(void)
{
    ButtonLogic_t button;
    button_logic_init(&button);

    handle_event(button_logic_update(&button, 1U, 4000U));
    handle_event(button_logic_update(&button, 1U, 4040U));

    handle_event(button_logic_update(&button, 0U, 4200U));
    handle_event(button_logic_update(&button, 0U, 4240U));

    assert(short_count == 2U);
    assert(pc_mode == 0U);

    printf("[PASS] Short press: PC -> POT\n");
}

int main(void)
{
    printf("=== DEV 2 BUTTON TEST ===\n");

    test_short_press();
    test_long_press();
    test_debounce();
    test_mode_switch_back();

    printf("All button tests passed!\n");

    return 0;
}
