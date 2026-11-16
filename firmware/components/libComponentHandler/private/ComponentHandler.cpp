// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "components/libComponentHandler/public/ComponentHandler.h"
#include "components/libUtils/public/firmwareUtils.h"
#include "components/libSensors/public/BME280Firmware.h"
#include "components/libSensors/public/SoilMoistureFirmware.h"
#include "components/libStorage/public/Storage.h"

#include <cinttypes>
#include <cstdio>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace FIRMWARE {

namespace {
constexpr const char *TAG = "ComponentHandler";
constexpr size_t kQueueLength = 10;

struct TaskContext {
    FIRMWARE::BME280::BME280Firmware *bme280;
    FIRMWARE::SOIL_MOISTURE::SoilMoistureFirmware *soilMoisture;
    FIRMWARE::STORAGE::Storage *storage;
    QueueHandle_t sensorQueue;
};
} // namespace

void
ComponentHandler::sensorTask(void *pvParameters) {
    auto *ctx = static_cast<TaskContext *>(pvParameters);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1000); // 1 Hz (1 second)

    while (true) {
        vTaskDelayUntil(&lastWakeTime, period);

        reading_t reading = {};
        reading.seq = (ctx->storage != nullptr) ? ctx->storage->nextSeq() : 0;
        reading.timestamp_ms = static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
        reading.flags = 0;

        float temp = 0.0f;
        float press = 0.0f;
        float hum = 0.0f;
        if (ctx->bme280 != nullptr && ctx->bme280->readBME280Data(&temp, &press, &hum)) {
            reading.temp = temp;
            reading.hum = hum;
            reading.pressure = press;
        } else {
            reading.flags |= (1 << 0); // Bit 0: BME280 read error flag
        }

        uint16_t rawAdc = 0;
        float moisturePercentage = 0.0f;
        if (ctx->soilMoisture != nullptr && ctx->soilMoisture->readSoilMoisture(&rawAdc, &moisturePercentage)) {
            reading.moisture = moisturePercentage;
        } else {
            reading.flags |= (1 << 1); // Bit 1: Soil moisture read error flag
        }

        if (xQueueSend(ctx->sensorQueue, &reading, 0) != pdPASS) {
            ESP_LOGW(TAG, "sensorTask - Queue full, dropped reading seq: %" PRIu32, reading.seq);
        }
    }
}

void
ComponentHandler::storageTask(void *pvParameters) {
    auto *ctx = static_cast<TaskContext *>(pvParameters);
    reading_t reading = {};

    while (true) {
        if (xQueueReceive(ctx->sensorQueue, &reading, portMAX_DELAY) == pdPASS) {
            const bool saved = (ctx->storage != nullptr) && ctx->storage->appendReading(reading);

            std::printf("[%s] seq=%" PRIu32 ", timestamp_ms=%" PRIu32 ", temp=%.2f C, hum=%.2f %%, pressure=%.2f hPa, moisture=%.2f %%, flags=0x%04" PRIX32 "\n",
                        saved ? "SD_OK" : "SD_ERR",
                        reading.seq,
                        reading.timestamp_ms,
                        reading.temp,
                        reading.hum,
                        reading.pressure,
                        reading.moisture,
                        reading.flags);
            std::fflush(stdout);
        }
    }
}

void
ComponentHandler::run() {
    if(!initI2CConnection()) {
        return;
    }

    if(!initADC()) {
        return;
    } 

    if(opt()._serialRun) {
        FIRMWARE::BME280::BME280Firmware bme280(_i2cBusHandle);
        FIRMWARE::SOIL_MOISTURE::SoilMoistureFirmware soilMoisture(_adcHandle);

        while(true) {
            bme280.run();
            soilMoisture.run();
        }
        return;
    }

    _sensorQueue = xQueueCreate(kQueueLength, sizeof(reading_t));
    if (_sensorQueue == nullptr) {
        ESP_LOGE(TAG, "run - Failed to create sensor queue");
        return;
    }

    //FIRMWARE::STORAGE::Storage::Config storageCfg;
    //storageCfg.maxFileSize = opt()._maxRingFileBytes;

    static FIRMWARE::STORAGE::Storage storage;
    if (!storage.init()) {
        ESP_LOGE(TAG, "run - Failed to initialize SD ring buffer storage");
        return;
    }

    static FIRMWARE::BME280::BME280Firmware bme280(_i2cBusHandle);
    static FIRMWARE::SOIL_MOISTURE::SoilMoistureFirmware soilMoisture(_adcHandle);

    static TaskContext taskCtx = {
        .bme280       = &bme280,
        .soilMoisture = &soilMoisture,
        .storage      = &storage,
        .sensorQueue  = _sensorQueue
    };

    ESP_LOGI(TAG, "Creating storage_task and sensor_task...");
    // Note: 6144 stack for storage_task because FatFs + VFS + stdio file ops use more stack than printf
    xTaskCreatePinnedToCore(storageTask, "storage_task", 6144, &taskCtx, 4, nullptr, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(sensorTask, "sensor_task", 4096, &taskCtx, 5, nullptr, tskNO_AFFINITY);
    ESP_LOGI(TAG, "Tasks started successfully");

    while(true) {
        vTaskDelay(portMAX_DELAY);
    }
}

bool
ComponentHandler::initI2CConnection() {
    return FIRMWARE::UTILS::FirmwareUtils::initI2CBus(_i2cBusHandle);
}

bool
ComponentHandler::initADC() {
    return FIRMWARE::UTILS::FirmwareUtils::initADC(_adcHandle);
}

} // namespace FIRMWARE
