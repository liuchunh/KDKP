/**
 * @file kalman_filter.c
 * @brief 卡尔曼滤波器模块实现
 *
 * @author INS Fusion Project
 * @date 2026-04-30
 */

#include "kalman_filter.h"
#include <math.h>

/* ================================================================
 *  一维卡尔曼滤波器 (KF1D) 实现
 * ================================================================ */

/**
 * @brief 初始化一维卡尔曼滤波器
 */
void kf1d_init(KF1D_State *state, const KF1D_Params *params,
               float x0, float p0) {
    state->x = x0;
    state->p = p0;
    state->k = 0.0f;
    (void)params; /* params 在 predict/update 阶段使用 */
}

/**
 * @brief 一维卡尔曼滤波器预测步骤 (匀速模型)
 */
void kf1d_predict(KF1D_State *state, const KF1D_Params *params) {
    /* x_pred = x (匀速模型，状态保持不变) */
    /* P_pred = P + q */
    state->p += params->q;
}

/**
 * @brief 带控制输入的一维卡尔曼滤波器预测步骤
 */
void kf1d_predict_with_control(KF1D_State *state, const KF1D_Params *params,
                              float u, float dt) {
    /* x_pred = x + u * dt */
    state->x += u * dt;
    /* P_pred = P + q */
    state->p += params->q;
    (void)dt; /* dt 已在状态更新中使用 */
}

/**
 * @brief 一维卡尔曼滤波器更新步骤
 */
float kf1d_update(KF1D_State *state, const KF1D_Params *params, float z) {
    /* 卡尔曼增益: K = P / (P + r) */
    state->k = state->p / (state->p + params->r);

    /* 状态更新: x = x + K * (z - x) */
    state->x = state->x + state->k * (z - state->x);

    /* 协方差更新: P = (1 - K) * P */
    state->p = (1.0f - state->k) * state->p;

    return state->x;
}

/**
 * @brief 获取一维卡尔曼滤波器当前状态估计值
 */
float kf1d_get_state(const KF1D_State *state) {
    return state->x;
}

/**
 * @brief 获取一维卡尔曼滤波器当前卡尔曼增益
 */
float kf1d_get_gain(const KF1D_State *state) {
    return state->k;
}

/* ================================================================
 *  二维卡尔曼滤波器 (KF2D) 实现
 * ================================================================ */

/**
 * @brief 初始化二维卡尔曼滤波器
 */
void kf2d_init(KF2D_State *state, const KF2D_Params *params,
               float x0, float v0, float p0_pos, float p0_vel) {
    state->x = x0;
    state->v = v0;

    /* 初始化协方差矩阵 P = diag(p0_pos, p0_vel) */
    state->p[0][0] = p0_pos;
    state->p[0][1] = 0.0f;
    state->p[1][0] = 0.0f;
    state->p[1][1] = p0_vel;

    /* 清零卡尔曼增益 */
    state->k[0][0] = 0.0f;
    state->k[0][1] = 0.0f;
    state->k[1][0] = 0.0f;
    state->k[1][1] = 0.0f;

    (void)params; /* params 在 predict/update 阶段使用 */
}

/**
 * @brief 内部辅助函数: 2x2 矩阵乘法 C = A * B
 */
static void mat2_mul(const float A[2][2], const float B[2][2], float C[2][2]) {
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1];
    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1];
}

/**
 * @brief 内部辅助函数: 2x2 矩阵转置 B = A^T
 */
static void mat2_trans(const float A[2][2], float B[2][2]) {
    B[0][0] = A[0][0];
    B[0][1] = A[1][0];
    B[1][0] = A[0][1];
    B[1][1] = A[1][1];
}

/**
 * @brief 二维卡尔曼滤波器预测步骤 (匀速模型)
 */
void kf2d_predict(KF2D_State *state, const KF2D_Params *params, float dt) {
    /* 状态转移矩阵 F = [[1, dt], [0, 1]] */
    float F[2][2] = {{1.0f, dt}, {0.0f, 1.0f}};
    float Ft[2][2];
    float FP[2][2];
    float FPFt[2][2];

    /* 状态预测: x = F * x */
    float x_new = state->x + state->v * dt;
    float v_new = state->v;
    state->x = x_new;
    state->v = v_new;

    /* 过程噪声协方差 Q = diag(q_pos, q_vel) */
    float Q[2][2] = {{params->q_pos, 0.0f}, {0.0f, params->q_vel}};

    /* 协方差预测: P = F * P * F^T + Q */
    mat2_trans(F, Ft);
    mat2_mul(F, state->p, FP);
    mat2_mul(FP, Ft, FPFt);

    state->p[0][0] = FPFt[0][0] + Q[0][0];
    state->p[0][1] = FPFt[0][1] + Q[0][1];
    state->p[1][0] = FPFt[1][0] + Q[1][0];
    state->p[1][1] = FPFt[1][1] + Q[1][1];
}

/**
 * @brief 带加速度控制输入的二维卡尔曼滤波器预测步骤
 */
