# BCA152 FreeRTOS Multisensor

## Project Overview

A room-monitoring firmware for the ESP32, built on top of FreeRTOS. It gathers temperature, humidity, and light readings, presents them on a compact OLED that the user pages through with a rotary encoder, triggers a buzzer whenever the temperature crosses out of a safe band, and blanks the screen automatically once the room has been idle for a while.

The aim behind this build wasn't just to produce a functioning sensor box. It was to genuinely work with FreeRTOS the way it's intended — independent tasks handling independent jobs, queues ferrying data between them, an event group broadcasting state changes, and a mutex guarding the one piece of shared state more than one task needs to touch.

## Features

- Pulls temperature and humidity from a DHT22 and light level from an LDR, both on a 2-second cadence.
- Displays readings on a 128x64 OLED, one at a time, and lets the user step between Temperature, Humidity, Light, and Motion pages using a rotary encoder.
- Emits a buzzer tone and tags the display with a `*` whenever temperature rises above 30°C or drops below 18°C.
- Uses a PIR motion sensor to determine room occupancy. After 15 seconds of no movement, the OLED goes dark. Sensing continues silently underneath, and the display returns the instant motion is picked up again.

## Learning Objectives

This project was built to demonstrate and practice the following embedded systems concepts:

1. Creating an ESP32 project using PlatformIO with the ESP-IDF framework.
2. Constructing and simulating a microcontroller circuit in Wokwi.
3. Interfacing sensors and actuators with an ESP32.
4. Organizing firmware into multiple source modules.
5. Creating and managing FreeRTOS tasks.
6. Assigning and justifying task priorities.
7. Distinguishing Running, Ready, Blocked, Suspended, and Deleted task states.
8. Using queues for inter-task communication.
9. Using a mutex to protect a shared resource.
10. Using an event group for event signaling.
11. Implementing periodic execution using `vTaskDelayUntil()`.
12. Implementing a simple embedded-system state machine.
13. Separating hardware-independent decision logic from hardware drivers.
14. Writing and executing automated unit tests using PlatformIO.
15. Performing static code analysis using PlatformIO.
16. Using Git incrementally and maintaining a professional GitHub repository.
17. Documenting a project for both academic assessment and public portfolio use.

## System Architecture

The firmware is organized around five FreeRTOS tasks that never communicate with each other directly. Anything that needs to cross a task boundary goes through a queue or the event group instead.

![Architecture diagram](docs/architecture-diagram.svg)

On the hardware side, the build centers on an ESP32 Dev Module alongside a DHT22 for temperature and humidity, an LDR for ambient light, an SSD1306 128x64 OLED connected over I2C, a KY-040 rotary encoder, a passive buzzer driven with LEDC PWM, and a PIR module for motion. Every component is simulated in Wokwi — no physical board was involved.

## Circuit

![Wokwi circuit](docs/images/wokwi-circuit.png)

The DHT22 communicates over a single data line on GPIO4. The LDR is wired to GPIO36, one of the ESP32's ADC1 input-only pins — the correct choice for analog sensing. The OLED communicates over I2C using GPIO18 (SDA) and GPIO19 (SCL). The rotary encoder is assigned GPIO32 and GPIO33 for CLK and DT; those pins were deliberately kept apart from the I2C lines after a conflict arose during earlier development. The buzzer is driven with LEDC PWM on GPIO25 rather than a plain digital pin, since a passive element needs an oscillating signal to produce sound. The PIR sensor occupies GPIO26.

## FreeRTOS Architecture

The five tasks and their priorities:

- **AlarmTask** — priority 3 (highest) — event-driven from `alarmQueue` — evaluates temperature and drives the buzzer.
- **SensorTask** — priority 2 — 2-second cycle via `vTaskDelayUntil` — reads DHT22 and LDR, pushes to `sensorQueue` and `alarmQueue`.
- **InputTask** — priority 2 — ISR-driven from the encoder — updates `modeQueue` via `xQueueOverwrite`.
- **MotionTask** — priority 2 — 500 ms poll — reads PIR, updates the event group, manages ACTIVE/INACTIVE.
- **DisplayTask** — priority 1 (lowest) — 200 ms poll — owns the OLED, renders the current mode, blanks on INACTIVE.

