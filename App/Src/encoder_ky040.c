#include "encoder_ky040.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>

#define ENC_CLK_PIN GPIO_PIN_2
#define ENC_DT_PIN  GPIO_PIN_3
#define ENC_SW_PIN  GPIO_PIN_4
#define ENC_PORT    GPIOA

#define ENC_DEBOUNCE_MS 30U

static uint8_t s_prevAB = 0;
static int8_t s_count = 0;
static int16_t s_pending = 0;

static uint8_t s_lastSW = 0;
static uint8_t s_stableSW = 0;
static uint32_t s_lastChange = 0;
static uint8_t s_initialized = 0;

/* Bang giai ma hai kenh CLK va DT */
static const int8_t s_table[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

/* Doc trang thai CLK va DT */
static uint8_t prvReadAB(void)
{
    uint8_t clk =
        HAL_GPIO_ReadPin(ENC_PORT, ENC_CLK_PIN)
        == GPIO_PIN_SET;

    uint8_t dt =
        HAL_GPIO_ReadPin(ENC_PORT, ENC_DT_PIN)
        == GPIO_PIN_SET;

    return (uint8_t)((clk << 1U) | dt);
}

/* Nut nhan SW co muc LOW khi nhan */
static uint8_t prvReadSW(void)
{
    return HAL_GPIO_ReadPin(ENC_PORT, ENC_SW_PIN)
           == GPIO_PIN_RESET;
}

/* Khoi tao Encoder */
void encoder_ky040_init(void)
{
    s_prevAB = prvReadAB();
    s_count = 0;
    s_pending = 0;

    s_lastSW = prvReadSW();
    s_stableSW = s_lastSW;
    s_lastChange = HAL_GetTick();

    s_initialized = 1;
}

/* Doc va giai ma Encoder */
EncoderEvent_t encoder_ky040_poll(void)
{
    if (!s_initialized)
    {
        return ENCODER_NONE;
    }

    uint8_t currentAB = prvReadAB();

    if (currentAB != s_prevAB)
    {
        uint8_t index =
            (uint8_t)((s_prevAB << 2U) | currentAB);

        int8_t change = s_table[index];

        if (change == 0)
        {
            s_count = 0;
        }
        else
        {
            s_count += change;

            if (s_count >= 4)
            {
                if (s_pending < 1000)
                    s_pending++;

                s_count = 0;
            }
            else if (s_count <= -4)
            {
                if (s_pending > -1000)
                    s_pending--;

                s_count = 0;
            }
        }

        s_prevAB = currentAB;
    }

    /* Chong doi nut nhan */
    uint32_t now = HAL_GetTick();
    uint8_t rawSW = prvReadSW();

    if (rawSW != s_lastSW)
    {
        s_lastSW = rawSW;
        s_lastChange = now;
    }

    if ((uint32_t)(now - s_lastChange)
        >= ENC_DEBOUNCE_MS)
    {
        if (rawSW != s_stableSW)
        {
            s_stableSW = rawSW;

            if (s_stableSW)
            {
                return ENCODER_PRESSED;
            }
        }
    }

    /* Tra ve su kien xoay */
    if (s_pending > 0)
    {
        s_pending--;
        return ENCODER_CW;
    }

    if (s_pending < 0)
    {
        s_pending++;
        return ENCODER_CCW;
    }

    return ENCODER_NONE;
}
