// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "components/libStorage/public/Storage.h"
#include "components/libComponentHandler/public/ComponentHandler.h"
#include "components/libUtils/public/defines.h"

#include <cerrno>
#include <cinttypes>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdmmc_cmd.h"

namespace FIRMWARE {
namespace STORAGE {

namespace {
constexpr const char *TAG          = "Storage";
constexpr const char *kMountPoint  = "/sdcard";
constexpr const char *kFileActive  = "/sdcard/buffer.csv";
constexpr const char *kFileRot1    = "/sdcard/buffer.1.csv";
constexpr const char *kFileRot2    = "/sdcard/buffer.2.csv";
constexpr const char *kNvsNs       = "storage";
constexpr const char *kNvsKeySeq   = "last_seq";
} // namespace


Storage::Storage() = default;

Storage::~Storage() {
    if (_activeFile != nullptr) {
        auto *activeFile = static_cast<FILE *>(_activeFile);
        std::fflush(activeFile);
        fsync(fileno(activeFile));
        std::fclose(activeFile);
        _activeFile = nullptr;
    }

    if (_isMounted && _card != nullptr) {
        auto *card = static_cast<sdmmc_card_t *>(_card);
        esp_vfs_fat_sdcard_unmount(kMountPoint, card);
        _isMounted = false;
    }

    if (_nvsHandle != 0) {
        nvs_close(_nvsHandle);
        _nvsHandle = 0;
    }
}

bool
Storage::init() {
    if (!initNvs()) {
        return false;
    }

    if (!mountSdCard()) {
        return false;
    }

    if (!openActiveFile()) {
        return false;
    }
    return true;
}

bool
Storage::initNvs() {
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(kNvsNs, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "initNvs - nvs_open failed: %s", esp_err_to_name(err));
        return false;
    }
    _nvsHandle = static_cast<uint32_t>(handle);

    uint32_t savedSeq = 0;
    err = nvs_get_u32(handle, kNvsKeySeq, &savedSeq);
    if (err == ESP_OK) {
        _persistedSeq = savedSeq;
        _nextSeq      = savedSeq;
        ESP_LOGI(TAG, "initNvs - Recovered last persisted seq: %" PRIu32, _persistedSeq);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        _persistedSeq = 0;
        _nextSeq      = 0;
        ESP_LOGI(TAG, "initNvs - No saved seq found, starting at 0");
    } else {
        ESP_LOGE(TAG, "initNvs - nvs_get_u32 failed: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

bool
Storage::persistSeqToNvs(uint32_t seq) {
    auto handle = static_cast<nvs_handle_t>(_nvsHandle);
    esp_err_t err = nvs_set_u32(handle, kNvsKeySeq, seq);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "persistSeqToNvs - nvs_set_u32 failed: %s", esp_err_to_name(err));
        return false;
    }
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "persistSeqToNvs - nvs_commit failed: %s", esp_err_to_name(err));
        return false;
    }
    _persistedSeq = seq;
    return true;
}

