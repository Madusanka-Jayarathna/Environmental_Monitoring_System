// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include "firmwareMain/public/FirmwareMain.h"
#include "firmwareComponents/libFirmwareUtils/public/firmwareUtils.h"
#include "firmwareComponents/libBME280/public/BME280Firmware.h"
#include "firmwareComponents/libSoilMoisture/public/SoilMoistureFirmware.h"

#include <cinttypes>
#include <cstdio>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace FIRMWARE {

namespace {
constexpr const char *TAG = "FirmwareMain";
constexpr size_t kQueueLength = 10;

// Sensor context to pass to sensor task
struct SensorTaskContext {
    FIRMWARE::BME280::BME280Firmware *bme280;
    FIRMWARE::SOIL_MOISTURE::SoilMoistureFirmware *soilMoisture;
    QueueHandle_t sensorQueue;
};
}

void
FirmwareMain::sensorTask(void *pvParameters) {
    auto *ctx = static_cast<SensorTaskContext *>(pvParameters);
    uint32_t seq = 0;
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1000); // 1 Hz (1 second)

    while (true) {
        vTaskDelayUntil(&lastWakeTime, period);

        reading_t reading = {};
        reading.seq = seq++;
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
FirmwareMain::loggerTask(void *pvParameters) {
    auto sensorQueue = static_cast<QueueHandle_t>(pvParameters);
    reading_t reading = {};

    while (true) {
        if (xQueueReceive(sensorQueue, &reading, portMAX_DELAY) == pdPASS) {
            std::printf("reading: seq=%" PRIu32 ", timestamp_ms=%" PRIu32 ", temp=%.2f C, hum=%.2f %%, pressure=%.2f hPa, moisture=%.2f %%, flags=0x%04" PRIX32 "\n",
                        reading.seq,
                        reading.timestamp_ms,
                        reading.temp,
                        reading.hum,
                        reading.pressure,
                        reading.moisture,
                        reading.flags);
        }
    }
}

void
FirmwareMain::run () {
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

    // Allocate sensor instances
    static FIRMWARE::BME280::BME280Firmware bme280(_i2cBusHandle);
    static FIRMWARE::SOIL_MOISTURE::SoilMoistureFirmware soilMoisture(_adcHandle);

    static SensorTaskContext taskCtx = {
        .bme280 = &bme280,
        .soilMoisture = &soilMoisture,
        .sensorQueue = _sensorQueue
    };

    xTaskCreatePinnedToCore(sensorTask, "sensor_task", 4096, &taskCtx, 5, nullptr, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(loggerTask, "logger_task", 4096, _sensorQueue, 4, nullptr, tskNO_AFFINITY);

    // Main task can block indefinitely or sleep
    while(true) {
        vTaskDelay(portMAX_DELAY);
    }
}

bool
FirmwareMain::initI2CConnection() {
    return FIRMWARE::UTILS::FirmwareUtils::initI2CBus(_i2cBusHandle);
}

bool
FirmwareMain::initADC() {
    return FIRMWARE::UTILS::FirmwareUtils::initADC(_adcHandle);
}

} // namespace FIRMWARE