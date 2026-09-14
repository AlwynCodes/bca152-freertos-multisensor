#pragma once

#include <stdbool.h>

// Pins & ADC Channels
#define DHT_PIN         16
#define LDR_ADC_CHANNEL 6  // GPIO 34 corresponds to ADC1 Channel 6

// Sensor Data Structure
typedef struct {
    float temperature;
    float humidity;
    int light_level;
    bool isValid;
} SensorData_t;

// Function Prototypes
void init_sensors(void);
bool read_sensors(SensorData_t *data);