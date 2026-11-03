// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include <algorithm>
#include <string_view>

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "firmwareComponents/libBME280/public/BME280Firmware.h"

namespace FIRMWARE {
namespace BME280 {

namespace {
constexpr std::string_view TAG = "FirmwareBME280";

const BME280Firmware::LinearCalibrationConfig kDefaultCalibration = {
    .temp  = { .rawLow = 0.0f, .actualLow = 0.0f, .rawHigh = 27.90f, .actualHigh = 25.0f },
    .hum   = { .rawLow = 0.0f, .actualLow = 0.0f, .rawHigh = 77.85f, .actualHigh = 92.0f },
    .press = { .rawLow = 900.0f, .actualLow = 900.0f, .rawHigh = 1100.0f, .actualHigh = 1100.0f }
};

BME280Firmware::LinearCoefficients calculateAxisCoefficients(const BME280Firmware::CalibrationPoints &pts) {
    float deltaRaw = pts.rawHigh - pts.rawLow;
    if (deltaRaw == 0.0f) {
        return { .slope = 1.0f, .offset = 0.0f }; // Prevent divide by zero fallback
    }

    float slope = (pts.actualHigh - pts.actualLow) / deltaRaw;
    float offset = pts.actualLow - (slope * pts.rawLow);

    return { .slope = slope, .offset = offset };
}
}

// --- Hardware & I2C Pin Setup ---
#define I2C_MASTER_SCL_IO           22      /*!< GPIO number for I2C SCL */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number for I2C SDA */
#define I2C_MASTER_NUM              0       /*!< I2C port number */
#define I2C_MASTER_FREQ_HZ          100000  /*!< I2C master clock frequency */
#define BME280_I2C_ADDR             0x76    /*!< Default I2C address for BME280 */

// --- BME280 Registers ---
#define BME280_REG_CHIP_ID          0xD0
#define BME280_CHIP_ID_VAL          0x60
#define BME280_REG_RESET            0xE0
#define BME280_REG_CTRL_HUM         0xF2
#define BME280_REG_STATUS           0xF3
#define BME280_REG_CTRL_MEAS        0xF4
#define BME280_REG_CONFIG           0xF5
#define BME280_REG_PRESS_MSB        0xF7
#define BME280_REG_CALIB00          0x88
#define BME280_REG_CALIB26          0xE1

BME280Firmware::BME280Firmware(i2c_master_bus_handle_t i2cBusHandle):
    _i2cBusHandle(i2cBusHandle),
    _devHandle(nullptr),
    _calibration({}),
    _tempCoeffs({1.0f, 0.0f}),
    _humCoeffs({1.0f, 0.0f}),
    _pressCoeffs({1.0f, 0.0f})
{
    computeCoefficients(kDefaultCalibration);
    initBME280();
}

BME280Firmware::BME280Firmware(i2c_master_bus_handle_t i2cBusHandle, const LinearCalibrationConfig &userConfig):
    _i2cBusHandle(i2cBusHandle),
    _devHandle(nullptr),
    _calibration({}),
    _tempCoeffs({1.0f, 0.0f}),
    _humCoeffs({1.0f, 0.0f}),
    _pressCoeffs({1.0f, 0.0f})
{
    computeCoefficients(userConfig);
    initBME280();
}

void
BME280Firmware::computeCoefficients(const LinearCalibrationConfig &userConfig) {
    _tempCoeffs  = calculateAxisCoefficients(userConfig.temp);
    _humCoeffs   = calculateAxisCoefficients(userConfig.hum);
    _pressCoeffs = calculateAxisCoefficients(userConfig.press);
}

void
BME280Firmware::setCalibrationConfig(const LinearCalibrationConfig &userConfig) {
    computeCoefficients(userConfig);
}

inline void
BME280Firmware::apply2PointCalibration(float *temp, float *press, float *hum) const {
    if (temp != nullptr) {
        *temp = (_tempCoeffs.slope * (*temp)) + _tempCoeffs.offset;
    }

    if (press != nullptr) {
        *press = (_pressCoeffs.slope * (*press)) + _pressCoeffs.offset;
    }

    if (hum != nullptr) {
        float calHum = (_humCoeffs.slope * (*hum)) + _humCoeffs.offset;
        *hum = std::clamp(calHum, 0.0f, 100.0f);
    }
}

bool
BME280Firmware::readCalibration(void) {
    uint8_t b1[24];
    uint8_t b2[7];
    uint8_t h1 = 0;

    if (!readRegs(BME280_REG_CALIB00, b1, sizeof(b1)) ||
        !readRegs(0xA1, &h1, sizeof(h1)) ||
        !readRegs(BME280_REG_CALIB26, b2, sizeof(b2))) {
        return false;
    }

    _calibration.digT1 = (uint16_t)(b1[1] << 8 | b1[0]);
    _calibration.digT2 = (int16_t)(b1[3] << 8 | b1[2]);
    _calibration.digT3 = (int16_t)(b1[5] << 8 | b1[4]);

    _calibration.digP1 = (uint16_t)(b1[7] << 8 | b1[6]);
    _calibration.digP2 = (int16_t)(b1[9] << 8 | b1[8]);
    _calibration.digP3 = (int16_t)(b1[11] << 8 | b1[10]);
    _calibration.digP4 = (int16_t)(b1[13] << 8 | b1[12]);
    _calibration.digP5 = (int16_t)(b1[15] << 8 | b1[14]);
    _calibration.digP6 = (int16_t)(b1[17] << 8 | b1[16]);
    _calibration.digP7 = (int16_t)(b1[19] << 8 | b1[18]);
    _calibration.digP8 = (int16_t)(b1[21] << 8 | b1[20]);
    _calibration.digP9 = (int16_t)(b1[23] << 8 | b1[22]);

    _calibration.digH1 = h1;
    _calibration.digH2 = (int16_t)(b2[1] << 8 | b2[0]);
    _calibration.digH3 = b2[2];
    _calibration.digH4 = (int16_t)((b2[3] << 4) | (b2[4] & 0x0F));
    if ((_calibration.digH4 & 0x0800) != 0) {
        _calibration.digH4 |= static_cast<int16_t>(0xF000);
    }
    _calibration.digH5 = (int16_t)((b2[5] << 4) | (b2[4] >> 4));
    if ((_calibration.digH5 & 0x0800) != 0) {
        _calibration.digH5 |= static_cast<int16_t>(0xF000);
    }
    _calibration.digH6 = (int8_t)b2[6];

    return true;
}

void
BME280Firmware::initBME280() {
    i2c_device_config_t devConfig   = {};
    devConfig.dev_addr_length       = I2C_ADDR_BIT_LEN_7;
    devConfig.device_address        = BME280_I2C_ADDR;
    devConfig.scl_speed_hz          = I2C_MASTER_FREQ_HZ;

    esp_err_t err = i2c_master_bus_add_device(_i2cBusHandle, &devConfig, &_devHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG.data(), "initBME280 - Failed to add BME280 to I2C Bus: %s", esp_err_to_name(err));
        _devHandle = nullptr;
        return;
    }

