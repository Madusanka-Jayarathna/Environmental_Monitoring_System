// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;


namespace FIRMWARE {
class FirmwareMain {
public:
    void run();


private:
    bool initI2CConnection();

private:
    i2c_master_bus_handle_t         _i2cBusHandle = nullptr;


};
} // namespace FIRMWARE