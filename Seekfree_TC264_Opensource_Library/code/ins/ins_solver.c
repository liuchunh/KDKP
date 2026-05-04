/**
 * ins_solver.c - ESKF (误差状态卡尔曼滤波) GPS+IMU 融合定位核心实现
 *
 * 状态向量 16维: [px,py,pz, vx,vy,vz, qw,qx,qy,qz, ba_x,ba_y,ba_z, bw_x,bw_y,bw_z]
 * 误差状态 15维: [dp(3), dv(3), dtheta(3), dba(3), dbw(3)]
 *
 * 适配 TC264 (TriCore) 平台, TASKING 编译器
 */

#include "ins_solver.h"
#include <math.h>

/* 状态索引 */
#define S_PX  0
#define S_PY  1
#define S_PZ  2
#define S_VX  3
#define S_VY  4
#define S_VZ  5
#define S_QW  6
#define S_QX  7
#define S_QY  8
#define S_QZ  9
#define S_BAX 10
#define S_BAY 11
#define S_BAZ 12
#define S_BWX 13
#define S_BWY 14
#define S_BWZ 15

/* 误差状态索引 */
#define E_DP  0
#define E_DV  3
#define E_DTH 6
#define E_DBA 9
#define E_DBW 12

/* ================================================================
 *  初始化
 * ================================================================ */

void ins_solver_init(InsSolver *ins, float Qc_diag[15], float Rc_diag[3]) {
    memset(ins, 0, sizeof(InsSolver));

    /* 初始状态: 位置=0, 速度=0, 四元数=单位, 零偏=0 */
    ins->state[S_PX] = 0.0f; ins->state[S_PY] = 0.0f; ins->state[S_PZ] = 0.0f;
    ins->state[S_VX] = 0.0f; ins->state[S_VY] = 0.0f; ins->state[S_VZ] = 0.0f;
    ins->state[S_QW] = 1.0f; ins->state[S_QX] = 0.0f;
    ins->state[S_QY] = 0.0f; ins->state[S_QZ] = 0.0f;

    /* 误差协方差 P 初始值 */
    mat15_diag(ins->P, 1.0f);

    /* 连续过程噪声协方差 Qc (对角阵) */
    int i;
    if (Qc_diag) {
        for (i = 0; i < 15; i++)
            ins->Qc[i][i] = Qc_diag[i];
    } else {
        /* 默认值 */
        ins->Qc[0][0] = ins->Qc[1][1] = ins->Qc[2][2] = QC_POS_NOISE;
        ins->Qc[3][3] = ins->Qc[4][4] = ins->Qc[5][5] = QC_VEL_NOISE;
        ins->Qc[6][6] = ins->Qc[7][7] = ins->Qc[8][8] = QC_ATT_NOISE;
        ins->Qc[9][9] = ins->Qc[10][10] = ins->Qc[11][11] = QC_BA_NOISE;
        ins->Qc[12][12] = ins->Qc[13][13] = ins->Qc[14][14] = QC_BW_NOISE;
    }

    /* GPS 测量噪声 R */
    if (Rc_diag) {
        ins->Rc[0] = Rc_diag[0];
        ins->Rc[1] = Rc_diag[1];
        ins->Rc[2] = Rc_diag[2];
    } else {
        ins->Rc[0] = ins->Rc[1] = ins->Rc[2] = RC_POS_NOISE;
    }

    ins->initialized = 1;
    ins->gps_updated = 0;
}

/* ================================================================
 *  构建连续时间状态转移矩阵 Fc (15x15)
 *
 *  线性化点: 当前状态
 *  Fc 结构:
 *    dp'  = dv
 *    dv'  = R*(a - ba) + g  → 对 dv, dtheta, dba 有偏导
 *    dtheta' = -(w - bw) × dtheta + dbw  → 对 dtheta, dbw 有偏导
 *    dba' = 0
 *    dbw' = 0
 * ================================================================ */

