/**
 * math_utils.h - 嵌入式矩阵/向量运算工具
 *
 * 用于 INS 卡尔曼滤波的最小化线性代数库
 * 所有矩阵使用静态数组，无动态内存分配
 * 适配 TC264 (TriCore) 平台
 */

#ifndef INS_MATH_UTILS_H_
#define INS_MATH_UTILS_H_

#include <math.h>
#include <string.h>

/* 常量和大小定义 */
#ifndef M_PI
#define M_PI                3.14159265358979323846f
#endif

#define ESKF_STATE_DIM      16      /* 全状态: [p(3) v(3) q(4) ba(3) bw(3)] */
#define ESKF_ERROR_DIM      15      /* 误差状态: [dp(3) dv(3) dtheta(3) dba(3) dbw(3)] */

typedef float Matrix3x3[3][3];
typedef float Matrix4x4[4][4];
typedef float Matrix15x15[15][15];
typedef float Vector3[3];
typedef float Vector4[4];
typedef float Vector15[15];
typedef float Vector16[16];

/* 向量运算 */
void vec3_set(Vector3 v, float x, float y, float z);
void vec3_add(Vector3 result, const Vector3 a, const Vector3 b);
void vec3_sub(Vector3 result, const Vector3 a, const Vector3 b);
void vec3_scale(Vector3 result, const Vector3 v, float s);
void vec3_cross(Vector3 result, const Vector3 a, const Vector3 b);
float vec3_dot(const Vector3 a, const Vector3 b);
float vec3_norm(const Vector3 v);
void vec3_normalize(Vector3 result, const Vector3 v);

/* 矩阵运算 */
void mat3_identity(Matrix3x3 m);
void mat3_transpose(Matrix3x3 result, const Matrix3x3 m);
void mat3_mul_vec(Vector3 result, const Matrix3x3 m, const Vector3 v);
void mat3_mul_vec_transpose(Vector3 result, const Matrix3x3 m, const Vector3 v);
void mat3_skew_symmetric(Matrix3x3 result, const Vector3 v);

/* 四元数运算 */
void quat_normalize(Vector4 q);
void quat_mul(Vector4 result, const Vector4 p, const Vector4 q);
void quat_to_rot_matrix(Matrix3x3 R, const Vector4 q);
void quat_update(Vector4 q, const Vector3 w, float dt);

/* 15x15 误差协方差专用 */
void mat15_diag(Matrix15x15 m, float diag_val);
void mat15_mul_transpose(Matrix15x15 P, const Matrix15x15 F);  /* P = F*P*F' */

#endif /* INS_MATH_UTILS_H_ */
