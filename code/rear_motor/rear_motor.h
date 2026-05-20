/*
 * rear_motor.h
 *
 * 后轮独立驱动模块
 * 目标速度 m/s -> 编码器目标脉冲 -> 前馈 + PID 修正 -> PWM 输出
 */

#ifndef CODE_REAR_MOTOR_H_
#define CODE_REAR_MOTOR_H_

#include "zf_common_headfile.h"

/* 车轮与编码器参数 */
#define REAR_WHEEL_DIAMETER_M        0.24f
#define REAR_GEAR_RATIO              1.6f
#define REAR_ENCODER_PPR             1024
#define REAR_EFFECTIVE_PPR           ((float)REAR_ENCODER_PPR * REAR_GEAR_RATIO)
#define REAR_WHEEL_CIRCUM_M          (3.14159265358979323846f * REAR_WHEEL_DIAMETER_M)

/* PID 参数: 载人旧工程参数 (当前科目一测试使用) */
#define REAR_KP                 10.0f
#define REAR_KI                 0.3f
#define REAR_KD                 0.8f
#define REAR_FF_GAIN            13.0f

/* PID 参数: 空载实测 (2026-05-13), 架上测试需要时切回 */
// #define REAR_KP              8.0f
// #define REAR_KI              0.5f
// #define REAR_KD              0.2f
// #define REAR_FF_GAIN         10.0f

/* PID 参数: 空载微调, 降低架空超调, 需要时切回 */
// #define REAR_KP              6.5f
// #define REAR_KI              0.25f
// #define REAR_KD              0.3f
// #define REAR_FF_GAIN         9.0f
#define REAR_PWM_HARD_LIMIT     9500
#define REAR_PWM_RATE_LIMIT     1000
#define REAR_INTEGRAL_LIMIT     2000.0f
#define REAR_INTEGRAL_THRESHOLD 60.0f

/* 速度限幅 (架上测试) */
#define REAR_SPEED_MAX_MPS      5.0f
#define REAR_SPEED_MIN_MPS      -5.0f

/** @brief 初始化后轮电机模块：PWM、GPIO方向和所有内部状态变量 */
void rear_motor_init(void);
/** @brief 停止后轮电机，复位所有速度目标、PID状态和编码器累加值，PWM置零 */
void rear_motor_stop(void);

/** @brief 设置后轮目标速度（m/s），自动钳位到限幅范围内，目标为零时清除PID积分 */
void rear_motor_set_target_mps(float target_mps);
/** @brief 每10ms定时调用：读取编码器累加差值并递增采样计数 */
void rear_motor_encoder_update_10ms(void);
/** @brief 主循环调用：每100ms执行一次PID控制，包含前馈+反馈计算 */
void rear_motor_pid_update_100ms(void);

/** @brief 获取当前设置的目标速度（m/s） */
float  rear_motor_get_target_mps(void);
/** @brief 获取实际速度（m/s），由编码器测量值换算得到 */
float  rear_motor_get_speed_mps(void);
/** @brief 获取当前PWM输出值 */
int16  rear_motor_get_pwm(void);
/** @brief 获取最近一次10ms编码器差值 */
int16  rear_motor_get_encoder_10ms(void);
/** @brief 获取上一次100ms周期内累计的编码器脉冲数 */
int32  rear_motor_get_encoder_100ms(void);

#endif /* CODE_REAR_MOTOR_H_ */
