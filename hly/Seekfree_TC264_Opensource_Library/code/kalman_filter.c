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

/**
 * @brief 初始化一维卡尔曼滤波器
 *
 * 设置初始状态估计值和初始协方差。卡尔曼增益初始化为 0。
 *
 * @param[out] state  滤波器状态结构体指针
 * @param[in]  params 滤波器参数结构体指针（q, r 在此函数中未使用，但在 predict/update 中使用）
 * @param[in]  x0     初始状态估计值
 * @param[in]  p0     初始估计协方差（越大表示越不确定）
 *
 * @example
 *   KF1D_State state;
 *   KF1D_Params params = { .q = 0.5f, .r = 2.0f };
 *   kf1d_init(&state, &params, 0.0f, 100.0f);
 */
void kf1d_init(KF1D_State *state, const KF1D_Params *params,
               float x0, float p0) {
    state->x = x0;
    state->p = p0;
    state->k = 0.0f;
    (void)params;
}

/**
 * @brief 一维卡尔曼滤波器预测步骤（无控制输入）
 *
 * 匀速模型预测: 状态不变，协方差增加过程噪声 q。
 * 公式: p = p + q
 *
 * @param[in,out] state  滤波器状态结构体指针
 * @param[in]     params 滤波器参数（使用 q 字段）
 */
void kf1d_predict(KF1D_State *state, const KF1D_Params *params) {
    state->p += params->q;
}

/**
 * @brief 一维卡尔曼滤波器预测步骤（带控制输入）
 *
 * 带控制量的预测: x = x + u * dt，协方差增加过程噪声 q。
 * 适用于已知控制量（如速度、加速度）的场景。
 *
 * @param[in,out] state  滤波器状态结构体指针
 * @param[in]     params 滤波器参数（使用 q 字段）
 * @param[in]     u      控制输入（如速度 m/s）
 * @param[in]     dt     时间步长 (秒)
 */
void kf1d_predict_with_control(KF1D_State *state, const KF1D_Params *params,
                              float u, float dt) {
    state->x += u * dt;
    state->p += params->q;
}

/**
 * @brief 一维卡尔曼滤波器更新步骤
 *
 * 使用测量值 z 更新状态估计和协方差。
 * 公式: K = p/(p+r), x = x + K*(z-x), p = (1-K)*p
 *
 * @param[in,out] state  滤波器状态结构体指针
 * @param[in]     params 滤波器参数（使用 r 字段）
 * @param[in]     z      测量值
 * @retval 更新后的状态估计值
 *
 * @note 必须先调用 kf1d_predict() 或 kf1d_predict_with_control()，再调用本函数。
 */
float kf1d_update(KF1D_State *state, const KF1D_Params *params, float z) {
    state->k = state->p / (state->p + params->r);
    state->x = state->x + state->k * (z - state->x);
    state->p = (1.0f - state->k) * state->p;
    return state->x;
}

/**
 * @brief 获取一维卡尔曼滤波器的当前状态估计值
 *
 * @param[in] state  滤波器状态结构体指针
 * @retval 当前状态估计值 x
 */
float kf1d_get_state(const KF1D_State *state) {
    return state->x;
}

/**
 * @brief 获取一维卡尔曼滤波器的当前卡尔曼增益
 *
 * 增益 K 反映滤波器对测量值 vs 预测值的信任分配。
 * K 接近 1 → 信任测量值；K 接近 0 → 信任预测值。
 *
 * @param[in] state  滤波器状态结构体指针
 * @retval 当前卡尔曼增益 K
 */
float kf1d_get_gain(const KF1D_State *state) {
    return state->k;
}

/* ================================================================
 *  二维卡尔曼滤波器
 * ================================================================ */

/**
 * @brief 2x2 矩阵乘法 C = A * B
 *
 * @param[in]  A  左矩阵 [2][2]
 * @param[in]  B  右矩阵 [2][2]
 * @param[out] C  结果矩阵 [2][2]（不能与 A 或 B 指向同一内存）
 */
static void mat2_mul(const float A[2][2], const float B[2][2], float C[2][2]) {
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1];
    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1];
}

/**
 * @brief 2x2 矩阵转置 B = A^T
 *
 * @param[in]  A  输入矩阵 [2][2]
 * @param[out] B  转置结果 [2][2]（可以与 A 指向同一内存，即原地转置）
 */
