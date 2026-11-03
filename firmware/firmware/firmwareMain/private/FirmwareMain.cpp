// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "firmwareMain/public/FirmwareMain.h"
#include "firmwareComponents/libFirmwareUtils/public/firmwareUtils.h"
#include "firmwareComponents/libBME280/public/BME280Firmware.h"


namespace FIRMWARE {

void
FirmwareMain::run () {
    if(!initI2CConnection()) {
        return;
    }

    FIRMWARE::BME280::BME280Firmware bme280(_i2cBusHandle);
    while(true) {
        bme280.run();
    }
}

bool
FirmwareMain::initI2CConnection() {
    return FIRMWARE::UTILS::FirmwareUtils::initI2CBus(_i2cBusHandle);
}

} // namespace FIRMWARE