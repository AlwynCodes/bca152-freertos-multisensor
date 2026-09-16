#include "system_tasks.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

#ifndef PIO_UNIT_TESTING
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "BCA152 FreeRTOS Multisensor");
    ESP_LOGI(TAG, "System starting...");

    create_system_tasks();
}
#endif