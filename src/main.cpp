#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sensors.h"

static const char *TAG = "MAIN";

void SensorTask(void *pvParameters) {
    SensorData_t sensorData;
    init_sensors();

    for (;;) {
        if (read_sensors(&sensorData)) {
            ESP_LOGI(TAG, "Temp: %.1f C | Hum: %.1f %% | Light ADC: %d",
                     sensorData.temperature, sensorData.humidity, sensorData.light_level);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting Sensor Task...");
    xTaskCreatePinToCore(SensorTask, "SensorTask", 3072, NULL, 2, NULL, 0);
}