#pragma once
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

// 完美适配你截图的 I2C 引脚
#define PCA9685_I2C_SDA_PIN   8
#define PCA9685_I2C_SCL_PIN   9
#define PCA9685_I2C_FREQ_HZ   100000
#define PCA9685_ADDR          0x40

// 舵机标定值（如果遇到舵机堵转，请微调这两个值）
#define SERVO_0_DEG_COUNT     130
#define SERVO_180_DEG_COUNT   490

esp_err_t pca9685_init(void);
esp_err_t pca9685_set_angle(uint8_t channel, uint8_t angle);
void pca9685_all_off(void);

#ifdef __cplusplus
}
#endif