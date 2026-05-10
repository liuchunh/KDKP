/**
 * pid_controller.c - PID 控制器模块实现
 *
 * 支持位置式和增量式 PID
 * 适配 TC264 (TriCore) 平台
 */

#include "pid_controller.h"
#include <math.h>

/**
 * @brief 初始化 PID 控制器参数
 *
 * 设置比例/积分/微分系数、控制周期、输出范围和工作模式。
 * 积分限幅自动设为输出范围的 40%，死区默认为 0。
 *
 * @param[out] params  PID 参数结构体指针
 * @param[in]  kp      比例系数
 * @param[in]  ki      积分系数
 * @param[in]  kd      微分系数
 * @param[in]  dt      控制周期 (秒)
 * @param[in]  out_min 输出下限
 * @param[in]  out_max 输出上限
 * @param[in]  mode    PID 模式: PID_POSITION (位置式) 或 PID_INCREMENT (增量式)
 *
 * @example
 *   PidParams params;
 *   pid_init(&params, 1.0f, 0.05f, 0.1f, 0.01f, 0.0f, 2.5f, PID_POSITION);
 */
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

/**
 * @brief 重置 PID 控制器状态
 *
 * 清零积分累计、历史误差和输出值。通常在模式切换或重新开始控制时调用。
 *
 * @param[out] state  PID 状态结构体指针
 *
 * @note 在教学→自动模式切换时必须调用，否则残留的积分项会导致首次输出异常。
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
 *
 * 限制积分项的最大绝对值，防止积分饱和（windup）。
 *
 * @param[out] params  PID 参数结构体指针
 * @param[in]  max     积分限幅绝对值
 */
void pid_set_integral_limit(PidParams *params, float max) {
    params->integral_max = fabsf(max);
}

/**
 * @brief 设置 PID 死区
 *
 * 当误差绝对值小于死区时，误差视为 0，控制器不动作。
 * 用于消除传感器噪声引起的微小抖动。
 *
 * @param[out] params  PID 参数结构体指针
 * @param[in]  zone    死区大小（正值）
 *
 * @note 转向 PID 建议死区 ~0.02rad（约 1°），避免舵机抖动。
 */
void pid_set_dead_zone(PidParams *params, float zone) {
    params->dead_zone = fabsf(zone);
}

/**
 * @brief 设置 PID 控制周期
 *
 * @param[out] params  PID 参数结构体指针
 * @param[in]  dt      控制周期 (秒)
 */
void pid_set_dt(PidParams *params, float dt) {
    params->dt = dt;
}

/**
 * @brief 浮点数限幅
 *
 * 将 val 限制在 [min, max] 范围内。
 *
 * @param[in] val  输入值
 * @param[in] min  下限
 * @param[in] max  上限
 * @retval 限幅后的值
 */
static inline float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/**
 * @brief 计算 PID 输出（标准形式）
 *
 * 根据目标值和当前值的误差，计算 PID 控制输出。
 * 支持位置式和增量式两种模式，内部自动处理死区、积分限幅和输出限幅。
 *
 * @param[in]  params   PID 参数结构体指针
 * @param[in,out] state PID 状态结构体指针（内部更新积分、误差历史）
 * @param[in]  target   目标值
 * @param[in]  current  当前值
 * @retval PID 控制输出（已限幅到 [output_min, output_max]）
 *
 * @example
 *   float speed = pid_compute(&params, &state, 2.0f, current_speed);
 *
 * @note 位置式 PID 的积分项会持续累积，长时间运行建议定期调用 pid_reset()。
 */
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

/**
 * @brief 计算 PID 输出（带微分滤波）
 *
 * 与 pid_compute() 功能相同，但对微分项进行一阶低通滤波，
 * 有效抑制传感器噪声引起的输出抖动。
 *
 * 滤波公式: d_filtered = alpha * d_raw + (1 - alpha) * d_filtered_prev
 *
 * @param[in]  params   PID 参数结构体指针
 * @param[in,out] state PID 状态结构体指针（内部更新积分、误差历史、微分滤波值）
 * @param[in]  target   目标值
 * @param[in]  current  当前值
 * @param[in]  alpha    低通滤波系数 (0~1)，越大越信任新测量值
 * @retval PID 控制输出（已限幅）
 *
 * @example
 *   float steer = pid_compute_filtered(&params, &state, 0.0f, -yaw_err, 0.2f);
 *
 * @note alpha 推荐范围 0.1~0.3。过小会导致响应迟钝，过大则滤波效果差。
 * @note 仅位置式 PID 使用 state->d_filter 存储滤波值，增量式 PID 的 d_filter 行为不同。
 */
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
