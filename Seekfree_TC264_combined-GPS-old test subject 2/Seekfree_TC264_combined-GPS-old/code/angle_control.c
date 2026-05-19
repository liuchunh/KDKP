/*
 * angle_control.c
 *
 * Angle motor position PID control with encoder feedback
 * Adapted to use pid_controller.h API
 */

#include "zf_common_headfile.h"
#include "angle_control.h"

AngleControl_TypeDef angle_ctrl;

#define ANGLE_DEGREE_PER_PULSE  (360.0f / (ANGLE_PPR * ANGLE_GEAR_RATIO))

/* Overflow handling: accumulate int16 into int32 */
int32 accumulated_encoder_count = 0;
static int16 prev_raw_count = 0;
static uint8 encoder_first_read = 1;

static int32 encoder_get_accumulated_count(void)
{
    int16 raw = encoder_get_count(ANGLE_ENCODER);

    if (encoder_first_read) {
        prev_raw_count = raw;
        accumulated_encoder_count = (int32)raw;
        encoder_first_read = 0;
    } else {
        int32 diff = (int32)raw - (int32)prev_raw_count;
        /* quad mode: encoder_get_count already divided by 4, range +/-8192 */
        if (diff > 8192) diff -= 16384;
        else if (diff < -8192) diff += 16384;
        accumulated_encoder_count += diff;
        prev_raw_count = raw;
    }
    return accumulated_encoder_count;
}

static float angle_control_get_angle_from_count(int32 count)
{
    return (float)(count - angle_ctrl.encoder_zero_count) * ANGLE_DEGREE_PER_PULSE * ANGLE_ENCODER_DIR;
}

void angle_control_init(void) {
    pwm_init(ANGLE_PWM_IN1, 10000, 0);
    pwm_init(ANGLE_PWM_IN2, 10000, 0);

    encoder_quad_init(ANGLE_ENCODER, ANGLE_ENCODER_A_PIN, ANGLE_ENCODER_B_PIN);

    pid_init(&angle_ctrl.params, ANGLE_DEFAULT_KP, ANGLE_DEFAULT_KI, ANGLE_DEFAULT_KD,
             ANGLE_PID_DT, -ANGLE_OUTPUT_MAX, ANGLE_OUTPUT_MAX, PID_POSITION);
    pid_set_dead_zone(&angle_ctrl.params, ANGLE_DEAD_BAND);
    pid_reset(&angle_ctrl.state);

    encoder_first_read = 1;
    accumulated_encoder_count = 0;
    prev_raw_count = 0;

    int32 zero_count = encoder_get_accumulated_count();
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;
    angle_ctrl.encoder_zero_count = zero_count;
    angle_ctrl.control_count = 0;
}

void angle_motor_set_pwm(int32 pwm_value) {
    pwm_value *= ANGLE_MOTOR_DIR;

    if (pwm_value > ANGLE_OUTPUT_MAX) {
        pwm_value = ANGLE_OUTPUT_MAX;
    } else if (pwm_value < -ANGLE_OUTPUT_MAX) {
        pwm_value = -ANGLE_OUTPUT_MAX;
    }

    if (pwm_value > 0 && pwm_value < ANGLE_OUTPUT_MIN) {
        pwm_value = ANGLE_OUTPUT_MIN;
    } else if (pwm_value < 0 && pwm_value > -ANGLE_OUTPUT_MIN) {
        pwm_value = -ANGLE_OUTPUT_MIN;
    }

    if (pwm_value > 0) {
        pwm_set_duty(ANGLE_PWM_IN1, pwm_value);
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    } else if (pwm_value < 0) {
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, -pwm_value);
    } else {
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    }
}

void angle_control_update(void) {
    int32 current_count = encoder_get_accumulated_count();

    angle_ctrl.current_angle = angle_control_get_angle_from_count(current_count);

    float error = angle_ctrl.target_angle - angle_ctrl.current_angle;
    if (error < ANGLE_DEAD_BAND && error > -ANGLE_DEAD_BAND) {
        pid_reset(&angle_ctrl.state);
        angle_motor_set_pwm(0);
        angle_ctrl.control_count++;
        return;
    }

    float pid_output = pid_compute(&angle_ctrl.params,
                                   &angle_ctrl.state,
                                   angle_ctrl.target_angle,
                                   angle_ctrl.current_angle);

    angle_motor_set_pwm((int32)pid_output);
    angle_ctrl.control_count++;
}

void angle_control_set_target(int32 target_angle) {
    float new_target;

    if (target_angle > ANGLE_MAX_DEGREE) {
        new_target = ANGLE_MAX_DEGREE;
    } else if (target_angle < ANGLE_MIN_DEGREE) {
        new_target = ANGLE_MIN_DEGREE;
    } else {
        new_target = (float)target_angle;
    }

    if (angle_ctrl.target_angle != new_target) {
        angle_ctrl.target_angle = new_target;
        pid_reset(&angle_ctrl.state);
    }
}

void angle_control_rotate_relative(int32 delta_angle) {
    angle_control_set_target((int32)(angle_ctrl.current_angle + (float)delta_angle));
}

int32 angle_control_get_current_angle(void) {
    return (int32)angle_ctrl.current_angle;
}

int32 angle_control_get_target_angle(void) {
    return (int32)angle_ctrl.target_angle;
}

void angle_control_reset(void) {
    pid_reset(&angle_ctrl.state);
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;
    encoder_first_read = 1;
    accumulated_encoder_count = 0;
    prev_raw_count = 0;
    int32 zero_count = encoder_get_accumulated_count();
    angle_ctrl.encoder_zero_count = zero_count;
    angle_ctrl.control_count = 0;
    pwm_set_duty(ANGLE_PWM_IN1, 0);
    pwm_set_duty(ANGLE_PWM_IN2, 0);
}