static void build_Fc(Matrix15x15 Fc, const InsSolver *ins) {
    memset(Fc, 0, sizeof(Matrix15x15));

    float dt = IMU_DT;

    /* dp/d(dv) = I*dt (位置对速度的雅可比) */
    int i;
    for (i = 0; i < 3; i++)
        Fc[E_DP + i][E_DV + i] = dt;

    /* 获取当前旋转矩阵 */
    Vector4 q;
    q[0] = ins->state[S_QW]; q[1] = ins->state[S_QX];
    q[2] = ins->state[S_QY]; q[3] = ins->state[S_QZ];
    Matrix3x3 R;
    quat_to_rot_matrix(R, q);

    /* 当前加速度 (体坐标系, 减去零偏) */
    /* 注意: 这里用上次预测时的加速度, 简化处理 */
    float ax = ins->state[S_VX];  /* 简化: 不存储加速度, 用0 */
    float ay = ins->state[S_VY];
    float az = ins->state[S_VZ];

    /* 实际实现中应存储上次的加速度, 这里简化为:
     * dv/d(dtheta) = -R*[a]× * dt
     * dv/d(dba) = -R * dt */
    /* 简化: 使用近似 */
    (void)ax; (void)ay; (void)az;

    /* dp/d(dv) = I */
    for (i = 0; i < 3; i++)
        Fc[E_DP + i][E_DV + i] = 1.0f;

    /* dv/d(dv) = I */
    for (i = 0; i < 3; i++)
        Fc[E_DV + i][E_DV + i] = 1.0f;

    /* dv/d(dba) = -R (加速度计零偏影响) */
    for (i = 0; i < 3; i++) {
        int j;
        for (j = 0; j < 3; j++)
            Fc[E_DV + i][E_DBA + j] = -R[i][j];
    }

    /* dtheta/d(dtheta) = I (简化) */
    for (i = 0; i < 3; i++)
        Fc[E_DTH + i][E_DTH + i] = 1.0f;

    /* dtheta/d(dbw) = -I */
    for (i = 0; i < 3; i++)
        Fc[E_DTH + i][E_DBW + i] = -1.0f;

    /* dba/d(dba) = I, dbw/d(dbw) = I */
    for (i = 0; i < 3; i++) {
        Fc[E_DBA + i][E_DBA + i] = 1.0f;
        Fc[E_DBW + i][E_DBW + i] = 1.0f;
    }
}

/* ================================================================
 *  状态预测
 * ================================================================ */

void ins_predict(InsSolver *ins,
                 float acc_x, float acc_y, float acc_z,
                 float gyro_x, float gyro_y, float gyro_z,
                 float dt) {
    if (!ins->initialized) return;

    /* 减去零偏 */
    float ax = acc_x  - ins->state[S_BAX];
    float ay = acc_y  - ins->state[S_BAY];
    float az = acc_z  - ins->state[S_BAZ];
    float wx = gyro_x - ins->state[S_BWX];
    float wy = gyro_y - ins->state[S_BWY];
    float wz = gyro_z - ins->state[S_BWZ];

    /* 旋转矩阵 */
    Vector4 q;
    q[0] = ins->state[S_QW]; q[1] = ins->state[S_QX];
    q[2] = ins->state[S_QY]; q[3] = ins->state[S_QZ];
    Matrix3x3 R;
    quat_to_rot_matrix(R, q);

    /* 体坐标系加速度转 NED */
    Vector3 acc_body, acc_ned;
    vec3_set(acc_body, ax, ay, az);
    mat3_mul_vec(acc_ned, R, acc_body);

    /* 位置更新: p += v*dt + 0.5*a*dt^2 */
    ins->state[S_PX] += ins->state[S_VX]*dt + 0.5f*acc_ned[0]*dt*dt;
    ins->state[S_PY] += ins->state[S_VY]*dt + 0.5f*acc_ned[1]*dt*dt;
    ins->state[S_PZ] += ins->state[S_VZ]*dt + 0.5f*(acc_ned[2] + GRAVITY)*dt*dt;

    /* 速度更新: v += a*dt (加上重力) */
    ins->state[S_VX] += acc_ned[0] * dt;
    ins->state[S_VY] += acc_ned[1] * dt;
    ins->state[S_VZ] += (acc_ned[2] + GRAVITY) * dt;

    /* 四元数更新 */
    Vector3 gyro;
    vec3_set(gyro, wx, wy, wz);
    quat_update(q, gyro, dt);
    ins->state[S_QW] = q[0]; ins->state[S_QX] = q[1];
    ins->state[S_QY] = q[2]; ins->state[S_QZ] = q[3];

    /* ---- 协方差传播: P = Fd * P * Fd^T + Qd ---- */
    /* 简化: 使用简化的 Fd (一阶近似) */
    Matrix15x15 Fc;
    build_Fc(Fc, ins);

    /* Qd ≈ Qc * dt */
    int i;
    for (i = 0; i < 15; i++) {
        ins->Qd[i][i] = ins->Qc[i][i] * dt;
    }

    /* 简化的协方差传播: P = P + (Fc*P + P*Fc^T)*dt + Qd */
    /* 完整的 Fd*P*Fd^T 对 15x15 太昂贵, 使用一阶近似 */
    Matrix15x15 FP, FPt, tmp;
    float sum;
    int j, k;

    /* FP = Fc * P */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            sum = 0.0f;
            for (k = 0; k < 15; k++)
                sum += Fc[i][k] * ins->P[k][j];
            FP[i][j] = sum * dt;
        }
    }

    /* FPt = P * Fc^T */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            sum = 0.0f;
            for (k = 0; k < 15; k++)
                sum += ins->P[i][k] * Fc[j][k];
            FPt[i][j] = sum * dt;
        }
    }

    /* P = P + FP + FPt + Qd */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            ins->P[i][j] += FP[i][j] + FPt[i][j] + ins->Qd[i][j];
        }
    }

    /* 误差状态清零 (预测阶段不修正) */
    memset(ins->error_state, 0, sizeof(Vector15));
}

