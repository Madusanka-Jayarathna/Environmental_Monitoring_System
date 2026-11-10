// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "unity.h"
#include "libComponentHandler/public/ComponentHandler.h"

using FIRMWARE::reading_t;

TEST_CASE("ComponentHandler: reading_t struct memory layout and defaults", "[ComponentHandler]")
{
    reading_t reading = {};
    reading.seq = 100;
    reading.timestamp_ms = 5000;
    reading.temp = 24.5f;
    reading.hum = 55.0f;
    reading.pressure = 1013.25f;
    reading.moisture = 40.2f;
    reading.flags = 0;

    TEST_ASSERT_EQUAL_UINT32(100, reading.seq);
    TEST_ASSERT_EQUAL_UINT32(5000, reading.timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 24.5f, reading.temp);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 55.0f, reading.hum);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1013.25f, reading.pressure);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 40.2f, reading.moisture);
    TEST_ASSERT_EQUAL_HEX32(0, reading.flags);
}

TEST_CASE("ComponentHandler: Error flag bit manipulation", "[ComponentHandler]")
{
    constexpr uint32_t FLAG_BME280_ERR = (1 << 0);
    constexpr uint32_t FLAG_SOIL_ERR   = (1 << 1);

    reading_t reading = {};
    reading.flags |= FLAG_BME280_ERR;

    TEST_ASSERT_TRUE((reading.flags & FLAG_BME280_ERR) != 0);
    TEST_ASSERT_FALSE((reading.flags & FLAG_SOIL_ERR) != 0);

    reading.flags |= FLAG_SOIL_ERR;
    TEST_ASSERT_TRUE((reading.flags & FLAG_BME280_ERR) != 0);
    TEST_ASSERT_TRUE((reading.flags & FLAG_SOIL_ERR) != 0);
}
