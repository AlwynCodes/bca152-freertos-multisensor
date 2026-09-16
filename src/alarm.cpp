#include "alarm.h"
#include "config.h"

#ifdef PIO_UNIT_TESTING

TemperatureAlarmState evaluateTemperature(float temperature)
{
    if (temperature < LOW_TEMPERATURE_LIMIT) {
        return LOW_TEMPERATURE;
    }

    if (temperature > HIGH_TEMPERATURE_LIMIT) {
        return HIGH_TEMPERATURE;
    }

    return NORMAL;
}

#else

#include "rtos_objects.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TemperatureAlarmState evaluateTemperature(float temperature)
{
    if (temperature < LOW_TEMPERATURE_LIMIT) {
        return LOW_TEMPERATURE;
    }

    if (temperature > HIGH_TEMPERATURE_LIMIT) {
        return HIGH_TEMPERATURE;
    }

    return NORMAL;
}

void init_alarm(void)
{
    gpio_config_t config = {};
    config.pin_bit_mask = (1ULL << BUZZER_GPIO);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&config);

    gpio_set_level(
        static_cast<gpio_num_t>(BUZZER_GPIO),
        0
    );
}

void alarm_task(void *arg)
{
    (void)arg;

    SensorData data{};

    for (;;) {
        if (xQueueReceive(
                alarmQueue,
                &data,
                portMAX_DELAY
            ) == pdTRUE) {

            TemperatureAlarmState state =
                evaluateTemperature(data.temperature);

            gpio_set_level(
                static_cast<gpio_num_t>(BUZZER_GPIO),
                state == NORMAL ? 0 : 1
            );

            if (state == NORMAL) {
                xEventGroupClearBits(
                    systemEvents,
                    EVENT_ALARM
                );
            } else {
                xEventGroupSetBits(
                    systemEvents,
                    EVENT_ALARM
                );
            }
        }
    }
}

#endif