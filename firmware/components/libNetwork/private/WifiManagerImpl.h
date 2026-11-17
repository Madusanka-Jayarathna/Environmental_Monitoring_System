// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once
#include <atomic>
#include <cstdint>
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "components/libNetwork/public/WifiManager.h"

namespace FIRMWARE {
namespace NETWORK {

struct WifiManagerImpl {
    WifiManager::Option             _opt{};
    EventGroupHandle_t              _eventGroup         = nullptr;
    esp_netif_t                     *_staNetif          = nullptr;
    esp_event_handler_instance_t    _wifiAnyInstance    = nullptr;
    esp_event_handler_instance_t    _ipGotInstance      = nullptr;

    std::atomic<bool>               _connected{false};
    std::atomic<bool>               _reconnectNeeded{false};
    uint32_t                        _currentBackoffMs   = 1000;

    static void onWifiEvent(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData);
    static void onIpEvent(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData);
};

} // namespace NETWORK
} // namespace FIRMWARE
