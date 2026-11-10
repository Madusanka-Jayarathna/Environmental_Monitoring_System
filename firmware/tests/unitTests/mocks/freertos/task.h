#pragma once

#include "FreeRTOS.h"

inline void vTaskDelay(TickType_t ticks) {
    (void)ticks;
}

inline TickType_t xTaskGetTickCount(void) {
    return 0;
}

inline void vTaskDelayUntil(TickType_t *previousWakeTime, TickType_t timeIncrement) {
    if (previousWakeTime) {
        *previousWakeTime += timeIncrement;
    }
}
