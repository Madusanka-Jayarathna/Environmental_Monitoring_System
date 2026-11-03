// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <cstddef>
#include <cstdint>

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;

struct i2c_master_dev_t;
using i2c_master_dev_handle_t = i2c_master_dev_t*;

namespace FIRMWARE {
namespace BME280 {

class BME280Firmware {
public:
    struct BME280Calibration {
        // Temperature
        uint16_t digT1;
        int16_t  digT2;
        int16_t  digT3;

        // Pressure
        uint16_t digP1;
        int16_t  digP2;
        int16_t  digP3;
        int16_t  digP4;
        int16_t  digP5;
        int16_t  digP6;
        int16_t  digP7;
        int16_t  digP8;
        int16_t  digP9;

        // Humidity
        uint8_t  digH1;
        int16_t  digH2;
        uint8_t  digH3;
        int16_t  digH4;
        int16_t  digH5;
        int8_t   digH6;

        int32_t  tFine;
    };

    // User input bounds for calibration
    struct CalibrationPoints {
        float rawLow;
        float actualLow;
        float rawHigh;
        float actualHigh;
    };

    struct LinearCalibrationConfig {
        CalibrationPoints temp;
        CalibrationPoints hum;
        CalibrationPoints press;
    };

    // Pre-computed constants stored per-axis
    struct LinearCoefficients {
        float slope;
        float offset;
    };

public:
    explicit BME280Firmware(i2c_master_bus_handle_t i2cBusHandle);
    BME280Firmware(i2c_master_bus_handle_t i2cBusHandle, const LinearCalibrationConfig &userConfig);

    void run();
    void setCalibrationConfig(const LinearCalibrationConfig &userConfig);

private:
    void initBME280();
    bool readBME280Data(float *temp, float *press, float *hum);
    bool readCalibration(void);

    float compensateTemperature(int32_t adcT);
    float compensatePressure(int32_t adcP);
    float compensateHumidity(int32_t adcH);

    void computeCoefficients(const LinearCalibrationConfig &userConfig);
    void apply2PointCalibration(float *temp, float *press, float *hum) const;

    bool writeRegs(uint8_t regAddr, uint8_t data);
    bool readRegs(uint8_t regAddr, uint8_t *data, size_t len);

private:
    i2c_master_bus_handle_t     _i2cBusHandle;
    i2c_master_dev_handle_t     _devHandle;
    BME280Calibration           _calibration;

    // Direct pre-computed constants (Calculated once)
    LinearCoefficients          _tempCoeffs;
    LinearCoefficients          _humCoeffs;
    LinearCoefficients          _pressCoeffs;
};

} // namespace BME280
} // namespace FIRMWARE
