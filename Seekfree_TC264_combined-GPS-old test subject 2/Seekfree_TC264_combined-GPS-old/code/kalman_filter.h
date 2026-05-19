/**
 * kalman_filter.h - 1D/2D 卡尔曼滤波器
 *
 * 用于传感器数据平滑 (如 ToF 测距、GPS 单点等)
 * 独立于 ESKF, 可单独使用
 *
 * 适配 TC264 (TriCore) 平台
 */

#ifndef KALMAN_FILTER_H_
#define KALMAN_FILTER_H_

#include "common.h"

/* ================================================================
 *  一维卡尔曼滤波器 (单传感器平滑)
 * ================================================================ */

typedef struct {
    float q;   /* 过程噪声方差 */
    float r;   /* 测量噪声方差 */
} KF1D_Params;

typedef struct {
    float x;   /* 状态估计值 */
    float p;   /* 估计协方差 */
    float k;   /* 卡尔曼增益 */
} KF1D_State;

void  kf1d_init(KF1D_State *state, const KF1D_Params *params,
                float x0, float p0);
void  kf1d_predict(KF1D_State *state, const KF1D_Params *params);
void  kf1d_predict_with_control(KF1D_State *state, const KF1D_Params *params,
                               float u, float dt);
float kf1d_update(KF1D_State *state, const KF1D_Params *params, float z);
float kf1d_get_state(const KF1D_State *state);
float kf1d_get_gain(const KF1D_State *state);

/* ================================================================
 *  二维卡尔曼滤波器 (位置-速度模型)
 * ================================================================ */

typedef struct {
    float q_pos;   /* 位置过程噪声 */
    float q_vel;   /* 速度过程噪声 */
    float r_pos;   /* 位置测量噪声 */
    float r_vel;   /* 速度测量噪声 (可选) */
} KF2D_Params;

typedef struct {
    float x;       /* 位置估计 */
    float v;       /* 速度估计 */
    float p[2][2]; /* 协方差矩阵 */
    float k[2][2]; /* 卡尔曼增益 */
} KF2D_State;

void  kf2d_init(KF2D_State *state, const KF2D_Params *params,
                float x0, float v0, float p0_pos, float p0_vel);
void  kf2d_predict(KF2D_State *state, const KF2D_Params *params, float dt);
void  kf2d_predict_accel(KF2D_State *state, const KF2D_Params *params,
                        float a, float dt);
void  kf2d_update_pos(KF2D_State *state, const KF2D_Params *params,
                     float z_pos);
void  kf2d_update_pos_vel(KF2D_State *state, const KF2D_Params *params,
                         float z_pos, float z_vel);
float kf2d_get_position(const KF2D_State *state);
float kf2d_get_velocity(const KF2D_State *state);
float kf2d_get_position_std(const KF2D_State *state);

#endif /* KALMAN_FILTER_H_ */
