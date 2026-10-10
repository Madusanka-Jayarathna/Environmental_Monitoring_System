// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "testMain/public/TestFirmware.h"
#include "testComponents/public/ESP32Tests.h"


#include <filesystem>

namespace FIRMWARE {
namespace TEST {

bool TestFirmware::runFirmwareTest(void) const {
    if (_opt._runComponentTest) {
        std::cout << "this one is called\n";
        FIRMWARE::TEST::ESP32Tests esp32Tests;
        esp32Tests.runBlinkTest();
        std::cout << "this one exit\n";

        return runComponentTest();
    }
    return true;
}

bool TestFirmware::runComponentTest(void) const {
    return true;
}


} // namespace TEST
} // namespace FIRMWARE