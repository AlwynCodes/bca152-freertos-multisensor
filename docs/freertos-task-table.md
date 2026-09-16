# FreeRTOS Task Design

| Task | Responsibility | Period/Trigger | Priority | IPC | Typical Blocked Condition |
|---|---|---:|---:|---|---|
| SensorTask | Read DHT22 and LDR periodically | 2 s | 2 | Sensor queues | `vTaskDelayUntil()` |
| DisplayTask | Own and update OLED | 250 ms | 1 | Sensor queue, motion queue, event group | Delay / waiting for data |
| InputTask | Rotary encoder navigation | 20 ms | 3 | Display-mode queue | `vTaskDelay()` |
| MotionTask | Monitor PIR activity | 100 ms | 3 | Event group, motion queue | `vTaskDelay()` |
| AlarmTask | Evaluate temperature and buzzer | Sensor update / 100 ms loop | 2 | Sensor queue, event group | Queue receive / delay |
| StateTask | Manage ACTIVE/INACTIVE state | Event + 500 ms timeout | 2 | Event group | `xEventGroupWaitBits()` |
| TaskA | Initial FreeRTOS diagnostic task | 1 s | 1 | Serial mutex | `vTaskDelay()` |
| TaskB | Initial FreeRTOS diagnostic task | 2 s | 1 | Serial mutex | `vTaskDelay()` |

## Scheduling rationale

MotionTask and InputTask use priority 3 because PIR activity and user navigation should be serviced with relatively low latency. SensorTask and AlarmTask use priority 2 because environmental sampling and alarm evaluation tolerate more latency than direct input handling. DisplayTask uses priority 1 because display refresh is less urgent.

Every continuously running task performs finite work and then blocks with a delay or waits on an IPC primitive. No task intentionally busy-loops.
