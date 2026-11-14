
#include "bsp_oled.h"
#include "stm32f1xx_hal.h"

#include <stddef.h>
#include <string.h>

/* I2C1 do CubeMX khoi tao */
extern I2C_HandleTypeDef hi2c1;

#define OLED_ADDR  (OLED_I2C_ADDRESS << 1)
#define OLED_TIMEOUT_MS  50U
#define OLED_PAGES (OLED_HEIGHT / 8U)

/* 1KB framebuffer */
static uint8_t oled_buffer[OLED_PAGES][OLED_WIDTH];
static uint8_t oled_ready = 0U;

/*
 * Font 5x7: A-Z, 0-9 va ky tu thong dung.
 * Chu thuong duoc chuyen thanh chu hoa.
 */
static const uint8_t font_digits[10][5] =
{
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}  /* 9 */
};

static const uint8_t font_letters[26][5] =
{
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x7F,0x20,0x18,0x20,0x7F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x03,0x04,0x78,0x04,0x03}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}  /* Z */
};

static uint8_t prvSendCommand(uint8_t command)
{
    uint8_t data[2] = {0x00U, command};

    return (HAL_I2C_Master_Transmit(
        &hi2c1,
        OLED_ADDR,
        data,
        sizeof(data),
        OLED_TIMEOUT_MS
    ) == HAL_OK) ? 1U : 0U;
}

static const uint8_t *prvGetGlyph(char ch)
{
    static const uint8_t blank[5] =
        {0U,0U,0U,0U,0U};

    static const uint8_t dot[5] =
        {0U,0x60U,0x60U,0U,0U};

    static const uint8_t colon[5] =
        {0U,0x36U,0x36U,0U,0U};

    static const uint8_t dash[5] =
        {0x08U,0x08U,0x08U,0x08U,0x08U};

    static const uint8_t percent[5] =
        {0x63U,0x13U,0x08U,0x64U,0x63U};

    static const uint8_t slash[5] =
        {0x20U,0x10U,0x08U,0x04U,0x02U};

    if (ch >= 'a' && ch <= 'z')
    {
        ch = (char)(ch - 'a' + 'A');
    }

    if (ch >= '0' && ch <= '9')
    {
        return font_digits[ch - '0'];
    }

    if (ch >= 'A' && ch <= 'Z')
    {
        return font_letters[ch - 'A'];
    }

    switch (ch)
    {
        case '.': return dot;
        case ':': return colon;
        case '-': return dash;
        case '%': return percent;
        case '/': return slash;
        default:  return blank;
    }
}

/* Khoi tao SSD1306 */
uint8_t bsp_oled_init(void)
{
    static const uint8_t init_cmds[] =
    {
        0xAEU,       /* Display OFF */
        0xD5U, 0x80U,
        0xA8U, 0x3FU, /* MUX 64 */
        0xD3U, 0x00U,
        0x40U,
        0x8DU, 0x14U, /* Charge pump */
        0x20U, 0x02U, /* Page addressing */
        0xA1U,
        0xC8U,
        0xDAU, 0x12U,
        0x81U, 0x7FU,
        0xD9U, 0xF1U,
        0xDBU, 0x40U,
        0xA4U,
        0xA6U,
        0xAFU        /* Display ON */
    };

    oled_ready = 0U;

    if (HAL_I2C_IsDeviceReady(
            &hi2c1,
            OLED_ADDR,
            2U,
            OLED_TIMEOUT_MS) != HAL_OK)
    {
        return 0U;
    }

    for (size_t i = 0U;
         i < sizeof(init_cmds);
         i++)
    {
        if (prvSendCommand(init_cmds[i]) == 0U)
        {
            return 0U;
        }
    }

    memset(oled_buffer, 0, sizeof(oled_buffer));
    oled_ready = 1U;

    bsp_oled_update();

    return 1U;
}

/* Xoa framebuffer */
void bsp_oled_clear(void)
{
    memset(oled_buffer, 0, sizeof(oled_buffer));
}

/* Ghi chuoi vao mot dong 8 pixel */
void bsp_oled_write_line(
    uint8_t row,
    const char *text)
{
    if ((row >= OLED_PAGES) || (text == NULL))
    {
        return;
    }

    memset(oled_buffer[row], 0, OLED_WIDTH);

    uint8_t col = 0U;

    while ((*text != '\0') &&
           ((uint16_t)col + 6U <= OLED_WIDTH))
    {
        const uint8_t *glyph =
            prvGetGlyph(*text);

        for (uint8_t i = 0U; i < 5U; i++)
        {
            oled_buffer[row][col++] = glyph[i];
        }

        /* Khoang cach giua hai ky tu */
        oled_buffer[row][col++] = 0U;

        text++;
    }
}

/* Gui framebuffer ra OLED */
void bsp_oled_update(void)
{
    if (oled_ready == 0U)
    {
        return;
    }

    uint8_t packet[OLED_WIDTH + 1U];

    packet[0] = 0x40U; /* Data stream */

    for (uint8_t page = 0U;
         page < OLED_PAGES;
         page++)
    {
        if (prvSendCommand(
                (uint8_t)(0xB0U | page)) == 0U)
        {
            return;
        }

        if (prvSendCommand(0x00U) == 0U)
        {
            return;
        }

        if (prvSendCommand(0x10U) == 0U)
        {
            return;
        }

        memcpy(
            &packet[1],
            oled_buffer[page],
            OLED_WIDTH
        );

        if (HAL_I2C_Master_Transmit(
                &hi2c1,
                OLED_ADDR,
                packet,
                sizeof(packet),
                OLED_TIMEOUT_MS) != HAL_OK)
        {
            return;
        }
    }
}
