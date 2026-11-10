# Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
# Use of this source code is governed by the MIT license.

idf_component_register(
    SRCS 
        "componentTests/ESP32Tests.cpp"
    INCLUDE_DIRS 
        "componentTests"
        "functionalityTests"
    PRIV_REQUIRES
        driver 
        esp_adc
        fatfs
        led_strip
        unity
    )
