// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "components/libUtils/public/firmwareUtils.h"
#include "components/libUtils/public/defines.h"

namespace FIRMWARE {
namespace UTILS {

namespace {
constexpr char TAG[] = "FirmwareUtils";
constexpr adc_unit_t kAdcUnit = ADC_UNIT_1;
}

bool 
FirmwareUtils::initI2CBus(i2c_master_bus_handle_t& busHandle) {
    ESP_LOGI(TAG, "\ninitI2CBus - I2C Bus Initialization Starting");
    
    i2c_master_bus_config_t busConfig = {};
    busConfig.i2c_port = I2C_NUM_0;
    busConfig.sda_io_num = static_cast<gpio_num_t>(I2C_MASTER_SDA_IO);
    busConfig.scl_io_num = static_cast<gpio_num_t>(I2C_MASTER_SCL_IO);
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.flags.enable_internal_pullup = true;
    
    esp_err_t err = i2c_new_master_bus(&busConfig, &busHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "initI2CBus - Failed to initialize I2C bus: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "\ninitI2CBus - I2C Bus Initialization Succes");
    return true; 
}

bool
FirmwareUtils::initADC(adc_oneshot_unit_handle_t& adcHandle) {
    ESP_LOGI(TAG, "\ninitADC - ADC Unit Initialization Starting");

    adc_oneshot_unit_init_cfg_t initConfig = {};
    initConfig.unit_id = kAdcUnit;
    initConfig.ulp_mode = ADC_ULP_MODE_DISABLE;

    esp_err_t err = adc_oneshot_new_unit(&initConfig, &adcHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "initADC - Failed to create ADC unit: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "\ninitADC - ADC Unit Initialization Success");
    return true;
}

bool
FirmwareUtils::initNVS() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "initNVS - Erasing NVS partition");
        if (nvs_flash_erase() != ESP_OK) {
            return false;
        }
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "initNVS - nvs_flash_init failed: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

bool
FirmwareUtils::initStatusPin(uint8_t p, bool isOutput) {
    gpio_num_t pin = static_cast<gpio_num_t>(p);
    if (pin == GPIO_NUM_NC) {
        return true;
    }
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = (1ULL << static_cast<uint32_t>(pin));
    cfg.mode         = isOutput ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT ;
    cfg.pull_up_en   = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type    = GPIO_INTR_DISABLE;

    if (gpio_config(&cfg) != ESP_OK) {
        ESP_LOGE(TAG, "initStatusLed - Failed to configure GPIO %d", static_cast<int>(pin));
        return false;
    }

    if(isOutput) {
        gpio_set_level(pin, 0);
    }
    return true;
}

void
FirmwareUtils::setStatusPin(uint8_t p, bool isHigh) {
    gpio_num_t pin = static_cast<gpio_num_t>(p);
    if (pin != GPIO_NUM_NC) {
        gpio_set_level(pin, isHigh ? 1 : 0);
    }
}


} // namespace UTILS
} // namespace FIRMWARE
