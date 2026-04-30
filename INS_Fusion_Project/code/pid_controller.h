/**
 * @file pid_controller.h
 * @brief PID 控制器模块
 *
 * 提供位置式 PID 和增量式 PID 两种实现，
 * 支持积分限幅（Anti-windup）、输出限幅、微分滤波。
 * 适用于嵌入式 TC264 平台的电机/舵机控制。
 *
 * @author INS Fusion Project
 * @date 2026-04-30
 */

#ifndef PID_CONTROLLER_H_
#define PID_CONTROLLER_H_

#include "common.h"

/**
 * @brief PID 控制器工作模式
 */
typedef enum {
    PID_POSITION = 0,   /**< 位置式 PID */
    PID_INCREMENTAL     /**< 增量式 PID */
} PidMode;

/**
 * @brief PID 控制器参数结构体
 *
 * @note 所有增益参数均为无量纲系数（已内部处理单位换算）
 */
typedef struct {
    float kp;               /**< 比例增益 */
    float ki;               /**< 积分增益 */
    float kd;               /**< 微分增益 */
    float output_min;       /**< 输出下限 */
    float output_max;       /**< 输出上限 */
    float integral_max;     /**< 积分项限幅值（Anti-windup） */
    float dead_zone;        /**< 死区阈值（误差小于此值时不调节） */
    float dt;               /**< 控制周期 (s) */
    PidMode mode;           /**< PID 模式: PID_POSITION 或 PID_INCREMENTAL */
} PidParams;

/**
 * @brief PID 控制器状态结构体
 *
 * @note 该结构体保存控制器内部状态，每个被控对象应有独立的实例
 */
typedef struct {
    float integral;         /**< 积分累加和 */
    float prev_error;       /**< 上一次误差 */
    float prev_prev_error;  /**< 上上次误差（增量式PID使用） */
    float output;           /**< 当前输出值 */
    float d_filter;         /**< 微分项滤波后的值 */
} PidState;

/**
 * @brief 初始化 PID 控制器参数
 *
 * @param[out] params     PID 参数结构体指针
 * @param[in]  kp         比例增益
 * @param[in]  ki         积分增益
 * @param[in]  kd         微分增益
 * @param[in]  dt         控制周期 (s)
 * @param[in]  out_min    输出下限
 * @param[in]  out_max    输出上限
 * @param[in]  mode       PID 模式 (PID_POSITION 或 PID_INCREMENTAL)
 *
 * @note 积分限幅默认设为输出限幅的 80%，可通过 pid_set_integral_limit() 单独修改
 * @note 死区默认为 0，可通过 pid_set_dead_zone() 设置
 *
 * @example
 * ```c
 * PidParams speed_pid;
 * pid_init(&speed_pid, 1.0f, 0.1f, 0.05f, 0.01f, -100.0f, 100.0f, PID_POSITION);
 * ```
 */
void pid_init(PidParams *params, float kp, float ki, float kd,
              float dt, float out_min, float out_max, PidMode mode);

/**
 * @brief 初始化 PID 控制器状态（清零内部变量）
 *
 * @param[out] state  PID 状态结构体指针
 *
 * @example
 * ```c
 * PidState speed_state;
 * pid_reset(&speed_state);
 * ```
 */
void pid_reset(PidState *state);

/**
 * @brief 设置积分限幅值
 *
 * @param[in,out] params  PID 参数结构体指针
 * @param[in]     max     积分项最大绝对值
 *
 * @note 积分限幅用于防止积分饱和（Anti-windup），建议设为输出限幅的 50%~100%
 *
 * @example
 * ```c
 * pid_set_integral_limit(&speed_pid, 80.0f);
 * ```
 */
void pid_set_integral_limit(PidParams *params, float max);

/**
 * @brief 设置死区阈值
 *
 * @param[in,out] params  PID 参数结构体指针
 * @param[in]     zone    死区大小（误差绝对值小于此值时输出为 0）
 *
 * @note 适用于舵机等存在机械间隙的执行器，避免在目标附近频繁抖动
 *
 * @example
 * ```c
 * pid_set_dead_zone(&steer_pid, 0.02f);  // 误差 < 0.02 时不调节
 * ```
 */
void pid_set_dead_zone(PidParams *params, float zone);

/**
 * @brief 设置控制周期
 *
 * @param[in,out] params  PID 参数结构体指针
 * @param[in]     dt      控制周期 (s)
 *
 * @example
 * ```c
 * pid_set_dt(&speed_pid, 0.01f);  // 10ms 控制周期
 * ```
 */
void pid_set_dt(PidParams *params, float dt);

/**
 * @brief PID 控制器核心计算函数
 *
 * 根据目标值和当前值计算控制输出。
 *
 * @param[in]  params   PID 参数结构体指针
 * @param[in,out] state PID 状态结构体指针（内部更新）
 * @param[in]  target   目标值
 * @param[in]  current  当前测量值
 *
 * @return float 控制输出值，范围 [output_min, output_max]
 *
 * @note 第一次调用时 state 应已通过 pid_reset() 清零
 * @note 位置式 PID 公式: output = Kp*e + Ki*∫e + Kd*de/dt
 * @note 增量式 PID 公式: Δu = Kp*(e-e1) + Ki*e + Kd*(e-2*e1+e2)
 *
 * @example
 * ```c
 * // 速度控制: 目标 2.0 m/s, 当前 1.5 m/s
 * float pwm = pid_compute(&speed_pid, &speed_state, 2.0f, 1.5f);
 * motor_set_pwm(pwm);
 * ```
 */
float pid_compute(const PidParams *params, PidState *state,
                  float target, float current);

/**
 * @brief 带微分滤波的 PID 计算函数
 *
 * 对微分项进行一阶低通滤波，减少测量噪声引起的输出抖动。
 *
 * @param[in]      params    PID 参数结构体指针
 * @param[in,out]  state     PID 状态结构体指针（内部更新）
 * @param[in]      target    目标值
 * @param[in]      current   当前测量值
 * @param[in]      alpha     滤波系数 (0~1)，越小滤波越强，典型值 0.1~0.3
 *
 * @return float 控制输出值，范围 [output_min, output_max]
 *
 * @note alpha=1 时等价于无滤波的 pid_compute()
 * @note 适用于陀螺仪、编码器等噪声较大的传感器反馈
 *
 * @example
 * ```c
 * // 转向控制，微分滤波 alpha=0.2
 * float steer = pid_compute_filtered(&steer_pid, &steer_state,
 *                                     target_yaw, current_yaw, 0.2f);
 * servo_set_angle(steer);
 * ```
 */
float pid_compute_filtered(const PidParams *params, PidState *state,
                           float target, float current, float alpha);

#endif /* PID_CONTROLLER_H_ */
