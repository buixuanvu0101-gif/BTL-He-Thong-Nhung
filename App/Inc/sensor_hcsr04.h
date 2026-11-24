#ifndef SENSOR_HCSR04_H
#define SENSOR_HCSR04_H

#include <stdbool.h>
#include "stm32f1xx_hal.h"

/* Khoi tao semaphore va TIM2 Input Capture */
bool sensor_hcsr04_init(void);

/* Doc khoang cach - goi tu FreeRTOS Task */
bool sensor_hcsr04_read(float *distance_cm);

/* Goi tu HAL_TIM_IC_CaptureCallback */
void sensor_hcsr04_on_capture(TIM_HandleTypeDef *htim);

#endif
