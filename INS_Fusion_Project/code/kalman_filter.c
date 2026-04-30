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
 *
 * @param[out] state   滤波器状态结构体指针
 * @param[in]  params  滤波器参数结构体指针
 * @param[in]  x0      初始状态估计值 (单位: 与物理量相同)
 * @param[in]  p0      初始估计误差协方差 (单位: 状态量单位^2)
 *
 * @note p0 通常设为较大值（如 100~1000），表示初始时刻不确定
 * @note 若不确定初始值，可设 x0 = 第一帧测量值
 *
 * @example
 * KF1D_Params params = { .q = 0.01f, .r = 0.1f };
 * KF1D_State state;
 * kf1d_init(&state, &params, 0.0f, 100.0f);
 */
void kf1d_init(KF1D_State *state, const KF1D_Params *params,
               float x0, float p0) {
    state->x = x0;
    state->p = p0;
    state->k = 0.0f;
    (void)params;
}

/**
 * @brief 一维卡尔曼滤波器预测步骤 (匀速模型)
 *
 * 状态预测: x_pred = x (匀速模型，状态不变)
 * 协方差预测: P_pred = P + q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新协方差）
 * @param[in]     params  滤波器参数结构体指针
 *
 * @note 对于匀速运动模型，预测步骤仅增加不确定性
 * @note 若有外部控制输入（如加速度），请使用 kf1d_predict_with_control()
 *
 * @example
 * kf1d_predict(&state, &params);
 */
void kf1d_predict(KF1D_State *state, const KF1D_Params *params) {
    state->p += params->q;
}

/**
 * @brief 带控制输入的一维卡尔曼滤波器预测步骤
 *
 * 状态预测: x_pred = x + u * dt
 * 协方差预测: P_pred = P + q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新状态和协方差）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     u       控制输入 (如加速度, 单位: m/s^2)
 * @param[in]     dt      时间步长 (s)
 *
 * @example
 * // 已知加速度 a=1.5 m/s^2, dt=0.01s
 * kf1d_predict_with_control(&state, &params, 1.5f, 0.01f);
 */
void kf1d_predict_with_control(KF1D_State *state, const KF1D_Params *params,
                              float u, float dt) {
    state->x += u * dt;
    state->p += params->q;
}

/**
 * @brief 一维卡尔曼滤波器更新步骤
 *
 * 卡尔曼增益: K = P / (P + r)
 * 状态更新: x = x + K * (z - x)
 * 协方差更新: P = (1 - K) * P
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新状态、协方差、增益）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     z       测量值 (单位: 与状态量相同)
 *
 * @return float 更新后的状态估计值 (单位: 与输入物理量相同)
 *
 * @note 每次收到新的测量值时调用一次
 * @note 调用顺序: 先 predict() 再 update()
 *
 * @example
 * float gps_x = read_gps_x();  // 单位: m
 * float filtered_x = kf1d_update(&state, &params, gps_x);
 */
float kf1d_update(KF1D_State *state, const KF1D_Params *params, float z) {
    state->k = state->p / (state->p + params->r);
    state->x = state->x + state->k * (z - state->x);
    state->p = (1.0f - state->k) * state->p;
    return state->x;
}

/**
 * @brief 获取一维卡尔曼滤波器当前状态估计值
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 当前状态估计值 (单位: 与初始化时的物理量相同)
 *
 * @example
 * float pos = kf1d_get_state(&state);  // 获取滤波后的位置 (m)
 */
float kf1d_get_state(const KF1D_State *state) {
    return state->x;
}

/**
 * @brief 获取一维卡尔曼滤波器当前卡尔曼增益
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 当前卡尔曼增益 (无量纲, 范围 [0, 1])
 *
 * @note K 越接近 0 表示越信任模型预测，K 越接近 1 表示越信任测量值
 *
 * @example
 * float k = kf1d_get_gain(&state);
 * // 信任度: 模型 70%, 测量 30%
 */
float kf1d_get_gain(const KF1D_State *state) {
    return state->k;
}

/* ================================================================
 *  二维卡尔曼滤波器 (KF2D) 实现
 * ================================================================ */

/**
 * @brief 内部辅助函数：2x2 矩阵乘法 C = A * B
 *
 * @param[in]  A  2x2 矩阵 A
 * @param[in]  B  2x2 矩阵 B
 * @param[out] C  2x2 结果矩阵 C = A * B
 */
static void mat2_mul(const float A[2][2], const float B[2][2], float C[2][2]) {
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1];
    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1];
}

