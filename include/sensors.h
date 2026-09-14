#pragma once

typedef struct {
    float temperature;
    float humidity;
    int light_level;
    bool motion_detected;
} SensorData_t;

void init_sensors(void);
bool read_sensors(SensorData_t *data);