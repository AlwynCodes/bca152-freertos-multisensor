#ifndef SENSORS_H
#define SENSORS_H

#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
    int lightLevel;
    bool motionDetected;
} SensorData;

void init_sensors(void);
bool read_sensors(SensorData *data);

#endif
