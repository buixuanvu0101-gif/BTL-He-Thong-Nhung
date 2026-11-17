#ifndef TASK_SENSOR_H
#define TASK_SENSOR_H

#include "FreeRTOS.h"
#include <stdint.h>

/* Tao FreeRTOS Sensor Task */
typedef struct
{
    uint32_t hcsr04_init_fail;
    uint32_t dht11_read_fail;
    uint32_t hcsr04_read_fail;
    uint32_t tim4_start_fail;
    uint32_t queue_send_fail;
    uint32_t samples_sent;
} SensorTaskStats_t;

BaseType_t task_sensor_create(void);

/* Goi tu Task, khong goi tu ISR */
void task_sensor_get_stats(SensorTaskStats_t *out);

#endif
