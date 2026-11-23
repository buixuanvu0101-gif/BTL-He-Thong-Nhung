
#ifndef DISPLAY_FORMAT_H
#define DISPLAY_FORMAT_H

#include "app_cfg.h"

/* Do dai bo dem cho moi dong */
#define DISPLAY_LINE_LENGTH 24U

/* Du lieu chuoi de hien thi OLED */
typedef struct
{
    char temperature[DISPLAY_LINE_LENGTH];
    char humidity[DISPLAY_LINE_LENGTH];
    char distance[DISPLAY_LINE_LENGTH];
    char threshold[DISPLAY_LINE_LENGTH];
    char source[DISPLAY_LINE_LENGTH];
    char alarm[DISPLAY_LINE_LENGTH];
} DisplayText_t;

/* Chuyen SystemState thanh chuoi */
void display_format_data(
    const SystemState_t *pState,
    DisplayText_t *pText
);

#endif
