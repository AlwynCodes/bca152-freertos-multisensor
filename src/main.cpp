// src/main.cpp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "system_tasks.h"

extern "C" void app_main(void) {
    // 1. Initialize Inter-Task Communication (Queues/Mutexes)
    
    // 2. Launch FreeRTOS Tasks
    xTaskCreatePinToCore(SensorTask, "SensorTask", TASK_STACK_SIZE, NULL, SENSOR_TASK_PRIO, NULL, 0);
}