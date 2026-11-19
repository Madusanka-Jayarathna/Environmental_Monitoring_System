
namespace FIRMWARE {

// ---- MQTT ---
#define MQTT_BROKER_URL             "mqtt://192.168.1.12:1883"
#define MQTT_NODE                   "node1"
// ---- WIFI Setup  ----
#define WIFI_SSID                   "SLT-Fiber-2.4G_77a8"
#define WIFI_PASSWORD               "back4262"
#define WIFI_INDIC_PIN              2

// --- Hardware & SPI Pin Setup ---
#define SPI_MISO_PIN                19          
#define SPI_MOSI_PIN                23       
#define SPI_CLK_PIN                 18
#define SPI_CS_PIN                  5           
#define MAX_FILE_SIZE               1024 * 1024

// --- Hardware & I2C Pin Setup ---
#define I2C_MASTER_SCL_IO           22      /*!< GPIO number for I2C SCL */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number for I2C SDA */
#define I2C_MASTER_NUM              0       /*!< I2C port number */
#define I2C_MASTER_FREQ_HZ          100000  /*!< I2C master clock frequency */
#define BME280_I2C_ADDR             0x76    /*!< Default I2C address for BME280 */

// --- BME280 Registers ---
#define BME280_REG_CHIP_ID          0xD0
#define BME280_CHIP_ID_VAL          0x60
#define BME280_REG_RESET            0xE0
#define BME280_REG_CTRL_HUM         0xF2
#define BME280_REG_STATUS           0xF3
#define BME280_REG_CTRL_MEAS        0xF4
#define BME280_REG_CONFIG           0xF5
#define BME280_REG_PRESS_MSB        0xF7
#define BME280_REG_CALIB00          0x88
#define BME280_REG_CALIB26          0xE1

} // namespace FIRMWARE
