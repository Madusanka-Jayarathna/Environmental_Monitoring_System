// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "components/libNetwork/public/WifiManager.h"
#include "components/libNetwork/private/WifiManagerImpl.h"
#include "components/libUtils/public/firmwareUtils.h"
#include <algorithm>
#include <cinttypes>
#include <cstring>
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/task.h"

namespace FIRMWARE {
namespace NETWORK {

namespace {
constexpr const char *TAG = "WifiManager";
} // namespace

void
WifiManagerImpl::onWifiEvent(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData) {
    auto *impl = static_cast<WifiManagerImpl *>(arg);
    if (impl == nullptr || eventBase != WIFI_EVENT) {
        return;
    }

    switch (eventId) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "STA started -> initiating connection to '%s'...", impl->_opt._ssid);
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED: {
            auto *disc = static_cast<wifi_event_sta_disconnected_t *>(eventData);
            const bool wasConnected = impl->_connected.exchange(false);

            xEventGroupClearBits(impl->_eventGroup, WifiManager::kConnectedBit);
            xEventGroupSetBits(impl->_eventGroup, WifiManager::kLostBit);
            FIRMWARE::UTILS::FirmwareUtils::setStatusPin(impl->_opt._statusLedPin, false);

            if (wasConnected) {
                ESP_LOGW(TAG, "Wi-Fi state: LOST (reason=%u) -> scheduling auto-reconnect", disc->reason);
            } else {
                ESP_LOGW(TAG, "Wi-Fi connect attempt failed (reason=%u) -> will retry with backoff", disc->reason);
            }

            impl->_reconnectNeeded.store(true);
            break;
        }

        default:
            break;
    }
}

void
WifiManagerImpl::onIpEvent(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData) {
    auto *impl = static_cast<WifiManagerImpl *>(arg);
    if (impl == nullptr || eventBase != IP_EVENT || eventId != IP_EVENT_STA_GOT_IP) {
        return;
    }

    auto *event = static_cast<ip_event_got_ip_t *>(eventData);
    impl->_connected.store(true);
    impl->_reconnectNeeded.store(false);
    impl->_currentBackoffMs = impl->_opt._initialBackoffMs;

    xEventGroupClearBits(impl->_eventGroup, WifiManager::kLostBit);
    xEventGroupSetBits(impl->_eventGroup, WifiManager::kConnectedBit);
    FIRMWARE::UTILS::FirmwareUtils::setStatusPin(impl->_opt._statusLedPin, true);

    ESP_LOGI(TAG, "Wi-Fi state: CONNECTED | IP=" IPSTR, IP2STR(&event->ip_info.ip));
}

WifiManager::WifiManager() = default;

WifiManager::~WifiManager() {
    auto *impl = static_cast<WifiManagerImpl *>(_impl);
    if (impl != nullptr) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, impl->_wifiAnyInstance);
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, impl->_ipGotInstance);
        esp_wifi_stop();
        esp_wifi_deinit();
        if (impl->_eventGroup != nullptr) {
            vEventGroupDelete(impl->_eventGroup);
        }
        delete impl;
        _impl = nullptr;
    }
}

bool
WifiManager::init() {
    FIRMWARE::UTILS::FirmwareUtils::initStatusPin(opt()._statusLedPin, true);

    auto *impl              = new WifiManagerImpl();
    impl->_opt              = _opt;
    impl->_currentBackoffMs = opt()._initialBackoffMs;
    impl->_eventGroup       = xEventGroupCreate();
    
    if (impl->_eventGroup    == nullptr) {
        ESP_LOGE(TAG, "init - Failed to create Wi-Fi event group");
        delete impl;
        return false;
    }

    // Start in LOST/disconnected state until IP is acquired
    xEventGroupSetBits(impl->_eventGroup, kLostBit);
    _eventGroup = impl->_eventGroup;
    _impl       = impl;

    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "init - esp_netif_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "init - esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        return false;
    }

    impl->_staNetif = esp_netif_create_default_wifi_sta();
    if (impl->_staNetif == nullptr) {
        ESP_LOGE(TAG, "init - Failed to create default Wi-Fi STA netif");
        return false;
    }

    wifi_init_config_t wifiInitCfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&wifiInitCfg) != ESP_OK) {
        ESP_LOGE(TAG, "init - esp_wifi_init failed");
        return false;
    }

    if (esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                            &WifiManagerImpl::onWifiEvent, impl,
                                            &impl->_wifiAnyInstance) != ESP_OK) {
        ESP_LOGE(TAG, "init - Failed to register WIFI_EVENT handler");
        return false;
    }

    if (esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                            &WifiManagerImpl::onIpEvent, impl,
                                            &impl->_ipGotInstance) != ESP_OK) {
        ESP_LOGE(TAG, "init - Failed to register IP_EVENT handler");
        return false;
    }

    wifi_config_t wifiCfg = {};
    std::strncpy(reinterpret_cast<char *>(wifiCfg.sta.ssid), opt()._ssid, sizeof(wifiCfg.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char *>(wifiCfg.sta.password), opt()._password, sizeof(wifiCfg.sta.password) - 1);
    wifiCfg.sta.threshold.authmode = (std::strlen(opt()._password) > 0) ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    wifiCfg.sta.pmf_cfg.capable    = true;
    wifiCfg.sta.pmf_cfg.required   = false;

    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK ||
        esp_wifi_set_config(WIFI_IF_STA, &wifiCfg) != ESP_OK ||
        esp_wifi_start() != ESP_OK) {
        ESP_LOGE(TAG, "init - Failed to configure/start Wi-Fi STA");
        return false;
    }

    esp_wifi_set_inactive_time(WIFI_IF_STA, 3);

    ESP_LOGI(TAG, "init - Wi-Fi manager initialized for SSID '%s'", opt()._ssid);
    return true;
}

bool
WifiManager::isConnected() const {
    auto *impl = static_cast<WifiManagerImpl *>(_impl);
    return (impl != nullptr) && impl->_connected.load();
}

bool
WifiManager::waitForConnection(uint32_t timeoutMs) const {
    if (_eventGroup == nullptr) {
        return false;
    }
    const TickType_t ticks = (timeoutMs == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs);
    const EventBits_t bits = xEventGroupWaitBits(_eventGroup, kConnectedBit, pdFALSE, pdTRUE, ticks);
    return (bits & kConnectedBit) != 0;
}

void
WifiManager::runLoop() {
    auto *impl = static_cast<WifiManagerImpl *>(_impl);
    if (impl == nullptr) {
        return;
    }

    ESP_LOGI(TAG, "wifi_task started (backoff: %" PRIu32 " ms .. %" PRIu32 " ms)", opt()._initialBackoffMs, opt()._maxBackoffMs);

    while (true) {
        if (impl->_reconnectNeeded.exchange(false)) {
            ESP_LOGI(TAG, "Wi-Fi state: RECONNECTING in %" PRIu32 " ms...", impl->_currentBackoffMs);
            vTaskDelay(pdMS_TO_TICKS(impl->_currentBackoffMs));

            // Exponential backoff: 1s -> 2s -> 4s -> 8s (capped)
            impl->_currentBackoffMs = std::min(impl->_currentBackoffMs * 2, opt()._maxBackoffMs);

            if (!impl->_connected.load()) {
                esp_wifi_connect();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

} // namespace NETWORK
} // namespace FIRMWARE
