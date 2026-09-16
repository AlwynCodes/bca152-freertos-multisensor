#include "system_tasks.h"
#include "esp_log.h"
#include <cstdio>

static const char *TAG = "MAIN";

extern "C" __attribute__((weak)) void app_main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0); // Disable stdout buffering
    
    ESP_LOGI(TAG, "BCA152 FreeRTOS Multisensor");
    ESP_LOGI(TAG, "System starting...");

    create_system_tasks();
}