// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "testComponents/public/ESP32Tests.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "led_strip.h"
#include "esp_chip_info.h"
#include "esp_system.h"


namespace FIRMWARE {
namespace TEST {
namespace COMPTEST {


ESP32Tests::ESP32Tests( void ) {
    _chip = checkChipInfo();
}


bool
ESP32Tests::configurePin(const uint8_t pin, const PortDir dir) {
    ESP_ERROR_CHECK(gpio_reset_pin(static_cast<gpio_num_t>(pin)));
    ESP_ERROR_CHECK(gpio_set_direction(static_cast<gpio_num_t>(pin), (dir == PortDir::OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT));
    
    ESP_LOGI(TAG_ESP.data(), "configurePin - Configuring GPIO: %d as direction: %s", pin, (dir == PortDir::OUTPUT) ? "OUTPUT":"INPUT");
    return true;
}

bool
ESP32Tests::blink() {
    if (_chip == TargetChip::ESP32) {
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
ESP32Tests::blinkBoardLED() {
    if (_chip == TargetChip::ESP32) {
        if(configurePin(2, PortDir::OUTPUT)) {
            return blink();
        }
    } else if (_chip == TargetChip::ESP32S3) {
        led_strip_config_t stripConfig = {};
        stripConfig.strip_gpio_num = 48;
        stripConfig.max_leds = 1;
        stripConfig.led_pixel_format = LED_PIXEL_FORMAT_GRB;
        stripConfig.led_model = LED_MODEL_WS2812;

        led_strip_rmt_config_t rmtConfig = {};
        rmtConfig.resolution_hz = 10 * 1000 * 1000; // 10MHz
        rmtConfig.flags.with_dma = 0;   
        
        led_strip_handle_t led_strip;

        esp_err_t err = led_strip_new_rmt_device(&stripConfig, &rmtConfig, &led_strip);
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
ESP32Tests::runSoilSensorTest(void) {
    ESP_LOGI(TAG_ESP.data(), "\nrunSoilSensorTest - Soil Moisture Sensor Test Starting");
    
    adc_oneshot_unit_handle_t adcHandle = nullptr;
    
    adc_oneshot_unit_init_cfg_t initConfig = {};
    initConfig.unit_id = ADC_UNIT_1;
    
    esp_err_t err = adc_oneshot_new_unit(&initConfig, &adcHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "runSoilSensorTest - Failed to Initialize ADC Unit: %s", esp_err_to_name(err));
        return false;
    }

    adc_oneshot_chan_cfg_t config = {};
    config.bitwidth = ADC_BITWIDTH_DEFAULT;
    config.atten = ADC_ATTEN_DB_12;
    
    const adc_channel_t adcChannel = ADC_CHANNEL_4;
    err = adc_oneshot_config_channel(adcHandle, adcChannel, &config);
    if(err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "runSoilSensorTest - Failed to Configure ADC Channel: %s", esp_err_to_name(err));
        adc_oneshot_del_unit(adcHandle);
        return false;
    }

    int rawVal = 0;
    err = adc_oneshot_read(adcHandle, adcChannel, &rawVal);
    if(err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "runSoilSensorTest - Failed to Read ADC Value: %s", esp_err_to_name(err));
        adc_oneshot_del_unit(adcHandle);
        return false;
    }

    err = adc_oneshot_del_unit(adcHandle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_ESP.data(), "runSoilSensorTest - Warning: Failed to release ADC handle: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG_ESP.data(), "runSoilSensorTest - Soil Moisture ADC Reading = %d PASSED", rawVal);
    return true;
}

bool
ESP32Tests::runBME280Test(void) {
    ESP_LOGI(TAG_ESP.data(), "\nrunBME280Test - BME280 Test Starting");
    
    i2c_master_bus_config_t busConfig = {};
    busConfig.i2c_port = I2C_NUM_0;
    busConfig.sda_io_num = static_cast<gpio_num_t>(21);
    busConfig.scl_io_num = static_cast<gpio_num_t>(22);
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.flags.enable_internal_pullup = true;
    
    i2c_master_bus_handle_t busHandle = nullptr;
    esp_err_t err = i2c_new_master_bus(&busConfig, &busHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "runBME280Test - Failed to initialize I2C bus: %s", esp_err_to_name(err));
        return false;
    }

    const uint8_t addresses[] = {0x76, 0x77};
    bool found = false;
    for (size_t i = 0; i < sizeof(addresses) / sizeof(addresses[0]); ++i) {
        esp_err_t ret = i2c_master_probe(busHandle, addresses[i], 100);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG_ESP.data(), "runBME280Test - BME280 at I2C address 0x%02X PASSED", addresses[i]);
            found = true;
        }
    }
    if (!found) {
        ESP_LOGE(TAG_ESP.data(), "runBME280Test - Failed to Detect I2C Device at 0x76 or 0x77");
    }

    err = i2c_del_master_bus(busHandle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_ESP.data(), "runBME280Test - Warning: Failed to release I2C bus handle: %s", esp_err_to_name(err));
    }

    return found;
}


bool
ESP32Tests::run(void) {
    if(_chip == TargetChip::Undefined) {
        ESP_LOGE(TAG_ESP.data(), "runBlinkTest - ESP32 board not identified!");
        return false;
    }

    bool isSuccess = true;
    isSuccess = isSuccess &
                runBlinkTest() &
                runBME280Test() &
                runSoilSensorTest();

    return isSuccess;
}

bool 
ESP32Tests::runBlinkTest(void) {
    ESP_LOGI(TAG_ESP.data(), "\nrunBlinkTest - ESP32 Blink Test Starting");
    bool isSuccess = false;

    isSuccess = blinkBoardLED(); 
    if(isSuccess) {
        ESP_LOGI(TAG_ESP.data(), "runBlinkTest - ESP32 Blink Test PASSED");
    }
    return isSuccess;
}

} // namespace COMPTEST
} // namespace TEST
} // namespace FIRMWARE
