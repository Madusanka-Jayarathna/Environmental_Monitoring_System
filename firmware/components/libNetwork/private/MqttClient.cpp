// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "components/libNetwork/public/MqttClient.h"
#include "components/libNetwork/private/MqttClientImpl.h"

#include "esp_log.h"

namespace FIRMWARE {
namespace NETWORK {

namespace {
constexpr const char *TAG = "MqttClient";
} // namespace

void
MqttClientImpl::onMqttEvent(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData) {
    auto *impl = static_cast<MqttClientImpl *>(handlerArgs);
    auto *event = static_cast<esp_mqtt_event_handle_t>(eventData);
    if (impl == nullptr || event == nullptr) {
        return;
    }

    switch (static_cast<esp_mqtt_event_id_t>(eventId)) {
        case MQTT_EVENT_CONNECTED:
            impl->_connected.store(true);
            ESP_LOGI(TAG, "MQTT state: CONNECTED to %s", impl->_opt._mqttBrokerURL);
            break;

        case MQTT_EVENT_DISCONNECTED:
            impl->_connected.store(false);
            ESP_LOGW(TAG, "MQTT state: DISCONNECTED");
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT PUBACK received for msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR (type=%d)", event->error_handle ? event->error_handle->error_type : -1);
            break;

        default:
            break;
    }
}

MqttClient::MqttClient() = default;

MqttClient::~MqttClient() {
    auto *impl = static_cast<MqttClientImpl *>(_impl);
    if (impl != nullptr) {
        if (impl->_client != nullptr) {
            esp_mqtt_client_stop(impl->_client);
            esp_mqtt_client_destroy(impl->_client);
        }
        delete impl;
        _impl = nullptr;
    }
}

bool
MqttClient::init() {
    auto *impl = new MqttClientImpl();
    impl->_opt = _opt;

    esp_mqtt_client_config_t mqttCfg = {};
    mqttCfg.broker.address.uri                  = _opt._mqttBrokerURL;
    mqttCfg.session.keepalive                   = static_cast<int>(_opt._keepaliveSec);
    mqttCfg.network.reconnect_timeout_ms        = 3000;
    mqttCfg.network.timeout_ms                  = 5000;

    impl->_client = esp_mqtt_client_init(&mqttCfg);
    if (impl->_client == nullptr) {
        ESP_LOGE(TAG, "init - Failed to initialize esp_mqtt_client");
        delete impl;
        return false;
    }

    if (esp_mqtt_client_register_event(impl->_client, MQTT_EVENT_ANY,
                                       &MqttClientImpl::onMqttEvent, impl) != ESP_OK) {
        ESP_LOGE(TAG, "init - Failed to register MQTT event handler");
        esp_mqtt_client_destroy(impl->_client);
        delete impl;
        return false;
    }

    _impl = impl;
    ESP_LOGI(TAG, "init - MqttClient configured for %s (topic=%s)",
             _opt._mqttBrokerURL, _opt._publishTopic);
    return true;
}

bool
MqttClient::start() {
    auto *impl = static_cast<MqttClientImpl *>(_impl);
    if (impl == nullptr || impl->_client == nullptr) {
        return false;
    }
    if (impl->_started.exchange(true)) {
        return true;
    }
    const esp_err_t err = esp_mqtt_client_start(impl->_client);
    if (err != ESP_OK) {
        impl->_started.store(false);
        ESP_LOGE(TAG, "start - esp_mqtt_client_start failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool
MqttClient::stop() {
    auto *impl = static_cast<MqttClientImpl *>(_impl);
    if (impl == nullptr || impl->_client == nullptr) {
        return false;
    }
    if (!impl->_started.exchange(false)) {
        return true;
    }
    impl->_connected.store(false);
    return esp_mqtt_client_stop(impl->_client) == ESP_OK;
}

bool
MqttClient::isConnected() const {
    auto *impl = static_cast<MqttClientImpl *>(_impl);
    return (impl != nullptr) && impl->_connected.load();
}

int
MqttClient::publishJson(const char *jsonPayload, int qos) {
    auto *impl = static_cast<MqttClientImpl *>(_impl);
    if (impl == nullptr || impl->_client == nullptr || !impl->_connected.load() || jsonPayload == nullptr) {
        return -1;
    }

    return esp_mqtt_client_publish(
        impl->_client,
        _opt._publishTopic,
        jsonPayload,
        0,    // 0 = compute length automatically via strlen
        qos,  // QoS 1 (at-least-once)
        0     // retain = 0
    );
}

} // namespace NETWORK
} // namespace FIRMWARE
