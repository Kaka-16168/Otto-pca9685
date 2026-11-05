#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <esp_adc/adc_oneshot.h>
#include <driver/gpio.h>

#define OTTO_VERSION_AUTO 0
#define OTTO_VERSION_CAMERA 1
#define OTTO_VERSION_NO_CAMERA 2

#ifndef OTTO_HARDWARE_VERSION
#define OTTO_HARDWARE_VERSION OTTO_VERSION_AUTO
#endif

enum OttoCameraType {
    OTTO_CAMERA_NONE = 0,
    OTTO_CAMERA_OV2640 = 1,
    OTTO_CAMERA_OV3660 = 2,
    OTTO_CAMERA_UNKNOWN = 99,
};

#define OTTO_OV2640_PID_1 0x2640
#define OTTO_OV2640_PID_2 0x2626
#define OTTO_OV3660_PID 0x3660

struct HardwareConfig {
    gpio_num_t power_charge_detect_pin;
    adc_unit_t power_adc_unit;
    adc_channel_t power_adc_channel;

    gpio_num_t right_leg_pin;
    gpio_num_t right_foot_pin;
    gpio_num_t left_leg_pin;
    gpio_num_t left_foot_pin;
    gpio_num_t left_hand_pin;
    gpio_num_t right_hand_pin;

    int audio_input_sample_rate;
    int audio_output_sample_rate;
    bool audio_use_simplex;

    gpio_num_t audio_i2s_gpio_ws;
    gpio_num_t audio_i2s_gpio_bclk;
    gpio_num_t audio_i2s_gpio_din;
    gpio_num_t audio_i2s_gpio_dout;

    gpio_num_t audio_i2s_mic_gpio_ws;
    gpio_num_t audio_i2s_mic_gpio_sck;
    gpio_num_t audio_i2s_mic_gpio_din;
    gpio_num_t audio_i2s_spk_gpio_dout;
    gpio_num_t audio_i2s_spk_gpio_bclk;
    gpio_num_t audio_i2s_spk_gpio_lrck;

    gpio_num_t display_backlight_pin;
    gpio_num_t display_mosi_pin;
    gpio_num_t display_clk_pin;
    gpio_num_t display_dc_pin;
    gpio_num_t display_rst_pin;
    gpio_num_t display_cs_pin;

    gpio_num_t i2c_sda_pin;
    gpio_num_t i2c_scl_pin;
};

constexpr HardwareConfig CAMERA_VERSION_CONFIG = {
    // 摄像头版本保留原样，不修改
    .power_charge_detect_pin = GPIO_NUM_NC,
    .power_adc_unit = ADC_UNIT_1,
    .power_adc_channel = ADC_CHANNEL_1,

    .right_leg_pin = GPIO_NUM_43,
    .right_foot_pin = GPIO_NUM_44,
    .left_leg_pin = GPIO_NUM_5,
    .left_foot_pin = GPIO_NUM_6,
    .left_hand_pin = GPIO_NUM_4,
    .right_hand_pin = GPIO_NUM_7,

    .audio_input_sample_rate = 16000,
    .audio_output_sample_rate = 16000,
    .audio_use_simplex = false,

    .audio_i2s_gpio_ws = GPIO_NUM_40,
    .audio_i2s_gpio_bclk = GPIO_NUM_42,
    .audio_i2s_gpio_din = GPIO_NUM_41,
    .audio_i2s_gpio_dout = GPIO_NUM_39,

    .audio_i2s_mic_gpio_ws = GPIO_NUM_NC,
    .audio_i2s_mic_gpio_sck = GPIO_NUM_NC,
    .audio_i2s_mic_gpio_din = GPIO_NUM_NC,
    .audio_i2s_spk_gpio_dout = GPIO_NUM_NC,
    .audio_i2s_spk_gpio_bclk = GPIO_NUM_NC,
    .audio_i2s_spk_gpio_lrck = GPIO_NUM_NC,

    .display_backlight_pin = GPIO_NUM_38,
    .display_mosi_pin = GPIO_NUM_45,
    .display_clk_pin = GPIO_NUM_48,
    .display_dc_pin = GPIO_NUM_47,
    .display_rst_pin = GPIO_NUM_1,
    .display_cs_pin = GPIO_NUM_NC,

    .i2c_sda_pin = GPIO_NUM_15,
    .i2c_scl_pin = GPIO_NUM_16,
};

