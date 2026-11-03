// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "firmwareMain/public/FirmwareMain.h"
#include "firmwareComponents/libFirmwareUtils/public/firmwareUtils.h"
#include "firmwareComponents/libBME280/public/BME280Firmware.h"
#include "firmwareComponents/libSoilMoisture/public/SoilMoistureFirmware.h"


namespace FIRMWARE {

void
FirmwareMain::run () {
    if(!initI2CConnection()) {
        return;
    }

    if(!initADC()) {
        return;
    } 

    FIRMWARE::BME280::BME280Firmware bme280(_i2cBusHandle);
    FIRMWARE::SOIL_MOISTURE::SoilMoistureFirmware soilMoisture(_adcHandle);

    while(true) {
        bme280.run();
        soilMoisture.run();
    }
}

bool
FirmwareMain::initI2CConnection() {
    return FIRMWARE::UTILS::FirmwareUtils::initI2CBus(_i2cBusHandle);
}

bool
FirmwareMain::initADC() {
    return FIRMWARE::UTILS::FirmwareUtils::initADC(_adcHandle);
}

} // namespace FIRMWARE