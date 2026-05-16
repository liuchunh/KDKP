#ifndef _MOTOR_PID_H_
#define _MOTOR_PID_H_

#include "zf_common_headfile.h"

/* ============================================================
 * PID 控制器接口
 * 使用前馈+PID结构，适合电机速度控制
 * ============================================================ */

/* PID 参数结构体 */
typedef struct {
    float kp;           /* 比例系数 */
    float ki;           /* 积分系数 */
    float kd;           /* 微分系数 */
    float ff_gain;      /* 前馈增益 */
    float integral;     /* 积分累积 */
    float prev_error;   /* 上次误差 */
    float target;       /* 目标值 */
    int16  output;      /* 输出值 */
    int16  prev_output; /* 上次输出 (用于变化率限幅) */
    uint8  enabled;     /* 是否启用 */
} MotorPID;

/* 安全参数 */
#define PWM_HARD_LIMIT     9500    /* PWM硬限幅 95% */
#define PWM_RATE_LIMIT     1000    /* PWM每周期最大变化量 */
#define INTEGRAL_LIMIT     2000    /* 积分限幅 */
#define INTEGRAL_THRESHOLD 60.0f   /* 积分分离阈值 */

/* 速度转换参数 (根据实际车模修改!) */
#define WHEEL_DIAMETER_M  0.24f   /* 轮子直径 (m) */
#define WHEEL_CIRCUM_M    (3.14159f * WHEEL_DIAMETER_M)  /* 轮周长 */
#define ENCODER_PPR       4096    /* 编码器四倍频后脉冲数/转 */
#define GEAR_RATIO        2       /* 减速比: 编码器转N圈，轮子转1圈 */
#define EFFECTIVE_PPR     (ENCODER_PPR * GEAR_RATIO)
#define SAMPLE_SEC        0.1f    /* 采样周期 (秒) */

/* 函数声明 */

/**
 * motor_pid_init() - 初始化PID控制器
 * @pid:        PID结构体指针
 * @kp/ki/kd:   PID参数
 * @ff_gain:    前馈增益
 */
void motor_pid_init(MotorPID *pid, float kp, float ki, float kd, float ff_gain);

/**
 * motor_pid_set_target() - 设置目标速度
 * @pid:        PID结构体指针
 * @target:     目标脉冲数/100ms
 */
void motor_pid_set_target(MotorPID *pid, float target);

/**
 * motor_pid_update() - 执行PID计算 (每100ms调用一次)
 * @pid:        PID结构体指针
 * @feedback:   当前脉冲数/100ms (编码器反馈)
 * 返回:        PWM输出值
 */
int16 motor_pid_update(MotorPID *pid, int32 feedback);

/**
 * motor_pid_stop() - 停止PID控制
 * @pid:        PID结构体指针
 */
void motor_pid_stop(MotorPID *pid);

/**
 * motor_pid_reset() - 重置PID状态
 * @pid:        PID结构体指针
 */
void motor_pid_reset(MotorPID *pid);

/**
 * pulses_to_mps() - 脉冲数转速度 (m/s)
 * @pulses:     脉冲数/100ms
 */
float pulses_to_mps(int32 pulses);

/**
 * mps_to_kmh() - m/s 转 km/h
 */
float mps_to_kmh(float mps);

#endif /* _MOTOR_PID_H_ */
