#ifndef CONFIG_H
#define CONFIG_H

#define DHT_GPIO                 16
#define LDR_ADC_CHANNEL          ADC_CHANNEL_6   // GPIO34
#define PIR_GPIO                 27
#define ENCODER_A_GPIO           25
#define ENCODER_B_GPIO           26
#define ENCODER_BUTTON_GPIO      33
#define OLED_SDA_GPIO            21
#define OLED_SCL_GPIO            22
#define OLED_I2C_ADDRESS         0x3C
#define BUZZER_GPIO              32

#define SENSOR_PERIOD_MS         2000
#define INACTIVITY_TIMEOUT_MS    15000

#define LOW_TEMPERATURE_LIMIT    18.0f
#define HIGH_TEMPERATURE_LIMIT   30.0f

#endif
