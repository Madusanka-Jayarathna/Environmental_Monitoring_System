// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "ESP32Tests.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_vfs_fat.h"
#include "led_strip.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include <cstdint>
#include <string_view>


namespace FIRMWARE {
namespace TEST {
namespace COMPTEST {

inline constexpr std::string_view TAG_ESP = "ComponentTests";

ESP32Tests::ESP32Tests( void ) {
    _chip = checkChipInfo();
}

bool 
ESP32Tests::initI2CBus(i2c_master_bus_handle_t& busHandle) {
    ESP_LOGI(TAG_ESP.data(), "\ninitI2CBus - I2C Bus Initialization Starting");
    
    i2c_master_bus_config_t busConfig = {};
    busConfig.i2c_port = I2C_NUM_0;
    busConfig.sda_io_num = static_cast<gpio_num_t>(21);
    busConfig.scl_io_num = static_cast<gpio_num_t>(22);
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.flags.enable_internal_pullup = true;
    
    esp_err_t err = i2c_new_master_bus(&busConfig, &busHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "initI2CBus - Failed to initialize I2C bus: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG_ESP.data(), "\ninitI2CBus - I2C Bus Initialization Succes");

    return true; 
}



bool
ESP32Tests::configurePin(unsigned char pin, const PortDir dir) {
    ESP_ERROR_CHECK(gpio_reset_pin(static_cast<gpio_num_t>(pin)));
    ESP_ERROR_CHECK(gpio_set_direction(static_cast<gpio_num_t>(pin), (dir == PortDir::OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT));
    
    ESP_LOGI(TAG_ESP.data(), "configurePin - Configuring GPIO: %d as direction: %s", pin, (dir == PortDir::OUTPUT) ? "OUTPUT":"INPUT");
    return true;
}

bool
ESP32Tests::blink() {
    if (_chip == TargetChip::ESP32) {
        bool flag = false;
        for(uint8_t i=0; i<5; ++i) {
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
ESP32Tests::runBME280Test(i2c_master_bus_handle_t& busHandle) {
    ESP_LOGI(TAG_ESP.data(), "\nrunBME280Test - BME280 Test Starting");

    if (busHandle == nullptr) {
        ESP_LOGE(TAG_ESP.data(), "runBME280Test - I2C Bus Not Initialized");
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
    
    return found;
}


bool
ESP32Tests::runSDCardTest() {
    ESP_LOGI(TAG_ESP.data(), "\nrunSDCardTest - SPI SD Card Test Starting");
    
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    
    spi_bus_config_t busConfig = {};
    busConfig.mosi_io_num = static_cast<gpio_num_t>(23);
    busConfig.miso_io_num = static_cast<gpio_num_t>(19);
    busConfig.sclk_io_num = static_cast<gpio_num_t>(18);
    busConfig.quadwp_io_num = -1;
    busConfig.quadhd_io_num = -1;
    
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &busConfig, SDSPI_DEFAULT_DMA);
    if (err != ESP_OK) {
        ESP_LOGI(TAG_ESP.data(), "runSDCardTest - Failed to Initialize SPI Bus: %s", esp_err_to_name(err));
        return false;
    }

    sdspi_device_config_t devConfig = SDSPI_DEVICE_CONFIG_DEFAULT();
    devConfig.gpio_cs = static_cast<gpio_num_t>(5);
    devConfig.host_id = SPI2_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mountConfig = {};
    mountConfig.format_if_mount_failed = false;
    mountConfig.max_files = 3;

    sdmmc_card_t *card;
    err = esp_vfs_fat_sdspi_mount("/sdcard", &host, &devConfig, &mountConfig, &card);
    
    bool isSuccess = false;
    if (err == ESP_OK) {
        uint64_t capacityMb = (static_cast<uint64_t>(card->csd.capacity) * card->csd.sector_size) / (1024 * 1024);
        ESP_LOGI(TAG_ESP.data(), "runSDCardTest - SD Card Mounted. Capacity: %llu MB", capacityMb);
        
        esp_vfs_fat_sdcard_unmount("/sdcard", card);
        isSuccess = true;
    } else {
        ESP_LOGE(TAG_ESP.data(), "runSDCardTest - SD Card Mount failed: %s", esp_err_to_name(err));
    }

    spi_bus_free(SPI2_HOST);
    return isSuccess;
}

bool ESP32Tests::runOLEDTest(i2c_master_bus_handle_t& busHandle) {
    ESP_LOGI(TAG_ESP.data(), "\nrunOLEDTest - OLED Display Test Starting");

    if (busHandle == nullptr) {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - I2C Bus Not Initialized");
        return false;
    }

    const uint8_t oledAddrs[] = {0x3C, 0x3D};
    uint8_t foundOLEDAddr = 0;
    
    for (size_t i = 0; i < sizeof(oledAddrs) / sizeof(oledAddrs[0]); i++) {
        esp_err_t err = i2c_master_probe(busHandle, oledAddrs[i], 100);
        if (err == ESP_OK) {
            foundOLEDAddr = oledAddrs[i];
            ESP_LOGI(TAG_ESP.data(), "runOLEDTest - Found OLED Display at 0x%02X", foundOLEDAddr);
            break;
        }
    }

    if (foundOLEDAddr == 0) {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - FAIL: OLED Display not responding at 0x3C or 0x3D");
        return false;
    }

    i2c_device_config_t _devConfig = {};
    _devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    _devConfig.device_address = foundOLEDAddr;
    _devConfig.scl_speed_hz = 100000;

    i2c_master_dev_handle_t _oledDevHandle = nullptr;
    esp_err_t err = i2c_master_bus_add_device(busHandle, &_devConfig, &_oledDevHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - Failed to add OLED device to I2C bus: %s", esp_err_to_name(err));
        return false;
    }

    // 1. SSD1306 Initialization
    const uint8_t _initSequence[] = {
        0x00,       // Control byte: Command Mode
        0xAE,       // Display OFF
        0x8D, 0x14, // Enable Internal Charge Pump
        0x20, 0x00, // Set Memory Addressing Mode to Horizontal
        0xA1,       // Segment Remap
        0xC8,       // COM Scan Direction
        0xAF        // Display ON
    };

    err = i2c_master_transmit(_oledDevHandle, _initSequence, sizeof(_initSequence), 100);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - Failed to send initialization sequence");
        i2c_master_bus_rm_device(_oledDevHandle);
        return false;
    }

    // 2. Clear Screen to Black First
    uint8_t _displayBuffer[1 + (128 * 64 / 8)];
    _displayBuffer[0] = 0x40; // Control byte: Data Mode
    for (size_t i = 1; i < sizeof(_displayBuffer); ++i) {
        _displayBuffer[i] = 0x00;
    }
    i2c_master_transmit(_oledDevHandle, _displayBuffer, sizeof(_displayBuffer), 500);

    // 3. Draw Thick Vertical Stripe Pattern (Visible 8-pixel-wide bars)
    for (size_t i = 1; i < sizeof(_displayBuffer); ++i) {
        _displayBuffer[i] = (i % 8 < 4) ? 0xFF : 0x00;
    }

    err = i2c_master_transmit(_oledDevHandle, _displayBuffer, sizeof(_displayBuffer), 500);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_ESP.data(), "runOLEDTest - Vertical bar pattern displayed. Holding for 4 seconds.");
        vTaskDelay(pdMS_TO_TICKS(4000)); // Hold for 4 seconds

        // Clear screen back to black before test completion
        for (size_t i = 1; i < sizeof(_displayBuffer); ++i) {
            _displayBuffer[i] = 0x00;
        }
        i2c_master_transmit(_oledDevHandle, _displayBuffer, sizeof(_displayBuffer), 500);
    } else {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - Failed to transfer frame buffer data");
    }

    i2c_master_bus_rm_device(_oledDevHandle);
    return (err == ESP_OK);
}




/*
bool ESP32Tests::runOLEDTest(i2c_master_bus_handle_t& busHandle) {
    ESP_LOGI(TAG_ESP.data(), "\nrunOLEDTest - OLED Display Test Starting");

    if (busHandle == nullptr) {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - I2C Bus Not Initialized");
        return false;
    }

    const uint8_t oledAddrs[] = {0x3C, 0x3D};
    uint8_t foundOLEDAddr = 0;
    
    for (size_t i = 0; i < sizeof(oledAddrs) / sizeof(oledAddrs[0]); i++) {
        esp_err_t err = i2c_master_probe(busHandle, oledAddrs[i], 100);
        if (err == ESP_OK) {
            foundOLEDAddr = oledAddrs[i];
            ESP_LOGI(TAG_ESP.data(), "runOLEDTest - Found OLED Display at 0x%02X", foundOLEDAddr);
            break;
        }
    }

    if (foundOLEDAddr == 0) {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - FAIL: OLED Display not responding at 0x3C or 0x3D");
        return false;
    }

    i2c_device_config_t devConfig = {};
    devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    devConfig.device_address = foundOLEDAddr;
    devConfig.scl_speed_hz = 100000;

    i2c_master_dev_handle_t oledDevHandle = nullptr;
    esp_err_t err = i2c_master_bus_add_device(busHandle, &devConfig, &oledDevHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - Failed to add OLED device to I2C bus: %s", esp_err_to_name(err));
        return false;
    }

    bool testPassed = false;
    uint8_t screenTestCmd[] = {0x00, 0xAF, 0xA5};
    
    err = i2c_master_transmit(oledDevHandle, screenTestCmd, sizeof(screenTestCmd), 100);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_ESP.data(), "runOLEDTest - OLED test pattern sent. Screen should light up fully.");
        vTaskDelay(pdMS_TO_TICKS(2000));

        uint8_t _screenNormalCmd[] = {0x00, 0xA4};
        i2c_master_transmit(oledDevHandle, _screenNormalCmd, sizeof(_screenNormalCmd), 100);
        testPassed = true;
    } else {
        ESP_LOGE(TAG_ESP.data(), "runOLEDTest - Could not send initialization command to OLED at 0x%02X", foundOLEDAddr);
    }

    i2c_master_bus_rm_device(oledDevHandle);
    return testPassed;
}*/

bool
ESP32Tests::run(void) {
    if(_chip == TargetChip::Undefined) {
        ESP_LOGE(TAG_ESP.data(), "runBlinkTest - ESP32 board not identified!");
        return false;
    }

    
    i2c_master_bus_handle_t busHandle = nullptr;
    bool isI2COk = initI2CBus(busHandle); 


    bool blinkOk        = opt()._runBlinkTest ? runBlinkTest() : true;
    bool bme280Ok       = isI2COk ? ( opt()._runBME280Test ? runBME280Test(busHandle) : true ) : false;
    bool soilSensorOk   = opt()._runSoilSensorTest ? runSoilSensorTest() : true;
    bool sdCardOk       = opt()._runSDCardTest ? runSDCardTest() : true;
    bool oledOk         = isI2COk ? ( opt()._runOLEDTest ? runOLEDTest(busHandle) : true ) : false;

    if (busHandle != nullptr) {
        esp_err_t err = i2c_del_master_bus(busHandle);
        if (err != ESP_OK) {
            ESP_LOGW(TAG_ESP.data(), "run - Warning: Failed to release I2C bus handle: %s", esp_err_to_name(err));
        }
    }

    return (blinkOk && bme280Ok && sdCardOk && soilSensorOk && oledOk);
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
