# DEV 2 – Display, Alarm and Button

## 1. Chức năng

Dev 2 phụ trách:
- Hiển thị dữ liệu lên OLED SSD1306.
- Hiển thị nguồn ngưỡng POT/PC.
- Điều khiển LED xanh và LED đỏ.
- Điều khiển Buzzer.
- Xử lý nút nhấn ngắn/dài và chống dội.
- Gửi sự kiện nút nhấn đến Dev 3.

## 2. Giao tiếp với các Dev

- Đọc `gSystemState` bằng `xSystemStateMutex`.
- Lấy trạng thái cảnh báo từ `alarm_flags`.
- Gửi `UiEvent_t` vào `xUiEventQueue`.
- Không trực tiếp đọc DHT11, HC-SR04 hoặc ADC.
- Không trực tiếp nhận lệnh UART.

## 3. Các file chính

- `task_display.c/.h`: FreeRTOS Display Task.
- `button_logic.c/.h`: Xử lý nút nhấn.
- `display_format.c/.h`: Định dạng chuỗi hiển thị.
- `bsp_alarm.c/.h`: Logic và giao diện cảnh báo.
- `bsp_button.c/.h`: Giao diện đọc nút.
- `bsp_oled.c/.h`: Giao diện điều khiển OLED.

## 4. Phần đã viết

- Khung task dùng `vTaskDelayUntil()`.
- Đọc SystemState bằng Mutex.
- Chuẩn bị dữ liệu hiển thị.
- Logic LED và Buzzer.
- Nhận biết nhấn ngắn/dài.
- Gửi sự kiện nút qua Queue.

## 5. Phần chưa hoàn thành

- Driver SSD1306 giao tiếp STM32 HAL.
- Hàm điều khiển GPIO thực tế cho LED/Buzzer.
- Hàm đọc chân GPIO của nút nhấn.
- Kiểm thử tích hợp FreeRTOS.
- Kiểm tra stack và heap trên STM32.
- Kiểm thử phần cứng thực tế.

## 6. Lưu ý tích hợp

- Khởi tạo phần cứng và IPC trước khi tạo task.
- Không giữ Mutex trong khi giao tiếp OLED.
- Không dùng HAL_Delay trong FreeRTOS task.
- Không sửa trực tiếp `main.c` hoặc `FreeRTOSConfig.h` trên nhánh Dev 2.
- Kiểm tra Queue đầy, lỗi Mutex và thời gian chạy task.
- Chỉ merge PR sau khi kiểm tra khả năng biên dịch và các yêu cầu của nhóm.