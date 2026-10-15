# Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
# Use of this source code is governed by the MIT license.

idf_component_register(
    SRCS 
        "testMain/private/TestFirmware.cpp"
        "testComponents/private/ESP32Tests.cpp"
    INCLUDE_DIRS 
        "testMain/public"
    PRIV_INCLUDE_DIRS
        "."
    PRIV_REQUIRES
        driver 
        led_strip
    )
