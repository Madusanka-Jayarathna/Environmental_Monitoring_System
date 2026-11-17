#pragma once 

#include <cstdint>

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;

struct adc_oneshot_unit_ctx_t;
using adc_oneshot_unit_handle_t = adc_oneshot_unit_ctx_t*;

namespace FIRMWARE {
namespace UTILS {
class FirmwareUtils {

public:
    FirmwareUtils()                                 = delete;
    ~FirmwareUtils()                                = delete;
    FirmwareUtils(const FirmwareUtils&)             = delete;
    FirmwareUtils& operator=(const FirmwareUtils&)  = delete;

    static bool initI2CBus(i2c_master_bus_handle_t& busHandle);
    static bool initADC(adc_oneshot_unit_handle_t& adcHandle);
    static bool initNVS();

    static bool initStatusPin(uint8_t p, bool isOutput);
    static void setStatusPin(uint8_t p, bool isHigh);

private:

};

} // namespace UTILS
} // namespace FIRMWARE