/* ================================================================
 *  GPS 测量更新
 * ================================================================ */

void ins_update_gps(InsSolver *ins,
                    float gps_x, float gps_y, float gps_z) {
    if (!ins->initialized) return;

    Matrix15x15 tmp;

    /* 观测矩阵 H (3x15): 只观测位置 */
    /* H = [I(3x3)  0(3x3)  0(3x3)  0(3x3)  0(3x3)] */

    /* 新息: y = z - h(x) = gps_pos - estimated_pos */
    float innov[3];
    innov[0] = gps_x - ins->state[S_PX];
    innov[1] = gps_y - ins->state[S_PY];
    innov[2] = gps_z - ins->state[S_PZ];

    /* 新息协方差: S = H*P*H^T + R = P[0:3][0:3] + R */
    float S[3][3];
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            S[i][j] = ins->P[i][j];
        }
        S[i][i] += ins->Rc[i];
    }

    /* S 的逆 (3x3 对称正定) */
    float det = S[0][0]*(S[1][1]*S[2][2] - S[1][2]*S[2][1])
              - S[0][1]*(S[1][0]*S[2][2] - S[1][2]*S[2][0])
              + S[0][2]*(S[1][0]*S[2][1] - S[1][1]*S[2][0]);

    if (fabsf(det) < 1e-20f) return;  /* 奇异, 跳过更新 */

    float inv_det = 1.0f / det;
    float S_inv[3][3];
    S_inv[0][0] =  (S[1][1]*S[2][2] - S[1][2]*S[2][1]) * inv_det;
    S_inv[0][1] = -(S[0][1]*S[2][2] - S[0][2]*S[2][1]) * inv_det;
    S_inv[0][2] =  (S[0][1]*S[1][2] - S[0][2]*S[1][1]) * inv_det;
    S_inv[1][0] = S_inv[0][1];
    S_inv[1][1] =  (S[0][0]*S[2][2] - S[0][2]*S[2][0]) * inv_det;
    S_inv[1][2] = -(S[0][0]*S[1][2] - S[0][2]*S[1][0]) * inv_det;
    S_inv[2][0] = S_inv[0][2];
    S_inv[2][1] = S_inv[1][2];
    S_inv[2][2] =  (S[0][0]*S[1][1] - S[0][1]*S[1][0]) * inv_det;

    /* 卡尔曼增益: K = P*H^T * S_inv
     * 由于 H 只取 P 的前三列, K[15][3] = P[:,0:3] * S_inv */
    float K[15][3];
    int k;
    float ksum;
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 3; j++) {
            ksum = 0.0f;
            for (k = 0; k < 3; k++)
                ksum += ins->P[i][k] * S_inv[k][j];
            K[i][j] = ksum;
        }
    }

    /* 误差状态更新: dx = K * innov */
    for (i = 0; i < 15; i++) {
        ins->error_state[i] = 0.0f;
        for (j = 0; j < 3; j++)
            ins->error_state[i] += K[i][j] * innov[j];
    }

    /* ---- 将误差状态注入全状态 ---- */
    /* 位置修正 */
    ins->state[S_PX] += ins->error_state[E_DP + 0];
    ins->state[S_PY] += ins->error_state[E_DP + 1];
    ins->state[S_PZ] += ins->error_state[E_DP + 2];

    /* 速度修正 */
    ins->state[S_VX] += ins->error_state[E_DV + 0];
    ins->state[S_VY] += ins->error_state[E_DV + 1];
    ins->state[S_VZ] += ins->error_state[E_DV + 2];

    /* 姿态修正: q = q * exp(dtheta/2) */
    float dth[3];
    dth[0] = ins->error_state[E_DTH + 0];
    dth[1] = ins->error_state[E_DTH + 1];
    dth[2] = ins->error_state[E_DTH + 2];
    float dth_norm = sqrtf(dth[0]*dth[0] + dth[1]*dth[1] + dth[2]*dth[2]);
    if (dth_norm > 1e-10f) {
        Vector4 dq;
        float half = dth_norm * 0.5f;
        float sinc = sinf(half) / dth_norm;
        dq[0] = cosf(half);
        dq[1] = dth[0] * sinc;
        dq[2] = dth[1] * sinc;
        dq[3] = dth[2] * sinc;
        Vector4 q_old;
        q_old[0] = ins->state[S_QW]; q_old[1] = ins->state[S_QX];
        q_old[2] = ins->state[S_QY]; q_old[3] = ins->state[S_QZ];
        Vector4 q_new;
        quat_mul(q_new, q_old, dq);
        quat_normalize(q_new);
        ins->state[S_QW] = q_new[0]; ins->state[S_QX] = q_new[1];
        ins->state[S_QY] = q_new[2]; ins->state[S_QZ] = q_new[3];
    }

    /* 零偏修正 */
    ins->state[S_BAX] += ins->error_state[E_DBA + 0];
    ins->state[S_BAY] += ins->error_state[E_DBA + 1];
    ins->state[S_BAZ] += ins->error_state[E_DBA + 2];
    ins->state[S_BWX] += ins->error_state[E_DBW + 0];
    ins->state[S_BWY] += ins->error_state[E_DBW + 1];
    ins->state[S_BWZ] += ins->error_state[E_DBW + 2];

    /* ---- 协方差更新: P = (I - K*H) * P ---- */
    /* 简化: 只更新位置相关的行 */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            float kh = 0.0f;
            int m;
            for (m = 0; m < 3; m++)
                kh += K[i][m] * ((m == j) ? 1.0f : 0.0f);
            tmp[i][j] = (i == j ? 1.0f : 0.0f) - kh;
        }
    }

    /* P = tmp * P (原地) */
    Matrix15x15 P_new;
    float psum;
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            psum = 0.0f;
            for (k = 0; k < 15; k++)
                psum += tmp[i][k] * ins->P[k][j];
            P_new[i][j] = psum;
        }
    }
    memcpy(ins->P, P_new, sizeof(Matrix15x15));

    /* 误差状态清零 */
    memset(ins->error_state, 0, sizeof(Vector15));
    ins->gps_updated = 1;
}