    uint8_t chipId = 0;
    if (!readRegs(BME280_REG_CHIP_ID, &chipId, 1)) {
        _devHandle = nullptr;
        return;
    }

    if (chipId != BME280_CHIP_ID_VAL) {
        ESP_LOGE(TAG.data(), "initBME280 - Invalid Chip ID: 0x%02X (Expected: 0x%02X)", chipId, BME280_CHIP_ID_VAL);
        _devHandle = nullptr;
        return;
    }
    ESP_LOGI(TAG.data(), "BME280 detected successfully! Chip ID: 0x%02X", chipId);

    if (!readCalibration()) {
        _devHandle = nullptr;
        return;
    }

    if (!writeRegs(BME280_REG_CTRL_HUM, 0x01)   ||
        !writeRegs(BME280_REG_CONFIG, 0x00)     ||
        !writeRegs(BME280_REG_CTRL_MEAS, 0x27)) {
        _devHandle = nullptr;
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
}

float 
BME280Firmware::compensateTemperature(int32_t adcT) {
    int32_t var1, var2;
    var1 = ((((adcT >> 3) - ((int32_t)_calibration.digT1 << 1))) * ((int32_t)_calibration.digT2)) >> 11;
    var2 = (((((adcT >> 4) - ((int32_t)_calibration.digT1)) * ((adcT >> 4) - ((int32_t)_calibration.digT1))) >> 12) * ((int32_t)_calibration.digT3)) >> 14;
    _calibration.tFine = var1 + var2;

    return (float)((_calibration.tFine * 5 + 128) >> 8) / 100.0f;
}

float 
BME280Firmware::compensatePressure(int32_t adcP) {
    int64_t var1, var2, p;
    var1 = ((int64_t)_calibration.tFine) - 128000;
    var2 = var1 * var1 * (int64_t)_calibration.digP6;
    var2 = var2 + ((var1 * (int64_t)_calibration.digP5) << 17);
    var2 = var2 + (((int64_t)_calibration.digP4) << 35);
    var1 = ((var1 * var1 * (int64_t)_calibration.digP3) >> 8) + ((var1 * (int64_t)_calibration.digP2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_calibration.digP1) >> 33;

    if (var1 == 0) { 
        return 0;
    }

    p = 1048576 - adcP;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)_calibration.digP9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)_calibration.digP8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)_calibration.digP7) << 4);

    return (float)p / 25600.0f;
}

