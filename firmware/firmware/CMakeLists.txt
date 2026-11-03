# Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
# Use of this source code is governed by the MIT license.

idf_component_register(
    SRCS 
        "firmwareMain/private/FirmwareMain.cpp"
        "firmwareComponents/libBME280/private/BME280Firmware.cpp"
        "firmwareComponents/libFirmwareUtils/private/firmwareUtils.cpp"
    INCLUDE_DIRS 
        "firmwareMain/public"
    PRIV_INCLUDE_DIRS
        "."
    PRIV_REQUIRES
        driver 
        esp_adc
        fatfs
        led_strip
    )
