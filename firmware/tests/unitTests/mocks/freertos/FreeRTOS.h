#pragma once

#include <cstdint>

typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
#define pdPASS 1
#define pdFAIL 0
#define portMAX_DELAY ((TickType_t)0xFFFFFFFFUL)
#define tskNO_AFFINITY 0x7FFFFFFF
