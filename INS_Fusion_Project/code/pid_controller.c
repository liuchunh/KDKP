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
 *
 * @param[out] params     PID 参数结构体指针
 * @param[in]  kp         比例增益 (无量纲)
 * @param[in]  ki         积分增益 (无量纲)
 * @param[in]  kd         微分增益 (无量纲)
 * @param[in]  dt         控制周期 (s)
 * @param[in]  out_min    输出下限
 * @param[in]  out_max    输出上限
 * @param[in]  mode       PID 模式 (PID_POSITION 或 PID_INCREMENTAL)
 *
 * @note 积分限幅默认设为输出范围的 40%，可通过 pid_set_integral_limit() 修改
 * @note 死区默认为 0，可通过 pid_set_dead_zone() 设置
 *
 * @example
 * PidParams speed_pid;
 * pid_init(&speed_pid, 1.0f, 0.1f, 0.05f, 0.01f, -100.0f, 100.0f, PID_POSITION);
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
 * @brief 初始化 PID 控制器状态（清零内部变量）
 *
 * @param[out] state  PID 状态结构体指针
 *
 * @note 每个被控对象应有独立的 PidState 实例，首次使用前必须调用此函数
 *
 * @example
 * PidState speed_state;
 * pid_reset(&speed_state);
 */
void pid_reset(PidState *state) {
    state->integral        = 0.0f;
    state->prev_error      = 0.0f;
    state->prev_prev_error = 0.0f;
    state->output          = 0.0f;
    state->d_filter        = 0.0f;
}

/**
 * @brief 设置积分限幅值 (Anti-windup)
 *
 * @param[in,out] params  PID 参数结构体指针
 * @param[in]     max     积分项最大绝对值
 *
 * @note 积分限幅用于防止积分饱和，建议设为输出限幅的 50%~100%
 *
 * @example
 * pid_set_integral_limit(&speed_pid, 80.0f);
 */
void pid_set_integral_limit(PidParams *params, float max) {
    params->integral_max = fabsf(max);
}

/**
 * @brief 设置死区阈值
 *
 * @param[in,out] params  PID 参数结构体指针
 * @param[in]     zone    死区大小（误差绝对值小于此值时输出为 0）
 *
 * @note 适用于舵机等存在机械间隙的执行器，避免在目标附近频繁抖动
 *
 * @example
 * pid_set_dead_zone(&steer_pid, 0.02f);  // 误差 < 0.02 时不调节
 */
void pid_set_dead_zone(PidParams *params, float zone) {
    params->dead_zone = fabsf(zone);
}

/**
 * @brief 设置控制周期
 *
 * @param[in,out] params  PID 参数结构体指针
 * @param[in]     dt      控制周期 (s)
 *
 * @example
 * pid_set_dt(&speed_pid, 0.01f);  // 10ms 控制周期
 */
void pid_set_dt(PidParams *params, float dt) {
    params->dt = dt;
}

/**
 * @brief 内部辅助函数：限幅
 *
 * @param[in] val  输入值
 * @param[in] min  下限
 * @param[in] max  上限
 *
 * @return float 限幅后的值，范围 [min, max]
 */
static inline float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/**
 * @brief PID 控制器核心计算函数
 *
 * 根据目标值和当前值计算控制输出。支持位置式和增量式两种模式。
 *
 * @param[in]     params   PID 参数结构体指针
 * @param[in,out] state    PID 状态结构体指针（内部更新积分、微分等状态）
 * @param[in]     target   目标值
 * @param[in]     current  当前测量值
 *
 * @return float 控制输出值，范围 [output_min, output_max]
 *
 * @note 第一次调用时 state 应已通过 pid_reset() 清零
 * @note 位置式 PID 公式: output = Kp*e + Ki*integral(e) + Kd*de/dt
 * @note 增量式 PID 公式: delta_u = Kp*(e-e1) + Ki*e*dt + Kd*(e-2*e1+e2)/dt
 *
 * @example
 * // 速度控制: 目标 2.0 m/s, 当前 1.5 m/s
 * float pwm = pid_compute(&speed_pid, &speed_state, 2.0f, 1.5f);
 * motor_set_pwm(pwm);
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
        /* ============ 增量式 PID ============ */
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
 * @brief 带微分滤波的 PID 计算函数
 *
 * 对微分项进行一阶低通滤波，减少测量噪声引起的输出抖动。
 *
 * @param[in]     params   PID 参数结构体指针
 * @param[in,out] state    PID 状态结构体指针（内部更新）
 * @param[in]     target   目标值
 * @param[in]     current  当前测量值
 * @param[in]     alpha    滤波系数 (0~1)，越小滤波越强，典型值 0.1~0.3
 *
 * @return float 控制输出值，范围 [output_min, output_max]
 *
 * @note alpha=1 时等价于无滤波的 pid_compute()
 * @note 适用于陀螺仪、编码器等噪声较大的传感器反馈
 *
 * @example
 * // 转向控制，微分滤波 alpha=0.2
 * float steer = pid_compute_filtered(&steer_pid, &steer_state,
 *                                     target_yaw, current_yaw, 0.2f);
 * servo_set_angle(steer);
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