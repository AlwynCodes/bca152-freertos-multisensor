#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

extern "C" void app_main(void) {
    // Print required initialization messages
    printf("BCA152 FreeRTOS Multisensor\n");
    printf("System starting...\n");
    
    ESP_LOGI(TAG, "System initialization complete.");
}