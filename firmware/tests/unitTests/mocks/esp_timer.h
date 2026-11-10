#pragma once

#include <cstdint>

inline uint64_t esp_timer_get_time(void) {
    return 1000000ULL; // Mock 1000ms
}
