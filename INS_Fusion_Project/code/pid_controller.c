/**
 * @file pid_controller.c
 * @brief PID 控制器模块实现
 *
 * @author INS Fusion Project
 * @date 2026-04-30
 */

#include "pid_controller.h"
#include <math.h>

/**
 * @brief 初始化 PID 控制器参数
 */
void pid_init(PidParams *params, float kp, float ki, float kd,
              float dt, float out_min, float out_max, PidMode mode) {
    params->kp          = kp;
    params->ki          = ki;
    params->kd          = kd;
    params->dt          = dt;
    params->output_min  = out_min;
    params->output_max  = out_max;
    params->integral_max = fabsf(out_max - out_min) * 0.4f;  /* 默认输出范围的40% */
    params->dead_zone   = 0.0f;
    params->mode        = mode;
}

/**
 * @brief 初始化 PID 控制器状态
 */
void pid_reset(PidState *state) {
    state->integral        = 0.0f;
    state->prev_error      = 0.0f;
    state->prev_prev_error = 0.0f;
    state->output          = 0.0f;
    state->d_filter        = 0.0f;
}

/**
 * @brief 设置积分限幅值
 */
void pid_set_integral_limit(PidParams *params, float max) {
    params->integral_max = fabsf(max);
}

/**
 * @brief 设置死区阈值
 */
void pid_set_dead_zone(PidParams *params, float zone) {
    params->dead_zone = fabsf(zone);
}

/**
 * @brief 设置控制周期
 */
void pid_set_dt(PidParams *params, float dt) {
    params->dt = dt;
}

/**
 * @brief 内部辅助函数: 限幅
 */
static inline float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/**
 * @brief PID 控制器核心计算
 */
float pid_compute(const PidParams *params, PidState *state,
                  float target, float current) {
    float error = target - current;

    /* 死区处理 */
    if (fabsf(error) < params->dead_zone) {
        error = 0.0f;
    }

    if (params->mode == PID_POSITION) {
        /* ============ 位置式 PID ============ */
        /* 积分项累加 */
        state->integral += error * params->dt;
        state->integral = clampf(state->integral,
                                 -params->integral_max,
                                  params->integral_max);

        /* 微分项 (一阶后向差分) */
        float derivative = (error - state->prev_error) / params->dt;

        /* PID 输出 */
        float p_term = params->kp * error;
        float i_term = params->ki * state->integral;
        float d_term = params->kd * derivative;

        state->output = p_term + i_term + d_term;

    } else {
        /* ============ 增量式 PID ============ */
        /* 增量 Δu = Kp*(e-e1) + Ki*e*dt + Kd*(e-2*e1+e2)/dt */
        float delta_error   = error - state->prev_error;
        float delta2_error  = error - 2.0f * state->prev_error + state->prev_prev_error;

        float increment = params->kp * delta_error
                        + params->ki * error * params->dt
                        + params->kd * delta2_error / params->dt;

        state->output += increment;
        state->prev_prev_error = state->prev_error;
    }

    /* 更新状态 */
    state->prev_error = error;

    /* 输出限幅 */
    state->output = clampf(state->output,
                           params->output_min,
                           params->output_max);

    return state->output;
}

/**
 * @brief 带微分滤波的 PID 计算
 */
float pid_compute_filtered(const PidParams *params, PidState *state,
                           float target, float current, float alpha) {
    float error = target - current;

    /* 死区处理 */
    if (fabsf(error) < params->dead_zone) {
        error = 0.0f;
    }

    /* 一阶低通滤波微分项: D_filtered = alpha * D_raw + (1-alpha) * D_prev */
    float derivative_raw = (error - state->prev_error) / params->dt;
    state->d_filter = alpha * derivative_raw + (1.0f - alpha) * state->d_filter;

    if (params->mode == PID_POSITION) {
        /* 积分项 */
        state->integral += error * params->dt;
        state->integral = clampf(state->integral,
                                 -params->integral_max,
                                  params->integral_max);

        float p_term = params->kp * error;
        float i_term = params->ki * state->integral;
        float d_term = params->kd * state->d_filter;

        state->output = p_term + i_term + d_term;

    } else {
        /* 增量式: 用滤波后的微分替代原始微分 */
        float delta_error   = error - state->prev_error;
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
