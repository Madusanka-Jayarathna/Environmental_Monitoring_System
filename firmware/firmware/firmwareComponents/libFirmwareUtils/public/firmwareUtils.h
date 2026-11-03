#pragma once 

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;

namespace FIRMWARE {
namespace UTILS {
class FirmwareUtils {

public:
    FirmwareUtils()                                 = delete;
    ~FirmwareUtils()                                = delete;
    FirmwareUtils(const FirmwareUtils&)             = delete;
    FirmwareUtils& operator=(const FirmwareUtils&)  = delete;

    static bool initI2CBus(i2c_master_bus_handle_t& busHandle);

private:

};

} // namespace UTILS
} // namespace FIRMWARE
