# Nội dung báo cáo Dev 2 — OLED, nút nhấn và cảnh báo

**Cơ sở:** Các file ZIP của nhóm gửi ngày 23/11/2026. Kết quả dưới đây là kiểm thử mock trên máy tính, **chưa phải thử nghiệm trên STM32 thật**.

## 1. Bài trình bày ngắn (khoảng 1 phút)

> Em phụ trách Dev 2, gồm giao tiếp I2C1, khung SPI, driver OLED SSD1306, Display Task FreeRTOS, nút nhấn và phần xuất cảnh báo LED/buzzer. Em tổ chức mã theo ba lớp HAL, BSP và App để dễ tích hợp và kiểm thử độc lập. Display Task đọc SystemState bằng mutex, đọc bit cảnh báo từ Event Group, hiển thị OLED và gửi sự kiện nút sang Dev 3 bằng queue. Em đã bổ sung kiểm thử mock cho đường đi dữ liệu và xử lý lỗi; kiểm thử trên board vẫn cần thực hiện khi tích hợp toàn nhóm.

## 2. Sơ đồ luồng dữ liệu

```text
DHT11/HC-SR04 -- Dev 1 --> xSensorQueue --> Dev 3 task_comm
                                               |
                                 SystemState (Mutex)
                                 xAlarmEventGroup
                                               |
                                               v
Button PB10 --> Dev 2 task_display ----------> OLED I2C1 PB6/PB7
                     |                     \--> LED PC13/PC14
                     |                     \--> PWM buzzer PA6 (Dev 3)
                     +--> xUiEventQueue ------> Dev 3 task_comm
```

**Phân chia trách nhiệm:** Dev 2 không trực tiếp đọc cảm biến, không nhận lệnh UART và không tự tính ngưỡng. Dev 3 quyết định trạng thái cảnh báo và cập nhật dữ liệu dùng chung.

## 3. Những file quan trọng cần hiểu

| File | Nhiệm vụ |
| --- | --- |
| `hal/src/hal_i2c.c` | Giao diện I2C độc lập với chip, kiểm tra tham số và timeout |
| `hal/src/hal_i2c_stm32.c` | I2C1 STM32 HAL, khóa mutex bảo vệ bus |
| `hal/src/hal_spi.c` | Khung SPI; trả NOT_SUPPORTED vì chưa có thiết bị SPI |
| `BSP/Src/bsp_oled.c` | Lệnh SSD1306, font, framebuffer và truyền từng page |
| `App/Src/task_display.c` | Task đọc state, cảnh báo, OLED và nút nhấn |
| `App/Src/button_logic.c` | Chống dội 40 ms, phân biệt nhấn ngắn và dài 1500 ms |
| `BSP/Src/bsp_button.c` | Đọc PB10 qua API GPIO của Dev 1 |
| `App/Src/display_format.c` | Định dạng Temp/Humi/Dist/Limit/Source/Status |
| `App/Src/display_outputs.c` | Callback kết nối LED và buzzer với các driver khác |
| `BSP/Src/bsp_led_adapter.c` | LED xanh PC13 và LED đỏ PC14 |
| `App/Src/button_exti.c` | Phương án ISR dùng FromISR, chỉ bật khi Leader đổi cấu hình PB10 |

## 4. Chu kỳ và bộ nhớ

- Display Task được tạo bằng `xTaskCreate`, stack **512 words = 2048 byte** trên STM32F103 32-bit, độ ưu tiên `tskIDLE_PRIORITY + 1`. Phải đo stack high-water mark trên board trước khi khẳng định đủ RAM.
- Chế độ mặc định đọc nút mỗi **20 ms**, chống dội **40 ms**, nhấn dài từ **1500 ms**.
- Nội dung OLED được kiểm tra mỗi **200 ms**, chỉ vẽ lại khi dữ liệu thay đổi.
- SSD1306 có **128 × 64 pixel**, 1 bit/pixel nên framebuffer static cần **1024 byte**.
- Màn hình chia **8 page**, mỗi page 128 byte. Truyền một page mỗi vòng lặp giúp tránh chặn đọc nút quá lâu.
- Có hai mutex khác nhau: `xSystemStateMutex` bảo vệ dữ liệu dùng chung; mutex trong HAL I2C bảo vệ bus I2C1.
- Khi OLED lỗi, task đếm lỗi và thử khởi tạo lại sau 1000 ms; các chức năng nút và cảnh báo vẫn được xử lý.

