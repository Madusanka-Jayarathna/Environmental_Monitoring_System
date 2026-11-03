// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;

struct adc_oneshot_unit_ctx_t;
using adc_oneshot_unit_handle_t = adc_oneshot_unit_ctx_t*;

namespace FIRMWARE {
class FirmwareMain {
public:
    void run();

private:
    bool initI2CConnection();
    bool initADC();

private:
    i2c_master_bus_handle_t         _i2cBusHandle = nullptr;
    adc_oneshot_unit_handle_t       _adcHandle    = nullptr;

};
} // namespace FIRMWARE