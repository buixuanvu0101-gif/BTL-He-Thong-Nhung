#ifndef SENSOR_DHT11_H
#define SENSOR_DHT11_H

#include <stdbool.h>

/*
 * Doc nhiet do va do am tu DHT11.
 * Tra ve true neu du lieu hop le.
 */
bool sensor_dht11_read(
    float *temperature_c,
    float *humidity_percent
);

#endif
