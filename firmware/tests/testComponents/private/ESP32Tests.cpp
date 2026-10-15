// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "testComponents/public/ESP32Tests.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "led_strip.h"
#include "esp_chip_info.h"
#include "esp_system.h"


namespace FIRMWARE {
namespace TEST {
namespace COMPTEST {

bool
ESP32Tests::configurePin(const uint8_t pin, const PortDir dir) {
    ESP_ERROR_CHECK(gpio_reset_pin(static_cast<gpio_num_t>(pin)));
    ESP_ERROR_CHECK(gpio_set_direction(static_cast<gpio_num_t>(pin), (dir == PortDir::OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT));
    
    ESP_LOGI(TAG_ESP.data(), "configurePin - Configuring GPIO: %d as direction: %s", pin, (dir == PortDir::OUTPUT) ? "OUTPUT":"INPUT");
    return true;
}

bool
ESP32Tests::blink(const TargetChip &chip) {
    if (chip == TargetChip::ESP32) {
        bool flag = false;
        for(uint8_t i=0; i<10; ++i) {
            ESP_ERROR_CHECK(gpio_set_level(static_cast<gpio_num_t>(2), flag));
            flag = !flag;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        return true;
    }
    return false;
}

bool
ESP32Tests::blink(const led_strip_handle_t &led_strip) {
    for(uint8_t i=0; i<3; ++i) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 100, 0, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 100, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 0, 100));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    return true;
}
        


bool
ESP32Tests::blinkBoardLED(const TargetChip &chip) {
    if (chip == TargetChip::ESP32) {
        if(configurePin(2, PortDir::OUTPUT)) {
            return blink(chip);
        }
    } else if (chip == TargetChip::ESP32S3) {
        led_strip_config_t strip_config = {};
        strip_config.strip_gpio_num = 48;
        strip_config.max_leds = 1;
        strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB;
        strip_config.led_model = LED_MODEL_WS2812;

        led_strip_rmt_config_t rmt_config = {};
        rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz
        rmt_config.flags.with_dma = 0;   
        
        led_strip_handle_t led_strip;

        esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
        if (err == ESP_OK) {
            return blink(led_strip);
        }
        ESP_LOGE(TAG_ESP.data(), "blinkBoardLED - Failed to configure ESP32S3 board LED");
        return false;
    }
    return false;
}
ESP32Tests::TargetChip
ESP32Tests::checkChipInfo() {
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);

    if (chipInfo.model == CHIP_ESP32S3) {
        ESP_LOGI(TAG_ESP.data(), "checkChipInfo - ESP32S3 board identified");
        return TargetChip::ESP32S3;
    } else if (chipInfo.model == CHIP_ESP32) {
        ESP_LOGI(TAG_ESP.data(), "checkChipInfo - ESP32 board identified");
        return TargetChip::ESP32;
    } else {
        return TargetChip::Undefined;
    }
    return TargetChip::Undefined; 
}

bool 
ESP32Tests::runBlinkTest(void) {
    TargetChip chip = checkChipInfo();
    bool isSuccess = false;

    if(chip == TargetChip::Undefined) {
        ESP_LOGE(TAG_ESP.data(), "runBlinkTest - ESP32 board not identified!");
        isSuccess = false; 
    } else {
        isSuccess = blinkBoardLED(chip);
    }
    
    if(isSuccess) {
        ESP_LOGI(TAG_ESP.data(), "runBlinkTest - ESP32 blink test PASSED");
    }
    return isSuccess;
}

} // namespace COMPTEST
} // namespace TEST
} // namespace FIRMWARE

