// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "testMain/public/TestFirmware.h"
#include "testComponents/public/ESP32Tests.h"
#include "esp_log.h"


#include <filesystem>

namespace FIRMWARE {
namespace TEST {

bool 
TestFirmware::runFirmwareTest(void) const {
    bool isRunSuccess = true;
    if (_opt._runComponentTest) {
        isRunSuccess = runComponentTest() ? isRunSuccess : false;
    }

    if(isRunSuccess) {
        ESP_LOGI(TAG_ESP.data(), "runFirmwareTest - All Firmware Tests are PASSED");
    }
    return isRunSuccess;
}

bool 
TestFirmware::runComponentTest(void) const {
    bool isRunSuccess = true;
    if(_opt._runComponentTest) {
        ESP_LOGI(TAG_ESP.data(), "runComponentTest - Testing start");
        
        FIRMWARE::TEST::COMPTEST::ESP32Tests esp32Tests;
        esp32Tests.opt()._runBlinkTest      = true;
        esp32Tests.opt()._runBME280Test     = true;
        esp32Tests.opt()._runSoilSensorTest = false;
        esp32Tests.opt()._runSDCardTest     = false;
        esp32Tests.opt()._runOLEDTest       = false;
        isRunSuccess = esp32Tests.run();
    } else {
        ESP_LOGW(TAG_ESP.data(), "runComponentTest - Test didn't run because user disabled");
    }

    if(isRunSuccess) {
        ESP_LOGI(TAG_ESP.data(), "runComponentTest - All Component Tests are PASSED");
    }
    return isRunSuccess;
}


} // namespace TEST
} // namespace FIRMWARE