#pragma once
#include "pca9685_servo.h"

#ifdef __cplusplus
extern "C" {
#endif

void otto_pca_init(void);
void otto_pca_set_servo_angle(uint8_t servo_id, uint8_t angle);

#ifdef __cplusplus
}
#endif