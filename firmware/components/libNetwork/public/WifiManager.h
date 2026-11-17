// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <cstdint>
#include "components/libUtils/public/defines.h"

struct EventGroupDef_t;
typedef struct EventGroupDef_t *EventGroupHandle_t;

namespace FIRMWARE {
namespace NETWORK {

class WifiManager {
public:
    // FreeRTOS EventGroup bits for cross-task synchronization (used by F6, F7, F8)
    static constexpr uint32_t kConnectedBit = (1U << 0);
    static constexpr uint32_t kLostBit      = (1U << 1);

    struct Option {
        Option()                        = default;
        const char *_ssid               = WIFI_SSID;
        const char *_password           = WIFI_PASSWORD;
        uint8_t  _statusLedPin          = WIFI_INDIC_PIN;
        uint32_t    _initialBackoffMs   = 1000;       // 1 s initial retry delay
        uint32_t    _maxBackoffMs       = 8000;       // Capped at 8 s so reconnects stay snappy
    };

public:
    WifiManager();
    ~WifiManager();

    WifiManager(const WifiManager &) = delete;
    WifiManager &operator=(const WifiManager &) = delete;

    Option& opt() {return _opt;}
    const Option& opt() const {return _opt; }

    bool init();
    bool isConnected() const;
    bool waitForConnection(uint32_t timeoutMs) const;
    EventGroupHandle_t eventGroup() const { return _eventGroup; }
    void runLoop();

private:

private:
    Option              _opt;
    EventGroupHandle_t  _eventGroup = nullptr;
    void                *_impl       = nullptr;
};

} // namespace NETWORK
} // namespace FIRMWARE
