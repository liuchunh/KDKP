/**
 * pid_controller.c - PID 控制器模块实现
 *
 * 支持位置式和增量式 PID
 * 适配 TC264 (TriCore) 平台
 */

#include "pid_controller.h"
#include <math.h>

void pid_init(PidParams *params, float kp, float ki, float kd,
              float dt, float out_min, float out_max, PidMode mode) {
    params->kp          = kp;
    params->ki          = ki;
    params->kd          = kd;
    params->dt          = dt;
    params->output_min  = out_min;
    params->output_max  = out_max;
    params->integral_max = fabsf(out_max - out_min) * 0.4f;
    params->dead_zone   = 0.0f;
    params->mode        = mode;
}

void pid_reset(PidState *state) {
    state->integral        = 0.0f;
    state->prev_error      = 0.0f;
    state->prev_prev_error = 0.0f;
    state->output          = 0.0f;
    state->d_filter        = 0.0f;
}

void pid_set_integral_limit(PidParams *params, float max) {
    params->integral_max = fabsf(max);
}

void pid_set_dead_zone(PidParams *params, float zone) {
    params->dead_zone = fabsf(zone);
}

void pid_set_dt(PidParams *params, float dt) {
    params->dt = dt;
}

static inline float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

float pid_compute(const PidParams *params, PidState *state,
                  float target, float current) {
    float error = target - current;

    /* 死区处理 */
    if (fabsf(error) < params->dead_zone) {
        error = 0.0f;
    }

    if (params->mode == PID_POSITION) {
        /* 位置式 PID */
        state->integral += error * params->dt;
        state->integral = clampf(state->integral,
                                 -params->integral_max,
                                  params->integral_max);

        float derivative = (error - state->prev_error) / params->dt;

        float p_term = params->kp * error;
        float i_term = params->ki * state->integral;
        float d_term = params->kd * derivative;

        state->output = p_term + i_term + d_term;

    } else {
        /* 增量式 PID */
        float delta_error   = error - state->prev_error;
        float delta2_error  = error - 2.0f * state->prev_error + state->prev_prev_error;

        float increment = params->kp * delta_error
                        + params->ki * error * params->dt
                        + params->kd * delta2_error / params->dt;

        state->output += increment;
        state->prev_prev_error = state->prev_error;
    }

    state->prev_error = error;

    state->output = clampf(state->output,
                           params->output_min,
                           params->output_max);

    return state->output;
}

float pid_compute_filtered(const PidParams *params, PidState *state,
                           float target, float current, float alpha) {
    float error = target - current;

    if (fabsf(error) < params->dead_zone) {
        error = 0.0f;
    }

    /* 一阶低通滤波微分项 */
    float derivative_raw = (error - state->prev_error) / params->dt;
    state->d_filter = alpha * derivative_raw + (1.0f - alpha) * state->d_filter;

    if (params->mode == PID_POSITION) {
        state->integral += error * params->dt;
        state->integral = clampf(state->integral,
                                 -params->integral_max,
                                  params->integral_max);

        float p_term = params->kp * error;
        float i_term = params->ki * state->integral;
        float d_term = params->kd * state->d_filter;

        state->output = p_term + i_term + d_term;

    } else {
        float delta_error = error - state->prev_error;
        float increment = params->kp * delta_error
                        + params->ki * error * params->dt
                        + params->kd * state->d_filter * params->dt;

        state->output += increment;
        state->prev_prev_error = state->prev_error;
    }

    state->prev_error = error;

    state->output = clampf(state->output,
                           params->output_min,
                           params->output_max);

    return state->output;
}
