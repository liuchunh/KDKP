/**
 * math_utils.c - 嵌入式矩阵/向量运算实现
 */

#include "math_utils.h"

/* ========== Vector3 运算 ========== */

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
    if (n > 1e-8f) {
        float inv = 1.0f / n;
        result[0] = v[0] * inv;
        result[1] = v[1] * inv;
        result[2] = v[2] * inv;
    } else {
        result[0] = result[1] = result[2] = 0.0f;
    }
}

/* ========== Matrix3x3 运算 ========== */

void mat3_identity(Matrix3x3 m) {
    memset(m, 0, sizeof(Matrix3x3));
    m[0][0] = m[1][1] = m[2][2] = 1.0f;
}

void mat3_transpose(Matrix3x3 result, const Matrix3x3 m) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            result[i][j] = m[j][i];
}

void mat3_mul_vec(Vector3 result, const Matrix3x3 m, const Vector3 v) {
    result[0] = m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2];
    result[1] = m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2];
    result[2] = m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2];
}

void mat3_mul_vec_transpose(Vector3 result, const Matrix3x3 m, const Vector3 v) {
    /* R' * v, where R is rotation matrix */
    result[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2];
    result[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2];
    result[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2];
}

void mat3_skew_symmetric(Matrix3x3 result, const Vector3 v) {
    memset(result, 0, sizeof(Matrix3x3));
    result[0][1] = -v[2];  result[0][2] =  v[1];
    result[1][0] =  v[2];  result[1][2] = -v[0];
    result[2][0] = -v[1];  result[2][1] =  v[0];
}

/* ========== Quaternion 运算 ========== */

void quat_normalize(Vector4 q) {
    float n = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (n > 1e-8f) {
        float inv = 1.0f / n;
        q[0] *= inv; q[1] *= inv; q[2] *= inv; q[3] *= inv;
    }
}

void quat_mul(Vector4 result, const Vector4 p, const Vector4 q) {
    /* p ⊗ q: result = mat(p) * q */
    result[0] = p[0] * q[0] - p[1] * q[1] - p[2] * q[2] - p[3] * q[3];
    result[1] = p[1] * q[0] + p[0] * q[1] - p[3] * q[2] + p[2] * q[3];
    result[2] = p[2] * q[0] + p[3] * q[1] + p[0] * q[2] - p[1] * q[3];
    result[3] = p[3] * q[0] - p[2] * q[1] + p[1] * q[2] + p[0] * q[3];
}

void quat_to_rot_matrix(Matrix3x3 R, const Vector4 q) {
    float q0 = q[0], q1 = q[1], q2 = q[2], q3 = q[3];
    float q00 = q0 * q0, q11 = q1 * q1, q22 = q2 * q2, q33 = q3 * q3;
    R[0][0] = q00 + q11 - q22 - q33;
    R[0][1] = 2.0f * (q1 * q2 + q0 * q3);
    R[0][2] = 2.0f * (q1 * q3 - q0 * q2);
    R[1][0] = 2.0f * (q1 * q2 - q0 * q3);
    R[1][1] = q00 - q11 + q22 - q33;
    R[1][2] = 2.0f * (q2 * q3 + q0 * q1);
    R[2][0] = 2.0f * (q1 * q3 + q0 * q2);
    R[2][1] = 2.0f * (q2 * q3 - q0 * q1);
    R[2][2] = q00 - q11 - q22 + q33;
}

void quat_update(Vector4 q, const Vector3 w, float dt) {
    /* q(t+dt) = q(t) + 0.5 * dt * Omega(w) * q(t) */
    float half_dt = 0.5f * dt;
    float dq0 = -half_dt * (w[0] * q[1] + w[1] * q[2] + w[2] * q[3]);
    float dq1 =  half_dt * (w[0] * q[0] + w[2] * q[2] - w[1] * q[3]);
    float dq2 =  half_dt * (w[1] * q[0] - w[2] * q[1] + w[0] * q[3]);
    float dq3 =  half_dt * (w[2] * q[0] + w[1] * q[1] - w[0] * q[2]);
    q[0] += dq0; q[1] += dq1; q[2] += dq2; q[3] += dq3;
}

/* ========== 15x15 矩阵运算 ========== */

void mat15_diag(Matrix15x15 m, float diag_val) {
    memset(m, 0, sizeof(Matrix15x15));
    for (int i = 0; i < 15; i++)
        m[i][i] = diag_val;
}

void mat15_mul_transpose(Matrix15x15 P, const Matrix15x15 F) {
    /* P_new = F * P * F'
     * Step 1: temp = F * P
     * Step 2: P_new = temp * F'
     * 展开三层循环，15x15 需 3375 次乘加，TC264 200MHz 足够 */
    float temp[15][15];
    memset(temp, 0, sizeof(temp));
    for (int i = 0; i < 15; i++)
        for (int k = 0; k < 15; k++)
            for (int j = 0; j < 15; j++)
                temp[i][j] += F[i][k] * P[k][j];

    memset(P, 0, sizeof(Matrix15x15));
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            for (int k = 0; k < 15; k++)
                P[i][j] += temp[i][k] * F[j][k];  /* F[j][k] = F'[k][j] */
}