## 5. Kiểm thử thực tế đã làm

Chạy tại gốc repository:

```bash
bash tests/run_dev2.sh
```

**8/8 chương trình test mock đạt:** logic alarm; debounce nút; định dạng; callback LED/buzzer; Display Task + queue/mutex/Event Group; I2C/SSD1306 + phục hồi timeout; GPIO adapter; và ISR FromISR tùy chọn. Biên dịch host với `-Wall -Wextra -Werror -pedantic` và UndefinedBehaviorSanitizer.

**Chưa xác minh:** build firmware bằng ARM GCC, chạy FreeRTOS trên STM32, tín hiệu I2C thực, LED/buzzer đúng cực, stack/heap và kết quả hệ thống từ cảm biến thật.

## 6. Câu hỏi vấn đáp thường gặp

**1. Vì sao chọn I2C cho OLED?** OLED SSD1306 4 chân dùng VCC/GND/SCL/SDA. I2C1 của STM32 sử dụng PB6/PB7 và tiết kiệm chân GPIO.

**2. Tại sao địa chỉ 0x3C lại truyền thành 0x78?** 0x3C là địa chỉ 7-bit; STM32 HAL cần giá trị dịch trái một bit, tức `0x3C << 1 = 0x78`.

**3. Framebuffer là gì?** Bộ đệm 1024 byte lưu trạng thái pixel trong RAM trước khi gửi qua I2C; khai báo static để tránh cấp phát động.

**4. Tại sao chia thành 8 page?** Mỗi page cao 8 pixel; chia nhỏ giúp giới hạn thời gian truyền trong mỗi vòng Display Task.

**5. Mutex dùng để làm gì?** Tránh đọc SystemState đúng lúc Dev 3 đang ghi. Task sao chép state rồi nhả mutex **trước** khi gửi I2C để không giữ khóa quá lâu.

**6. Queue khác mutex thế nào?** Queue chuyển các sự kiện có thứ tự từ Dev 2 sang Dev 3; mutex bảo vệ dữ liệu chung, không mang sự kiện.

**7. Event Group làm gì?** Dev 3 đặt/xóa `ALARM_DISTANCE`; Dev 2 đọc bit này để chọn LED xanh/đỏ và buzzer.

**8. Vì sao dùng `vTaskDelayUntil()`?** Để giữ chu kỳ lấy mẫu 20 ms ổn định hơn `vTaskDelay()` khi thời gian xử lý thay đổi.

**9. Nhấn ngắn và dài được phân biệt thế nào?** Bộ lọc chống dội 40 ms; nhấn từ 1500 ms là dài. Chế độ polling phát hiện nhấn dài khi đang giữ, chế độ EXTI tùy chọn phát sự kiện khi nhả nút.

**10. Vì sao hiện tại không dùng FromISR ở chế độ mặc định?** Vì file `.ioc` của nhóm đang cấu hình PB10 ngắt cạnh lên, không đủ cả hai cạnh để đo thời gian giữ. Để chạy ngay mà không sửa CubeMX của Leader, em dùng polling 20 ms trong task. Em cũng chuẩn bị `button_exti.c` dùng `xQueueSendFromISR`, chờ Leader đổi PB10 sang ngắt hai cạnh và bật cờ biên dịch.

**11. Vì sao Dev 2 không trực tiếp cấu hình PWM buzzer?** Theo phân công, TIM3 PWM do Dev 3 phụ trách. Dev 2 chỉ yêu cầu bật/tắt thông qua callback `Buzzer_Write()`.

**12. Khi I2C lỗi thì xử lý thế nào?** Driver trả về trạng thái lỗi, Display Task không ghi nhận trang lỗi là đã hiển thị, sau đó thử khởi tạo lại OLED có giới hạn thời gian.

## 7. Trình bày trung thực phần còn thiếu

> Em đã hoàn thành và kiểm thử logic Dev 2 bằng mock, đồng thời chuẩn bị patch để Leader review trên GitHub. Phần cần kiểm chứng tiếp là build firmware ARM, chạy trên board, xác nhận cực tính LED và ngắt PB10. Trong bản `develop` em được cung cấp, `task_comm.c` chưa tiêu thụ queue cảm biến/UI và chưa đặt bit Event Group; Dev 3 và Leader cần hoàn thiện trước khi nhóm có thể demo hệ thống cảnh báo từ cảm biến thật.

**Không nói đã nạp STM32 hoặc đã chạy OLED thật nếu chưa thực hiện.**
