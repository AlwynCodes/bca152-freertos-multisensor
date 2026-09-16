#include "system_tasks.h"
#include "config.h"
#include "sensors.h"
#include "display.h"
#include "input.h"
#include "motion.h"
#include "alarm.h"
#include "rtos_objects.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

static const char *TAG = "TASKS";

static void sensor_task(void *arg)
{
    (void)arg;

    SensorData data{};
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (read_sensors(&data)) {
            xQueueOverwrite(sensorQueue, &data);
            xQueueOverwrite(displayQueue, &data);
            xQueueOverwrite(alarmQueue, &data);

            if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
                ESP_LOGI(
                    TAG,
                    "Temperature: %.2f C, Humidity: %.2f %%, Light: %d %%",
                    data.temperature,
                    data.humidity,
                    data.lightLevel
                );
                xSemaphoreGive(serialMutex);
            }
        }

        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(SENSOR_PERIOD_MS)
        );
    }
}

static void display_task(void *arg)
{
    (void)arg;

    SensorData data{};
    DisplayMode mode = DISPLAY_TEMPERATURE;

    for (;;) {
        int direction = 0;

        if (xQueueReceive(inputQueue, &direction, 0) == pdTRUE) {
            if (direction > 0) {
                mode = nextDisplayMode(mode);
            } else if (direction < 0) {
                mode = previousDisplayMode(mode);
            }
        }

        if (xQueueReceive(
                displayQueue,
                &data,
                pdMS_TO_TICKS(100)
            ) == pdTRUE) {
            display_sensor_data(&data, mode);
        }
    }
}

void create_system_tasks(void)
{
    init_sensors();
    init_display();
    init_input();
    init_motion();
    init_alarm();

    sensorQueue = xQueueCreate(1, sizeof(SensorData));
    displayQueue = xQueueCreate(1, sizeof(SensorData));
    alarmQueue = xQueueCreate(1, sizeof(SensorData));
    inputQueue = xQueueCreate(4, sizeof(int));

    serialMutex = xSemaphoreCreateMutex();
    systemEvents = xEventGroupCreate();

    if (sensorQueue == nullptr ||
        displayQueue == nullptr ||
        alarmQueue == nullptr ||
        inputQueue == nullptr ||
        serialMutex == nullptr ||
        systemEvents == nullptr) {
        ESP_LOGE(TAG, "FreeRTOS object creation failed.");
        return;
    }

    xTaskCreate(
        sensor_task,
        "SensorTask",
        4096,
        nullptr,
        2,
        nullptr
    );

    xTaskCreate(
        display_task,
        "DisplayTask",
        4096,
        nullptr,
        1,
        nullptr
    );

    xTaskCreate(
        input_task,
        "InputTask",
        4096,
        nullptr,
        3,
        nullptr
    );

    xTaskCreate(
        motion_task,
        "MotionTask",
        4096,
        nullptr,
        3,
        nullptr
    );

    xTaskCreate(
        alarm_task,
        "AlarmTask",
        4096,
        nullptr,
        2,
        nullptr
    );
}
