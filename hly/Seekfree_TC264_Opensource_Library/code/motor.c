/*
 * motor.c
 *
 *  Created on: 2026-04-16
 *      Author: ASUS1
 */

#include "zf_common_headfile.h"
#include "motor.h"

void motor_init(void) {
    pwm_init(pwm_L_IN1, 10000, 0);
    pwm_init(pwm_L_IN2, 10000, 0);
    encoder_quad_init(MOTOR_ENCODER, ENCODER_CH1, ENCODER_CH2);
}

void set_motor_speed(int32 speed) {
    if (speed >= 0) {
        pwm_set_duty(pwm_L_IN1, speed);
        pwm_set_duty(pwm_L_IN2, 0);
    }
    else {
        pwm_set_duty(pwm_L_IN1, 0);
        pwm_set_duty(pwm_L_IN2, -speed);
    }
}

int16 get_motor_speed(void) {
    return encoder_get_count(MOTOR_ENCODER);
}

void motor_speed_reset(void) {
    encoder_clear_count(MOTOR_ENCODER);
}
