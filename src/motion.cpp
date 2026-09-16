#include "motion.h"
#include "config.h"
#include "rtos_objects.h"
#include "system_state.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MOTION";

void init_motion(void)
{
    gpio_config_t config = {};
    config.pin_bit_mask = (1ULL << PIR_GPIO);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&config);
}

void motion_task(void *arg)
{
    (void)arg;

    SystemState state = SYSTEM_INACTIVE;
    TickType_t lastMotionTick = xTaskGetTickCount();

    for (;;) {
        bool motionDetected =
            gpio_get_level(static_cast<gpio_num_t>(PIR_GPIO)) != 0;

        if (motionDetected) {
            lastMotionTick = xTaskGetTickCount();
        }

        uint32_t elapsedMs =
            pdTICKS_TO_MS(xTaskGetTickCount() - lastMotionTick);

        state = evaluateSystemState(
            state,
            motionDetected,
            elapsedMs
        );

        if (motionDetected) {
            xEventGroupSetBits(systemEvents, EVENT_MOTION);
        } else {
            xEventGroupClearBits(systemEvents, EVENT_MOTION);
        }

        if (state == SYSTEM_ACTIVE) {
            xEventGroupSetBits(systemEvents, EVENT_ACTIVE);
        } else {
            xEventGroupClearBits(systemEvents, EVENT_ACTIVE);
        }

        if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(
                TAG,
                "Motion: %s, State: %s",
                motionDetected ? "ACTIVE" : "INACTIVE",
                state == SYSTEM_ACTIVE ? "ACTIVE" : "INACTIVE"
            );
            xSemaphoreGive(serialMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
