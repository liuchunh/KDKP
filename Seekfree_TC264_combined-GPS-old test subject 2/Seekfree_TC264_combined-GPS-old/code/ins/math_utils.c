/**
 * math_utils.c - 嵌入式矩阵/向量运算实现
 *
 * 适配 TC264 (TriCore) 平台, TASKING 编译器
 * 所有函数使用静态数组, 无动态内存分配
 */

#include "math_utils.h"

/* ================================================================
 *  向量运算 (Vector3)
 * ================================================================ */

void vec3_set(Vector3 v, float x, float y, float z) {
    v[0] = x; v[1] = y; v[2] = z;
}

void vec3_add(Vector3 result, const Vector3 a, const Vector3 b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
}

void vec3_sub(Vector3 result, const Vector3 a, const Vector3 b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
}

void vec3_scale(Vector3 result, const Vector3 v, float s) {
    result[0] = v[0] * s;
    result[1] = v[1] * s;
    result[2] = v[2] * s;
}

void vec3_cross(Vector3 result, const Vector3 a, const Vector3 b) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

float vec3_dot(const Vector3 a, const Vector3 b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float vec3_norm(const Vector3 v) {
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vec3_normalize(Vector3 result, const Vector3 v) {
    float n = vec3_norm(v);
    if (n > 1e-10f) {
        float inv = 1.0f / n;
        result[0] = v[0] * inv;
        result[1] = v[1] * inv;
        result[2] = v[2] * inv;
    } else {
        result[0] = 0.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;
    }
}

/* ================================================================
 *  矩阵运算 (Matrix3x3)
 * ================================================================ */

void mat3_identity(Matrix3x3 m) {
    memset(m, 0, sizeof(Matrix3x3));
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
}

void mat3_transpose(Matrix3x3 result, const Matrix3x3 m) {
    Matrix3x3 tmp;
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            tmp[i][j] = m[j][i];
    memcpy(result, tmp, sizeof(Matrix3x3));
}

void mat3_mul_vec(Vector3 result, const Matrix3x3 m, const Vector3 v) {
    Vector3 tmp;
    int i;
    for (i = 0; i < 3; i++)
        tmp[i] = m[i][0] * v[0] + m[i][1] * v[1] + m[i][2] * v[2];
    memcpy(result, tmp, sizeof(Vector3));
}

void mat3_mul_vec_transpose(Vector3 result, const Matrix3x3 m, const Vector3 v) {
    /* result = M^T * v */
    Vector3 tmp;
    int i;
    for (i = 0; i < 3; i++)
        tmp[i] = m[0][i] * v[0] + m[1][i] * v[1] + m[2][i] * v[2];
    memcpy(result, tmp, sizeof(Vector3));
}

void mat3_skew_symmetric(Matrix3x3 result, const Vector3 v) {
    memset(result, 0, sizeof(Matrix3x3));
    result[0][1] = -v[2];
    result[0][2] =  v[1];
    result[1][0] =  v[2];
    result[1][2] = -v[0];
    result[2][0] = -v[1];
    result[2][1] =  v[0];
}

/* ================================================================
 *  四元数运算 (Hamilton convention: q = [w, x, y, z])
 * ================================================================ */

void quat_normalize(Vector4 q) {
    float n = sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    if (n > 1e-10f) {
        float inv = 1.0f / n;
        q[0] *= inv;
        q[1] *= inv;
        q[2] *= inv;
        q[3] *= inv;
    } else {
        q[0] = 1.0f;
        q[1] = 0.0f;
        q[2] = 0.0f;
        q[3] = 0.0f;
    }
}

void quat_mul(Vector4 result, const Vector4 p, const Vector4 q) {
    Vector4 tmp;
    tmp[0] = p[0]*q[0] - p[1]*q[1] - p[2]*q[2] - p[3]*q[3];
    tmp[1] = p[0]*q[1] + p[1]*q[0] + p[2]*q[3] - p[3]*q[2];
    tmp[2] = p[0]*q[2] - p[1]*q[3] + p[2]*q[0] + p[3]*q[1];
    tmp[3] = p[0]*q[3] + p[1]*q[2] - p[2]*q[1] + p[3]*q[0];
    memcpy(result, tmp, sizeof(Vector4));
}

void quat_to_rot_matrix(Matrix3x3 R, const Vector4 q) {
    float w = q[0], x = q[1], y = q[2], z = q[3];
    float x2 = x*x, y2 = y*y, z2 = z*z;
    float xy = x*y, xz = x*z, yz = y*z;
    float wx = w*x, wy = w*y, wz = w*z;

    R[0][0] = 1.0f - 2.0f*(y2 + z2);
    R[0][1] = 2.0f*(xy - wz);
    R[0][2] = 2.0f*(xz + wy);
    R[1][0] = 2.0f*(xy + wz);
    R[1][1] = 1.0f - 2.0f*(x2 + z2);
    R[1][2] = 2.0f*(yz - wx);
    R[2][0] = 2.0f*(xz - wy);
    R[2][1] = 2.0f*(yz + wx);
    R[2][2] = 1.0f - 2.0f*(x2 + y2);
}

void quat_update(Vector4 q, const Vector3 w, float dt) {
    /* q_new = q * dq, 其中 dq 由角速度 w 积分得到 */
    float half_dt = 0.5f * dt;
    float wx = w[0] * half_dt;
    float wy = w[1] * half_dt;
    float wz = w[2] * half_dt;

    /* 小角度近似: dq ≈ [1, wx, wy, wz] */
    float nw = sqrtf(wx*wx + wy*wy + wz*wz);
    float cos_nw, sin_nw, k;
    if (nw > 1e-10f) {
        cos_nw = cosf(nw);
        sin_nw = sinf(nw);
        k = sin_nw / nw;
    } else {
        cos_nw = 1.0f;
        k = 1.0f;
    }

    Vector4 dq;
    dq[0] = cos_nw;
    dq[1] = wx * k;
    dq[2] = wy * k;
    dq[3] = wz * k;

    quat_mul(q, q, dq);
    quat_normalize(q);
}

/* ================================================================
 *  15x15 误差协方差专用运算
 * ================================================================ */

void mat15_diag(Matrix15x15 m, float diag_val) {
    memset(m, 0, sizeof(Matrix15x15));
    int i;
    for (i = 0; i < 15; i++)
        m[i][i] = diag_val;
}

void mat15_mul_transpose(Matrix15x15 P, const Matrix15x15 F) {
    /* 计算 P = F * P * F^T (原地操作) */
    Matrix15x15 FP, result;
    float sum;
    int i, j, k;

    /* FP = F * P */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            sum = 0.0f;
            for (k = 0; k < 15; k++)
                sum += F[i][k] * P[k][j];
            FP[i][j] = sum;
        }
    }

    /* result = FP * F^T */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            sum = 0.0f;
            for (k = 0; k < 15; k++)
                sum += FP[i][k] * F[j][k];  /* F^T[k][j] = F[j][k] */
            result[i][j] = sum;
        }
    }

    memcpy(P, result, sizeof(Matrix15x15));
}
