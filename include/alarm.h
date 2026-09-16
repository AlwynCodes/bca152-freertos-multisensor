#ifndef ALARM_H
#define ALARM_H

#include "sensors.h"

typedef enum {
    NORMAL = 0,
    LOW_TEMPERATURE,
    HIGH_TEMPERATURE
} TemperatureAlarmState;

TemperatureAlarmState evaluateTemperature(float temperature);
void init_alarm(void);
void alarm_task(void *arg);

#endif
