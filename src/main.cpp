#include "system_tasks.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

extern "C" __attribute__((weak)) void app_main(void)
{
    ESP_LOGI(TAG, "BCA152 FreeRTOS Multisensor");
    ESP_LOGI(TAG, "System starting...");

    create_system_tasks();
}