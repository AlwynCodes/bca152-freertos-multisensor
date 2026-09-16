#include "rtos_objects.h"

QueueHandle_t sensorQueue = nullptr;
QueueHandle_t displayQueue = nullptr;
QueueHandle_t alarmQueue = nullptr;
QueueHandle_t inputQueue = nullptr;

SemaphoreHandle_t serialMutex = nullptr;
EventGroupHandle_t systemEvents = nullptr;
