// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/i2c_master.h"
#include "components/libSensors/public/firmwareUtils.h"

namespace FIRMWARE {
namespace UTILS {

namespace {
constexpr char TAG[] = "FirmwareUtils";
constexpr adc_unit_t kAdcUnit = ADC_UNIT_1;
}

// --- Hardware & I2C Pin Setup ---
#define I2C_MASTER_SCL_IO           22      /*!< GPIO number for I2C SCL */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number for I2C SDA */

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

} // namespace UTILS
} // namespace FIRMWARE
