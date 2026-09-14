#include "sensors.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "SENSORS";
static adc_oneshot_unit_handle_t adc1_handle;

void init_sensors(void) {
    // Configure ADC for LDR on GPIO 34
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc1_handle, (adc_channel_t)LDR_ADC_CHANNEL, &config);

    ESP_LOGI(TAG, "Sensor drivers initialized.");
}

bool read_sensors(SensorData_t *data) {
    if (!data) return false;

    // Read LDR raw analog value
    int raw_adc = 0;
    if (adc_oneshot_read(adc1_handle, (adc_channel_t)LDR_ADC_CHANNEL, &raw_adc) == ESP_OK) {
        data->light_level = raw_adc;
    } else {
        data->light_level = -1;
    }

    // Simulated/Placeholder DHT readings for now
    data->temperature = 25.5f;
    data->humidity = 60.0f;
    data->isValid = true;

    return true;
}