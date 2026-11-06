#include "pca9685_servo.h"
#include <math.h>
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PCA9685";
static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;

#define MODE1      0x00
#define MODE2      0x01
#define PRESCALE   0xFE
#define LED0_ON_L  0x06

static esp_err_t write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(s_dev, buf, 2, 100 / portTICK_PERIOD_MS);
}

static esp_err_t read_reg(uint8_t reg, uint8_t *val) {
    return i2c_master_transmit_receive(s_dev, &reg, 1, val, 1, 100 / portTICK_PERIOD_MS);
}

esp_err_t pca9685_init(void) {
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PCA9685_I2C_SDA_PIN,
        .scl_io_num = PCA9685_I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_bus), TAG, "i2c bus fail");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCA9685_ADDR,
        .scl_speed_hz = PCA9685_I2C_FREQ_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev), TAG, "add dev fail");

    write_reg(MODE1, 0x10);
    vTaskDelay(pdMS_TO_TICKS(5));
    write_reg(MODE1, 0x00);
    vTaskDelay(pdMS_TO_TICKS(5));
    write_reg(MODE2, 0x04);

    uint8_t prescale = (uint8_t)(roundf(25000000.0f / (4096.0f * 50.0f)) - 1.0f);
    uint8_t old_mode;
    read_reg(MODE1, &old_mode);
    write_reg(MODE1, (old_mode & 0x7F) | 0x10);
    vTaskDelay(pdMS_TO_TICKS(2));
    write_reg(PRESCALE, prescale);
    write_reg(MODE1, old_mode);
    vTaskDelay(pdMS_TO_TICKS(5));
    write_reg(MODE1, old_mode | 0x80);

    pca9685_all_off();
    ESP_LOGI(TAG, "PCA9685 Ready");
    return ESP_OK;
}

esp_err_t pca9685_set_angle(uint8_t channel, uint8_t angle) {
    if (channel > 15) return ESP_ERR_INVALID_ARG;
    if (angle > 180) angle = 180;
    uint16_t counts = SERVO_0_DEG_COUNT + ((uint32_t)(SERVO_180_DEG_COUNT - SERVO_0_DEG_COUNT) * angle) / 180;
    uint8_t buf[5] = {LED0_ON_L + 4 * channel, 0, 0, counts & 0xFF, (counts >> 8) & 0x0F};
    return i2c_master_transmit(s_dev, buf, 5, 100 / portTICK_PERIOD_MS);
}

void pca9685_all_off(void) {
    uint8_t buf[5] = {LED0_ON_L, 0, 0, 0, 0};
    for (int ch = 0; ch < 16; ch++) {
        buf[0] = LED0_ON_L + 4 * ch;
        i2c_master_transmit(s_dev, buf, 5, 100 / portTICK_PERIOD_MS);
    }
}
