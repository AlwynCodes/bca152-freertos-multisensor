#include "system_state.h"
#include "config.h"

SystemState evaluateSystemState(
    SystemState currentState,
    bool motionDetected,
    uint32_t elapsedMs)
{
    if (motionDetected) {
        return SYSTEM_ACTIVE;
    }

    if (currentState == SYSTEM_ACTIVE &&
        elapsedMs >= INACTIVITY_TIMEOUT_MS) {
        return SYSTEM_INACTIVE;
    }

    return currentState;
}