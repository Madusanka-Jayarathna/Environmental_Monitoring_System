#pragma once

#include "esp_err.h"
#include <cstddef>
#include <cstdint>

struct i2c_master_bus_t {
    int port;
};
using i2c_master_bus_handle_t = i2c_master_bus_t*;

struct i2c_master_dev_t {
    uint8_t addr;
};
using i2c_master_dev_handle_t = i2c_master_dev_t*;

typedef enum {
    I2C_NUM_0 = 0,
    I2C_NUM_1
} i2c_port_num_t;

typedef int gpio_num_t;

typedef enum {
    I2C_CLK_SRC_DEFAULT = 0
} i2c_clock_source_t;

typedef enum {
    I2C_ADDR_BIT_LEN_7 = 0
} i2c_addr_bit_len_t;

typedef struct {
    i2c_port_num_t i2c_port;
    gpio_num_t sda_io_num;
    gpio_num_t scl_io_num;
    i2c_clock_source_t clk_source;
    uint8_t glitch_ignore_cnt;
    struct {
        uint32_t enable_internal_pullup: 1;
    } flags;
} i2c_master_bus_config_t;

typedef struct {
    i2c_addr_bit_len_t dev_addr_length;
    uint16_t device_address;
    uint32_t scl_speed_hz;
} i2c_device_config_t;

inline esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *bus_config, i2c_master_bus_handle_t *ret_bus_handle) {
    if (!bus_config || !ret_bus_handle) return ESP_ERR_INVALID_ARG;
    *ret_bus_handle = new i2c_master_bus_t{0};
    return ESP_OK;
}

inline esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus_handle) {
    delete bus_handle;
    return ESP_OK;
}

inline esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus_handle, const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_handle) {
    if (!bus_handle || !dev_config || !ret_handle) return ESP_ERR_INVALID_ARG;
    *ret_handle = new i2c_master_dev_t{static_cast<uint8_t>(dev_config->device_address)};
    return ESP_OK;
}

inline esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t handle) {
    delete handle;
    return ESP_OK;
}

// BME280 Mock register table
inline uint8_t g_mock_bme280_regs[256] = {};
inline void init_mock_bme280_regs() {
    g_mock_bme280_regs[0xD0] = 0x60; // BME280 CHIP_ID
    // Calibration registers: dig_T1 = 27504, dig_T2 = 26435, dig_T3 = -1000
    g_mock_bme280_regs[0x88] = 0x70; g_mock_bme280_regs[0x89] = 0x6B;
    g_mock_bme280_regs[0x8A] = 0x43; g_mock_bme280_regs[0x8B] = 0x67;
    g_mock_bme280_regs[0x8C] = 0x18; g_mock_bme280_regs[0x8D] = 0xFC;
}

inline esp_err_t i2c_master_transmit(i2c_master_dev_handle_t i2c_dev, const uint8_t *write_buffer, size_t write_size, int xfer_timeout_ms) {
    (void)i2c_dev; (void)write_buffer; (void)write_size; (void)xfer_timeout_ms;
    return ESP_OK;
}

inline esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t i2c_dev, const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size, int xfer_timeout_ms) {
    (void)xfer_timeout_ms;
    if (!i2c_dev) return ESP_ERR_INVALID_ARG;
    if (write_size == 1 && read_buffer) {
        uint8_t reg = write_buffer[0];
        for (size_t i = 0; i < read_size; ++i) {
            read_buffer[i] = g_mock_bme280_regs[(reg + i) & 0xFF];
        }
    }
    return ESP_OK;
}