/* ================================================================
 *  状态获取
 * ================================================================ */

void ins_get_position(const InsSolver *ins, float *x, float *y, float *z) {
    *x = ins->state[S_PX];
    *y = ins->state[S_PY];
    *z = ins->state[S_PZ];
}

void ins_get_velocity(const InsSolver *ins, float *vx, float *vy, float *vz) {
    *vx = ins->state[S_VX];
    *vy = ins->state[S_VY];
    *vz = ins->state[S_VZ];
}

void ins_get_attitude(const InsSolver *ins, float *pitch, float *roll, float *yaw) {
    float qw = ins->state[S_QW];
    float qx = ins->state[S_QX];
    float qy = ins->state[S_QY];
    float qz = ins->state[S_QZ];

    /* 四元数转欧拉角 */
    *pitch = asinf(2.0f*(qw*qy - qz*qx));
    *roll  = atan2f(2.0f*(qw*qx + qy*qz), 1.0f - 2.0f*(qx*qx + qy*qy));
    *yaw   = atan2f(2.0f*(qw*qz + qx*qy), 1.0f - 2.0f*(qy*qy + qz*qz));
}

void ins_set_initial_pose(InsSolver *ins,
                          float x, float y, float z,
                          float yaw_deg) {
    ins->state[S_PX] = x;
    ins->state[S_PY] = y;
    ins->state[S_PZ] = z;

    float yaw_rad = yaw_deg * M_PI / 180.0f;
    float half = yaw_rad * 0.5f;
    ins->state[S_QW] = cosf(half);
    ins->state[S_QX] = 0.0f;
    ins->state[S_QY] = 0.0f;
    ins->state[S_QZ] = sinf(half);

    ins->initialized = 1;
}
