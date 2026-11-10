# Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
# Use of this source code is governed by the MIT license.

idf_component_register(
    SRCS 
        "libComponentHandler/private/ComponentHandler.cpp"
        "libSensors/private/BME280Firmware.cpp"
        "libSensors/private/SoilMoistureFirmware.cpp"
        "libSensors/private/firmwareUtils.cpp"
    INCLUDE_DIRS 
        ".."
        "."
        "libComponentHandler/public"
        "libSensors/public"
    PRIV_INCLUDE_DIRS
        "."
    PRIV_REQUIRES
        driver 
        esp_adc
        esp_timer
        fatfs
        led_strip
    )