void kf2d_predict_accel(KF2D_State *state, const KF2D_Params *params,
                        float a, float dt) {
    float F[2][2] = {{1.0f, dt}, {0.0f, 1.0f}};
    float Ft[2][2];
    float FP[2][2];
    float FPFt[2][2];

    /* 状态预测: x = x + v*dt + 0.5*a*dt^2, v = v + a*dt */
    state->x += state->v * dt + 0.5f * a * dt * dt;
    state->v += a * dt;

    /* 协方差预测 */
    float Q[2][2] = {{params->q_pos, 0.0f}, {0.0f, params->q_vel}};
    mat2_trans(F, Ft);
    mat2_mul(F, state->p, FP);
    mat2_mul(FP, Ft, FPFt);

    state->p[0][0] = FPFt[0][0] + Q[0][0];
    state->p[0][1] = FPFt[0][1] + Q[0][1];
    state->p[1][0] = FPFt[1][0] + Q[1][0];
    state->p[1][1] = FPFt[1][1] + Q[1][1];
}

/**
 * @brief 二维卡尔曼滤波器 - 仅位置观测更新
 */
void kf2d_update_pos(KF2D_State *state, const KF2D_Params *params,
                     float z_pos) {
    /* 观测矩阵 H = [[1, 0]] (仅观测位置)
     * S = H * P * H^T + R = P[0][0] + r_pos
     * K = P * H^T * S^(-1)
     * x = x + K * (z - H * x)
     * P = (I - K * H) * P
     */

    float S = state->p[0][0] + params->r_pos;
    float S_inv = (S > 1e-10f) ? (1.0f / S) : 0.0f;

    /* 卡尔曼增益 K = P * H^T / S */
    float K0 = state->p[0][0] * S_inv;  /* K[0] = P[0][0] / S */
    float K1 = state->p[1][0] * S_inv;  /* K[1] = P[1][0] / S */

    state->k[0][0] = K0;
    state->k[0][1] = 0.0f;
    state->k[1][0] = K1;
    state->k[1][1] = 0.0f;

    /* 新息 */
    float innov = z_pos - state->x;

    /* 状态更新 */
    state->x += K0 * innov;
    state->v += K1 * innov;

    /* 协方差更新: P = (I - K*H) * P
     * (I - K*H) = [[1-K0, 0], [-K1, 1]]
     * 新P[0][0] = (1-K0)*P[0][0]
     * 新P[0][1] = (1-K0)*P[0][1]
     * 新P[1][0] = -K1*P[0][0] + P[1][0]
     * 新P[1][1] = -K1*P[0][1] + P[1][1]
     */
    float p00 = state->p[0][0];
    float p01 = state->p[0][1];
    state->p[0][0] = (1.0f - K0) * p00;
    state->p[0][1] = (1.0f - K0) * p01;
    state->p[1][0] = -K1 * p00 + state->p[1][0];
    state->p[1][1] = -K1 * p01 + state->p[1][1];
}

/**
 * @brief 二维卡尔曼滤波器 - 位置+速度同时观测更新
 */
void kf2d_update_pos_vel(KF2D_State *state, const KF2D_Params *params,
                         float z_pos, float z_vel) {
    /* 观测矩阵 H = I (直接观测位置和速度)
     * S = P + R
     * K = P * S^(-1)
     * x = x + K * (z - x)
     * P = (I - K) * P
     */

    /* S = P + R */
    float S[2][2];
    S[0][0] = state->p[0][0] + params->r_pos;
    S[0][1] = state->p[0][1];
    S[1][0] = state->p[1][0];
    S[1][1] = state->p[1][1] + params->r_vel;

    /* 2x2 矩阵求逆 S^(-1) */
    float det = S[0][0] * S[1][1] - S[0][1] * S[1][0];
    if (fabsf(det) < 1e-10f) return; /* 奇异矩阵，跳过更新 */

    float inv_det = 1.0f / det;
    float S_inv[2][2];
    S_inv[0][0] =  S[1][1] * inv_det;
    S_inv[0][1] = -S[0][1] * inv_det;
    S_inv[1][0] = -S[1][0] * inv_det;
    S_inv[1][1] =  S[0][0] * inv_det;

    /* K = P * S^(-1) */
    float K[2][2];
    mat2_mul(state->p, S_inv, K);

    state->k[0][0] = K[0][0];
    state->k[0][1] = K[0][1];
    state->k[1][0] = K[1][0];
    state->k[1][1] = K[1][1];

    /* 新息 */
    float innov_pos = z_pos - state->x;
    float innov_vel = z_vel - state->v;

    /* 状态更新 */
    state->x += K[0][0] * innov_pos + K[0][1] * innov_vel;
    state->v += K[1][0] * innov_pos + K[1][1] * innov_vel;

    /* 协方差更新: P = (I - K) * P */
    float new_p[2][2];
    float IK[2][2] = {{1.0f - K[0][0], -K[0][1]},
                       {-K[1][0], 1.0f - K[1][1]}};
    mat2_mul(IK, state->p, new_p);

    state->p[0][0] = new_p[0][0];
    state->p[0][1] = new_p[0][1];
    state->p[1][0] = new_p[1][0];
    state->p[1][1] = new_p[1][1];
}

/**
 * @brief 获取二维卡尔曼滤波器的位置估计值
 */
float kf2d_get_position(const KF2D_State *state) {
    return state->x;
}

/**
 * @brief 获取二维卡尔曼滤波器的速度估计值
 */
float kf2d_get_velocity(const KF2D_State *state) {
    return state->v;
}

/**
 * @brief 获取二维卡尔曼滤波器的位置不确定度（标准差）
 */
float kf2d_get_position_std(const KF2D_State *state) {
    return (state->p[0][0] > 0.0f) ? sqrtf(state->p[0][0]) : 0.0f;
}