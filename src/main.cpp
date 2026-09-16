#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "FREE_RTOS";

// Task A
void TaskA(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Task A running");

        // Block for 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task B
void TaskB(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Task B running");

        // Block for 2 seconds
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

extern "C" void app_main(void)
{
    xTaskCreate(
        TaskA,          // Task function
        "Task A",       // Task name
        2048,           // Stack size
        NULL,           // Parameter
        2,              // Priority
        NULL            // Task handle
    );

    xTaskCreate(
        TaskB,
        "Task B",
        2048,
        NULL,
        2,
        NULL
    );
}