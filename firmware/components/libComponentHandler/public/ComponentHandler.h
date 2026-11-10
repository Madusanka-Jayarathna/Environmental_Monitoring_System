// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <cstdint>

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;

struct adc_oneshot_unit_ctx_t;
using adc_oneshot_unit_handle_t = adc_oneshot_unit_ctx_t*;

struct QueueDefinition;
typedef struct QueueDefinition* QueueHandle_t;

namespace FIRMWARE {

struct reading_t {
    uint32_t seq;
    uint32_t timestamp_ms;
    float    temp;
    float    hum;
    float    pressure;
    float    moisture;
    uint32_t flags;
};

class ComponentHandler {
public:
    struct Option {
        Option() = default;
        bool _serialRun = false;
    };

public:
    void run();
    Option& opt() { return _opt; }
    const Option& opt() const { return _opt; }

private:
    bool initI2CConnection();
    bool initADC();

    static void sensorTask(void *pvParameters);
    static void loggerTask(void *pvParameters);

private:
    i2c_master_bus_handle_t         _i2cBusHandle = nullptr;
    adc_oneshot_unit_handle_t       _adcHandle    = nullptr;
    QueueHandle_t                   _sensorQueue  = nullptr;
    Option                          _opt;
};

} // namespace FIRMWARE