/**
 * @brief 内部辅助函数：2x2 矩阵转置 B = A^T
 *
 * @param[in]  A  2x2 矩阵 A
 * @param[out] B  2x2 转置结果 B = A^T
 */
static void mat2_trans(const float A[2][2], float B[2][2]) {
    B[0][0] = A[0][0];
    B[0][1] = A[1][0];
    B[1][0] = A[0][1];
    B[1][1] = A[1][1];
}

/**
 * @brief 初始化二维卡尔曼滤波器
 *
 * @param[out] state    滤波器状态结构体指针
 * @param[in]  params   滤波器参数结构体指针
 * @param[in]  x0       初始位置估计值 (m)
 * @param[in]  v0       初始速度估计值 (m/s)
 * @param[in]  p0_pos   初始位置协方差 (m^2)，通常设为较大值
 * @param[in]  p0_vel   初始速度协方差 ((m/s)^2)，通常设为较大值
 *
 * @example
 * KF2D_Params params = { .q_pos=0.01f, .q_vel=0.1f, .r_pos=0.5f, .r_vel=1000.0f };
 * KF2D_State state;
 * kf2d_init(&state, &params, 0.0f, 0.0f, 100.0f, 100.0f);
 */
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

/**
 * @brief 二维卡尔曼滤波器预测步骤 (匀速模型)
 *
 * 使用匀速运动模型:
 *   x_pred = x + v * dt
 *   v_pred = v
 *   P_pred = F * P * F^T + Q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新状态和协方差）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     dt      时间步长 (s)
 *
 * @example
 * kf2d_predict(&state, &params, 0.01f);  // 10ms 预测
 */
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

/**
 * @brief 带加速度控制输入的二维卡尔曼滤波器预测步骤
 *
 * 使用匀加速运动模型:
 *   x_pred = x + v*dt + 0.5*a*dt^2
 *   v_pred = v + a*dt
 *   P_pred = F * P * F^T + Q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新状态和协方差）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     a       加速度控制输入 (m/s^2)
 * @param[in]     dt      时间步长 (s)
 *
 * @example
 * // IMU 测得加速度 2.0 m/s^2
 * kf2d_predict_accel(&state, &params, 2.0f, 0.01f);
 */
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

/**
 * @brief 二维卡尔曼滤波器 - 仅位置观测更新
 *
 * 当只有位置测量（如 GPS）而无速度测量时使用。
 * 观测矩阵 H = [[1, 0]]，仅观测位置分量。
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新状态、协方差、增益）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     z_pos   位置测量值 (m)
 *
 * @note 调用顺序: 先 predict() 再 update_pos()
 *
 * @example
 * float gps_x = read_gps_x();  // GPS 位置 (m)
 * kf2d_update_pos(&state, &params, gps_x);
 */
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

/**
 * @brief 二维卡尔曼滤波器 - 位置+速度同时观测更新
 *
 * 当同时有位置和速度测量时使用（如 GPS 同时提供位置和速度）。
 * 观测矩阵 H = I (2x2 单位矩阵)。
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新状态、协方差、增益）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     z_pos   位置测量值 (m)
 * @param[in]     z_vel   速度测量值 (m/s)
 *
 * @note 需要同时有位置和速度观测时才调用，否则用 kf2d_update_pos()
 *
 * @example
 * // GPS 同时输出位置和速度
 * kf2d_update_pos_vel(&state, &params, gps_x, gps_v);
 */
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

/**
 * @brief 获取二维卡尔曼滤波器的位置估计值
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 位置估计值 (m)
 *
 * @example
 * float pos = kf2d_get_position(&state);  // 滤波后位置 (m)
 */
float kf2d_get_position(const KF2D_State *state) {
    return state->x;
}

/**
 * @brief 获取二维卡尔曼滤波器的速度估计值
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 速度估计值 (m/s)
 *
 * @example
 * float vel = kf2d_get_velocity(&state);  // 滤波后速度 (m/s)
 */
float kf2d_get_velocity(const KF2D_State *state) {
    return state->v;
}

/**
 * @brief 获取二维卡尔曼滤波器的位置不确定度（标准差）
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 位置标准差 (m)
 *
 * @note 返回值 = sqrt(P[0][0])，可用于判断滤波器是否收敛
 * @note 收敛后通常 < 0.5m；若持续 > 5m 需检查 GPS 信号质量
 *
 * @example
 * float std = kf2d_get_position_std(&state);
 * if (std > 5.0f) {
 *     // 位置不确定度太大，暂停导航
 * }
 */
float kf2d_get_position_std(const KF2D_State *state) {
    return (state->p[0][0] > 0.0f) ? sqrtf(state->p[0][0]) : 0.0f;
}