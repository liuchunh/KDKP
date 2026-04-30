/**
 * @file kalman_filter.h
 * @brief 卡尔曼滤波器模块
 *
 * 提供以下滤波器实现：
 *   - 一维卡尔曼滤波 (KF1D): 适用于单传感器数据融合、噪声滤除
 *   - 二维卡尔曼滤波 (KF2D): 适用于位置-速度模型，1 阶运动学系统
 *
 * 所有滤波器均为静态内存分配，适合嵌入式 TC264 平台。
 *
 * @author INS Fusion Project
 * @date 2026-04-30
 */

#ifndef KALMAN_FILTER_H_
#define KALMAN_FILTER_H_

#include "common.h"

/* ================================================================
 *  一维卡尔曼滤波器 (KF1D)
 * ================================================================ */

/**
 * @brief 一维卡尔曼滤波器参数结构体
 *
 * @note 参数含义:
 *   - q: 过程噪声方差，越大则滤波器越信任模型预测（响应快，但平滑性差）
 *   - r: 测量噪声方差，越大则滤波器越信任测量值（响应慢，但更平滑）
 *   - 典型调参方法: 先用静止数据统计测量噪声 r，再手动调节 q
 */
typedef struct {
    float q;    /**< 过程噪声方差 (单位: 与状态量单位相同^2) */
    float r;    /**< 测量噪声方差 (单位: 与测量量单位相同^2) */
} KF1D_Params;

/**
 * @brief 一维卡尔曼滤波器状态结构体
 */
typedef struct {
    float x;    /**< 状态估计值 (单位: 与输入物理量相同) */
    float p;    /**< 估计误差协方差 (单位: 状态量单位^2) */
    float k;    /**< 卡尔曼增益 (无量纲) */
} KF1D_State;

/**
 * @brief 初始化一维卡尔曼滤波器
 *
 * @param[out] state   滤波器状态结构体指针
 * @param[in]  params  滤波器参数结构体指针
 * @param[in]  x0      初始状态估计值 (单位: 与物理量相同)
 * @param[in]  p0      初始估计误差协方差 (单位: 状态量单位^2)
 *
 * @note p0 通常设为较大值（如 100~1000），表示初始时刻对估计值不确定
 * @note 若不确定初始值，可设 x0 = 第一帧测量值
 *
 * @example
 * KF1D_Params kf_params = { .q = 0.01f, .r = 0.1f };
 * KF1D_State kf_state;
 * kf1d_init(&kf_state, &kf_params, 0.0f, 100.0f);
 */
void kf1d_init(KF1D_State *state, const KF1D_Params *params,
               float x0, float p0);

/**
 * @brief 一维卡尔曼滤波器预测步骤
 *
 * 状态预测: x_pred = x (匀速模型，状态不变)
 * 协方差预测: P_pred = P + q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新）
 * @param[in]     params  滤波器参数结构体指针
 *
 * @note 对于匀速运动模型，预测步骤仅增加不确定性
 * @note 若有外部控制输入（如加速度），请使用 kf1d_predict_with_control()
 *
 * @example
 * kf1d_predict(&kf_state, &kf_params);
 */
void kf1d_predict(KF1D_State *state, const KF1D_Params *params);

/**
 * @brief 带控制输入的一维卡尔曼滤波器预测步骤
 *
 * 状态预测: x_pred = x + u * dt
 * 协方差预测: P_pred = P + q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     u       控制输入 (如加速度, 单位: m/s^2)
 * @param[in]     dt      时间步长 (s)
 *
 * @example
 * kf1d_predict_with_control(&kf_state, &kf_params, 1.5f, 0.01f);
 */
void kf1d_predict_with_control(KF1D_State *state, const KF1D_Params *params,
                              float u, float dt);

/**
 * @brief 一维卡尔曼滤波器更新步骤
 *
 * 卡尔曼增益: K = P / (P + r)
 * 状态更新: x = x + K * (z - x)
 * 协方差更新: P = (1 - K) * P
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新）
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
 * float filtered_x = kf1d_update(&kf_state, &kf_params, gps_x);
 */
float kf1d_update(KF1D_State *state, const KF1D_Params *params, float z);

/**
 * @brief 获取一维卡尔曼滤波器当前状态估计值
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 当前状态估计值 (单位: 与初始化时的物理量相同)
 *
 * @example
 * float pos = kf1d_get_state(&kf_state);  // 获取滤波后的位置 (m)
 */
float kf1d_get_state(const KF1D_State *state);

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
 * float k = kf1d_get_gain(&kf_state);
 * // 信任度: 模型 70%, 测量 30%
 */
float kf1d_get_gain(const KF1D_State *state);

/* ================================================================
 *  二维卡尔曼滤波器 (KF2D) - 位置-速度模型
 * ================================================================ */

/**
 * @brief 二维卡尔曼滤波器参数结构体
 *
 * @note 适用于 1 阶运动学系统: 位置 + 速度
 * @note 状态向量: [位置, 速度]^T
 * @note 状态转移矩阵: [[1, dt], [0, 1]]
 */
