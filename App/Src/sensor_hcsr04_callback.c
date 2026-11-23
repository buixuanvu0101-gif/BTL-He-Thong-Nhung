#include "sensor_hcsr04.h"

/*
 * HAL TIM2 Input Capture callback.
 * Chi de mot dinh nghia callback trong firmware.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if ((htim != NULL) &&
        (htim->Instance == TIM2))
    {
        sensor_hcsr04_on_capture(htim);
    }
}
