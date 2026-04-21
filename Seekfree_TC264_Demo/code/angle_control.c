/*
 * angle_control.c
 *
 *  Created on: 2026-04-19
 *      Author: ASUS1
 */

#include "zf_common_headfile.h"
#include "angle_control.h"

AngleControl_TypeDef angle_ctrl;

// Convert encoder pulses to degrees (pulse + direction encoder)
#define ANGLE_DEGREE_PER_PULSE  (360.0f / (ANGLE_PPR * ANGLE_GEAR_RATIO))

void angle_control_init(void) {
    // Initialize PWM for both channels
    pwm_init(ANGLE_PWM_IN1, 10000, 0);  // 10kHz PWM, 0% duty
    pwm_init(ANGLE_PWM_IN2, 10000, 0);  // 10kHz PWM, 0% duty

    // Initialize encoder in pulse+direction mode
    encoder_dir_init(ANGLE_ENCODER, ANGLE_ENCODER_COUNT_PIN, ANGLE_ENCODER_DIR_PIN);

    // Initialize PID
    PID_Init(&angle_ctrl.pid, 1.0f, 0.05f, 0.01f, 8000);
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;
    angle_ctrl.last_count = 0;
    angle_ctrl.control_count = 0;

    // Reset
    angle_control_reset();
}

void angle_motor_set_pwm(int32 pwm_value) {
    if (pwm_value > 0) {
        pwm_set_duty(ANGLE_PWM_IN1, pwm_value);  // Forward
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    } else if (pwm_value < 0) {
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, -pwm_value); // Reverse
    } else {
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    }
}

void angle_control_update(void) {
    int32 current_count = encoder_get_count(ANGLE_ENCODER);
    angle_ctrl.current_angle = (int32)(current_count * ANGLE_DEGREE_PER_PULSE);

    float pid_output = PID_Compute(&angle_ctrl.pid,
                                   (float)angle_ctrl.target_angle,
                                   (float)angle_ctrl.current_angle);
    angle_motor_set_pwm((int32)pid_output);
    angle_ctrl.control_count++;
}

void angle_control_set_target(int32 target_angle) {
    if (target_angle > ANGLE_MAX_DEGREE) {
        angle_ctrl.target_angle = ANGLE_MAX_DEGREE;
    } else if (target_angle < ANGLE_MIN_DEGREE) {
        angle_ctrl.target_angle = ANGLE_MIN_DEGREE;
    } else {
        angle_ctrl.target_angle = target_angle;
    }
}

int32 angle_control_get_current_angle(void) {
    return angle_ctrl.current_angle;
}

void angle_control_reset(void) {
    PID_Reset(&angle_ctrl.pid);
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;
    angle_ctrl.last_count = 0;
    angle_ctrl.control_count = 0;
    pwm_set_duty(ANGLE_PWM_IN1, 0);
    pwm_set_duty(ANGLE_PWM_IN2, 0);
    encoder_clear_count(ANGLE_ENCODER);
}
