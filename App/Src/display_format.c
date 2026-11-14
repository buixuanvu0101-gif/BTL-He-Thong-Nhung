
#include "display_format.h"

#include <stdio.h>
#include <stddef.h>

void display_format_data(
    const SystemState_t *pState,
    DisplayText_t *pText)
{
    if ((pState == NULL) || (pText == NULL))
    {
        return;
    }

    snprintf(
        pText->temperature,
        sizeof(pText->temperature),
        "Temp: %.1f C",
        (double)pState->sensor.temperature_c
    );

    snprintf(
        pText->humidity,
        sizeof(pText->humidity),
        "Humi: %.1f %%",
        (double)pState->sensor.humidity_percent
    );

    snprintf(
        pText->distance,
        sizeof(pText->distance),
        "Dist: %.1f cm",
        (double)pState->sensor.distance_cm
    );

    snprintf(
        pText->threshold,
        sizeof(pText->threshold),
        "Limit: %.1f cm",
        (double)pState->distance_threshold_cm
    );

    snprintf(
        pText->source,
        sizeof(pText->source),
        "Source: %s",
        pState->threshold_source ==
        THRESHOLD_SOURCE_PC ? "PC" : "POT"
    );

    snprintf(
        pText->alarm,
        sizeof(pText->alarm),
        "Status: %s",
        pState->alarm_flags != 0U
            ? "ALARM" : "NORMAL"
    );
}