float 
BME280Firmware::compensateHumidity(int32_t adcH) {
    int32_t h;
    h = (_calibration.tFine - ((int32_t)76800));
    
    h = (
            ((((adcH << 14) - (((int32_t)_calibration.digH4) << 20) - (((int32_t)_calibration.digH5) * h)) + ((int32_t)16384)) >> 15) *
            ((
                (((((h * ((int32_t)_calibration.digH6)) >> 10) * (((h * ((int32_t)_calibration.digH3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * 
                ((int32_t)_calibration.digH2) + 
                8192
            ) >> 14)
        );

    h = (h - (((((h >> 15) * (h >> 15)) >> 7) * ((int32_t)_calibration.digH1)) >> 4));
    h = (h < 0 ? 0 : h);
    h = (h > 419430400 ? 419430400 : h);
    
    return (float)(h >> 12) / 1024.0f;
}

bool
BME280Firmware::writeRegs(uint8_t regAddr, uint8_t data) {
    uint8_t writeBuff[2] = {regAddr, data};
    esp_err_t err = i2c_master_transmit(_devHandle, writeBuff, sizeof(writeBuff), -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG.data(), "writeRegs - Failed to Transmit: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool
BME280Firmware::readRegs(uint8_t regAddr, uint8_t *data, size_t len) {
    esp_err_t err = i2c_master_transmit_receive(_devHandle, &regAddr, 1, data, len, -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG.data(), "readRegs - Failed to Receive Transmit: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool
BME280Firmware::readBME280Data(float *temp, float *press, float *hum) {
    uint8_t raw[8];
    if (!readRegs(BME280_REG_PRESS_MSB, raw, 8)) {
        return false;
    }

    int32_t adcP = (int32_t)(((uint32_t)raw[0] << 12) | ((uint32_t)raw[1] << 4) | ((uint32_t)raw[2] >> 4));
    int32_t adcT = (int32_t)(((uint32_t)raw[3] << 12) | ((uint32_t)raw[4] << 4) | ((uint32_t)raw[5] >> 4));
    int32_t adcH = (int32_t)(((uint32_t)raw[6] << 8) | (uint32_t)raw[7]);

    *temp   = compensateTemperature(adcT);
    *press  = compensatePressure(adcP);
    *hum    = compensateHumidity(adcH);

    apply2PointCalibration(temp, press, hum);

    return true;
}

void
BME280Firmware::run() {
    if (_i2cBusHandle == nullptr || _devHandle == nullptr) {
        ESP_LOGE(TAG.data(), "run - BME280 Firmware Not Initialized");
        return;
    }

    for (int i = 1; i < 4; ++i) {
        float temp      = 0.0f;
        float pressure  = 0.0f;
        float humidity  = 0.0f;

        if (readBME280Data(&temp, &pressure, &humidity)) {
            ESP_LOGI(TAG.data(), "run - Reading [%d/3] -> Temp: %.2f C | Humidity: %.2f %% | Pressure: %.2f hPa", i, temp, humidity, pressure);
        } else {
            ESP_LOGE(TAG.data(), "run - Reading [%d/3] Failed!", i);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    ESP_LOGI(TAG.data(), "\nrun - Finished 3 BME280 Telemetry Reads");
}

} // namespace BME280
} // namespace FIRMWARE
