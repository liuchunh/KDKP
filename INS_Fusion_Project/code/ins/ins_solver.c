/**
 * ins_solver.c - ESKF GPS+IMU融合定位核心实现
 *
 * 从 MATLAB InsSolver.m 移植到 TC264 嵌入式C
 * 关键简化: ODE45 -> 欧拉积分 (适用于100Hz高频IMU)
 */

#include "ins_solver.h"

#define GRAVITY_VEC  {0.0f, 0.0f, -GRAVITY}   /* NED坐标系, g沿-z方向 */

/* ========== 内部辅助 ========== */

static void state_init(Vector16 state) {
    /* 初始状态: 位置(0,0,0), 速度0, 单位四元数, 零偏0 */
    memset(state, 0, sizeof(Vector16));
    state[6] = 1.0f;        /* qw = 1, 单位四元数 */
}

static void error_state_init(Vector15 es) {
    memset(es, 0, sizeof(Vector15));
}

static void build_Fd(Matrix15x15 Fd, const Matrix3x3 R_nb,
                     const Vector3 a_hat, const Vector3 w_hat,
                     float dt) {
    /* 构建离散误差状态转移矩阵 Fd
     * a_hat = acc - ba_hat (补偿零偏后的加速度)
     * w_hat = gyro - bw_hat (补偿零偏后的角速度)
     *
     * 结构 (15x15):
     *   [  I₃     dt*I₃    A       B       -R'*dt²/2 ]
     *   [  0₃     I₃       C       D       -R'*dt    ]
     *   [  0₃     0₃       E       F        0₃       ]
     *   [  0₃     0₃       0₃      I₃       0₃       ]
     *   [  0₃     0₃       0₃      0₃       I₃       ]
     *
     * 其中 A,B,C,D,E,F 使用泰勒展开近似:
     *   A = -R' * [a_hat]× * (dt²/2*I - dt³/6*[w]× + dt⁴/24*[w]×²)
     *   B = -R' * [a_hat]× * (-dt³/6*I + dt⁴/24*[w]× - dt⁵/120*[w]×²)
     *   C = -R' * [a_hat]× * (dt*I - dt²/2*[w]× + dt³/6*[w]×²)
     *   D = -A
     *   E = I - dt*[w]× + dt²/2*[w]×²
     *   F = -dt*I + dt²/2*[w]× - dt³/6*[w]×²
     */

    float dt2 = dt * dt;
    float dt3 = dt2 * dt;

    /* 反对称矩阵 */
    Matrix3x3 skew_a, skew_w;
    mat3_skew_symmetric(skew_a, a_hat);
    mat3_skew_symmetric(skew_w, w_hat);

    /* skew_w² = skew_w * skew_w */
    Matrix3x3 skew_w2;
    memset(skew_w2, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int k = 0; k < 3; k++)
            for (int j = 0; j < 3; j++)
                skew_w2[i][j] += skew_w[i][k] * skew_w[k][j];

    Matrix3x3 I3;
    mat3_identity(I3);

    /* 计算中间矩阵 A, B, C, D, E, F (3x3) */
    Matrix3x3 term1, term2, term3, temp;
    Matrix3x3 A, B, C, D, E, F_mat;

    /* E = I - dt*skew_w + dt2/2*skew_w2 */
    memset(E, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            E[i][j] = I3[i][j] - dt * skew_w[i][j] + 0.5f * dt2 * skew_w2[i][j];

    /* F = -dt*I + dt2/2*skew_w - dt3/6*skew_w2 */
    memset(F_mat, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            F_mat[i][j] = -dt * I3[i][j] + 0.5f * dt2 * skew_w[i][j]
                         - (dt3 / 6.0f) * skew_w2[i][j];

    /* term_C = dt*I - dt2/2*skew_w + dt3/6*skew_w2 */
    Matrix3x3 term_C;
    memset(term_C, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            term_C[i][j] = dt * I3[i][j] - 0.5f * dt2 * skew_w[i][j]
                         + (dt3 / 6.0f) * skew_w2[i][j];

    /* term_A = dt2/2*I - dt3/6*skew_w + dt4/24*skew_w2 */
    float dt4 = dt3 * dt;
    Matrix3x3 term_A;
    memset(term_A, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            term_A[i][j] = 0.5f * dt2 * I3[i][j] - (dt3 / 6.0f) * skew_w[i][j]
                         + (dt4 / 24.0f) * skew_w2[i][j];

    /* term_B = -dt3/6*I + dt4/24*skew_w - dt5/120*skew_w2 */
    float dt5 = dt4 * dt;
    Matrix3x3 term_B;
    memset(term_B, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            term_B[i][j] = -(dt3 / 6.0f) * I3[i][j] + (dt4 / 24.0f) * skew_w[i][j]
                         - (dt5 / 120.0f) * skew_w2[i][j];

    /* C = -R' * skew_a * term_C, A = -R' * skew_a * term_A, B = -R' * skew_a * term_B */
    Matrix3x3 sa_tc, sa_ta, sa_tb;
    memset(sa_tc, 0, sizeof(Matrix3x3));
    memset(sa_ta, 0, sizeof(Matrix3x3));
    memset(sa_tb, 0, sizeof(Matrix3x3));

    for (int i = 0; i < 3; i++)
        for (int k = 0; k < 3; k++) {
            for (int j = 0; j < 3; j++) {
                sa_tc[i][j] += skew_a[i][k] * term_C[k][j];
                sa_ta[i][j] += skew_a[i][k] * term_A[k][j];
                sa_tb[i][j] += skew_a[i][k] * term_B[k][j];
            }
        }

    /* R' * sa_tc, R' * sa_ta, R' * sa_tb */
    memset(C, 0, sizeof(Matrix3x3));
    memset(A, 0, sizeof(Matrix3x3));
    memset(B, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int k = 0; k < 3; k++) {
            for (int j = 0; j < 3; j++) {
                C[i][j] -= R_nb[k][i] * sa_tc[k][j];   /* -R' * ... */
                A[i][j] -= R_nb[k][i] * sa_ta[k][j];
                B[i][j] -= R_nb[k][i] * sa_tb[k][j];
            }
        }

    /* D = -A，即 D = R' * skew_a * term_B */
    memset(D, 0, sizeof(Matrix3x3));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            D[i][j] = -A[i][j];

    /* 组装 15x15 Fd，全部初始化为0 */
    memset(Fd, 0, sizeof(Matrix15x15));

    /* 块 (0,0): I₃ */
    for (int i = 0; i < 3; i++) Fd[i][i] = 1.0f;

    /* 块 (0,1): dt*I₃ */
    for (int i = 0; i < 3; i++) Fd[i][3+i] = dt;

    /* 块 (0,2): A (3x3) */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[i][6+j] = A[i][j];

    /* 块 (0,3): B (3x3) */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[i][9+j] = B[i][j];

    /* 块 (0,4): -R' * dt²/2 */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[i][12+j] = -R_nb[j][i] * 0.5f * dt2;

    /* 块 (1,1): I₃ */
    for (int i = 0; i < 3; i++) Fd[3+i][3+i] = 1.0f;

    /* 块 (1,2): C (3x3) */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[3+i][6+j] = C[i][j];

    /* 块 (1,3): D (3x3) */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[3+i][9+j] = D[i][j];

    /* 块 (1,4): -R' * dt */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[3+i][12+j] = -R_nb[j][i] * dt;

    /* 块 (2,2): E (3x3) */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[6+i][6+j] = E[i][j];

    /* 块 (2,3): F (3x3) */
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            Fd[6+i][9+j] = F_mat[i][j];

    /* 块 (3,3): I₃ */
    for (int i = 0; i < 3; i++) Fd[9+i][9+i] = 1.0f;

    /* 块 (4,4): I₃ */
    for (int i = 0; i < 3; i++) Fd[12+i][12+i] = 1.0f;
}

static void build_Qd(Matrix15x15 Qd, const Matrix15x15 Qc, float dt) {
    /* 简化: Qd ≈ Qc * dt (一阶近似)
     * 正式使用时用 ins_solver_init 初始化的 Qc 对角值 */
    memcpy(Qd, Qc, sizeof(Matrix15x15));
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            Qd[i][j] *= dt;
}

/* ========== 核心公开 API ========== */

void ins_solver_init(InsSolver *ins, float Qc_diag[15], float Rc_diag[3]) {
    memset(ins, 0, sizeof(InsSolver));

    state_init(ins->state);
    error_state_init(ins->error_state);

    /* 初始化协方差 P */
    mat15_diag(ins->P, 0.01f);
    ins->P[0][0] = ins->P[1][1] = ins->P[2][2] = 0.1f;   /* 位置不确定性较大 */

    /* 设置过程噪声 Qc */
    memset(ins->Qc, 0, sizeof(Matrix15x15));
    if (Qc_diag) {
        for (int i = 0; i < 15; i++)
            ins->Qc[i][i] = Qc_diag[i];
    } else {
        ins->Qc[0][0] = ins->Qc[1][1] = ins->Qc[2][2] = QC_POS_NOISE;
        ins->Qc[6][6] = ins->Qc[7][7] = ins->Qc[8][8] = QC_ATT_NOISE;
    }

    /* 设置测量噪声 Rc */
    if (Rc_diag) {
        ins->Rc[0] = Rc_diag[0];
        ins->Rc[1] = Rc_diag[1];
        ins->Rc[2] = Rc_diag[2];
    } else {
        ins->Rc[0] = ins->Rc[1] = ins->Rc[2] = RC_POS_NOISE;
    }

    ins->initialized = 1;
}

void ins_predict(InsSolver *ins,
                 float acc_x, float acc_y, float acc_z,
                 float gyro_x, float gyro_y, float gyro_z,
                 float dt) {
    if (!ins->initialized) return;

    Vector16 *s = &ins->state;
    float *p  = *s + 0;      /* 位置 [0:2] */
    float *v  = *s + 3;      /* 速度 [3:5] */
    float *q  = *s + 6;      /* 四元数 [6:9] */
    float *ba = *s + 10;     /* 加计零偏 [10:12] */
    float *bw = *s + 13;     /* 陀螺零偏 [13:15] */

    /* 1. 补偿零偏 */
    Vector3 a_m, w_m;
    a_m[0] = acc_x - ba[0];
    a_m[1] = acc_y - ba[1];
    a_m[2] = acc_z - ba[2];
    w_m[0] = gyro_x - bw[0];
    w_m[1] = gyro_y - bw[1];
    w_m[2] = gyro_z - bw[2];

    /* 2. 更新姿态四元数 q(t+dt) = q(t) + 0.5*dt*Omega(w)*q(t) */
    quat_update(q, w_m, dt);
    quat_normalize(q);

    /* 3. 四元数转旋转矩阵 R_nb (navigation to body) */
    Matrix3x3 R_nb;
    quat_to_rot_matrix(R_nb, q);

    /* 4. 将加速度转到导航系并更新速度 dv = (R'*(a) + g) * dt */
    Vector3 a_n;
    mat3_mul_vec_transpose(a_n, R_nb, a_m);  /* a_n = R' * a_body */
    a_n[0] += 0.0f;
    a_n[1] += 0.0f;
    a_n[2] += GRAVITY;                        /* NED: g沿+z方向 */

    v[0] += a_n[0] * dt;
    v[1] += a_n[1] * dt;
    v[2] += a_n[2] * dt;

    /* 5. 位置更新 p += v * dt */
    p[0] += v[0] * dt;
    p[1] += v[1] * dt;
    p[2] += v[2] * dt;

    /* 6. 零偏随机游走 (零偏本身在误差状态中修正，此处名义值不变) */

    /* 7. 预测误差协方差 P = Fd*P*Fd' + Qd */
    build_Fd(ins->Fd, R_nb, a_m, w_m, dt);
    build_Qd(ins->Qd, ins->Qc, dt);

    Matrix15x15 Fd_copy;
    memcpy(Fd_copy, ins->Fd, sizeof(Matrix15x15));

    mat15_mul_transpose(ins->P, Fd_copy);  /* P = Fd * P * Fd' */

    /* P = P + Qd */
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            ins->P[i][j] += ins->Qd[i][j];

    /* 8. 清零误差状态 (每次修正后理应已经归零) */
    memset(ins->error_state, 0, sizeof(Vector15));
    ins->gps_updated = 0;
}

void ins_update_gps(InsSolver *ins,
                    float gps_x, float gps_y, float gps_z) {
    if (!ins->initialized) return;

    /* H = [I₃, 0₃ₓ₁₂], 直接使用位置误差 */
    Vector3 innovation;
    innovation[0] = gps_x - ins->state[0];
    innovation[1] = gps_y - ins->state[1];
    innovation[2] = gps_z - ins->state[2];

    /* S = H*P*H' + R = P[0:3,0:3] + diag(Rc) */
    float S[3][3];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            S[i][j] = ins->P[i][j];
    S[0][0] += ins->Rc[0];
    S[1][1] += ins->Rc[1];
    S[2][2] += ins->Rc[2];

    /* 求 S 的逆 (3x3 解析求逆) */
    float det = S[0][0] * (S[1][1] * S[2][2] - S[1][2] * S[2][1])
              - S[0][1] * (S[1][0] * S[2][2] - S[1][2] * S[2][0])
              + S[0][2] * (S[1][0] * S[2][1] - S[1][1] * S[2][0]);

    float S_inv[3][3];
    if (fabsf(det) > 1e-10f) {
        float inv_det = 1.0f / det;
        S_inv[0][0] =  (S[1][1] * S[2][2] - S[1][2] * S[2][1]) * inv_det;
        S_inv[0][1] = -(S[0][1] * S[2][2] - S[0][2] * S[2][1]) * inv_det;
        S_inv[0][2] =  (S[0][1] * S[1][2] - S[0][2] * S[1][1]) * inv_det;
        S_inv[1][0] = -(S[1][0] * S[2][2] - S[1][2] * S[2][0]) * inv_det;
        S_inv[1][1] =  (S[0][0] * S[2][2] - S[0][2] * S[2][0]) * inv_det;
        S_inv[1][2] = -(S[0][0] * S[1][2] - S[0][2] * S[1][0]) * inv_det;
        S_inv[2][0] =  (S[1][0] * S[2][1] - S[1][1] * S[2][0]) * inv_det;
        S_inv[2][1] = -(S[0][0] * S[2][1] - S[0][1] * S[2][0]) * inv_det;
        S_inv[2][2] =  (S[0][0] * S[1][1] - S[0][1] * S[1][0]) * inv_det;
    } else {
        memset(S_inv, 0, sizeof(S_inv));
    }

    /* K = P * H' * S_inv (K 是 15x3 矩阵，H'取 P 的前3列) */
    float K[15][3];
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 3; j++) {
            K[i][j] = 0;
            for (int k = 0; k < 3; k++)
                K[i][j] += ins->P[i][k] * S_inv[k][j];
        }

    /* 误差状态修正: dx = K * innovation */
    Vector15 dx;
    memset(dx, 0, sizeof(Vector15));
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 3; j++)
            dx[i] += K[i][j] * innovation[j];

    /* 注入全状态 */
    /* 位置修正 */
    ins->state[0] += dx[0];
    ins->state[1] += dx[1];
    ins->state[2] += dx[2];

    /* 速度修正 */
    ins->state[3] += dx[3];
    ins->state[4] += dx[4];
    ins->state[5] += dx[5];

    /* 姿态修正: q_new = q ⊗ [1, dtheta/2] */
    Vector4 dq = {1.0f, 0.5f * dx[6], 0.5f * dx[7], 0.5f * dx[8]};
    quat_normalize(dq);
    Vector4 q_temp;
    memcpy(q_temp, &ins->state[6], sizeof(Vector4));
    quat_mul(&ins->state[6], q_temp, dq);
    quat_normalize(&ins->state[6]);

    /* 零偏修正 */
    ins->state[10] += dx[9];
    ins->state[11] += dx[10];
    ins->state[12] += dx[11];
    ins->state[13] += dx[12];
    ins->state[14] += dx[13];
    ins->state[15] += dx[14];

    /* 协方差更新: P = (I - K*H) * P
     * 等价于 P(i,j) -= sum_k K(i,k) * P(k,j) for k in 0..2 */
    float P_old[15][15];
    memcpy(P_old, ins->P, sizeof(Matrix15x15));
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            for (int k = 0; k < 3; k++)
                ins->P[i][j] -= K[i][k] * P_old[k][j];

    ins->gps_updated = 1;
}

void ins_get_position(const InsSolver *ins, float *x, float *y, float *z) {
    *x = ins->state[0];
    *y = ins->state[1];
    *z = ins->state[2];
}

void ins_get_velocity(const InsSolver *ins, float *vx, float *vy, float *vz) {
    *vx = ins->state[3];
    *vy = ins->state[4];
    *vz = ins->state[5];
}

void ins_get_attitude(const InsSolver *ins, float *pitch, float *roll, float *yaw) {
    /* 从四元数提取欧拉角 (ZYX顺序: yaw-pitch-roll)
     * R_nb = [cr*cy-sr*sp*sy,  cr*sy+sr*sp*cy, -sr*cp]
     *         [   -cp*sy,          cp*cy,          sp   ]
     *         [sr*cy+cr*sp*sy,  sr*sy-cr*sp*cy,  cr*cp ]
     */
    float q0 = ins->state[6];
    float q1 = ins->state[7];
    float q2 = ins->state[8];
    float q3 = ins->state[9];

    float R12 = 2.0f * (q0 * q1 + q2 * q3);       /* -sr*cp or just a term */
    float R22 = q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3;
    float R31 = 2.0f * (q0 * q1 - q2 * q3);
    float R32 = 2.0f * (q1 * q3 + q0 * q2);       /* sp */
    float R33 = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

    /* pitch = asin(R32) = asin(2*(q1*q3 + q0*q2)) */
    *pitch = asinf(R32);

    /* roll = atan2(-R31, R33) */
    *roll = atan2f(-R31, R33);

    /* yaw = atan2(R12, R22) */
    *yaw = atan2f(R12, R22);
}

void ins_set_initial_pose(InsSolver *ins,
                          float x, float y, float z,
                          float yaw_deg) {
    /* 设置初始位置 */
    ins->state[0] = x;
    ins->state[1] = y;
    ins->state[2] = z;

    /* 从 yaw 角设置初始四元数 (pitch=roll=0, 仅航向) */
    float half_yaw = yaw_deg * (float)M_PI / 360.0f;  /* 度 -> 弧度半角 */
    ins->state[6] = cosf(half_yaw);   /* qw */
    ins->state[7] = 0.0f;              /* qx */
    ins->state[8] = 0.0f;              /* qy */
    ins->state[9] = sinf(half_yaw);   /* qz */
}
