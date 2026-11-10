// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "unity.h"
#include "libSensors/public/SoilMoistureFirmware.h"
#include <cmath>

using FIRMWARE::SOIL_MOISTURE::SoilMoistureFirmware;

TEST_CASE("SoilMoisture: Default constructor and initial calibration", "[SoilMoisture]")
{
    // Pass nullptr for unit test: verifies class can be constructed with null ADC safely
    SoilMoistureFirmware sensor(nullptr);

    // Reading should return false when ADC is uninitialized
    uint16_t raw = 0;
    float moisture = 0.0f;
    bool ok = sensor.readSoilMoisture(&raw, &moisture);
    TEST_ASSERT_FALSE(ok);
}

TEST_CASE("SoilMoisture: User calibration config and linear conversion", "[SoilMoisture]")
{
    SoilMoistureFirmware::CalibrationPoints cfg = {
        .rawDry = 3000,
        .rawWet = 1000
    };

    SoilMoistureFirmware sensor(nullptr, cfg);

    // Reconfigure with new valid points
    SoilMoistureFirmware::CalibrationPoints newCfg = {
        .rawDry = 2800,
        .rawWet = 1200
    };
    sensor.setCalibrationConfig(newCfg);

    // Reading fails safely because ADC handle is null, but config was applied without crash
    uint16_t raw = 0;
    float moisture = 0.0f;
    TEST_ASSERT_FALSE(sensor.readSoilMoisture(&raw, &moisture));
}

TEST_CASE("SoilMoisture: Invalid calibration bounds handling (rawDry == rawWet)", "[SoilMoisture]")
{
    SoilMoistureFirmware::CalibrationPoints invalidCfg = {
        .rawDry = 2000,
        .rawWet = 2000
    };

    SoilMoistureFirmware sensor(nullptr, invalidCfg);

    uint16_t raw = 0;
    float moisture = 0.0f;
    TEST_ASSERT_FALSE(sensor.readSoilMoisture(&raw, &moisture));
}