Inter-task communication:

- `sensorQueue` (length 5) — SensorTask → DisplayTask
- `alarmQueue` (length 5) — SensorTask → AlarmTask
- `modeQueue` (length 1, overwrite) — InputTask → DisplayTask
- `systemEvents` (event group) — `EVENT_ACTIVE` (bit 0), `EVENT_MOTION` (bit 1), `EVENT_ALARM` (bit 2)
- `serialMutex` — protects `printf` / stdout across DisplayTask, AlarmTask, MotionTask

## Hardware / Simulated Components

- ESP32 Dev Module
- DHT22 temperature and humidity sensor
- LDR (photoresistor) for light level
- SSD1306 128x64 OLED display (I2C)
- KY-040 rotary encoder
- Passive buzzer
- PIR motion sensor

All components are simulated in Wokwi.

## Pin Configuration

- **DHT22 data** — GPIO 4
- **LDR analog out** — GPIO 36 (VP)
- **OLED SDA** — GPIO 18
- **OLED SCL** — GPIO 19
- **Buzzer** — GPIO 25 (LEDC PWM)
- **Encoder CLK** — GPIO 32
- **Encoder DT** — GPIO 33
- **PIR OUT** — GPIO 26

The DHT22 relies on a single data pin. The LDR sits on GPIO36, one of the ESP32's ADC1 input-only pins — the correct choice for analog sensing. The OLED communicates over I2C. The encoder pins were intentionally separated from the I2C lines after a conflict came up earlier in development. The buzzer runs on LEDC PWM rather than a plain digital output, because a passive buzzer requires an oscillating signal to produce sound.

## Task Design

**SensorTask** (priority 2) samples the DHT22 and LDR every 2 seconds and publishes the combined reading into two separate queues — one destined for the display, the other for the alarm logic — because a FreeRTOS queue delivers each item to a single receiver.

**DisplayTask** (priority 1) is the sole owner of the OLED. No other task writes to the screen directly. It pulls from the sensor queue, inspects the currently selected display mode, and consults the event group to decide whether the system should be showing anything at all.

**InputTask** (priority 2) reads the rotary encoder through an interrupt handler and deposits the current display mode into a single-item queue via `xQueueOverwrite`, since only the newest selection matters.

**AlarmTask** (priority 3, the top priority in the system) monitors temperature and drives the buzzer. It's given the highest priority because each wake-up does very little work, so the cost of that priority is nearly nothing — yet a genuine temperature problem is handled immediately rather than waiting behind a display refresh.

**MotionTask** (priority 2) polls the PIR every 500 ms and determines whether the system should be considered ACTIVE or INACTIVE, setting and clearing bits in a shared event group that other tasks can inspect without needing a direct handle to MotionTask itself.

## Inter-Task Communication

Every exchange between tasks runs through either a FreeRTOS queue or the event group. No shared global variables are used for passing data.

- **Queue**: Sensor readings travel from SensorTask to both DisplayTask and AlarmTask through two independent queues (a single FreeRTOS queue can only hand each item to one receiver).
- **Queue with overwrite**: The selected display mode lives in a length-1 queue that InputTask writes with `xQueueOverwrite` — only the newest mode matters, and DisplayTask reads it with `xQueuePeek` so the value isn't consumed.
- **Event group**: `systemEvents` holds bit flags for state signaling — `EVENT_ACTIVE`, `EVENT_MOTION`, and `EVENT_ALARM` — set and cleared by producer tasks and read non-destructively by consumers.
- **Mutex**: `serialMutex` guards the shared serial output, since DisplayTask, AlarmTask, and MotionTask can all print at unpredictable times, and serial writes aren't inherently safe when issued from multiple tasks at once.

## State Machine

![State machine diagram](docs/state-machine-diagram.svg)

