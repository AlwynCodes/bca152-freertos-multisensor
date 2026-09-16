#include "sensors.h"
#include "config.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSORS";
static adc_oneshot_unit_handle_t adc1_handle = nullptr;

static bool dht22_read(float *temperature, float *humidity)
{
    if (temperature == nullptr || humidity == nullptr) {
        return false;
    }

    gpio_set_direction(static_cast<gpio_num_t>(DHT_GPIO), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(DHT_GPIO), 0);
    vTaskDelay(pdMS_TO_TICKS(2));
    gpio_set_level(static_cast<gpio_num_t>(DHT_GPIO), 1);

    gpio_set_direction(static_cast<gpio_num_t>(DHT_GPIO), GPIO_MODE_INPUT);

    // DHT22 timing is hardware-cycle based. This implementation is kept
    // in the sensor module so SensorTask only deals with SensorData.
    // Wokwi provides the DHT22 response on the configured GPIO.
    uint8_t data[5] = {};
    int bitCount = 0;

    int level = 0;
    TickType_t start = xTaskGetTickCount();

    while (gpio_get_level(static_cast<gpio_num_t>(DHT_GPIO)) == 1) {
        if (xTaskGetTickCount() - start > pdMS_TO_TICKS(1)) {
            return false;
        }
    }

    for (int i = 0; i < 80; ++i) {
        TickType_t begin = xTaskGetTickCount();

        while (gpio_get_level(static_cast<gpio_num_t>(DHT_GPIO)) == level) {
            if (xTaskGetTickCount() - begin > pdMS_TO_TICKS(1)) {
                return false;
            }
        }

        level = !level;
        (void)level;
    }

    // Use the ESP-IDF task timing resolution for the actual bit stream.
    // A GPIO-only DHT22 driver requires microsecond timing, so perform
    // the complete transaction with the ESP timer below.
    gpio_set_direction(static_cast<gpio_num_t>(DHT_GPIO), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(DHT_GPIO), 0);
    esp_rom_delay_us(1200);
    gpio_set_level(static_cast<gpio_num_t>(DHT_GPIO), 1);
    esp_rom_delay_us(30);
    gpio_set_direction(static_cast<gpio_num_t>(DHT_GPIO), GPIO_MODE_INPUT);

    uint32_t timeout = 100;
    while (gpio_get_level(static_cast<gpio_num_t>(DHT_GPIO)) == 1) {
        if (timeout-- == 0) return false;
        esp_rom_delay_us(1);
    }
    while (gpio_get_level(static_cast<gpio_num_t>(DHT_GPIO)) == 0) {
        if (timeout-- == 0) return false;
        esp_rom_delay_us(1);
    }
    while (gpio_get_level(static_cast<gpio_num_t>(DHT_GPIO)) == 1) {
        if (timeout-- == 0) return false;
        esp_rom_delay_us(1);
    }

    for (int byteIndex = 0; byteIndex < 5; ++byteIndex) {
        uint8_t value = 0;

        for (int bit = 0; bit < 8; ++bit) {
            timeout = 100;

            while (gpio_get_level(static_cast<gpio_num_t>(DHT_GPIO)) == 0) {
                if (timeout-- == 0) return false;
                esp_rom_delay_us(1);
            }

            uint32_t highTime = 0;
            while (gpio_get_level(static_cast<gpio_num_t>(DHT_GPIO)) == 1) {
                if (highTime++ > 100) return false;
                esp_rom_delay_us(1);
            }

            value <<= 1;
            if (highTime > 40) {
                value |= 1;
            }
        }

        data[byteIndex] = value;
        bitCount += 8;
    }

    if (bitCount != 40) {
        return false;
    }

    uint8_t checksum =
        static_cast<uint8_t>(
            data[0] + data[1] + data[2] + data[3]
        );

    if (checksum != data[4]) {
        return false;
    }

    int16_t rawHumidity =
        static_cast<int16_t>((data[0] << 8) | data[1]);

    int16_t rawTemperature =
        static_cast<int16_t>((data[2] << 8) | data[3]);

    *humidity = rawHumidity / 10.0f;

    if (rawTemperature & 0x8000) {
        rawTemperature &= 0x7FFF;
        *temperature = -(rawTemperature / 10.0f);
    } else {
        *temperature = rawTemperature / 10.0f;
    }

    return true;
}

void init_sensors(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {};
    init_config.unit_id = ADC_UNIT_1;
    init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
    init_config.ulp_mode = ADC_ULP_MODE_DISABLE;

    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {};
    config.atten = ADC_ATTEN_DB_12;
    config.bitwidth = ADC_BITWIDTH_DEFAULT;

    adc_oneshot_config_channel(
        adc1_handle,
        LDR_ADC_CHANNEL,
        &config
    );

    gpio_set_direction(
        static_cast<gpio_num_t>(DHT_GPIO),
        GPIO_MODE_OUTPUT
    );
    gpio_set_level(static_cast<gpio_num_t>(DHT_GPIO), 1);

    ESP_LOGI(TAG, "Sensor drivers initialized.");
}

bool read_sensors(SensorData *data)
{
    if (data == nullptr) {
        return false;
    }

    int raw_adc = 0;

    if (adc_oneshot_read(
            adc1_handle,
            LDR_ADC_CHANNEL,
            &raw_adc) != ESP_OK) {
        data->lightLevel = -1;
    } else {
        // The lab asks for a documented relative representation such
        // as 0-100%, not an unimplemented calibrated lux value.
        data->lightLevel = (raw_adc * 100) / 4095;
    }

    float temperature = 0.0f;
    float humidity = 0.0f;

    if (!dht22_read(&temperature, &humidity)) {
        return false;
    }

    data->temperature = temperature;
    data->humidity = humidity;

    return true;
}
