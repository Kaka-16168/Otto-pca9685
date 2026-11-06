#include "otto_pca_adapter.h"
#include "esp_log.h"

static const uint8_t s_channel_map[6] = { 0, 1, 2, 3, 4, 5 };

void otto_pca_init(void) {
    pca9685_init();
}

void otto_pca_set_servo_angle(uint8_t servo_id, uint8_t angle) {
    if (servo_id > 5) return;
    pca9685_set_angle(s_channel_map[servo_id], angle);
}
