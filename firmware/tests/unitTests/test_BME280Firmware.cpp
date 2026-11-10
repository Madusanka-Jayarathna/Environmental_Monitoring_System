// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "unity.h"
#include "libSensors/public/BME280Firmware.h"

using FIRMWARE::BME280::BME280Firmware;

TEST_CASE("BME280: Constructor with null bus handle handles error gracefully", "[BME280]")
{
    // Test constructor when I2C bus is null
    BME280Firmware bme(nullptr);

    float temp = 0.0f;
    float press = 0.0f;
    float hum = 0.0f;

    // Read should fail safely without crashing
    bool ok = bme.readBME280Data(&temp, &press, &hum);
    TEST_ASSERT_FALSE(ok);
}

TEST_CASE("BME280: Custom calibration config applied without crash", "[BME280]")
{
    BME280Firmware::LinearCalibrationConfig cfg = {
        .temp  = { .rawLow = 0.0f, .actualLow = 0.0f, .rawHigh = 30.0f, .actualHigh = 30.0f },
        .hum   = { .rawLow = 0.0f, .actualLow = 0.0f, .rawHigh = 80.0f, .actualHigh = 80.0f },
        .press = { .rawLow = 950.0f, .actualLow = 950.0f, .rawHigh = 1050.0f, .actualHigh = 1050.0f }
    };

    BME280Firmware bme(nullptr, cfg);

    // Update calibration
    bme.setCalibrationConfig(cfg);

    float temp = 0.0f;
    float press = 0.0f;
    float hum = 0.0f;
    TEST_ASSERT_FALSE(bme.readBME280Data(&temp, &press, &hum));
}
