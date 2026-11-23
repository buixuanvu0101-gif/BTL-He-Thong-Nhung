#include "sensor_hcsr04.h"

#include "hal_gpio.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdint.h>

#define HCSR04_TIMEOUT_MS   40U
#define HCSR04_MIN_ECHO_US  116U
#define HCSR04_MAX_ECHO_US  23200U

static SemaphoreHandle_t s_echoDone = NULL;
static volatile uint8_t s_captureState = 0U;
static volatile uint16_t s_risingTick = 0U;
static volatile uint16_t s_echoWidthUs = 0U;

/* TIM4 phai dang chay tai 1 MHz */
static void prvDelayUs(uint16_t delay)
{
    uint16_t start =
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);

    while ((uint16_t)(
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim4)
        - start) < delay)
    {
    }
}

bool sensor_hcsr04_init(void)
{
    /* Chi chap nhan timer 1 us/tick */
    if ((htim2.Instance != TIM2) ||
        (htim4.Instance != TIM4) ||
        (TIM2->PSC != 71U) ||
        (TIM2->ARR != 65535U) ||
        (TIM4->PSC != 71U) ||
        (TIM4->ARR != 65535U))
    {
        return false;
    }

    if (s_echoDone != NULL)
    {
        return true;
    }

    s_echoDone = xSemaphoreCreateBinary();

    if (s_echoDone == NULL)
    {
        return false;
    }

    s_captureState = 0U;

    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim2,
        TIM_CHANNEL_1,
        TIM_INPUTCHANNELPOLARITY_RISING
    );

    if (HAL_TIM_IC_Start_IT(
            &htim2, TIM_CHANNEL_1) != HAL_OK)
    {
        vSemaphoreDelete(s_echoDone);
        s_echoDone = NULL;
        return false;
    }

    return true;
}

bool sensor_hcsr04_read(float *distance_cm)
{
    if ((distance_cm == NULL) ||
        (s_echoDone == NULL) ||
        ((TIM4->CR1 & TIM_CR1_CEN) == 0U))
    {
        return false;
    }

    /* Xoa thong bao cu neu co */
    (void)xSemaphoreTake(s_echoDone, 0U);

    taskENTER_CRITICAL();

    s_echoWidthUs = 0U;
    s_captureState = 1U;

    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim2,
        TIM_CHANNEL_1,
        TIM_INPUTCHANNELPOLARITY_RISING
    );

    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);

    taskEXIT_CRITICAL();

    /* Phat xung TRIG */
    APP_GPIO_Trig_Reset();
    prvDelayUs(3U);

    APP_GPIO_Trig_Set();
    prvDelayUs(12U);
    APP_GPIO_Trig_Reset();

    /* Cho ngat ECHO bao da do xong */
    if (xSemaphoreTake(
            s_echoDone,
            pdMS_TO_TICKS(HCSR04_TIMEOUT_MS) + 1U
        ) != pdTRUE)
    {
        taskENTER_CRITICAL();
        s_captureState = 0U;
        __HAL_TIM_SET_CAPTUREPOLARITY(
            &htim2,
            TIM_CHANNEL_1,
            TIM_INPUTCHANNELPOLARITY_RISING
        );
        taskEXIT_CRITICAL();
        return false;
    }

    uint16_t width = s_echoWidthUs;
    s_captureState = 0U;

    if ((width < HCSR04_MIN_ECHO_US) ||
        (width > HCSR04_MAX_ECHO_US))
    {
        return false;
    }

    *distance_cm = (float)width / 58.0f;
    return true;
}

/*
 * Ham nay duoc goi tu HAL callback cua TIM2.
 * KHONG tu dinh nghia HAL_TIM_IC_CaptureCallback
 * tai day de tranh trung callback voi Leader.
 */
void sensor_hcsr04_on_capture(TIM_HandleTypeDef *htim)
{
    if ((htim == NULL) ||
        (htim->Instance != TIM2) ||
        (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1) ||
        (s_echoDone == NULL))
    {
        return;
    }

    uint16_t captured =
        (uint16_t)HAL_TIM_ReadCapturedValue(
            htim, TIM_CHANNEL_1
        );

    if (s_captureState == 1U)
    {
        s_risingTick = captured;
        s_captureState = 2U;

        __HAL_TIM_SET_CAPTUREPOLARITY(
            htim,
            TIM_CHANNEL_1,
            TIM_INPUTCHANNELPOLARITY_FALLING
        );
    }
    else if (s_captureState == 2U)
    {
        s_echoWidthUs =
            (uint16_t)(captured - s_risingTick);

        s_captureState = 3U;

        __HAL_TIM_SET_CAPTUREPOLARITY(
            htim,
            TIM_CHANNEL_1,
            TIM_INPUTCHANNELPOLARITY_RISING
        );

        BaseType_t higherPriorityWoken = pdFALSE;

        xSemaphoreGiveFromISR(
            s_echoDone,
            &higherPriorityWoken
        );

        portYIELD_FROM_ISR(higherPriorityWoken);
    }
}
