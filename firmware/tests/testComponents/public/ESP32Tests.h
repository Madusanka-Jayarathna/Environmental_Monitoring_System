// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once
#include <iostream>
#include <string_view>
#include <vector>
#include <cstdint>

#include "led_strip.h"

namespace FIRMWARE {
namespace TEST {
namespace COMPTEST {
    inline constexpr std::string_view TAG_ESP = "ComponentTests";

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

public:
    ESP32Tests();
    bool run(void);
    

private:
    bool runBlinkTest( void );


    TargetChip checkChipInfo();
    bool blinkBoardLED(void);
    bool runBME280Test(void);
    bool runSoilSensorTest(void);

    bool blink(const led_strip_handle_t &led_strip);
    bool blink();
    bool configurePin(const uint8_t pin, const PortDir dir);

private:
    ESP32Tests::TargetChip          _chip;

};
} // namespace COMPTEST
} // namespace TEST
} // namespace FIRMWARE

