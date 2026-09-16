#ifndef DISPLAY_H
#define DISPLAY_H

#include "sensors.h"

typedef enum {
    DISPLAY_TEMPERATURE = 0,
    DISPLAY_HUMIDITY,
    DISPLAY_LIGHT,
    DISPLAY_MOTION
} DisplayMode;

DisplayMode nextDisplayMode(DisplayMode mode);
DisplayMode previousDisplayMode(DisplayMode mode);

void init_display(void);
void display_sensor_data(const SensorData *data, DisplayMode mode);

#endif
