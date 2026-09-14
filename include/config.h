#pragma once

// Hardware GPIO Pins
#define DHT_PIN         16
#define LDR_PIN         34
#define PIR_PIN         17
#define OLED_SDA_PIN    21
#define OLED_SCL_PIN    22

// FreeRTOS Task Parameters
#define TASK_STACK_SIZE 2048
#define SENSOR_TASK_PRIO 2
#define DISPLAY_TASK_PRIO 1