// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <cstdint>
#include "components/libUtils/public/defines.h"

namespace FIRMWARE {
namespace NETWORK {

class MqttClient {
public:
    struct Option {
        Option()                    = default;
        const char  *_mqttBrokerURL = MQTT_BROKER_URL; // Laptop IP on hotspot/LAN
        const char  *_nodeId        = MQTT_NODE;
        const char  *_publishTopic  = "farm/node1/readings";
        uint32_t    _keepaliveSec   = 30;
    };

public:
    MqttClient();
    ~MqttClient();

    MqttClient(const MqttClient &) = delete;
    MqttClient &operator=(const MqttClient &) = delete;

    Option& opt() { return _opt; }
    const Option& opt() const {  return _opt; }
    
    bool init();
    bool start();
    bool stop();
    bool isConnected() const;

    /**
     * @brief Publish a JSON payload with QoS 1.
     * @return MQTT message ID (>= 0) on enqueue success, or -1 on failure.
     */
    int publishJson(const char *jsonPayload, int qos = 1);

private:
    Option      _opt;
    void        *_impl = nullptr;
};

} // namespace NETWORK
} // namespace FIRMWARE