The system carries two states, ACTIVE and INACTIVE, both tracked entirely by MotionTask. PIR motion keeps the system in ACTIVE and resets the idle timer. After 15 seconds without motion, the system moves to INACTIVE and the OLED goes dark. The moment the PIR fires again, the system jumps straight back to ACTIVE. Only PIR motion counts — rotating the encoder does not reset the timer, since the spec defines activity as room occupancy rather than user input.

## Repository Structure

```
bca152-freertos-multisensor/
├── include/              header files
├── lib/dht22/            custom ESP-IDF DHT22 bit-bang driver
├── src/                  task and driver source files
│   ├── main.cpp
│   ├── display.cpp
│   ├── input.cpp
│   ├── alarm.cpp
│   ├── motion.cpp
│   ├── rtos_objects.cpp
│   ├── temperature_logic.cpp
│   ├── display_logic.cpp
│   └── motion_logic.cpp
├── test/                 native unit tests (Unity framework)
│   ├── test_temperature/
│   ├── test_display/
│   └── test_motion/
├── docs/
│   ├── architecture-diagram.svg
│   ├── state-machine-diagram.svg
│   ├── functional-verification.md
│   ├── fault-experiments.md
│   └── images/
├── diagram.json          Wokwi circuit
├── wokwi.toml            Wokwi configuration
├── platformio.ini        PlatformIO configuration
└── README.md
```

## Screenshots

**OLED showing a temperature reading:**

![OLED showing a temperature reading](docs/images/oled-temperature.png)

**OLED showing the Motion page with motion detected:**

![OLED showing the Motion page](docs/images/oled-motion.png)

**Terminal output during a normal run:**

![Terminal output during a normal run](docs/images/terminal-normal.png)

**Terminal showing an alarm state transition:**

![Terminal showing an alarm state transition](docs/images/terminal-alarm.png)

## Getting Started

1. Install Visual Studio Code.
2. Install the **PlatformIO IDE** extension.
3. Install the **Wokwi for VS Code** extension.
4. Clone the repository:
   git clone https://github.com/AlwynCodes/bca152-freertos-multisensor.git
5. Open the folder in VS Code.

## Building the Project

This project uses PlatformIO with the ESP-IDF framework.

Build for the ESP32:
  pio run -e esp32dev

Expected output: `[SUCCESS]`

## Running the Wokwi Simulation

1. Open `diagram.json` in VS Code.
2. Click the green **▶ Play** button in the top-right of the Wokwi window (or press `F1` and run `Wokwi: Start Simulator`).
3. The simulated circuit renders on the Wokwi canvas.
4. Watch the terminal at the bottom for output.

Try interacting with the DHT22 sliders, the LDR slider, the encoder knob, and the PIR trigger to observe the system's behavior.

## Unit Testing

Logic that never touches hardware lives in separate `*_logic.cpp` files so it can be tested on the host machine. The suite contains 18 tests in total, covering:

- `evaluateTemperature()` — 7 tests, boundary behavior at 18°C and 30°C
- `nextDisplayMode()` / `previousDisplayMode()` — 6 tests, forward/reverse wraparound
- `evaluateSystemState()` — 5 tests, idle timeout and motion override

Run them with:
  pio test -e native

Expected output: `18 test cases: 18 succeeded`.

## Static Code Analysis

`pio check -e esp32dev` invokes cppcheck with `check_skip_packages = yes`, so it inspects only the project's own sources and not the ESP-IDF toolchain headers. The final run comes back clean.
  pio check -e esp32dev

Expected output: `No defects found`.

## Functional Verification

A complete verification record — with observed behavior for 20 tests spanning boot, sensors, display, encoder navigation, alarm behavior, motion state, and concurrency — lives in [`docs/functional-verification.md`](docs/functional-verification.md).

## Fault Experiments

Three deliberate FreeRTOS faults were introduced and observed, then rolled back: stripping a task's blocking delay, boosting a task's priority above everything else, and removing the serial mutex. Results and analysis live in [`docs/fault-experiments.md`](docs/fault-experiments.md).

