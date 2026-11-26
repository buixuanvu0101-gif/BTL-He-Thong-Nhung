# Việc Leader, Dev 1 và Dev 3 cần phối hợp

## Leader — quản lý CubeMX và tích hợp

- [ ] Review PR Dev 2, đặc biệt `Core/Src/freertos.c` và `CMakeLists.txt`.
- [ ] Xác nhận LED xanh PC13 active-low, LED đỏ PC14 active-high và điện trở giới hạn dòng phù hợp; buzzer PA6 TIM3_CH1.
- [ ] Xác nhận PB10 là nút nhấn; PB11 có vai trò gì.
- [ ] Nếu dùng EXTI: đổi PB10 từ RISING sang RISING_FALLING trong `.ioc`, giữ EXTI15_10 priority 7, bật `DEV2_BUTTON_MODE_EXTI=1` trong CMake. Không bật nếu chưa đổi CubeMX.
- [ ] Nếu thành viên khác cũng định nghĩa `HAL_GPIO_EXTI_Callback`, phải hợp nhất thành một callback.
- [ ] Kiểm tra kết quả `app_ipc_init()`, `hal_i2c_stm32_init()` và từng `task_create()`.
- [ ] Chốt priority COMM +3, SENSOR +2, DISPLAY +1; đo stack/heap trên board.
- [ ] Build ARM và nạp board, kiểm tra OLED I2C1, LED và PWM.

## Dev 3 — phần chưa có trong ZIP develop

- [ ] `task_comm.c` nhận `xSensorQueue` để cập nhật `gSystemState.sensor`.
- [ ] Nhận `xUiEventQueue` và xử lý `UI_EVENT_BUTTON_SHORT/LONG`.
- [ ] Chỉ ghi `gSystemState` khi đã giữ `xSystemStateMutex`.
- [ ] Đặt/xóa `ALARM_DISTANCE` bằng `xEventGroupSetBits/ClearBits`.
- [ ] Thống nhất `THRESHOLD_SOURCE_POT` và `THRESHOLD_SOURCE_ENC` (cùng giá trị 0, nhưng Dev 2 in POT còn Dev 3 in ENC).
- [ ] Hoàn thiện so sánh ngưỡng ADC/PC, kiểm tra dữ liệu cảm biến hợp lệ.

## Dev 1 — GPIO và sensor

- [ ] Giữ `APP_GPIO_Button_Read()` đọc PB10 active-low, báo Dev 2 nếu đổi pin.
- [ ] Tránh trùng định nghĩa EXTI callback nếu bật ISR mode.
- [ ] Bổ sung cờ hợp lệ DHT11/HC-SR04 theo yêu cầu của nhóm.
