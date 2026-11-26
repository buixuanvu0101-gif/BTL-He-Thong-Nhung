#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT
FLAGS=(-std=c11 -Wall -Wextra -Werror -pedantic -O0 -g)
INCLUDES=(-IApp/Inc -IBSP/Inc -Ihal/inc)
run() {
  local name="$1"; shift
  echo "---- $name ----"
  "$CC" "${FLAGS[@]}" "${INCLUDES[@]}" "$@" -o "$OUT/$name"
  "$OUT/$name"
}
run alarm tests/test_alarm.c BSP/Src/bsp_alarm.c
run button tests/test_button.c App/Src/button_logic.c
run display tests/test_display.c App/Src/display_format.c BSP/Src/bsp_alarm.c
run outputs tests/test_display_outputs.c App/Src/display_outputs.c BSP/Src/bsp_alarm.c
run task_display -Itests/mock_freertos tests/test_task_display.c \
  App/Src/task_display.c App/Src/button_logic.c App/Src/display_format.c \
  App/Src/display_outputs.c BSP/Src/bsp_alarm.c
run oled_i2c tests/test_dev2_oled_i2c.c BSP/Src/bsp_oled.c \
  hal/src/hal_i2c.c hal/src/hal_spi.c
run gpio_adapter -Itests/mock_hal tests/test_dev2_gpio_adapter.c \
  BSP/Src/bsp_led_adapter.c BSP/Src/bsp_button.c \
  BSP/Src/bsp_alarm_stm32.c BSP/Src/bsp_alarm.c
run exti_fromisr -DDEV2_BUTTON_MODE_EXTI=1 -Itests/mock_freertos \
  -Itests/mock_hal tests/test_dev2_exti.c App/Src/button_exti.c
printf '\nALL 8 DEV2 TESTS PASS (HOST MOCK; NOT A BOARD TEST)\n'