constexpr HardwareConfig NON_CAMERA_VERSION_CONFIG = {
    // ================== 以下为你修改后的引脚 ==================
    .power_charge_detect_pin = GPIO_NUM_21,
    .power_adc_unit = ADC_UNIT_2,
    .power_adc_channel = ADC_CHANNEL_3,

    // 【废弃】原生舵机引脚。因为改用 PCA9685，这里赋安全值防报错，实际不再驱动
    .right_leg_pin = GPIO_NUM_17,
    .right_foot_pin = GPIO_NUM_18,
    .left_leg_pin = GPIO_NUM_38,
    .left_foot_pin = GPIO_NUM_39,
    .left_hand_pin = GPIO_NUM_41,
    .right_hand_pin = GPIO_NUM_42,

    .audio_input_sample_rate = 16000,
    .audio_output_sample_rate = 24000, // 如果想提高音质可改24000，需确认引擎支持
    .audio_use_simplex = true,

    // 全局 I2S（某些板子会用到，当前不用）
    .audio_i2s_gpio_ws = GPIO_NUM_NC,
    .audio_i2s_gpio_bclk = GPIO_NUM_NC,
    .audio_i2s_gpio_din = GPIO_NUM_NC,
    .audio_i2s_gpio_dout = GPIO_NUM_NC,

    // 麦克风 (ICS43434)
    .audio_i2s_mic_gpio_ws = GPIO_NUM_4,
    .audio_i2s_mic_gpio_sck = GPIO_NUM_5,
    .audio_i2s_mic_gpio_din = GPIO_NUM_6,
    // 功放 (MAX98357)
    .audio_i2s_spk_gpio_dout = GPIO_NUM_7,
    .audio_i2s_spk_gpio_bclk = GPIO_NUM_15,
    .audio_i2s_spk_gpio_lrck = GPIO_NUM_16,

    // 屏幕引脚 (ST7789)
    .display_backlight_pin = GPIO_NUM_21,
    .display_mosi_pin = GPIO_NUM_11,
    .display_clk_pin = GPIO_NUM_12,
    .display_dc_pin = GPIO_NUM_13,
    .display_rst_pin = GPIO_NUM_10,
    .display_cs_pin = GPIO_NUM_14,

    // I2C 引脚 (PCA9685)
    .i2c_sda_pin = GPIO_NUM_8,
    .i2c_scl_pin = GPIO_NUM_9,
};

// 以下宏定义保持不变
#define CAMERA_XCLK (GPIO_NUM_3)
#define CAMERA_PCLK (GPIO_NUM_10)
#define CAMERA_VSYNC (GPIO_NUM_17)
#define CAMERA_HSYNC (GPIO_NUM_18)
#define CAMERA_D0 (GPIO_NUM_12)
#define CAMERA_D1 (GPIO_NUM_14)
#define CAMERA_D2 (GPIO_NUM_21)
#define CAMERA_D3 (GPIO_NUM_13)
#define CAMERA_D4 (GPIO_NUM_11)
#define CAMERA_D5 (GPIO_NUM_9)
#define CAMERA_D6 (GPIO_NUM_46)
#define CAMERA_D7 (GPIO_NUM_8)
#define CAMERA_PWDN (GPIO_NUM_NC)
#define CAMERA_RESET (GPIO_NUM_NC)
#define CAMERA_XCLK_FREQ (16000000)
#define LEDC_TIMER (LEDC_TIMER_0)
#define LEDC_CHANNEL (LEDC_CHANNEL_0)

#define LCD_TYPE_ST7789_SERIAL
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false
#define DISPLAY_INVERT_COLOR true
#define DISPLAY_RGB_ORDER LCD_RGB_ELEMENT_ORDER_RGB
#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE 3

#define BOOT_BUTTON_GPIO GPIO_NUM_0

#endif