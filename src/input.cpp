#include "input.h"
#include "config.h"
#include "display.h"
#include "rtos_objects.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void init_input(void)
{
    gpio_config_t config = {};
    config.pin_bit_mask =
        (1ULL << ENCODER_A_GPIO) |
        (1ULL << ENCODER_B_GPIO) |
        (1ULL << ENCODER_BUTTON_GPIO);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&config);
}

void input_task(void *arg)
{
    (void)arg;

    int previousA =
        gpio_get_level(static_cast<gpio_num_t>(ENCODER_A_GPIO));

    for (;;) {
        int currentA =
            gpio_get_level(static_cast<gpio_num_t>(ENCODER_A_GPIO));

        if (currentA != previousA) {
            int currentB =
                gpio_get_level(static_cast<gpio_num_t>(ENCODER_B_GPIO));

            int direction = (currentA == currentB) ? 1 : -1;
            xQueueSend(inputQueue, &direction, 0);

            previousA = currentA;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