bool
Storage::mountSdCard() {
    esp_vfs_fat_sdmmc_mount_config_t mountConfig = {};
    mountConfig.format_if_mount_failed = false;
    mountConfig.max_files              = 5;
    mountConfig.allocation_unit_size   = 16 * 1024;
    mountConfig.disk_status_check_enable = false;
    mountConfig.use_one_fat            = false;
    

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = 10000; // 10 MHz for stable jumper wire operation

    spi_bus_config_t busCfg = {};
    busCfg.mosi_io_num = static_cast<gpio_num_t>(SPI_MOSI_PIN);
    busCfg.miso_io_num = static_cast<gpio_num_t>(SPI_MISO_PIN);
    busCfg.sclk_io_num = static_cast<gpio_num_t>(SPI_CLK_PIN);
    busCfg.quadwp_io_num = -1;
    busCfg.quadhd_io_num = -1;
    busCfg.data4_io_num = -1;
    busCfg.data5_io_num = -1;
    busCfg.data6_io_num = -1;
    busCfg.data7_io_num = -1;
    busCfg.data_io_default_level = false;
    busCfg.max_transfer_sz = 4096;
    busCfg.flags = 0;
    busCfg.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO;
    busCfg.intr_flags = 0;

    esp_err_t err = spi_bus_initialize(static_cast<spi_host_device_t>(host.slot), &busCfg, SDSPI_DEFAULT_DMA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mountSdCard - spi_bus_initialize failed: %s", esp_err_to_name(err));
        return false;
    }

    sdspi_device_config_t slotConfig = SDSPI_DEVICE_CONFIG_DEFAULT();
    slotConfig.gpio_cs = static_cast<gpio_num_t>(SPI_CS_PIN);
    slotConfig.host_id = static_cast<spi_host_device_t>(host.slot);

    sdmmc_card_t *card = nullptr;
    err = esp_vfs_fat_sdspi_mount(kMountPoint, &host, &slotConfig, &mountConfig, &card);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mountSdCard - Failed to mount SD card (%s)", esp_err_to_name(err));
        return false;
    }

    _card = card;
    _isMounted = true;
    sdmmc_card_print_info(stdout, card);
    return true;
}

bool
Storage::openActiveFile() {
    struct stat st{};
    const bool isNewFile = (stat(kFileActive, &st) != 0);

    auto *activeFile = std::fopen(kFileActive, "a");
    if (activeFile == nullptr) {
        ESP_LOGE(TAG, "openActiveFile - Failed to open %s (errno=%d)", kFileActive, errno);
        return false;
    }
    _activeFile = activeFile;

    if (isNewFile || st.st_size == 0) {
        std::fprintf(activeFile, "seq,timestamp_ms,temp,hum,pressure,moisture,flags\n");
        std::fflush(activeFile);
        fsync(fileno(activeFile));
    }

    return true;
}

void
Storage::rotateIfNeeded() {
    struct stat st{};
    if (stat(kFileActive, &st) != 0) {
        return;
    }

    if (static_cast<size_t>(st.st_size) < static_cast<size_t>(opt()._maxFileSize)) {
        return;
    }

    ESP_LOGW(TAG, "rotateIfNeeded - Rotating ring buffer (%ld >= %zu bytes)",
             static_cast<long>(st.st_size), static_cast<size_t>(opt()._maxFileSize));

    if (_activeFile != nullptr) {
        auto *activeFile = static_cast<FILE *>(_activeFile);
        std::fflush(activeFile);
        fsync(fileno(activeFile));
        std::fclose(activeFile);
        _activeFile = nullptr;
    }

    // Ring policy: drop oldest (buffer.2.csv), shift buffer.1 -> buffer.2, active -> buffer.1
    unlink(kFileRot2);
    rename(kFileRot1, kFileRot2);
    if (rename(kFileActive, kFileRot1) != 0) {
        ESP_LOGE(TAG, "rotateIfNeeded - Rename failed (errno=%d)", errno);
    }

    openActiveFile();
}

uint32_t
Storage::nextSeq() {
    return ++_nextSeq;
}

bool
Storage::appendReading(const reading_t &reading) {
    if (!_isMounted) {
        return false;
    }

    rotateIfNeeded();

    if (_activeFile == nullptr && !openActiveFile()) {
        return false;
    }

    auto *activeFile = static_cast<FILE *>(_activeFile);
    const int written = std::fprintf(
        activeFile,
        "%" PRIu32 ",%" PRIu32 ",%.2f,%.2f,%.2f,%.2f,0x%04" PRIX32 "\n",
        reading.seq,
        reading.timestamp_ms,
        reading.temp,
        reading.hum,
        reading.pressure,
        reading.moisture,
        reading.flags
    );

    if (written <= 0) {
        ESP_LOGE(TAG, "appendReading - Write failed for seq=%" PRIu32, reading.seq);
        return false;
    }

    std::fflush(activeFile);
    fsync(fileno(activeFile));

    // Commit sequence to NVS AFTER SD flush so reboot never causes duplicates or gaps
    return persistSeqToNvs(reading.seq);
}

} // namespace STORAGE
} // namespace FIRMWARE
