#ifndef ENCODER_KY040_H
#define ENCODER_KY040_H

typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_CW,
    ENCODER_CCW,
    ENCODER_PRESSED
} EncoderEvent_t;

/* Khoi tao trang thai Encoder */
void encoder_ky040_init(void);

/* Doc Encoder va tra ve su kien */
EncoderEvent_t encoder_ky040_poll(void);

#endif
