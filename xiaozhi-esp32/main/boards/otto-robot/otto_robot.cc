#include <driver/i2c_master.h>
#include <driver/ledc.h>
#include <driver/spi_common.h>
#include <driver/gpio.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_log.h>
#include <esp_rom_sys.h>
#include <esp_timer.h>
#include <esp_sleep.h>
#include <esp_task_wdt.h> // 【新增】引入看门狗头文件
#include <new>
#include <string>
#include "cJSON.h"

// ... [保留原有的头文件引入] ...

#define TAG "OttoRobot"
extern void InitializeOttoController(const HardwareConfig& hw_config);

using McpReturnType = ReturnValue;

class OttoRobot : public WifiBoard {
    // ... [保留原有的成员变量] ...

    void InitializeOttoController() { 
        otto_pca_init(); 
        ESP_LOGI(TAG, ">>> 开机逐个恢复至 90 度 <<<");
        for (int i = 0; i < 6; i++) {
            otto_pca_set_servo_angle(i, 90);
            vTaskDelay(pdMS_TO_TICKS(50)); 
        }
        ::InitializeOttoController(hw_config_); 
    }

    void InitializeMcpTools() {
        auto& mcp_server = McpServer::GetInstance();

        mcp_server.AddTool(
            "self.set_alarm",
            "Set an alarm for a specific number of seconds.",
            PropertyList({
                Property("seconds", kPropertyTypeInteger, 1, 86400)
            }),
            [this](const PropertyList& properties) -> McpReturnType {
                int seconds = properties["seconds"].value<int>();
                if (seconds <= 0) seconds = 10; 

                if (alarm_timer_ != nullptr) {
                    esp_timer_stop(alarm_timer_);
                    esp_timer_delete(alarm_timer_);
                    alarm_timer_ = nullptr;
                }

                esp_timer_create_args_t timer_args = {
                    .callback = [](void* arg) {
                        ESP_LOGW(TAG, "⏰ 主人，时间到了！");
                        // 回调中执行耗时操作，必须喂狗
                        esp_task_wdt_reset(); 
                    },
                    .arg = nullptr,
                    .dispatch_method = ESP_TIMER_TASK,
                    .name = "otto_alarm"
                };

                if (esp_timer_create(&timer_args, &alarm_timer_) == ESP_OK) {
                    esp_timer_start_once(alarm_timer_, seconds * 1000000ULL);
                    ESP_LOGI(TAG, "⏰ 闹钟已设置，%d 秒后响铃", seconds);
                    return true;
                }
                return false;
            }
        );

        mcp_server.AddTool(
            "self.shutdown",
            "Shutdown the robot. Turns off screen and servos, then enters deep sleep.",
            PropertyList(),
            [this](const PropertyList& properties) -> McpReturnType {
                ESP_LOGW(TAG, "🔌 收到关机指令，准备进入深度睡眠...");
                pca9685_all_off();
                GetBacklight()->SetBrightness(0);
                gpio_set_level(hw_config_.display_backlight_pin, 0);
                gpio_hold_en(hw_config_.display_backlight_pin);
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_deep_sleep_start();
                return true;
            }
        );
    }

public:
    OttoRobot() : boot_button_(BOOT_BUTTON_GPIO), audio_codec_(nullptr) {
        // ... [保留原有的初始化代码] ...
        
        // 【核心修复】在显示初始化前，将当前任务（主任务）加入看门狗监控
        esp_task_wdt_add(NULL); 

        InitializeSpi();
        InitializeLcdDisplay();
        InitializeButtons();
        InitializePowerManager();
        
        // ... [音频初始化代码] ...

        InitializeOttoController();
        InitializeMcpTools();

        ws_control_server_ = nullptr;
        GetBacklight()->RestoreBrightness();
    }
    
    // ... [保留原有的 GetAudioCodec, GetDisplay 等方法] ...
};
DECLARE_BOARD(OttoRobot);
