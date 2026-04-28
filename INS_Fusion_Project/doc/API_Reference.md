# INS Fusion Project — 全部代码函数用法详解

---

## 目录

- [1. 工程文件总览](#1-工程文件总览)
- [2. math_utils.h/c — 嵌入式矩阵/向量运算库](#2-math_utilshc--嵌入式矩阵向量运算库)
- [3. ins_solver.h/c — ESKF 卡尔曼滤波核心](#3-ins_solverhc--eskf-卡尔曼滤波核心)
- [4. imu_task.h/c — IMU963RA 数据采集任务](#4-imu_taskhc--imu963ra-数据采集任务)
- [5. fusion_task.h/c — 融合调度层](#5-fusion_taskhc--融合调度层)
- [6. user/cpu0_main.c — 主入口](#6-usercpu0_mainc--主入口)
- [7. user/isr.c — 中断服务](#7-userisrc--中断服务)
- [8. 调用关系总图](#8-调用关系总图)

---

## 1. 工程文件总览

```
INS_Fusion_Project/
├── user/
│   ├── cpu0_main.c               ← 主入口，初始化 + 主循环
│   └── isr.c                     ← 中断服务 (PIT定时器 + UART3 GPS接收)
├── code/
│   ├── ins/
│   │   ├── math_utils.h          ← 类型定义 (Matrix3x3, Vector3, Quaternion...)
│   │   ├── math_utils.c          ← 矩阵/向量/四元数 基础运算实现
│   │   ├── ins_solver.h          ← InsSolver 结构体 + ESKF 核心API声明
│   │   └── ins_solver.c          ← ESKF 实现 (预测 / GPS更新 / Fd构建)
│   ├── imu_task.h                ← ImuData 结构体 + IMU963RA 驱动API声明
│   ├── imu_task.c                ← IMU963RA 读取 / 零偏标定实现
│   ├── fusion_task.h             ← NavState 结构体 + 融合调度API声明
│   └── fusion_task.c             ← 融合调度 (IMU预测→GPS更新→调试输出)
└── doc/
    └── API_Reference.md          ← 本文件
```

**依赖关系（底层→上层）：**

```
math_utils  →  ins_solver  →  fusion_task  →  cpu0_main
                    ↑              ↑
imu_task  ──────────┘              │
                                   │
zf_device_imu963ra (Seekfree库) ───┘
zf_device_gnss     (Seekfree库) ───┘
```

---

## 2. math_utils.h/c — 嵌入式矩阵/向量运算库

### 文件：`code/ins/math_utils.h`、`code/ins/math_utils.c`

最低层数学库，所有矩阵/向量/四元数运算的基础。**全部使用静态数组，无动态分配，适配嵌入式 RTOS-less 环境。**

---

### 2.1 类型定义

```c
typedef float Matrix3x3[3][3];    // 3×3 矩阵 (旋转矩阵、反对称矩阵等)
typedef float Matrix4x4[4][4];    // 4×4 矩阵 (四元数乘法矩阵)
typedef float Matrix15x15[15][15]; // 15×15 误差协方差矩阵
typedef float Vector3[3];         // 3维向量 (位置、速度、角速度等)
typedef float Vector4[4];         // 4维向量 (四元数)
typedef float Vector15[15];       // 15维误差状态向量
typedef float Vector16[16];       // 16维全状态向量
```

---

### 2.2 Vector3 向量运算

#### `vec3_set(Vector3 v, float x, float y, float z)`

设置向量的三个分量。

| 参数 | 类型 | 方向 | 含义 |
|------|------|------|------|
| `v` | `Vector3` | 输出 | 目标向量 |
| `x, y, z` | `float` | 输入 | 各分量值 |

**用法示例：**
```c
Vector3 pos;
vec3_set(pos, 10.0f, 0.0f, -5.0f);  // pos = (10, 0, -5)
```

---

#### `vec3_add(Vector3 result, const Vector3 a, const Vector3 b)`

向量加法：`result = a + b`

| 参数 | 类型 | 方向 | 含义 |
|------|------|------|------|
| `result` | `Vector3` | 输出 | a + b |
| `a, b` | `Vector3` | 输入 | 加数 |

---

#### `vec3_sub(Vector3 result, const Vector3 a, const Vector3 b)`

向量减法：`result = a - b`

---

#### `vec3_scale(Vector3 result, const Vector3 v, float s)`

向量数乘：`result = v * s`

---

#### `vec3_cross(Vector3 result, const Vector3 a, const Vector3 b)`

向量叉乘：`result = a × b`

**注意：** 叉乘结果是一个垂直于 a 和 b 的向量，方向由右手定则确定。

**用法示例：**
```c
Vector3 a = {1, 0, 0};
Vector3 b = {0, 1, 0};
Vector3 cross;
vec3_cross(cross, a, b);  // cross = (0, 0, 1)
```

---

#### `float vec3_dot(const Vector3 a, const Vector3 b)`

向量点乘：返回 `a · b`

| 返回 | 含义 |
|------|------|
| `float` | 点乘结果 (标量) |

---

#### `float vec3_norm(const Vector3 v)`

向量模长：返回 `|v| = sqrt(vx² + vy² + vz²)`

---

#### `vec3_normalize(Vector3 result, const Vector3 v)`

向量归一化：`result = v / |v|`

**注意：** 当 |v| ≤ 1e-8 时，result 将被置为零向量 (防止除零)。

---

### 2.3 Matrix3x3 矩阵运算

#### `mat3_identity(Matrix3x3 m)`

将矩阵 m 置为 3×3 单位矩阵 `I₃`。

```
     [ 1  0  0 ]
I₃ = [ 0  1  0 ]
     [ 0  0  1 ]
```

---

#### `mat3_transpose(Matrix3x3 result, const Matrix3x3 m)`

矩阵转置：`result[i][j] = m[j][i]`

---

#### `mat3_mul_vec(Vector3 result, const Matrix3x3 m, const Vector3 v)`

矩阵乘向量：`result = m · v` (3×3 × 3×1 = 3×1)

**用法示例（旋转向量）：**
```c
Matrix3x3 R;
Vector3 v_body = {1, 0, 0};
Vector3 v_nav;
mat3_mul_vec(v_nav, R, v_body);  // 将体坐标系向量转到导航坐标系
```

---

#### `mat3_mul_vec_transpose(Vector3 result, const Matrix3x3 m, const Vector3 v)`

转置矩阵乘向量：`result = m' · v` (即 m 的转置乘以 v)

**关键用途：** ESKF 中将体坐标系加速度转到导航坐标系——
```
a_n = R_nb' * a_body     // R_nb 是 "导航到机体" 的旋转矩阵
                         // 所以 R_nb' 是 "机体到导航"
```

---

#### `mat3_skew_symmetric(Matrix3x3 result, const Vector3 v)`

构造向量 v 的反对称矩阵 `[v]×`：

```
         [  0   -vz   vy ]
[v]×  =  [  vz   0   -vx ]
         [ -vy   vx   0  ]
```

**关键用途：** 用角速度向量 w 构造四元数运动学矩阵、叉乘的矩阵化表示。

---

### 2.4 Quaternion 四元数运算

#### `quat_normalize(Vector4 q)`

四元数归一化：`q = q / |q|`

**注意：** 四元数表示姿态时必须是单位四元数（|q|=1），每次更新后必须归一化。

---

#### `quat_mul(Vector4 result, const Vector4 p, const Vector4 q)`

四元数乘法 `p ⊗ q`（Hamilton 乘法）：

```
p ⊗ q = [ p0*q0 - p1*q1 - p2*q2 - p3*q3 ]
        [ p1*q0 + p0*q1 - p3*q2 + p2*q3 ]
        [ p2*q0 + p3*q1 + p0*q2 - p1*q3 ]
        [ p3*q0 - p2*q1 + p1*q2 + p0*q3 ]
```

**关键用途：** 旋转复合。`q_new = q_old ⊗ dq` 表示先在 dq 旋转后再在 q_old 旋转。

**用法示例（ESKF 姿态修正，见 `ins_update_gps`）：**
```c
// 误差姿态修正:  q_new = q_old ⊗ [1, dθ/2]
Vector4 dq = {1.0f, 0.5f * dtheta_x, 0.5f * dtheta_y, 0.5f * dtheta_z};
quat_normalize(dq);
quat_mul(&ins->state[6], q_old, dq);
```

---

#### `quat_to_rot_matrix(Matrix3x3 R, const Vector4 q)`

四元数 → 方向余弦矩阵（DCM，rotation matrix from navigation to body frame）：

```
R_nb = [ q0²+q1²-q2²-q3²      2(q1q2+q0q3)          2(q1q3-q0q2)      ]
       [   2(q1q2-q0q3)     q0²-q1²+q2²-q3²         2(q2q3+q0q1)      ]
       [   2(q1q3+q0q2)       2(q2q3-q0q1)        q0²-q1²-q2²+q3²     ]
```

**注意：** 四元数 q = [qw, qx, qy, qz] 对应 q[0]=qw, q[1]=qx, q[2]=qy, q[3]=qz

**用法示例（ESKF 预测中提取旋转矩阵）：**
```c
Matrix3x3 R_nb;
quat_to_rot_matrix(R_nb, ins->state + 6);  // 从当前四元数提取
```

---

#### `quat_update(Vector4 q, const Vector3 w, float dt)`

四元数时间积分：使用零阶四元数运动学方程

```
q(t+dt) = q(t) + 0.5 · dt · Ω(w) · q(t)

其中 Ω(w) = [  0  -wx  -wy  -wz ]
            [ wx    0   wz  -wy ]
            [ wy  -wz    0   wx ]
            [ wz   wy  -wx    0 ]
```

| 参数 | 含义 |
|------|------|
| `q` | 输入/输出 → 当前四元数 (原地更新) |
| `w` | 角速度向量 (rad/s, 体坐标系, 已扣零偏) |
| `dt` | 积分步长 (秒, 通常 0.01 = 100Hz) |

**重要：** 调用后通常需要 `quat_normalize(q)`，因为 Euler 积分会略微破坏单位模长约束。

**用法示例（ESKF 每步预测）：**
```c
quat_update(q, w_m, dt);      // w_m = gyro - bw (补偿零偏后的角速度)
quat_normalize(q);
```

---

### 2.5 15×15 矩阵运算

#### `mat15_diag(Matrix15x15 m, float diag_val)`

将 15×15 矩阵初始化为对角矩阵，所有对角元素 = `diag_val`，非对角元素 = 0。

**用法示例（初始化协方差）：**
```c
Matrix15x15 P;
mat15_diag(P, 0.01f);           // P = 0.01 * I₁₅
P[0][0] = P[1][1] = P[2][2] = 0.1f;  // 位置不确定性调大
```

---

#### `mat15_mul_transpose(Matrix15x15 P, const Matrix15x15 F)`

协方差传播核心运算：`P_new = F · P · F'`

计算量：15³ = 3375 次乘加，在 TC264 200MHz 下约需 200us。

**用法示例（ESKF 预测步骤）：**
```c
Matrix15x15 Fd_copy;
memcpy(Fd_copy, ins->Fd, sizeof(Matrix15x15));
mat15_mul_transpose(ins->P, Fd_copy);  // P = Fd * P * Fd'
```

---

## 3. ins_solver.h/c — ESKF 卡尔曼滤波核心

### 文件：`code/ins/ins_solver.h`、`code/ins/ins_solver.c`

从 MATLAB `InsSolver.m` 移植的误差状态卡尔曼滤波器（Error-State Kalman Filter, ESKF）。这是整个项目的核心算法。

**关键设计决策：**
- 使用 **欧拉积分** 代替 ODE45（100Hz IMU 下足够精确，计算量降低 >10倍）
- **3×3 解析求逆** 代替高斯消元（S 矩阵只有 3×3，因为 GPS 只观测位置）
- 协方差传播使用 **Fd · P · Fd' + Qd** 标准形式

---

### 3.1 数据结构

```
InsSolver 结构体:
┌────────────────────────────────────────────────┐
│ state[16]       全状态                          │
│   [0:2]   位置 p (x, y, z)  — NED坐标系          │
│   [3:5]   速度 v (vx, vy, vz) — NED坐标系         │
│   [6:9]   四元数 q (qw, qx, qy, qz)              │
│   [10:12] 加速度计零偏 ba                        │
│   [13:15] 陀螺仪零偏 bw                          │
├────────────────────────────────────────────────┤
│ error_state[15] 误差状态 (每次更新后归零)         │
│   [0:2]   位置误差 δp                            │
│   [3:5]   速度误差 δv                            │
│   [6:8]   姿态误差 δθ (轴角表示)                  │
│   [9:11]  加计零偏误差 δba                       │
│   [12:14] 陀螺零偏误差 δbw                       │
├────────────────────────────────────────────────┤
│ P[15][15]    误差协方差矩阵                      │
│ Qc[15][15]   连续过程噪声协方差                  │
│ Rc[3]        GPS测量噪声方差 (对角)              │
│ Fd[15][15]   离散状态转移矩阵 (中间变量)          │
│ Qd[15][15]   离散过程噪声协方差 (中间变量)         │
├────────────────────────────────────────────────┤
│ initialized   滤波器初始化标志                    │
│ gps_updated   本周期GPS是否已更新                 │
└────────────────────────────────────────────────┘
```

**坐标系约定：NED (北-东-地)，g = [0, 0, -9.8]**

---

### 3.2 宏定义（可标定参数）

| 宏 | 默认值 | 含义 |
|----|--------|------|
| `GRAVITY` | `9.80f` | 当地重力加速度 (m/s²)，成都约 9.79，北京约 9.80 |
| `IMU_DT` | `0.01f` | IMU 采样周期 (秒)，对应 100Hz |
| `GPS_DT` | `0.1f` | GPS 采样周期 (秒)，对应 10Hz |
| `QC_POS_NOISE` | `0.01f` | 位置过程噪声默认值 |
| `QC_ATT_NOISE` | `0.01f` | 姿态过程噪声默认值 |
| `RC_POS_NOISE` | `0.01f` | GPS位置测量噪声默认值 |

---

### 3.3 公开函数

#### `void ins_solver_init(InsSolver *ins, float Qc_diag[15], float Rc_diag[3])`

初始化 ESKF 滤波器。

| 参数 | 类型 | 方向 | 含义 |
|------|------|------|------|
| `ins` | `InsSolver*` | 输出 | 滤波器实例 |
| `Qc_diag` | `float[15]` | 输入 | 15维过程噪声对角元素，传 NULL 使用默认值 |
| `Rc_diag` | `float[3]` | 输入 | GPS测量噪声对角 [var_x, var_y, var_z]，传 NULL 使用默认值 |

**内部做了什么：**
1. 将全状态归零，四元数设为单位四元数 `[1,0,0,0]`（无旋转）
2. 误差状态归零
3. P 矩阵初始化为对角的 0.01（位置对角 0.1，表示初始位置不确定性较大）
4. Qc 和 Rc 按传入参数或默认值设置
5. 置 `initialized = 1`

**用法示例：**
```c
InsSolver ins;
float Qc_diag[15] = {0.001,0.001,0.001, 0,0,0, 0.01,0.01,0.01, 1e-4,1e-4,1e-4, 1e-4,1e-4,1e-4};
float Rc_diag[3] = {1.0, 1.0, 2.0};
ins_solver_init(&ins, Qc_diag, Rc_diag);
```

---

#### `void ins_predict(InsSolver *ins, float acc_x, float acc_y, float acc_z, float gyro_x, float gyro_y, float gyro_z, float dt)`

IMU 驱动的状态预测（每步 100Hz 调用）。这是整个 ESKF 计算量最大的函数。

| 参数 | 类型 | 含义 |
|------|------|------|
| `ins` | `InsSolver*` | 滤波器实例 |
| `acc_x, acc_y, acc_z` | `float` | 加速度计测量值 (m/s², **体坐标系**, 已扣除静止零偏) |
| `gyro_x, gyro_y, gyro_z` | `float` | 陀螺仪测量值 (rad/s, **体坐标系**, 已扣除静止零偏) |
| `dt` | `float` | 积分步长 (秒, 通常 = IMU_DT = 0.01) |

**执行流程（8个步骤）：**

```
步骤1: 补偿零偏     a_m = acc - ba,  w_m = gyro - bw
步骤2: 姿态更新     q(t+dt) = q(t) + 0.5·dt·Ω(w_m)·q(t)  (欧拉积分)
步骤3: 四元数归一化  quat_normalize(q)
步骤4: 提取旋转矩阵  R_nb = quat_to_rot_matrix(q)
步骤5: 速度更新     a_n = R_nb' * a_m + g,  v += a_n * dt
步骤6: 位置更新     p += v * dt
步骤7: 协方差预测   P = Fd·P·Fd' + Qd
      其中 Fd 由 build_Fd() 构建 (15x15 泰勒展开)
      其中 Qd ≈ Qc * dt (一阶离散化)
步骤8: 误差状态归零  memset(error_state, 0, 15)
```

**重要：**
- 传入的加速度必须是**体坐标系**（IMU 装在车上，敏感轴随车身旋转）
- 陀螺仪零偏 `bw`、加速度计零偏 `ba` 由 ESKF 在线估计，无需手动维护
- 函数内部会修改 `ins->state`、`ins->P`、`ins->Fd`、`ins->Qd`

---

#### `void ins_update_gps(InsSolver *ins, float gps_x, float gps_y, float gps_z)`

GPS 位置测量更新（10Hz 调用）。用 GPS 测量值修正 ESKF 预测状态。

| 参数 | 类型 | 含义 |
|------|------|------|
| `ins` | `InsSolver*` | 滤波器实例 |
| `gps_x, gps_y, gps_z` | `float` | GPS 位置 (m, **NED 本地坐标系**) |

**执行流程（5个步骤）：**

```
步骤1: 计算新息 innovation = [gps_x - px, gps_y - py, gps_z - pz]
步骤2: 计算新息协方差 S = H·P·H' + R = P[0:3,0:3] + diag(Rc)
步骤3: 解析求逆 S⁻¹ (3×3 行列式展开)
步骤4: 计算卡尔曼增益 K = P·H'·S⁻¹ (K 是 15×3)
步骤5: 误差状态修正 dx = K · innovation
       然后注入全状态:
         - 位置 += dx[0:2]
         - 速度 += dx[3:5]
         - 姿态: q = q ⊗ [1, dx[6:8]/2]
         - 零偏 += dx[9:14]
步骤6: 协方差更新 P = (I - K·H)·P
```

**坐标系注意：** 传入的是本地 NED 坐标，不是经纬度。GPS 经纬度→NED 的转换由上层 `fusion_gps_update()` 完成。

**用法示例：**
```c
// 从 GPS 经纬度转换为本地 NED 后
float x_ned = ...;  // 北向偏移(m)
float y_ned = ...;  // 东向偏移(m)
float z_ned = ...;  // 地下向偏移(NED z轴向下, m)
ins_update_gps(&ins, x_ned, y_ned, z_ned);
```

---

#### `void ins_get_position(const InsSolver *ins, float *x, float *y, float *z)`

读取当前 ESKF 估计的位置。

| 参数 | 类型 | 方向 | 含义 |
|------|------|------|------|
| `ins` | `const InsSolver*` | 输入 | 滤波器实例 |
| `x, y, z` | `float*` | 输出 | 位置 (m, NED) |

---

#### `void ins_get_velocity(const InsSolver *ins, float *vx, float *vy, float *vz)`

读取当前 ESKF 估计的速度。

| 参数 | 类型 | 方向 | 含义 |
|------|------|------|------|
| `ins` | `const InsSolver*` | 输入 | 滤波器实例 |
| `vx, vy, vz` | `float*` | 输出 | 速度 (m/s, NED) |

---

#### `void ins_get_attitude(const InsSolver *ins, float *pitch, float *roll, float *yaw)`

从 ESKF 四元数提取欧拉角（ZYX 旋转顺序: 先 yaw, 再 pitch, 最后 roll）。

| 参数 | 类型 | 方向 | 含义 |
|------|------|------|------|
| `ins` | `const InsSolver*` | 输入 | 滤波器实例 |
| `pitch` | `float*` | 输出 | 俯仰角 (rad, 范围 ±π/2) |
| `roll` | `float*` | 输出 | 横滚角 (rad, 范围 ±π) |
| `yaw` | `float*` | 输出 | 航向角 (rad, 范围 ±π, 0=北) |

**转换公式（从四元数直接计算）：**
```
pitch = asin(2*(q1*q3 + q0*q2))
roll  = atan2(-2*(q0*q1 - q2*q3), q0² - q1² - q2² + q3²)
yaw   = atan2( 2*(q0*q1 + q2*q3), q0² - q1² + q2² - q3²)
```

**注意：** pitch 范围是 ±π/2（与 `asinf` 一致），若俯仰角超限会出现万向节死锁——但卡丁车比赛中俯仰角通常在 ±15° 范围内，不会遇到此问题。

---

#### `void ins_set_initial_pose(InsSolver *ins, float x, float y, float z, float yaw_deg)`

设置 ESKF 初始位置和航向（初始对准）。

| 参数 | 类型 | 含义 |
|------|------|------|
| `ins` | `InsSolver*` | 滤波器实例 |
| `x, y, z` | `float` | 初始位置 (m, NED) |
| `yaw_deg` | `float` | 初始航向角 (**度**, 0=北, 90=东) |

**四元数设置：** q = [cos(yaw/2), 0, 0, sin(yaw/2)]（pitch=0, roll=0, 仅设 yaw）

**用法示例：**
```c
// 发车区对准北偏东30度
ins_set_initial_pose(&ins, 0.0f, 0.0f, 0.0f, 30.0f);
```

---

### 3.4 内部静态函数（不直接调用，供 ins_predict 内部使用）

#### `static void build_Fd(Matrix15x15 Fd, const Matrix3x3 R_nb, const Vector3 a_hat, const Vector3 w_hat, float dt)`

构建 15×15 离散误差状态转移矩阵 `Fd`。这是整个 ESKF 中最复杂的矩阵运算。

**Fd 的块结构：**

| 行列 | 0:3 | 3:6 | 6:9 | 9:12 | 12:15 |
|------|-----|-----|-----|------|-------|
| **0:3** (δp) | I₃ | dt·I₃ | **A** | **B** | -R'·dt²/2 |
| **3:6** (δv) | 0 | I₃ | **C** | **D** | -R'·dt |
| **6:9** (δθ) | 0 | 0 | **E** | **F** | 0 |
| **9:12** (δba) | 0 | 0 | 0 | I₃ | 0 |
| **12:15** (δbw) | 0 | 0 | 0 | 0 | I₃ |

其中 A,B,C,D,E,F 是 3×3 子矩阵，使用泰勒级数展开（到 dt⁵ 项）逼近闭式解。

**MATLAB 对照：**
| MATLAB (`InsSolver.m`) | C (`ins_solver.c`) |
|---|---|
| `getExponentMatFd()` | `build_Fd()` |
| `A, B, C, D, E, F` | `A, B, C, D, E, F_mat` |
| `skew_a_hat, skew_omega_hat` | `skew_a, skew_w` |
| `predCbn` (C_nb) | `R_nb` (实际上是同一个矩阵) |

---

#### `static void build_Qd(Matrix15x15 Qd, const Matrix15x15 Qc, float dt)`

一阶近似构建离散过程噪声协方差：`Qd = Qc * dt`

**注意：** 这是一个简化。MATLAB 原版用了更复杂的 `Qd = dt·Q̂ + dt²/2·(Fc·Q̂ + Q̂·Fc')`。在当前 100Hz 采样率下，一阶近似误差 < 1%。

---

## 4. imu_task.h/c — IMU963RA 数据采集任务

### 文件：`code/imu_task.h`、`code/imu_task.c`

封装 Seekfree IMU963RA 驱动，负责传感器初始化、原始数据读取、物理单位转换、零偏标定。

---

### 4.1 数据结构

```c
typedef struct {
    float acc_x, acc_y, acc_z;    // 加速度 (m/s², 体坐标系, 已扣零偏)
    float gyro_x, gyro_y, gyro_z; // 角速度 (rad/s, 体坐标系, 已扣零偏)
    float mag_x, mag_y, mag_z;    // 磁场 (Gauss, 体坐标系)
    uint8 fresh;                  // 1=本轮有新数据, 消费后清0
} ImuData;

extern ImuData g_imu;   // 全局单例，100Hz 更新
```

---

### 4.2 函数

#### `uint8 imu_task_init(void)`

初始化 IMU963RA 传感器（SPI 模式，约 10MHz）。

| 返回 | 含义 |
|------|------|
| `0` | 初始化成功 |
| `1` | 初始化失败（传感器自检不通过或通信异常） |

**内部做了什么：**
1. 清零 `g_imu`
2. 调用 `imu963ra_init()`（Seekfree 库函数——配置 SPI、复位、量程设置、自检、磁力计 Hub 初始化）
3. 延时 100ms 等待传感器稳定
4. （注释掉的）可选自动标定 `imu_task_calibrate(100)`

**默认量程配置（在 `zf_device_imu963ra.h` 中）：**
- 加速度计：±8G（`IMU963RA_ACC_SAMPLE_DEFAULT = IMU963RA_ACC_SAMPLE_SGN_8G`）
- 陀螺仪：±2000dps（`IMU963RA_GYRO_SAMPLE_DEFAULT = IMU963RA_GYRO_SAMPLE_SGN_2000DPS`）
- 磁力计：8 Gauss（`IMU963RA_MAG_SAMPLE_DEFAULT = IMU963RA_MAG_SAMPLE_8G`）

> 如需改量程，修改 `zf_device_imu963ra.h` 中的 `IMU963RA_ACC_SAMPLE_DEFAULT` 等宏。
> 卡丁快跑车速 ≤25km/h，转弯加速度一般 < 2G，±8G 量程足够。

---

#### `void imu_task_read(void)`

从 IMU963RA 读取一帧完整数据（加速度 + 陀螺 + 磁力计），转换为物理单位，扣除静止标定零偏，写全局 `g_imu`。

**调用频率：** 100Hz（每 10ms 一次）

**数据转换链：**
```
IMU963RA 原始 ADC 值 (int16)
    ↓  imu963ra_acc_transition() 宏     = raw / transition_factor[0]
    ↓  imu963ra_gyro_transition() 宏    = raw / transition_factor[1]
数值范围 [−量程, +量程] (float)
    ↓  * 9.8 (加速度: g → m/s²)
    ↓  * π/180 (角速度: deg/s → rad/s)
物理单位 (m/s², rad/s)
    ↓  − g_acc_bias / g_gyro_bias (静止标定零偏)
最终输出 → g_imu
```

**transition_factor 默认值** (在驱动初始化时设定)：
- `imu963ra_transition_factor[0]` = 4098（±8G → 每 LSB = 8*9.8/4098 ≈ 0.019 m/s²）
- `imu963ra_transition_factor[1]` = 14.3（±2000dps → 每 LSB = 2000/14.3 ≈ 140 °/s）
- `imu963ra_transition_factor[2]` = 3000（8G → 每 LSB = 8/3000 ≈ 0.0027 Gauss）

---

#### `void imu_task_calibrate(uint16 num_samples)`

IMU 静止零偏标定。**必须在车辆完全静止时调用。**

| 参数 | 含义 |
|------|------|
| `num_samples` | 采样帧数，推荐 100~500（对应 1~5 秒） |

**原理：**
- 静止时，陀螺仪输出应为 0；实际输出均值 = 零偏
- 静止时，加速度计应只测到重力（x=y≈0, z≈-9.8）；实际输出与理想值的差 = 零偏

**零偏存储：** 函数内部更新文件级全局变量 `g_acc_bias[3]` 和 `g_gyro_bias[3]`，后续每次 `imu_task_read()` 自动扣除。

**用法示例：**
```c
// 车停在发车区，上电后标定 2 秒
imu_task_calibrate(200);   // 200 帧 × 10ms = 2秒
```

---

## 5. fusion_task.h/c — 融合调度层

### 文件：`code/fusion_task.h`、`code/fusion_task.c`

最高层抽象，将 IMU 读取 / ESKF 预测 / GPS 更新 / 调试输出 组装成完整的融合定位流水线。

---

### 5.1 数据结构

```c
typedef struct {
    float x, y, z;            // 当前估计位置 (m, NED)
    float vx, vy, vz;         // 当前估计速度 (m/s, NED)
    float pitch, roll, yaw;   // 当前估计姿态 (rad)
    float pos_std;            // 位置标准差 ≈ sqrt(P[0][0]+P[1][1]+P[2][2])
    uint8 gps_valid;          // 最近一次GPS更新是否有效
} NavState;

extern NavState g_nav;        // 全局导航状态
```

---

### 5.2 函数

#### `void fusion_init(void)`

初始化整个融合系统（IMU + ESKF + GPS 等待）。

**执行顺序：**
1. `imu_task_init()` — 初始化 IMU963RA
2. `ins_solver_init()` — 创建 ESKF 实例，设置噪声参数
3. `ins_set_initial_pose()` — 初始位置归零，航向暂设 0°
4. 重置 `g_nav.gps_valid = 0`

**重要：** 此处 GPS 初始化（`gnss_init(GN42A)`）在 `cpu0_main.c` 中更早执行。

---

#### `void fusion_imu_predict(void)`

检查 `g_imu.fresh` 标志，如果有新 IMU 数据，消费并执行一次 ESKF 预测，然后刷新 `g_nav`。

**调用频率：** 主循环中每轮检查（实际约 100Hz，与 PIT 中断同步）

**完整流程：**
```c
if (!g_imu.fresh) return;                          // 无新数据则跳过
g_imu.fresh = 0;                                    // 消费标志
g_imu_count++;

ins_predict(&g_ins,
    g_imu.acc_x, g_imu.acc_y, g_imu.acc_z,
    g_imu.gyro_x, g_imu.gyro_y, g_imu.gyro_z,
    IMU_DT);                                        // ESKF 预测

ins_get_position(&g_ins, &g_nav.x, &g_nav.y, &g_nav.z);
ins_get_velocity(&g_ins, &g_nav.vx, &g_nav.vy, &g_nav.vz);
ins_get_attitude(&g_ins, &g_nav.pitch, &g_nav.roll, &g_nav.yaw);
g_nav.pos_std = sqrtf(g_ins.P[0][0] + g_ins.P[1][1] + g_ins.P[2][2]);
```

---

#### `void fusion_gps_update(void)`

当有新的 GPS NMEA 帧到达时调用。执行 GPS→NED 坐标变换，然后 ESKF 测量更新。

**调用频率：** GPS 输出速率（约 10Hz）

**GPS 经纬度 → NED 本地坐标系变换：**

```
输入: lat, lon (度), height (m), 基准点 LAT0, LON0, H0
输出: x_n (北), y_e (东), z_d (下)

纬度差 1° ≈ 111320 m (常数)
经度差 1° ≈ 111320 × cos(LAT0) m (随纬度变化)

x_n = (lat - LAT0) × 111320.0
y_e = (lon - LON0) × 111320.0 × cos(LAT0 × π/180)
z_d = -(height - H0)       // NED: z轴向下为正
```

**重要：**
- 基准点 `LAT0`, `LON0`, `H0` 必须在实车调试时修改为赛场发车区实际经纬度！
- 当前默认值 `LAT0=30.0, LON0=104.0` 是成都附近，仅为占位。
- 比赛场地（如温州研讨会场地）经纬度需要实地用 GPS 采集几帧取平均。

**用法示例（修改基准点）：**
```c
// 在 fusion_gps_update() 中修改这三行：
static const double LAT0 = 27.xxxxx;   // 发车区纬度 (用GPS模块实测)
static const double LON0 = 120.xxxxx;  // 发车区经度
static const float  H0   = 5.0f;       // 发车区海拔高度 (m)
```

---

#### `void fusion_get_nav_state(NavState *nav)`

获取当前导航状态的拷贝。线程安全（只读拷贝，不修改滤波器内部状态）。

| 参数 | 方向 | 含义 |
|------|------|------|
| `nav` | 输出 | 导航状态副本，传 NULL 不执行 |

---

#### `void fusion_debug_print(void)`

通过调试串口 (UART0) 输出当前导航状态。输出格式：

```
NAV: pos=(12.34, -5.67, 0.12) yaw=45.3deg std=0.45m
```

**含义：**
| 字段 | 含义 |
|------|------|
| `pos=(x, y, z)` | NED 位置，单位 m |
| `yaw=xxx deg` | 航向角，单位度 (°) |
| `std=xxx m` | 位置标准差（ESKF 对自身精度的置信度估算） |

**std 解读：**
- `< 0.3m`：滤波器收敛良好，置信度高
- `0.3 ~ 1m`：中等精度
- `> 1m`：滤波器发散或 GPS 长时间未更新，需检查

---

## 6. user/cpu0_main.c — 主入口

### 文件：`user/cpu0_main.c`

TC264 CPU0 核主函数。初始化 → 主循环。调度逻辑完全通过标志位驱动（无 RTOS）。

---

### 6.1 全局变量

| 变量 | 类型 | 含义 |
|------|------|------|
| `g_imu_tick` | `volatile uint8` | PIT Ch1 中断置 1，主循环消费后清零。表示 IMU 采样时刻到达。 |
| `g_debug_tick` | `volatile uint8` | PIT Ch0 中断置 1，主循环消费后清零。表示调试输出时刻到达。 |

---

### 6.2 函数

#### `int core0_main(void)`

**初始化顺序：**

```
1. clock_init()         — 配置系统时钟
2. debug_init()         — 调试串口 (UART0)
3. gnss_init(GN42A)    — GPS 模块初始化 (UART3, 115200, 配置报文速率和类型)
4. pit_ms_init(Ch0, 100) — 调试定时器 100ms
5. pit_ms_init(Ch1, 10)  — IMU 定时器 10ms
6. fusion_init()         — 初始化 IMU963RA + ESKF
7. cpu_wait_event_ready()— 等待多核同步
```

**主循环（while(TRUE)）：**

```
┌──────────────────────────────────────────┐
│ 每一轮 (约 1ms, system_delay_ms(1)):      │
│                                           │
│ 检查 g_imu_tick:                          │
│   ├─ imu_task_read()    (读取IMU传感器)    │
│   └─ fusion_imu_predict() (ESKF预测)      │
│                                           │
│ 检查 gnss_flag (GPS新帧):                 │
│   └─ fusion_gps_update()  (ESKF更新)      │
│                                           │
│ 检查 g_debug_tick (100ms一次):            │
│   └─ 每10次(即1秒) → fusion_debug_print() │
└──────────────────────────────────────────┘
```

**时间分析：**

| 操作 | 频率 | 耗时估算 |
|------|------|----------|
| PIT Ch1 中断 | 100Hz | < 1us（仅置标志） |
| IMU 读取 + ESKF 预测 | 100Hz | ~ 250us |
| GPS 帧处理 + ESKF 更新 | ~10Hz | ~ 150us |
| 调试输出 | 1Hz | ~ 500us（串口阻塞） |
| **CPU 总负载** | — | **< 5% @ 200MHz** |

---

## 7. user/isr.c — 中断服务

### 文件：`user/isr.c`

在 Seekfree 原版 isr.c 基础上修改，将 PIT Ch1 的空白中断改为 `g_imu_tick = 1` 置标志。

---

### 7.1 修改过的中断

#### `cc60_pit_ch0_isr` — PIT Ch0 调试中断

```
频率: 10Hz (100ms周期)
操作: g_debug_tick = 1
      pit_clear_flag(CCU60_CH0)
```

#### `cc60_pit_ch1_isr` — PIT Ch1 IMU 中断

```
频率: 100Hz (10ms周期)
操作: g_imu_tick = 1
      pit_clear_flag(CCU60_CH1)
```

#### `uart3_rx_isr` — UART3 GPS 接收中断

```
触发: GPS 模块发来数据
操作: gnss_uart_callback()  — 逐字节接收，组帧，校验
```

---

### 7.2 未使用的中断（保留接口）

以下中断保留了 ISR 骨架，正文被注释，如需使用取消注释即可：

| ISR | 用途 |
|-----|------|
| `exti_ch0_ch4_isr` | IMU660RC 外部中断 (若换用660系列IMU) |
| `exti_ch1_ch5_isr` | ToF 测距模块中断 |
| `exti_ch3_ch7_isr` | 摄像头 VSYNC 场同步 |
| `dma_ch5_isr` | 摄像头 DMA 传输 |
| `uart1_rx_isr` | 摄像头 UART |
| `uart2_rx_isr` | 无线模块 UART |

---

## 8. 调用关系总图

```
┌──────────────────────────────────────────────────────────────────┐
│                          core0_main()                            │
│                                                                  │
│  ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────────┐ │
│  │ gnss_init │   │pit_ms_init│  │pit_ms_init│  │ fusion_init  │ │
│  │(GN42A)   │   │(Ch0,100ms)│  │(Ch1,10ms) │  │       ↓      │ │
│  └──────────┘   └─────┬────┘   └─────┬────┘   │ imu_task_init│ │
│                       │              │         │ ins_solver_  │ │
│  ═══════════════════  │  PIT 中断 ═══│═══════  │   init       │ │
│                       ↓              ↓         │ ins_set_     │ │
│  ┌──────────────────────────────────────────┐  │   initial_pose│ │
│  │              isr.c                        │  └──────────────┘ │
│  │                                           │                  │
│  │  cc60_pit_ch0_isr:  g_debug_tick=1       │                  │
│  │  cc60_pit_ch1_isr:  g_imu_tick=1         │                  │
│  │  uart3_rx_isr:      gnss_uart_callback() │                  │
│  └──────────────────────────────────────────┘                  │
│                                                                  │
│  ═══════════════════  主循环  ═══════════════════════════════════ │
│                                                                  │
│  while(TRUE) {                                                   │
│                                                                  │
│    if(g_imu_tick) {            ← PIT Ch1 100Hz 触发              │
│      g_imu_tick=0;                                              │
│      ┌─────────────────┐                                        │
│      │ imu_task_read() │────────────┐                            │
│      │  ├ imu963ra_get │             │                           │
│      │  │   _acc()     │             │                           │
│      │  │   _gyro()    │             │                           │
│      │  │   _mag()     │             │                           │
│      │  └─单位转换+扣零偏│            │                           │
│      │  g_imu.fresh=1  │             │                           │
│      └─────────────────┘             │                           │
│                                      ↓                           │
│      ┌───────────────────┐                                       │
│      │fusion_imu_predict │                                       │
│      │ └→ ins_predict()  │ ← math_utils (quat_update,           │
│      │    ├ 姿态四元数更新 │            quat_to_rot_matrix,       │
│      │    ├ 速度积分+重力补偿│           mat3_mul_vec_transpose)    │
│      │    ├ 位置积分      │                                       │
│      │    ├ build_Fd()   │ ← 15x15 泰勒展开                      │
│      │    ├ mat15_mul_   │                                       │
│      │    │  transpose() │                                       │
│      │    └ P = FdPFd'+Qd│                                       │
│      └───────────────────┘                                       │
│    }                                                             │
│                                                                  │
│    if(gnss_flag) {             ← UART3 收到完整NMEA帧             │
│      ┌────────────────────┐                                      │
│      │ fusion_gps_update  │                                      │
│      │ ├ gnss_data_parse()│ ← $GNRMC + $GNGGA 解析               │
│      │ ├ lat/lon → NED    │ ← 经纬度→本地坐标                     │
│      │ └ ins_update_gps() │ ← 卡尔曼测量更新                     │
│      │   ├ 3x3 解析求逆   │                                      │
│      │   ├ K = PH'/S      │                                      │
│      │   ├ 全状态修正     │ ←位置+速度+四元数+零偏                 │
│      │   └ P = (I-KH)P    │                                      │
│      └────────────────────┘                                      │
│    }                                                             │
│                                                                  │
│    if(g_debug_tick) {          ← PIT Ch0 10Hz 触发               │
│      g_debug_tick=0;                                            │
│      fusion_debug_print();     ← 串口输出导航状态 (1Hz)           │
│    }                                                             │
│  }                                                               │
└──────────────────────────────────────────────────────────────────┘
```

---

## 附录 A：常用变量速查表

| 变量 | 定义位置 | 类型 | 含义 |
|------|----------|------|------|
| `g_imu` | `imu_task.c` | `ImuData` | IMU 最新数据 (100Hz 刷新) |
| `g_imu.fresh` | — | `uint8` | 新数据标志，ISR 写→主循环读 |
| `g_imu_tick` | `cpu0_main.c` | `volatile uint8` | IMU 定时器到点标志 |
| `g_debug_tick` | `cpu0_main.c` | `volatile uint8` | 调试定时器到点标志 |
| `g_nav` | `fusion_task.c` | `NavState` | 融合后导航状态 (实时) |
| `g_ins` | `fusion_task.c` | `static InsSolver` | ESKF 滤波器单例 |
| `gnss` | Seekfree库 | `gnss_info_struct` | GPS 最新解析数据 |
| `gnss_flag` | Seekfree库 | `uint8` | GPS 新帧到达标志 |
| `imu963ra_acc_x` | Seekfree库 | `int16` | 加速度计原始 ADC 值 |
| `imu963ra_gyro_x` | Seekfree库 | `int16` | 陀螺仪原始 ADC 值 |
| `imu963ra_transition_factor[3]` | Seekfree库 | `float[3]` | 原始值→物理值转换因子 |

## 附录 B：MATLAB → C 对照表

| MATLAB (`InsSolver.m`) | C (`ins_solver.c`) |
|---|---|
| `state0 = zeros(16,1); state0(7)=1` | `state_init()`, `state[6]=1.0f` |
| `errorstate0 = zeros(15,1)` | `error_state_init()` |
| `Qc0 = diag(Cov)` | `ins->Qc = diag(...)` |
| `Rc0 = diag([0.01,0.01,0.01])` | `ins->Rc = {1.0, 1.0, 2.0}` |
| `imuDynamics(t,state,x)` | `ins_predict()` 内联 |
| `statePrediction()` (ODE45) | `ins_predict()` (欧拉积分) |
| `getExponentMatFd(predCbn,step,a_hat,omega_hat)` | `build_Fd(Fd, R_nb, a_hat, w_hat, dt)` |
| `getPredCovarianceMatQd(Gc,Fc,Qc,step)` | `build_Qd(Qd, Qc, dt)` (简化为一阶) |
| `MeasurementUpdate(error_gps_noise,predP,predErrorState)` | `ins_update_gps(ins, gps_x, gps_y, gps_z)` |
| `QuatNormalize(state(i+1,7:10))` | `quat_normalize(q)` |
| `cnb = quat2cnb(quat)` | `quat_to_rot_matrix(R_nb, q)` |
| `QuatMulitMat(state(i+1,7:10),[1,0.5*...])` | `quat_mul(q_new, q_old, dq)` |
| `acc_noise, gyro_noise, gps_noise` | `g_imu.acc_*, g_imu.gyro_*, fusion_gps_update()` |

---

*文档版本: 1.0 · 生成日期: 2026-04-28*
