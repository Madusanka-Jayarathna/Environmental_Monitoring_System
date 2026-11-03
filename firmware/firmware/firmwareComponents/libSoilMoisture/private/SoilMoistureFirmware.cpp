// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include <algorithm>
#include <string_view>

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "firmwareComponents/libSoilMoisture/public/SoilMoistureFirmware.h"

namespace FIRMWARE {
namespace SOIL_MOISTURE {

namespace {
constexpr std::string_view TAG = "FirmwareSoilMoisture";

// Hardware Configuration for ESP32 ADC1
constexpr adc_channel_t kAdcChannel   = ADC_CHANNEL_6; // GPIO 34
constexpr adc_atten_t kAdcAttenuation = ADC_ATTEN_DB_12; // ESP-IDF v5.x 0-3.3V range

// Default 2-point raw calibration values (Dry Air = ~3000, Water = ~1200)
constexpr SoilMoistureFirmware::CalibrationPoints kDefaultCalibration = {
    .rawDry = 3000, // 0.0% Moisture
    .rawWet = 1200  // 100.0% Moisture
};
}

SoilMoistureFirmware::SoilMoistureFirmware(adc_oneshot_unit_handle_t adcHandle):
    _adcHandle(adcHandle),
    _coefficients({1.0f, 0.0f}),
    _isInitialized(false)
{
    computeCoefficients(kDefaultCalibration);
    initChannel();
}

SoilMoistureFirmware::SoilMoistureFirmware(adc_oneshot_unit_handle_t adcHandle, const CalibrationPoints &userConfig):
    _adcHandle(adcHandle),
    _coefficients({1.0f, 0.0f}),
    _isInitialized(false)
{
    computeCoefficients(userConfig);
    initChannel();
}

SoilMoistureFirmware::~SoilMoistureFirmware() {
}

void
SoilMoistureFirmware::computeCoefficients(const CalibrationPoints &userConfig) {
    float deltaRaw = static_cast<float>(userConfig.rawWet) - static_cast<float>(userConfig.rawDry);
    if (deltaRaw == 0.0f) {
        ESP_LOGE(TAG.data(), "computeCoefficients - Invalid calibration bounds (rawDry == rawWet)");
        _coefficients = { .slope = 0.0f, .offset = 0.0f };
        return;
    }

    _coefficients.slope = 100.0f / deltaRaw;
    _coefficients.offset = -(_coefficients.slope * static_cast<float>(userConfig.rawDry));
}

void
SoilMoistureFirmware::setCalibrationConfig(const CalibrationPoints &userConfig) {
    computeCoefficients(userConfig);
}

void
SoilMoistureFirmware::initChannel() {
    if (_adcHandle == nullptr) {
        ESP_LOGE(TAG.data(), "initChannel - ADC handle is null");
        _isInitialized = false;
        return;
    }

    adc_oneshot_chan_cfg_t chanConfig = {};
    chanConfig.atten = kAdcAttenuation;
    chanConfig.bitwidth = ADC_BITWIDTH_DEFAULT;

    esp_err_t err = adc_oneshot_config_channel(_adcHandle, kAdcChannel, &chanConfig);
    if (err != ESP_OK) {
        ESP_LOGE(TAG.data(), "initChannel - Failed to config ADC channel: %s", esp_err_to_name(err));
        _isInitialized = false;
        return;
    }

    _isInitialized = true;
    ESP_LOGI(TAG.data(), "Soil Moisture ADC Channel Initialized on GPIO 34 (ADC1_CH6)");
}

uint16_t
SoilMoistureFirmware::sampleADCAverage(size_t numSamples) {
    if (!_isInitialized || _adcHandle == nullptr) {
        return 0;
    }

    uint32_t sampleSum = 0;
    int rawValue = 0;

    for (size_t i = 0; i < numSamples; ++i) {
        esp_err_t err = adc_oneshot_read(_adcHandle, kAdcChannel, &rawValue);
        if (err == ESP_OK) {
            sampleSum += static_cast<uint32_t>(rawValue);
        } else {
            ESP_LOGE(TAG.data(), "sampleADCAverage - ADC read failed on sample %d: %s", i, esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Short 10ms sampling interval to settle noise
    }

    return static_cast<uint16_t>(sampleSum / numSamples);
}

float
SoilMoistureFirmware::applyCalibration(uint16_t rawAdc) const {
    float rawFloat = static_cast<float>(rawAdc);
    float percentage = (_coefficients.slope * rawFloat) + _coefficients.offset;

    return std::clamp(percentage, 0.0f, 100.0f);
}

bool
SoilMoistureFirmware::readSoilMoisture(uint16_t *rawAdcAverage, float *calibratedPercentage) {
    if (!_isInitialized) {
        ESP_LOGE(TAG.data(), "readSoilMoisture - ADC Not Initialized");
        return false;
    }

    uint16_t rawAvg = sampleADCAverage(10); // 10-sample moving average

    if (rawAdcAverage != nullptr) {
        *rawAdcAverage = rawAvg;
    }

    if (calibratedPercentage != nullptr) {
        *calibratedPercentage = applyCalibration(rawAvg);
    }

    return true;
}

void
SoilMoistureFirmware::run() {
    if (!_isInitialized) {
        ESP_LOGE(TAG.data(), "run - Soil Moisture Firmware Not Initialized");
        return;
    }

    ESP_LOGI(TAG.data(), "--- Starting 3 Soil Moisture Telemetry Reads ---");

    for (int i = 1; i <= 3; ++i) {
        uint16_t rawAdc = 0;
        float moisturePercentage = 0.0f;

        if (readSoilMoisture(&rawAdc, &moisturePercentage)) {
            ESP_LOGI(TAG.data(), "run - Reading [%d/3] -> Raw ADC: %u | Moisture: %.2f %%", 
                     i, rawAdc, moisturePercentage);
        } else {
            ESP_LOGE(TAG.data(), "run - Reading [%d/3] Failed!", i);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    ESP_LOGI(TAG.data(), "\nrun - Finished 3 Soil Moisture Telemetry Reads");
}

} // namespace SOIL_MOISTURE
} // namespace FIRMWARE