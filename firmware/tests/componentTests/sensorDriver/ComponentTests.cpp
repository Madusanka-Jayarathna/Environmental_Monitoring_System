// copyright 2026 All Rights Reserved
// Author : madusankaijayarathna@gmail.com (Madusanka Jayarathna)

#include "ComponentTests.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

namespace FIRMWARE {

void
ComponentTests::setPinValue(size_t pin, const bool flag){
    gpio_set_level(static_cast<gpio_num_t>(pin), flag);
}

void
ComponentTests::toggleFlag(bool& flag){
    flag = !flag;    
}

bool
ComponentTests::configurePins(const std::vector<size_t>& pinList, const std::vector<bool>& dirList) {
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
ComponentTests::runBlinkTest(void) {
    std::vector<size_t> pinList = {2};
    std::vector<bool> dirList = {true};
    if(!configurePins(pinList, dirList)){
        return;
    }
    
    bool flag = false;
    while (true) {
        ESP_LOGI(TAG.data(), "Turning the LED %s!", flag ? "ON" : "OFF");
        setPinValue(pinList[0], flag);
        toggleFlag(flag);        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
} // namespace FIRMWARE

