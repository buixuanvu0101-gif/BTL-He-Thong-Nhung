#include "hal_buzzer.h"
#include "tim.h"

#define BUZ_MIN_HZ  2000U
#define BUZ_MAX_HZ  4000U

static uint16_t s_freq = 2700U;
static uint8_t  s_duty = 50U;
static uint8_t  s_on   = 0U;

static void apply(void)
{
    /* APB1 prescaler = /2 nên clock timer = 2 x PCLK1 = 72 MHz */
    uint32_t tick_hz = (HAL_RCC_GetPCLK1Freq() * 2U)
                     / ((uint32_t)htim3.Init.Prescaler + 1U);
    uint32_t arr = (tick_hz / s_freq) - 1U;
    if (arr > 0xFFFFU) { arr = 0xFFFFU; }
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ((arr + 1U) * s_duty) / 100U);
}

void Buzzer_Init(void) { s_on = 0U; apply(); }

void Buzzer_On(void)
{
    if (!s_on) {
        apply();
        (void)HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        s_on = 1U;
    }
}

void Buzzer_Off(void)
{
    if (s_on) {
        (void)HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
        s_on = 0U;
    }
}

void Buzzer_SetFreq(uint16_t hz)
{
    if (hz < BUZ_MIN_HZ) { hz = BUZ_MIN_HZ; }
    if (hz > BUZ_MAX_HZ) { hz = BUZ_MAX_HZ; }
    s_freq = hz;
    apply();
}

void Buzzer_SetDuty(uint8_t percent)
{
    if (percent > 100U) { percent = 100U; }
    s_duty = percent;
    apply();
}

void Buzzer_Write(uint8_t on)
{
    if (on) { Buzzer_On(); } else { Buzzer_Off(); }
}