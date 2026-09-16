#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SYSTEM_ACTIVE = 0,
    SYSTEM_INACTIVE
} SystemState;

SystemState evaluateSystemState(
    SystemState currentState,
    bool motionDetected,
    uint32_t elapsedMs
);

#endif
