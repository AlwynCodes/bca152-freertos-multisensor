// src/system_tasks.cpp
#include "system_tasks.h"
#include "sensors.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void SensorTask(void *pvParameters) {
    SensorData_t data;
    init_sensors();

    for (;;) {
        if (read_sensors(&data)) {
            // Process or send data to FreeRTOS Queue
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}