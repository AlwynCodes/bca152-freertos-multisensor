#ifndef RTOS_OBJECTS_H
#define RTOS_OBJECTS_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "sensors.h"

#define EVENT_ACTIVE   BIT0
#define EVENT_MOTION   BIT1
#define EVENT_ALARM    BIT2

extern QueueHandle_t sensorQueue;
extern QueueHandle_t displayQueue;
extern QueueHandle_t alarmQueue;

extern QueueHandle_t inputQueue;

extern SemaphoreHandle_t serialMutex;
extern EventGroupHandle_t systemEvents;

#endif