## Engineering Decisions

Several design choices were made based on what the lab required and what behaved reliably in the simulator:

1. **Native ESP-IDF drivers over Arduino libraries.** Every existing PlatformIO library for the DHT22 (`beegee-tokyo/DHT sensor library for ESPx`, `Adafruit_DHT`) and SSD1306 (`Adafruit_SSD1306`, `ThingPulse`) leans on the Arduino framework, which the lab disallows. So custom native drivers were built instead — a bit-bang single-wire driver for the DHT22, and a minimal I2C driver for the SSD1306 built on the new ESP-IDF v6.0.1 `i2c_master` API.

2. **LEDC PWM for the buzzer.** A plain `gpio_set_level` call left the buzzer silent in Wokwi. Switching to LEDC PWM at 2 kHz brought it to life — and it's also the correct approach for a passive buzzer on real hardware.

3. **Pure logic separated from hardware.** Decision functions such as `evaluateTemperature()`, `nextDisplayMode()`, `previousDisplayMode()`, and `evaluateSystemState()` were pulled out into `*_logic.cpp` files with no hardware dependencies, so they can be unit-tested on the host machine.

4. **Event group over individual notifications.** A single event group with three bits (`EVENT_ACTIVE`, `EVENT_MOTION`, `EVENT_ALARM`) lets any task query system state without needing direct access to the task that produced it.

5. **Polled PIR, interrupt-driven encoder.** The PIR module already emits a debounced digital level, so polling it every 500 ms is the simpler and more reliable route. The encoder emits raw quadrature edges, so an ISR is used to capture them.

6. **Separate queues for separate consumers.** Because a FreeRTOS queue delivers each item to exactly one receiver, SensorTask pushes to both `sensorQueue` (for DisplayTask) and `alarmQueue` (for AlarmTask).

## Limitations

1. Wokwi emits a recurring `GPIO 18 is not usable, maybe conflict with others` warning from the I2C master driver. It appeared on every pin pair tested for the OLED (21/22, 15/16, 25/26, 18/19), and the OLED renders correctly in every case — so it reads as a Wokwi quirk under ESP-IDF v6.0.1, not a wiring or code issue.

2. The hand-written DHT22 bit-bang driver occasionally produces a transient bad reading when the simulated sensor value is changed very quickly (for example, dragging a slider in Wokwi).

3. A passive buzzer needs an oscillating signal to produce sound. A plain `gpio_set_level` didn't animate in the simulator, so the buzzer runs on LEDC PWM at 2 kHz.

4. Rotating the encoder does not reset the inactivity timeout. Only PIR motion counts as activity, matching the spec.

5. When a DHT22 read fails, the code falls back to 0°C instead of discarding that reading, which will incorrectly trip `LOW_TEMPERATURE`. A more careful implementation would skip the alarm check when the read is marked invalid.

6. There's up to ~700 ms of lag between a real motion event and the display reacting, since MotionTask polls the PIR every 500 ms and DisplayTask checks the event group every 200 ms. Not instant, but not noticeable in practice.

7. Wokwi reports 2 MB flash while the real ESP32 Dev Module has 4 MB. Purely a simulator default mismatch — no functional impact.

## Future Improvements

- Skip the alarm pipeline on invalid DHT reads instead of substituting 0°C.
- Add a median-of-3 filter on the DHT22 driver to smooth out transient outliers.
- Treat encoder rotation as activity, so turning the knob wakes the display.
- Add persistence (NVS) so the last-selected display mode and thresholds survive a reboot.
- Add WiFi + MQTT for remote monitoring and cloud logging.

## References and Acknowledgments

**References:**

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [Wokwi Simulator Docs](https://docs.wokwi.com/)
- [PlatformIO Docs](https://docs.platformio.org/)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [DHT22 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)

**Acknowledgments:**

Built for BCA152 Microcontrollers at Mindanao State University – Iligan Institute of Technology, College of Computer Studies, Department of Computer Applications.
