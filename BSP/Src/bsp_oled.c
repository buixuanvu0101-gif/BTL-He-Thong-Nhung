
#include "bsp_oled.h"
#include "hal_i2c.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define OLED_PAGES \
    (OLED_HEIGHT / 8U)

#define OLED_TIMEOUT_MS 50U

/* 128 x 64 / 8 = 1024 bytes */
static uint8_t oled_buffer[OLED_PAGES][OLED_WIDTH];

static uint8_t oled_ready = 0U;

/* ==========================================
 * FONT 5x7 - DIGITS
 * ========================================== */

static const uint8_t font_digits[10][5] =
{
    {0x3E,0x51,0x49,0x45,0x3E},
    {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30},
    {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E}
};

/* ==========================================
 * FONT 5x7 - LETTERS A-Z
 * ========================================== */

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

/* ==========================================
 * I2C COMMAND
 * ========================================== */

static uint8_t prvSendCommand(uint8_t command)
{
    uint8_t packet[2];

    packet[0] = 0x00U;
    packet[1] = command;

    return (hal_i2c_write(
        OLED_I2C_ADDRESS,
        packet,
        sizeof(packet),
        OLED_TIMEOUT_MS
    ) == DEV2_I2C_OK) ? 1U : 0U;
}

/* ==========================================
 * FONT LOOKUP
 * ========================================== */

static const uint8_t *prvGetGlyph(char ch)
{
    static const uint8_t blank[5] =
        {0,0,0,0,0};

    static const uint8_t dot[5] =
        {0,0x60,0x60,0,0};

    static const uint8_t colon[5] =
        {0,0x36,0x36,0,0};

    static const uint8_t dash[5] =
        {0x08,0x08,0x08,0x08,0x08};

    static const uint8_t percent[5] =
        {0x63,0x13,0x08,0x64,0x63};

    static const uint8_t slash[5] =
        {0x20,0x10,0x08,0x04,0x02};

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

/* ==========================================
 * OLED INITIALIZATION
 * ========================================== */

uint8_t bsp_oled_init(void)
{
    static const uint8_t init_cmds[] =
    {
        0xAEU,
        0xD5U, 0x80U,
        0xA8U, 0x3FU,
        0xD3U, 0x00U,
        0x40U,
        0x8DU, 0x14U,
        0x20U, 0x02U,
        0xA1U,
        0xC8U,
        0xDAU, 0x12U,
        0x81U, 0x7FU,
        0xD9U, 0xF1U,
        0xDBU, 0x40U,
        0xA4U,
        0xA6U,
        0xAFU
    };

    oled_ready = 0U;

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

    if (bsp_oled_update() == 0U)
    {
        oled_ready = 0U;
        return 0U;
    }

    return 1U;
}

/* ==========================================
 * CLEAR FRAMEBUFFER
 * ========================================== */

void bsp_oled_clear(void)
{
    memset(oled_buffer, 0, sizeof(oled_buffer));
}

/* ==========================================
 * WRITE TEXT LINE
 * ========================================== */

void bsp_oled_write_line(
    uint8_t row,
    const char *text)
{
    if ((row >= OLED_PAGES) ||
        (text == NULL))
    {
        return;
    }

    memset(
        oled_buffer[row],
        0,
        OLED_WIDTH
    );

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

        oled_buffer[row][col++] = 0U;

        text++;
    }
}

/* ==========================================
 * TRANSFER ONE PAGE (bounded I2C work)
 * ========================================== */

uint8_t bsp_oled_update_page(uint8_t page)
{
    if ((oled_ready == 0U) || (page >= OLED_PAGES))
    {
        return 0U;
    }

    if ((prvSendCommand((uint8_t)(0xB0U | page)) == 0U) ||
        (prvSendCommand(0x00U) == 0U) ||
        (prvSendCommand(0x10U) == 0U))
    {
        oled_ready = 0U;
        return 0U;
    }

    uint8_t packet[OLED_WIDTH + 1U];
    packet[0] = 0x40U;
    memcpy(&packet[1], oled_buffer[page], OLED_WIDTH);

    if (hal_i2c_write(OLED_I2C_ADDRESS, packet, sizeof(packet),
                      OLED_TIMEOUT_MS) != DEV2_I2C_OK)
    {
        oled_ready = 0U;
        return 0U;
    }
    return 1U;
}

uint8_t bsp_oled_update(void)
{
    if (oled_ready == 0U) { return 0U; }
    for (uint8_t page = 0U; page < OLED_PAGES; ++page)
    {
        if (bsp_oled_update_page(page) == 0U) { return 0U; }
    }
    return 1U;
}