typedef struct {
    float q_pos;    /**< 位置过程噪声方差 (单位: m^2) */
    float q_vel;    /**< 速度过程噪声方差 (单位: (m/s)^2) */
    float r_pos;    /**< 位置测量噪声方差 (单位: m^2) */
    float r_vel;    /**< 速度测量噪声方差 (单位: (m/s)^2)，若无速度观测则设为极大值 */
} KF2D_Params;

/**
 * @brief 二维卡尔曼滤波器状态结构体
 *
 * @note 状态向量 [x, v]^T:
 *   - x: 位置估计值 (m)
 *   - v: 速度估计值 (m/s)
 * @note 协方差矩阵 P 为 2x2 对称矩阵，存储为 p[0][0], p[0][1], p[1][1]
 */
typedef struct {
    float x;        /**< 位置估计值 (m) */
    float v;        /**< 速度估计值 (m/s) */
    float p[2][2];  /**< 估计误差协方差矩阵 (2x2, 对称) */
    float k[2][2];  /**< 卡尔曼增益矩阵 (2x2) */
} KF2D_State;

/**
 * @brief 初始化二维卡尔曼滤波器
 *
 * @param[out] state   滤波器状态结构体指针
 * @param[in]  params  滤波器参数结构体指针
 * @param[in]  x0      初始位置估计值 (m)
 * @param[in]  v0      初始速度估计值 (m/s)
 * @param[in]  p0_pos  初始位置协方差 (m^2)，通常设为较大值
 * @param[in]  p0_vel  初始速度协方差 ((m/s)^2)，通常设为较大值
 *
 * @example
 * KF2D_Params params = { .q_pos = 0.01f, .q_vel = 0.1f,
 *                        .r_pos = 0.5f, .r_vel = 1000.0f };
 * KF2D_State state;
 * kf2d_init(&state, &params, 0.0f, 0.0f, 100.0f, 100.0f);
 */
void kf2d_init(KF2D_State *state, const KF2D_Params *params,
               float x0, float v0, float p0_pos, float p0_vel);

/**
 * @brief 二维卡尔曼滤波器预测步骤
 *
 * 使用匀速运动模型:
 *   x_pred = x + v * dt
 *   v_pred = v
 *   P_pred = F * P * F^T + Q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     dt      时间步长 (s)
 *
 * @example
 * kf2d_predict(&state, &params, 0.01f);  // 10ms 预测
 */
void kf2d_predict(KF2D_State *state, const KF2D_Params *params, float dt);

/**
 * @brief 带加速度控制输入的二维卡尔曼滤波器预测步骤
 *
 * 使用匀加速运动模型:
 *   x_pred = x + v*dt + 0.5*a*dt^2
 *   v_pred = v + a*dt
 *   P_pred = F * P * F^T + Q
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     a       加速度控制输入 (m/s^2)
 * @param[in]     dt      时间步长 (s)
 *
 * @example
 * kf2d_predict_accel(&state, &params, 2.0f, 0.01f);
 */
void kf2d_predict_accel(KF2D_State *state, const KF2D_Params *params,
                        float a, float dt);

/**
 * @brief 二维卡尔曼滤波器 - 仅位置观测更新
 *
 * 当只有位置测量（如 GPS）而无速度测量时使用。
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     z_pos   位置测量值 (m)
 *
 * @note 调用顺序: 先 predict() 再 update_pos()
 *
 * @example
 * float gps_x = read_gps_x();
 * kf2d_update_pos(&state, &params, gps_x);
 */
void kf2d_update_pos(KF2D_State *state, const KF2D_Params *params,
                     float z_pos);

/**
 * @brief 二维卡尔曼滤波器 - 位置+速度同时观测更新
 *
 * 当同时有位置和速度测量时使用（如 GPS 提供位置和速度）。
 *
 * @param[in,out] state   滤波器状态结构体指针（内部更新）
 * @param[in]     params  滤波器参数结构体指针
 * @param[in]     z_pos   位置测量值 (m)
 * @param[in]     z_vel   速度测量值 (m/s)
 *
 * @example
 * kf2d_update_pos_vel(&state, &params, gps_x, gps_v);
 */
void kf2d_update_pos_vel(KF2D_State *state, const KF2D_Params *params,
                         float z_pos, float z_vel);

/**
 * @brief 获取二维卡尔曼滤波器的位置估计值
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 位置估计值 (m)
 *
 * @example
 * float pos = kf2d_get_position(&state);
 */
float kf2d_get_position(const KF2D_State *state);

/**
 * @brief 获取二维卡尔曼滤波器的速度估计值
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 速度估计值 (m/s)
 *
 * @example
 * float vel = kf2d_get_velocity(&state);
 */
float kf2d_get_velocity(const KF2D_State *state);

/**
 * @brief 获取二维卡尔曼滤波器的位置不确定度（标准差）
 *
 * @param[in] state  滤波器状态结构体指针
 *
 * @return float 位置标准差 (m)
 *
 * @note 返回值 = sqrt(P[0][0])，可用于判断滤波器是否收敛
 *
 * @example
 * float std = kf2d_get_position_std(&state);
 * if (std > 5.0f) { /* 位置不确定度太大 */ }
 */
float kf2d_get_position_std(const KF2D_State *state);

#endif /* KALMAN_FILTER_H_ */