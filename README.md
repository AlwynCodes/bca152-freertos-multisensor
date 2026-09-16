# Real-Time Multisensor Room Monitoring System

## Project Overview

BCA152 Microcontrollers Laboratory Activity No. 1.

The system is an ESP32 FreeRTOS-based room monitor simulated in Wokwi.

## Features

- Temperature measurement using DHT22
- Humidity measurement using DHT22
- Relative ambient-light measurement using LDR
- PIR motion detection
- OLED display
- Rotary encoder navigation
- Temperature alarm using buzzer
- ACTIVE and INACTIVE system states

## Learning Objectives

The project uses PlatformIO, ESP-IDF, native FreeRTOS APIs, queues, mutex protection, event signaling, periodic task execution, and a state machine.

## System Architecture

The application is divided into SensorTask, DisplayTask, InputTask, MotionTask, and AlarmTask.

## FreeRTOS Architecture

| Task | Priority | Responsibility |
|---|---:|---|
| SensorTask | 2 | DHT22 and LDR measurements every 2 s |
| DisplayTask | 1 | OLED ownership and selected measurement |
| InputTask | 3 | Rotary encoder navigation |
| MotionTask | 3 | PIR activity and ACTIVE/INACTIVE state |
| AlarmTask | 2 | Temperature alarm and buzzer |

## Hardware / Simulated Components

- ESP32
- DHT22
- Photoresistor / LDR
- PIR sensor
- Rotary encoder
- SSD1306 OLED
- Buzzer

## Pin Configuration

| Component | ESP32 |
|---|---|
| DHT22 | GPIO16 |
| LDR analog output | GPIO34 / ADC1_CH6 |
| PIR | GPIO27 |
| Encoder A | GPIO25 |
| Encoder B | GPIO26 |
| Encoder switch | GPIO33 |
| OLED SDA | GPIO21 |
| OLED SCL | GPIO22 |
| Buzzer | GPIO32 |

## Inter-Task Communication

Sensor data is communicated through FreeRTOS queues. A mutex protects shared Serial output. An event group signals ACTIVE, MOTION, and ALARM events.

## State Machine

The system supports ACTIVE and INACTIVE states.

- Motion returns the system to ACTIVE.
- The system enters INACTIVE after 15 seconds without motion.

## Repository Structure

```text
bca152-freertos-multisensor/
├── include/
├── lib/
├── src/
├── test/
├── docs/
├── platformio.ini
├── diagram.json
├── wokwi.toml
└── README.md
```

## Getting Started

Build the project with PlatformIO and run the Wokwi simulation.

## Building the Project

```text
pio run
```

## Running Unit Tests

```text
pio test
```

The project contains the required 13 deterministic unit tests.

## Static Code Analysis

```text
pio check
```

## Functional Verification

Record the actual observed results for FT-01 through FT-10 in the laboratory report.

## Engineering Decisions

Implementation follows the required ESP-IDF and native FreeRTOS architecture.

## Limitations

The relative light value is represented as 0–100% from the ADC reading. It is not presented as calibrated lux.

## Future Improvements

None are required for the laboratory implementation.

## References and Acknowledgments

Laboratory Activity No. 1, BCA152 Microcontrollers, Mindanao State University – Iligan Institute of Technology.
