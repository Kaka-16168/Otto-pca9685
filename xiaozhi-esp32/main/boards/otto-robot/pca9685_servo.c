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
    // 使用 portMAX_DELAY 确保数据必定发出去
    return i2c_master_transmit(s_dev, buf, 2, portMAX_DELAY); 
}

static esp_err_t read_reg(uint8_t reg, uint8_t *val) {
    return i2c_master_transmit_receive(s_dev, &reg, 1, val, 1, portMAX_DELAY);
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
    
    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C 总线初始化失败!");
        return ret;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCA9685_ADDR,
        .scl_speed_hz = PCA9685_I2C_FREQ_HZ,
    };
    ret = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCA9685 设备添加失败! 检查接线!");
        return ret;
    }

    // 初始化芯片
    write_reg(MODE1, 0x10);
    vTaskDelay(pdMS_TO_TICKS(10));
    write_reg(MODE1, 0x00);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // 设置 50Hz 频率
    uint8_t prescale = (uint8_t)(roundf(25000000.0f / (4096.0f * 50.0f)) - 1.0f);
    uint8_t old_mode;
    read_reg(MODE1, &old_mode);
    write_reg(MODE1, (old_mode & 0x7F) | 0x10);
    vTaskDelay(pdMS_TO_TICKS(5));
    write_reg(PRESCALE, prescale);
    write_reg(MODE1, old_mode);
    vTaskDelay(pdMS_TO_TICKS(5));
    write_reg(MODE1, old_mode | 0x80);

    ESP_LOGI(TAG, "PCA9685 初始化完毕，开始测试摆动...");
    
    // 暴力测试：让 0~5 通道的舵机从 10 度摆到 170 度，再摆回 90 度
    for (int i = 0; i < 6; i++) {
        pca9685_set_angle(i, 10);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    for (int i = 0; i < 6; i++) {
        pca9685_set_angle(i, 170);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    for (int i = 0; i < 6; i++) {
        pca9685_set_angle(i, 90); // 最终归位
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "舵机测试结束！");
    return ESP_OK;
}

esp_err_t pca9685_set_angle(uint8_t channel, uint8_t angle) {
    if (channel > 15) return ESP_ERR_INVALID_ARG;
    if (angle > 180) angle = 180;
    
    // 0度对应 102 (500us), 180度对应 512 (2500us)
    uint16_t counts = 102 + ((uint32_t)(512 - 102) * angle) / 180;
    
    uint8_t buf[5] = {LED0_ON_L + 4 * channel, 0, 0, counts & 0xFF, (counts >> 8) & 0x0F};
    return i2c_master_transmit(s_dev, buf, 5, portMAX_DELAY);
}

void pca9685_all_off(void) {
    uint8_t buf[5] = {LED0_ON_L, 0, 0, 0, 0};
    for (int ch = 0; ch < 16; ch++) {
        buf[0] = LED0_ON_L + 4 * ch;
        i2c_master_transmit(s_dev, buf, 5, portMAX_DELAY);
    }
}