static void mat2_trans(const float A[2][2], float B[2][2]) {
    B[0][0] = A[0][0];
    B[0][1] = A[1][0];
    B[1][0] = A[0][1];
    B[1][1] = A[1][1];
}

/**
 * @brief 初始化二维卡尔曼滤波器（位置-速度模型）
 *
 * 设置初始位置、速度和协方差对角元素。协方差非对角元素初始化为 0。
 *
 * @param[out] state    滤波器状态结构体指针
 * @param[in]  params   滤波器参数（在此函数中未使用，但在 predict/update 中使用）
 * @param[in]  x0       初始位置估计
 * @param[in]  v0       初始速度估计
 * @param[in]  p0_pos   初始位置协方差（越大表示越不确定）
 * @param[in]  p0_vel   初始速度协方差
 *
 * @example
 *   KF2D_State state;
 *   KF2D_Params params = { .q_pos = 0.1f, .q_vel = 0.5f, .r_pos = 1.0f, .r_vel = 0.5f };
 *   kf2d_init(&state, &params, 0.0f, 0.0f, 100.0f, 10.0f);
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
 * @brief 二维卡尔曼滤波器预测步骤（匀速模型）
 *
 * 状态转移: x = x + v*dt, v 不变。协方差传播: P = F*P*F^T + Q。
 *
 * @param[in,out] state  滤波器状态结构体指针
 * @param[in]     params 滤波器参数（使用 q_pos, q_vel）
 * @param[in]     dt     时间步长 (秒)
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
 * @brief 二维卡尔曼滤波器预测步骤（匀加速模型）
 *
 * 状态转移: x = x + v*dt + 0.5*a*dt^2, v = v + a*dt。
 * 适用于已知加速度的场景（如 IMU 测得的加速度）。
 *
 * @param[in,out] state  滤波器状态结构体指针
 * @param[in]     params 滤波器参数（使用 q_pos, q_vel）
 * @param[in]     a      已知加速度
 * @param[in]     dt     时间步长 (秒)
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
 * @brief 二维卡尔曼滤波器更新步骤（仅位置观测）
 *
 * 使用位置测量值更新状态。速度通过卡尔曼增益间接修正。
 * 适用于只有位置传感器（如 GPS）而无速度观测的场景。
 *
 * @param[in,out] state   滤波器状态结构体指针
 * @param[in]     params  滤波器参数（使用 r_pos）
 * @param[in]     z_pos   位置测量值
 *
 * @note 必须先调用 kf2d_predict() 或 kf2d_predict_accel()，再调用本函数。
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
 * @brief 二维卡尔曼滤波器更新步骤（位置+速度观测）
 *
 * 同时使用位置和速度测量值更新状态。适用于同时有位置和速度传感器的场景。
 *
 * @param[in,out] state   滤波器状态结构体指针
 * @param[in]     params  滤波器参数（使用 r_pos, r_vel）
 * @param[in]     z_pos   位置测量值
 * @param[in]     z_vel   速度测量值
 *
 * @note 协方差矩阵奇异（行列式接近 0）时会跳过更新，避免数值不稳定。
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
 * @brief 获取二维卡尔曼滤波器的当前位置估计
 *
 * @param[in] state  滤波器状态结构体指针
 * @retval 当前位置估计值 x
 */
float kf2d_get_position(const KF2D_State *state) {
    return state->x;
}

/**
 * @brief 获取二维卡尔曼滤波器的当前速度估计
 *
 * @param[in] state  滤波器状态结构体指针
 * @retval 当前速度估计值 v
 */
float kf2d_get_velocity(const KF2D_State *state) {
    return state->v;
}

/**
 * @brief 获取二维卡尔曼滤波器的当前位置标准差
 *
 * 从协方差矩阵对角元素 P[0][0] 提取，反映位置估计的不确定度。
 * 可用于 GPS 精度判断：std 越大，估计越不可靠。
 *
 * @param[in] state  滤波器状态结构体指针
 * @retval 位置标准差 (米)，P[0][0] 为负时返回 0
 */
float kf2d_get_position_std(const KF2D_State *state) {
    return (state->p[0][0] > 0.0f) ? sqrtf(state->p[0][0]) : 0.0f;
}
