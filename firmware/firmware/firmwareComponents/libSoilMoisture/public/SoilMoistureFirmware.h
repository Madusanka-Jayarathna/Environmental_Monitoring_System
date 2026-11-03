// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <cstddef>
#include <cstdint>

struct adc_oneshot_unit_ctx_t;
using adc_oneshot_unit_handle_t = adc_oneshot_unit_ctx_t*;

namespace FIRMWARE {
namespace SOIL_MOISTURE {

class SoilMoistureFirmware {
public:
    // 2-Point Linear Calibration Reference Points (Raw ADC vs Actual Moisture %)
    struct CalibrationPoints {
        uint16_t rawDry;     // Measured raw ADC in open air (0% moisture)
        uint16_t rawWet;     // Measured raw ADC submerged in water (100% moisture)
    };

    // Pre-computed constants calculated once at startup
    struct LinearCoefficients {
        float slope;
        float offset;
    };

public:
    explicit SoilMoistureFirmware(adc_oneshot_unit_handle_t adcHandle);
    SoilMoistureFirmware(adc_oneshot_unit_handle_t adcHandle, const CalibrationPoints &userConfig);
    ~SoilMoistureFirmware();

    void run();
    void setCalibrationConfig(const CalibrationPoints &userConfig);
    bool readSoilMoisture(uint16_t *rawAdcAverage, float *calibratedPercentage);

private:
    void initChannel();
    void computeCoefficients(const CalibrationPoints &userConfig);
    uint16_t sampleADCAverage(size_t numSamples = 10);
    float applyCalibration(uint16_t rawAdc) const;

private:
    adc_oneshot_unit_handle_t   _adcHandle;
    LinearCoefficients          _coefficients;
    bool                        _isInitialized;
};

} // namespace SOIL_MOISTURE
} // namespace FIRMWARE
