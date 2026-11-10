#pragma once

#include "FreeRTOS.h"
#include <cstddef>
#include <vector>

struct QueueDefinition {
    size_t itemSize;
    size_t length;
    std::vector<uint8_t> buffer;
};
typedef struct QueueDefinition* QueueHandle_t;

inline QueueHandle_t xQueueCreate(size_t length, size_t itemSize) {
    auto q = new QueueDefinition();
    q->itemSize = itemSize;
    q->length = length;
    return q;
}

inline int xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait) {
    (void)xQueue; (void)pvItemToQueue; (void)xTicksToWait;
    return pdPASS;
}

inline int xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait) {
    (void)xQueue; (void)pvBuffer; (void)xTicksToWait;
    return pdFAIL;
}
