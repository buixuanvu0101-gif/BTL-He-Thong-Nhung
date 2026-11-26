#include "display_format.h"

#include <stdio.h>
#include <stddef.h>

static int to_tenths(float v)
{
    return (int)(v * 10.0f + (v >= 0.0f ? 0.5f : -0.5f));
}

void display_format_data(
    const SystemState_t *pState,
    DisplayText_t *pText)
{
    if (pState == NULL || pText == NULL)
        return;

    if (pState->sensor.temperature_c < 0.0f)
    {
        snprintf(pText->temperature,
                 sizeof(pText->temperature), "Temp: ERR");
    }
    else
    {
        int t = to_tenths(pState->sensor.temperature_c);
        snprintf(pText->temperature,
                 sizeof(pText->temperature),
                 "Temp: %d.%d C", t / 10, t % 10);
    }

    if (pState->sensor.humidity_percent < 0.0f)
    {
        snprintf(pText->humidity,
                 sizeof(pText->humidity), "Humi: ERR");
    }
    else
    {
        int h = to_tenths(pState->sensor.humidity_percent);
        snprintf(pText->humidity,
                 sizeof(pText->humidity),
                 "Humi: %d.%d %%", h / 10, h % 10);
    }

    int d = to_tenths(pState->sensor.distance_cm);
    snprintf(pText->distance, sizeof(pText->distance),
             "Dist: %d.%d cm", d / 10, d % 10);

    int limit = to_tenths(pState->distance_threshold_cm);
    snprintf(pText->threshold, sizeof(pText->threshold),
             "Limit: %d.%d cm", limit / 10, limit % 10);

    snprintf(pText->source, sizeof(pText->source),
             "Source: %s",
             pState->threshold_source == THRESHOLD_SOURCE_PC
                 ? "PC" : "ENC");

    snprintf(pText->alarm, sizeof(pText->alarm),
             "Status: %s",
             pState->alarm_flags != 0U
                 ? "ALARM" : "NORMAL");
}
