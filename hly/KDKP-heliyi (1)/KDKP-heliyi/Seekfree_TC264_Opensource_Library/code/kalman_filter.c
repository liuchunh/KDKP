/**
 * kalman_filter.c - 1D/2D 卡尔曼滤波器实现
 *
 * 适配 TC264 (TriCore) 平台
 */

#include "kalman_filter.h"
#include <math.h>

/* ================================================================
 *  一维卡尔曼滤波器
 * ================================================================ */

void kf1d_init(KF1D_State *state, const KF1D_Params *params,
               float x0, float p0) {
    state->x = x0;
    state->p = p0;
    state->k = 0.0f;
    (void)params;
}

void kf1d_predict(KF1D_State *state, const KF1D_Params *params) {
    state->p += params->q;
}

void kf1d_predict_with_control(KF1D_State *state, const KF1D_Params *params,
                              float u, float dt) {
    state->x += u * dt;
    state->p += params->q;
}

float kf1d_update(KF1D_State *state, const KF1D_Params *params, float z) {
    state->k = state->p / (state->p + params->r);
    state->x = state->x + state->k * (z - state->x);
    state->p = (1.0f - state->k) * state->p;
    return state->x;
}

float kf1d_get_state(const KF1D_State *state) {
    return state->x;
}

float kf1d_get_gain(const KF1D_State *state) {
    return state->k;
}

/* ================================================================
 *  二维卡尔曼滤波器
 * ================================================================ */

static void mat2_mul(const float A[2][2], const float B[2][2], float C[2][2]) {
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1];
    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1];
}

static void mat2_trans(const float A[2][2], float B[2][2]) {
    B[0][0] = A[0][0];
    B[0][1] = A[1][0];
    B[1][0] = A[0][1];
    B[1][1] = A[1][1];
}

void kf2d_init(KF2D_State *state, const KF2D_Params *params,
               float x0, float v0, float p0_pos, float p0_vel) {
    state->x = x0;
    state->v = v0;
    state->p[0][0] = p0_pos;
    state->p[0][1] = 0.0f;
    state->p[1][0] = 0.0f;
    state->p[1][1] = p0_vel;
    state->k[0][0] = 0.0f;
    state->k[0][1] = 0.0f;
    state->k[1][0] = 0.0f;
    state->k[1][1] = 0.0f;
    (void)params;
}

void kf2d_predict(KF2D_State *state, const KF2D_Params *params, float dt) {
    float F[2][2] = {{1.0f, dt}, {0.0f, 1.0f}};
    float Ft[2][2];
    float FP[2][2];
    float FPFt[2][2];

    float x_new = state->x + state->v * dt;
    state->x = x_new;

    float Q[2][2] = {{params->q_pos, 0.0f}, {0.0f, params->q_vel}};
    mat2_trans(F, Ft);
    mat2_mul(F, state->p, FP);
    mat2_mul(FP, Ft, FPFt);

    state->p[0][0] = FPFt[0][0] + Q[0][0];
    state->p[0][1] = FPFt[0][1] + Q[0][1];
    state->p[1][0] = FPFt[1][0] + Q[1][0];
    state->p[1][1] = FPFt[1][1] + Q[1][1];
}

void kf2d_predict_accel(KF2D_State *state, const KF2D_Params *params,
                        float a, float dt) {
    float F[2][2] = {{1.0f, dt}, {0.0f, 1.0f}};
    float Ft[2][2];
    float FP[2][2];
    float FPFt[2][2];

    state->x += state->v * dt + 0.5f * a * dt * dt;
    state->v += a * dt;

    float Q[2][2] = {{params->q_pos, 0.0f}, {0.0f, params->q_vel}};
    mat2_trans(F, Ft);
    mat2_mul(F, state->p, FP);
    mat2_mul(FP, Ft, FPFt);

    state->p[0][0] = FPFt[0][0] + Q[0][0];
    state->p[0][1] = FPFt[0][1] + Q[0][1];
    state->p[1][0] = FPFt[1][0] + Q[1][0];
    state->p[1][1] = FPFt[1][1] + Q[1][1];
}

void kf2d_update_pos(KF2D_State *state, const KF2D_Params *params,
                     float z_pos) {
    float S = state->p[0][0] + params->r_pos;
    float S_inv = (S > 1e-10f) ? (1.0f / S) : 0.0f;

    float K0 = state->p[0][0] * S_inv;
    float K1 = state->p[1][0] * S_inv;

    state->k[0][0] = K0;
    state->k[0][1] = 0.0f;
    state->k[1][0] = K1;
    state->k[1][1] = 0.0f;

    float innov = z_pos - state->x;
    state->x += K0 * innov;
    state->v += K1 * innov;

    float p00 = state->p[0][0];
    float p01 = state->p[0][1];
    state->p[0][0] = (1.0f - K0) * p00;
    state->p[0][1] = (1.0f - K0) * p01;
    state->p[1][0] = -K1 * p00 + state->p[1][0];
    state->p[1][1] = -K1 * p01 + state->p[1][1];
}

void kf2d_update_pos_vel(KF2D_State *state, const KF2D_Params *params,
                         float z_pos, float z_vel) {
    float S[2][2];
    S[0][0] = state->p[0][0] + params->r_pos;
    S[0][1] = state->p[0][1];
    S[1][0] = state->p[1][0];
    S[1][1] = state->p[1][1] + params->r_vel;

    float det = S[0][0] * S[1][1] - S[0][1] * S[1][0];
    if (fabsf(det) < 1e-10f) return;

    float inv_det = 1.0f / det;
    float S_inv[2][2];
    S_inv[0][0] =  S[1][1] * inv_det;
    S_inv[0][1] = -S[0][1] * inv_det;
    S_inv[1][0] = -S[1][0] * inv_det;
    S_inv[1][1] =  S[0][0] * inv_det;

    float K[2][2];
    mat2_mul(state->p, S_inv, K);

    state->k[0][0] = K[0][0];
    state->k[0][1] = K[0][1];
    state->k[1][0] = K[1][0];
    state->k[1][1] = K[1][1];

    float innov_pos = z_pos - state->x;
    float innov_vel = z_vel - state->v;

    state->x += K[0][0] * innov_pos + K[0][1] * innov_vel;
    state->v += K[1][0] * innov_pos + K[1][1] * innov_vel;

    float new_p[2][2];
    float IK[2][2] = {{1.0f - K[0][0], -K[0][1]},
                       {-K[1][0], 1.0f - K[1][1]}};
    mat2_mul(IK, state->p, new_p);

    state->p[0][0] = new_p[0][0];
    state->p[0][1] = new_p[0][1];
    state->p[1][0] = new_p[1][0];
    state->p[1][1] = new_p[1][1];
}

float kf2d_get_position(const KF2D_State *state) {
    return state->x;
}

float kf2d_get_velocity(const KF2D_State *state) {
    return state->v;
}

float kf2d_get_position_std(const KF2D_State *state) {
    return (state->p[0][0] > 0.0f) ? sqrtf(state->p[0][0]) : 0.0f;
}
