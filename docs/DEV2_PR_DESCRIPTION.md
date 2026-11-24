# PR: feat(task-display): integrate OLED, button and alarm adapters

## Nội dung
- Display Task đọc SystemState bằng mutex, Event Group và xử lý nút PB10 mỗi 20 ms.
- Sự kiện SHORT/LONG được gửi qua `xUiEventQueue` tới Dev 3.
- OLED SSD1306 I2C1 truyền theo từng page, có xử lý lỗi và retry.
- LED PC13/PC14 được bind vào Display Task; buzzer qua PWM adapter `Buzzer_Write()` của Dev 3.
- Bổ sung phương án EXTI dùng `xQueueSendFromISR()` (mặc định tắt vì PB10 hiện chỉ RISING).
- Bổ sung SPI stub vào CMake và 8 bài kiểm thử host mock.

## Kiểm thử
- `bash tests/run_dev2.sh`: **8/8 chương trình PASS** (GCC host mock, UBSan).
- Đã kiểm tra cú pháp các file tích hợp với header STM32 HAL/FreeRTOS.
- Chưa build firmware ARM, chưa chạy phần cứng.

## Leader cần review
- `Core/Src/freertos.c`: thêm `display_outputs_bind_led(bsp_led_write)`.
- `CMakeLists.txt`: thêm nguồn `bsp_led_adapter.c`, `button_exti.c`, `hal_spi.c`.
- Chốt chân PC14/LED và PB10/nút, đo heap/stack.

## Dev 3 cần làm để hệ thống hoạt động
- Nhận `xSensorQueue`, `xUiEventQueue`; cập nhật `gSystemState` với mutex.
- Đặt/xóa `ALARM_DISTANCE` trong Event Group.
- Chốt priority COMM > SENSOR > DISPLAY, nguồn ngưỡng POT/ENC/PC.

Không merge trực tiếp vào `main` trước khi Leader kiểm tra.
