// copyright 2026 All Rights Reserved
// Author : madusankaijayarathna@gmail.com (Madusanka Jayarathna)

#include "testComponents/public/ESP32Tests.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "led_strip.h"

namespace FIRMWARE {
namespace TEST {

void
ESP32Tests::setPinValue(size_t pin, const bool flag){
    gpio_set_level(static_cast<gpio_num_t>(pin), flag);
}

void
ESP32Tests::toggleFlag(bool& flag){
    flag = !flag;    
}

bool
ESP32Tests::configurePins(const std::vector<size_t>& pinList, const std::vector<bool>& dirList) {
    if(pinList.empty() || pinList.size() != dirList.size())
        return false;

    for(int i=0; i<pinList.size(); ++i) {
        gpio_reset_pin(static_cast<gpio_num_t>(pinList[i]));
        gpio_set_direction(static_cast<gpio_num_t>(pinList[i]), (dirList[i] ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT));
        ESP_LOGI(TAG.data(), "Configuring GPIO %d as direction %s", pinList[i], dirList[i] ? "Output":"Input");
    }
    return true;
}


void 
ESP32Tests::runBlinkTest(void) {
    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = 48;
    strip_config.max_leds = 1;
    strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB;
    strip_config.led_model = LED_MODEL_WS2812;

    led_strip_rmt_config_t rmt_config = {};
    rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz
    rmt_config.flags.with_dma = 0;

    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    int i = 0;
    while (true) {
        ++i;
        // 1. Red ON
        ESP_LOGI(TAG.data(), "LED: RED");
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 100, 0, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 2. Off
        ESP_LOGI(TAG.data(), "LED: OFF");
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 3. Green ON
        ESP_LOGI(TAG.data(), "LED: GREEN");
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 100, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 4. Off
        ESP_LOGI(TAG.data(), "LED: OFF");
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 5. Blue ON
        ESP_LOGI(TAG.data(), "LED: BLUE");
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 0, 100));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 6. Off
        ESP_LOGI(TAG.data(), "LED: OFF");
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (i > 3) break;
    }
    
    //std::vector<size_t> pinList = {2};
    //std::vector<bool> dirList = {true};
    //if(!configurePins(pinList, dirList)){
        //return;
    //}
    
    //bool flag = false;
    //while (false) {
        //ESP_LOGI(TAG.data(), "Turning the LED %s!", flag ? "ON" : "OFF");
        //setPinValue(pinList[0], flag);
        //toggleFlag(flag);        
        //vTaskDelay(pdMS_TO_TICKS(1000));
    //}
    return;
}
} // namespace TEST
} // namespace FIRMWARE

