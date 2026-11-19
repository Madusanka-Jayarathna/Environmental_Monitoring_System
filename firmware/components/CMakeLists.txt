# Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
# Use of this source code is governed by the MIT license.

idf_component_register(
    SRCS 
        "libComponentHandler/private/ComponentHandler.cpp"
        "libSensors/private/BME280Firmware.cpp"
        "libSensors/private/SoilMoistureFirmware.cpp"
        "libUtils/private/firmwareUtils.cpp"
        "libStorage/private/Storage.cpp"
        "libNetwork/private/WifiManager.cpp"
        "libNetwork/private/MqttClient.cpp"
    INCLUDE_DIRS 
        ".."
        "."
        "libComponentHandler/public"
        "libSensors/public"
        "libUtils/public"
        "libStorage/public"
        "libNetwork/public"
    PRIV_INCLUDE_DIRS
        "."
    PRIV_REQUIRES
        driver
        esp_adc
        esp_event
        esp_netif
        esp_timer
        esp_wifi
        fatfs
        led_strip
        mqtt
        nvs_flash
        json
    )
