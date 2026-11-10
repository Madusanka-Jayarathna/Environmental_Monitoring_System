// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;

struct led_strip_t;
using led_strip_handle_t = led_strip_t*;

namespace FIRMWARE {
namespace TEST {
namespace COMPTEST {

class ESP32Tests {
public:
    enum class TargetChip {
        ESP32,
        ESP32S3,
        Undefined
    };

    enum class PortDir {
        INPUT,
        OUTPUT
    };

    struct Option {
        Option() = default;
        bool _runBlinkTest      = true;
        bool _runBME280Test     = true;
        bool _runSoilSensorTest = true;
        bool _runSDCardTest     = true;
        bool _runOLEDTest       = true;
    };

public:
    ESP32Tests();
    bool run(void);
    Option& opt() { return _opt; }
    const Option& opt() const { return _opt; }
    

private:
    bool runBlinkTest( void );


    TargetChip checkChipInfo();
    bool blinkBoardLED(void);
    bool runBME280Test(i2c_master_bus_handle_t& busHandle);
    bool runSoilSensorTest(void);
    bool runSDCardTest(void);
    bool runOLEDTest(i2c_master_bus_handle_t& busHandle);
    bool initI2CBus(i2c_master_bus_handle_t& busHandle);
    bool blink(const led_strip_handle_t &led_strip);
    bool blink();
    bool configurePin(unsigned char pin, const PortDir dir);

private:
    ESP32Tests::TargetChip          _chip;
    Option                          _opt;

};
} // namespace COMPTEST
} // namespace TEST
} // namespace FIRMWARE
