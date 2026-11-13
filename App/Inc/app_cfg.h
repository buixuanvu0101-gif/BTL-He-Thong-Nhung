
#ifndef APP_CFG_H
#define APP_CFG_H

#include <stdint.h>

/* Nguon nguong canh bao */
typedef enum
{
    THRESHOLD_SOURCE_POT = 0,
    THRESHOLD_SOURCE_PC = 1
} ThresholdSource_t;

/* Du lieu cam bien */
typedef struct
{
    float temperature_c;
    float humidity_percent;
    float distance_cm;
    uint16_t potentiometer_raw;
} SensorData_t;

/* Trang thai he thong */
typedef struct
{
    SensorData_t sensor;
    float distance_threshold_cm;
    ThresholdSource_t threshold_source;
    uint32_t alarm_flags;
} SystemState_t;

/* Bit canh bao */
#define ALARM_DISTANCE (1UL << 0)

/* Su kien nut bam */
typedef enum
{
    UI_EVENT_BUTTON_SHORT = 0,
    UI_EVENT_BUTTON_LONG = 1
} UiEvent_t;

#define DISPLAY_PERIOD_MS 200U

#endif
