
#ifndef DEV2_HAL_I2C_STM32_H
#define DEV2_HAL_I2C_STM32_H

#include "hal_i2c.h"

/*
 * DEV 2 - STM32 I2C BACKEND
 *
 * Su dung:
 * - STM32F1 HAL chinh thuc
 * - I2C1
 * - FreeRTOS Mutex
 *
 * Ham nay duoc goi mot lan sau khi
 * I2C1 va FreeRTOS heap da san sang,
 * truoc khi Display Task su dung OLED.
 */
Dev2I2cStatus_t hal_i2c_stm32_init(void);

#endif
