/**
 * ins_solver.h - 误差状态卡尔曼滤波 (ESKF) GPS+IMU 融合定位核心
 *
 * 算法来源: MATLAB InsSolver.m 移植到 C 嵌入式平台
 * 参考文献: Joan Sola, "Quaternion kinematics for the error-state Kalman filter"
 *
 * 状态向量 (16维): [px, py, pz, vx, vy, vz, qw, qx, qy, qz, ba_x, ba_y, ba_z, bw_x, bw_y, bw_z]
 * 误差状态 (15维): [dpx, dpy, dpz, dvx, dvy, dvz, dtheta_x, dtheta_y, dtheta_z, dba_x, dba_y, dba_z, dbw_x, dbw_y, dbw_z]
 * 测量: GPS 位置 [x, y, z]
 */

#ifndef INS_SOLVER_H_
#define INS_SOLVER_H_

#include "math_utils.h"

/* 可标定参数 (需根据实际硬件调整) */
#define GRAVITY         9.80f           /* 重力加速度 m/s^2 */
#define IMU_DT          0.01f           /* IMU采样周期 100Hz */
#define GPS_DT           0.1f           /* GPS采样周期 10Hz */

/* 过程噪声默认值 (Allan方差标定后可替换) */
#define QC_POS_NOISE     0.01f          /* 位置过程噪声 */
#define QC_VEL_NOISE     0.0f           /* 速度过程噪声 */
#define QC_ATT_NOISE     0.01f          /* 姿态过程噪声 */
#define QC_BA_NOISE      0.0f           /* 加计零偏驱动噪声 */
#define QC_BW_NOISE      0.0f           /* 陀螺零偏驱动噪声 */

/* 测量噪声默认值 */
#define RC_POS_NOISE     0.01f          /* GPS位置测量噪声方差 */

typedef struct {
    /* 全状态 */
    Vector16    state;                  /* 当前全状态 */
    Vector15    error_state;            /* 误差状态 (每次修正后归零) */
    Matrix15x15 P;                      /* 误差协方差矩阵 */
    Matrix15x15 Qc;                     /* 连续过程噪声协方差 */
    float       Rc[3];                  /* GPS测量噪声方差 (对角) */

    /* 中间变量 */
    Matrix15x15 Fd;                     /* 离散状态转移矩阵 */
    Matrix15x15 Qd;                     /* 离散过程噪声协方差 */

    /* 标志 */
    unsigned char initialized;          /* 滤波器已初始化 */
    unsigned char gps_updated;          /* 本周期GPS已更新 */
} InsSolver;

/* 滤波器核心函数 */
void ins_solver_init(InsSolver *ins, float Qc_diag[15], float Rc_diag[3]);

/* 状态预测 (IMU数据驱动, 100Hz) */
void ins_predict(InsSolver *ins,
                 float acc_x, float acc_y, float acc_z,   /* 加速度计 m/s^2 (体坐标系) */
                 float gyro_x, float gyro_y, float gyro_z, /* 陀螺仪 rad/s (体坐标系) */
                 float dt);

/* GPS位置测量更新 (10Hz) */
void ins_update_gps(InsSolver *ins,
                    float gps_x, float gps_y, float gps_z);

/* 获取当前估计位置 */
void ins_get_position(const InsSolver *ins, float *x, float *y, float *z);

/* 获取当前估计速度 */
void ins_get_velocity(const InsSolver *ins, float *vx, float *vy, float *vz);

/* 获取当前姿态欧拉角 (rad) */
void ins_get_attitude(const InsSolver *ins, float *pitch, float *roll, float *yaw);

/* 设置初始位置和航向 (初始对准) */
void ins_set_initial_pose(InsSolver *ins,
                          float x, float y, float z,
                          float yaw_deg);

#endif /* INS_SOLVER_H_ */
