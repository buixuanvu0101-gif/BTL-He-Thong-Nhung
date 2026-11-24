# DEV 2 — Handoff sau khi bo sung (23/11/2026)

> Co so: ZIP `BTL-He-Thong-Nhung-develop.zip` nguoi dung gui ngay 23/11/2026.
> Chua doi chieu commit HEAD tren GitHub thoi gian thuc; khong tu merge len main.

## Da co trong develop truoc ban va
- `hal_i2c.c`, `hal_i2c_stm32.c`: I2C1, mutex, timeout, backend HAL.
- `hal_spi.c`: khung SPI tra `DEV2_SPI_NOT_SUPPORTED` (khong co thiet bi SPI).
- `bsp_oled.c`: SSD1306 I2C 0x3C, 128x64, framebuffer static 1024 byte, font ASCII co ban.
- `button_logic.c`: debounce 40 ms, nhan dai 1500 ms.
- `display_format.c`: chuoi Temp/Humi/Dist/Limit/Source/Status.
- `display_outputs.c`: adapter LED va buzzer; `task_display.c`: mutex, event group, OLED.

## Da bo sung trong ban de xuat
- `task_display.c`: lay mau nut 20 ms, gui `UiEvent_t` vao `xUiEventQueue`;
  cap nhat OLED 200 ms theo tung page; retry I2C 1000 ms; bao loi queue/mutex/OLED.
- `bsp_button.c`: dung `APP_GPIO_Button_Read()` cua Dev 1, chan PB10 active-low.
- `bsp_led_adapter.c/.h`: bind LED xanh PC13 qua API Dev 1, LED do PC14;
  buzzer dung `Buzzer_Write` TIM3_CH1 PA6 cua Dev 3.
- `bsp_alarm_stm32.c`: bo mapping PB12/PB13/PB14 cu, dung adapter dung pinout.
- `bsp_oled_update_page()`: tra ve loi I2C; `bsp_oled_update()` tra ve ket qua.
- `button_exti.c/.h`: **phuong an EXTI tuy chon** gui queue bang
  `xQueueSendFromISR`, chua bat mac dinh vi `.ioc` PB10 dang RISING-only.
- `freertos.c`: them bind LED trong USER CODE, khong sua `.ioc`.
- `CMakeLists.txt`: them `bsp_led_adapter.c`, `button_exti.c`, `hal_spi.c`.
- `tests/run_dev2.sh`: 8 chuong trinh test host mock.

## Thuc te phan cung / giu nguyen hop dong API
| Thanh phan | Ket noi / API | Luu y |
| --- | --- | --- |
| OLED | PB6 SCL, PB7 SDA, I2C1 0x3C | Pull-up I2C phu hop |
| LED xanh | PC13, `APP_GPIO_LED_On/Off()` | Active-low theo Dev 1 |
| LED do | PC14 | Dang gia dinh active-high; phai kiem tra mach |
| Buzzer | PA6 TIM3_CH1, `Buzzer_Write()` | PWM do Dev 3 quan ly |
| Button | PB10, `APP_GPIO_Button_Read()` | Pull-up, nhan = LOW |
| PB11 | GPIO input khac | Chua duoc chot lam nut nao |
| SPI | Khong co thiet bi | API stub, khong khoi tao SPI hardware |

## Hai che do nut bam
1. **Mac dinh:** Polling 20 ms trong Display Task. Hoat dong voi `.ioc`
   hien tai, su dung `xQueueSend` trong task; **khong phai FromISR**.
2. **EXTI dung yeu cau:** Leader doi PB10 thanh `GPIO_MODE_IT_RISING_FALLING`,
   giu IRQ priority 7, bat `DEV2_BUTTON_MODE_EXTI=1` trong CMake.
   `HAL_GPIO_EXTI_Callback()` gui su kien sau khi nha nut qua `xQueueSendFromISR`.
   Neu Dev 1 cung dung callback nay thi phai gop callback, khong dinh nghia trung.
   Khi bat EXTI, polling khong gui su kien nua. Long-press phat khi nha nut.

## Viec can Leader va Dev 3 hoan thanh truoc khi goi la he thong hoan chinh
- `task_comm.c` trong ZIP develop chua goi `xQueueReceive(xSensorQueue, ...)`,
  `xQueueReceive(xUiEventQueue, ...)` hoac `xEventGroupSetBits/ClearBits`.
  Vi vay Dev 2 co the gui su kien, nhung Dev 3 chua xu ly va chua sinh canh bao.
- `Comm_TaskCreate()` dang dung priority `tskIDLE_PRIORITY + 1`, bang Display;
  theo phan cong can COMM > SENSOR > DISPLAY. Leader/Dev 3 chot lai.
- `THRESHOLD_SOURCE_ENC` va `THRESHOLD_SOURCE_POT` dang cung gia tri 0;
  Dev 3 in "ENC" con Dev 2 in "POT". Can thong nhat ten va nguon ADC.
- Kiem tra xac thuc chan PC14 va do dong LED, EXTI PB10, OLED I2C,
  RAM heap/stack, va board test.

## Ket qua kiem thu
`bash tests/run_dev2.sh` -> 8/8 chuong trinh test PASS voi GCC
`-Wall -Wextra -Werror -pedantic -fsanitize=undefined`.
**Khong dong nghia firmware STM32 build PASS hay test board PASS.**
