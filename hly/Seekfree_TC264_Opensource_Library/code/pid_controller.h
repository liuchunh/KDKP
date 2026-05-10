/**
 * pid_controller.h - PID 控制器模块
 *
 * 支持位置式和增量式 PID, 带抗积分饱和、死区、微分滤波
 * 适配 TC264 (TriCore) 平台
 */

#ifndef PID_CONTROLLER_H_
#define PID_CONTROLLER_H_

#include "common.h"

/* PID 模式 */
typedef enum {
    PID_POSITION = 0,      /* 位置式 PID */
    PID_INCREMENT          /* 增量式 PID */
} PidMode;

/* PID 参数 */
typedef struct {
    float kp;              /* 比例系数 */
    float ki;              /* 积分系数 */
    float kd;              /* 微分系数 */
    float dt;              /* 控制周期 (s) */
    float output_min;      /* 输出下限 */
    float output_max;      /* 输出上限 */
    float integral_max;    /* 积分限幅 */
    float dead_zone;       /* 死区 */
    PidMode mode;          /* PID 模式 */
} PidParams;

/* PID 状态 */
typedef struct {
    float integral;        /* 积分累计 */
    float prev_error;      /* 上次误差 */
    float prev_prev_error; /* 上上次误差 (增量式用) */
    float output;          /* 上次输出 */
    float d_filter;        /* 微分滤波值 */
} PidState;

/* 初始化 PID 参数 */
void pid_init(PidParams *params, float kp, float ki, float kd,
              float dt, float out_min, float out_max, PidMode mode);

/* 重置 PID 状态 */
void pid_reset(PidState *state);

/* 设置积分限幅 */
void pid_set_integral_limit(PidParams *params, float max);

/* 设置死区 */
void pid_set_dead_zone(PidParams *params, float zone);

/* 设置控制周期 */
void pid_set_dt(PidParams *params, float dt);

/* 计算 PID 输出 */
float pid_compute(const PidParams *params, PidState *state,
                  float target, float current);

/* 带微分滤波的 PID 输出 */
float pid_compute_filtered(const PidParams *params, PidState *state,
                           float target, float current, float alpha);

#endif /* PID_CONTROLLER_H_ */
