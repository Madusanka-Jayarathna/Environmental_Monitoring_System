#pragma once

#include "esp_err.h"
#include <cstdio>

#define ESP_LOGE(tag, format, ...) std::printf("[ERROR][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) std::printf("[WARN][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) std::printf("[INFO][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) std::printf("[DEBUG][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) std::printf("[VERBOSE][%s] " format "\n", tag, ##__VA_ARGS__)
