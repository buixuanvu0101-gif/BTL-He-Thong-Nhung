
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "app_cfg.h"
#include "display_format.h"
#include "bsp_alarm.h"

/* Tao du lieu he thong gia lap */
static SystemState_t create_mock_state(void)
{
    SystemState_t state = {0};

    state.sensor.temperature_c = 28.5f;
    state.sensor.humidity_percent = 65.0f;
    state.sensor.distance_cm = 15.2f;
    state.sensor.potentiometer_raw = 2048U;

    state.distance_threshold_cm = 20.0f;
    state.threshold_source = THRESHOLD_SOURCE_POT;
    state.alarm_flags = ALARM_DISTANCE;

    return state;
}

/* In du lieu gia lap OLED */
static void print_display(const DisplayText_t *text)
{
    printf("\n--- OLED DATA ---\n");
    printf("%s\n", text->temperature);
    printf("%s\n", text->humidity);
    printf("%s\n", text->distance);
    printf("%s\n", text->threshold);
    printf("%s\n", text->source);
    printf("%s\n", text->alarm);
}

/* Kiem tra POT va canh bao */
static void test_pot_alarm(void)
{
    SystemState_t state = create_mock_state();
    DisplayText_t text = {0};

    display_format_data(&state, &text);

    assert(strcmp(text.temperature,
                  "Temp: 28.5 C") == 0);

    assert(strcmp(text.humidity,
                  "Humi: 65.0 %") == 0);

    assert(strcmp(text.source,
                  "Source: POT") == 0);

    assert(strcmp(text.alarm,
                  "Status: ALARM") == 0);

    BspAlarmOutput_t output =
        bsp_alarm_evaluate(state.alarm_flags);

    assert(output.led_green == 0U);
    assert(output.led_red == 1U);
    assert(output.buzzer == 1U);

    printf("[PASS] POT alarm mode\n");
    print_display(&text);
}

/* Kiem tra PC va trang thai binh thuong */
static void test_pc_normal(void)
{
    SystemState_t state = create_mock_state();
    DisplayText_t text = {0};

    state.threshold_source = THRESHOLD_SOURCE_PC;
    state.distance_threshold_cm = 10.0f;
    state.alarm_flags = 0U;

    display_format_data(&state, &text);

    assert(strcmp(text.source,
                  "Source: PC") == 0);

    assert(strcmp(text.threshold,
                  "Limit: 10.0 cm") == 0);

    assert(strcmp(text.alarm,
                  "Status: NORMAL") == 0);

    BspAlarmOutput_t output =
        bsp_alarm_evaluate(state.alarm_flags);

    assert(output.led_green == 1U);
    assert(output.led_red == 0U);
    assert(output.buzzer == 0U);

    printf("[PASS] PC normal mode\n");
    print_display(&text);
}

/* Kiem tra gia tri nguong thay doi lien tuc */
static void test_continuous_threshold(void)
{
    SystemState_t state = create_mock_state();
    DisplayText_t text = {0};

    state.distance_threshold_cm = 12.5f;
    display_format_data(&state, &text);

    assert(strcmp(text.threshold,
                  "Limit: 12.5 cm") == 0);

    state.distance_threshold_cm = 18.7f;
    display_format_data(&state, &text);

    assert(strcmp(text.threshold,
                  "Limit: 18.7 cm") == 0);

    printf("[PASS] Continuous threshold display\n");
}

int main(void)
{
    printf("=== DEV 2 DISPLAY TEST ===\n");

    test_pot_alarm();
    test_pc_normal();
    test_continuous_threshold();

    printf("\nAll display tests passed!\n");

    return 0;
}
