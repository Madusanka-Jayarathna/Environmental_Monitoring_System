// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <atomic>
#include "mqtt_client.h"
#include "components/libNetwork/public/MqttClient.h"

namespace FIRMWARE {
namespace NETWORK {

struct MqttClientImpl {
    MqttClient::Option          _opt{};
    esp_mqtt_client_handle_t    _client = nullptr;
    std::atomic<bool>           _connected{false};
    std::atomic<bool>           _started{false};

    static void onMqttEvent(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData);
};

} // namespace NETWORK
} // namespace FIRMWARE
