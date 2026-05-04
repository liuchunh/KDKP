# INS Fusion Project — 全部代码函数用法详解

---

## 目录

- [1. 工程文件总览](#1-工程文件总览)
- [2. math_utils.h/c — 嵌入式矩阵/向量运算库](#2-math_utilshc--嵌入式矩阵向量运算库)
- [3. ins_solver.h/c — ESKF 卡尔曼滤波核心](#3-ins_solverhc--eskf-卡尔曼滤波核心)
- [4. imu_task.h/c — IMU963RA 数据采集任务](#4-imu_taskhc--imu963ra-数据采集任务)
- [5. fusion_task.h/c — 融合调度层](#5-fusion_taskhc--融合调度层)
- [6. nav_controller.h/c — 科目1 航点导航控制器](#6-nav_controllerhc--科目1-航点导航控制器)
- [7. user/cpu0_main.c — 主入口](#7-usercpu0_mainc--主入口)
- [8. user/isr.c — 中断服务](#8-userisrc--中断服务)
- [9. 调用关系总图](#9-调用关系总图)
- [10. NaN 防护机制详解](#10-nan-防护机制详解)

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
│   ├── fusion_task.c             ← 融合调度 (IMU预测→GPS更新→NaN防护→调试输出)
│   ├── nav_controller.h          ← 科目1 航点导航: Waypoint/NavMode 定义
│   └── nav_controller.c          ← 航点记录 / 距离航向误差 / 到达判定
└── doc/
    ├── API_Reference.md          ← 本文件
    └── Subject1_Usage_Guide.md   ← 科目1操作指南
```

**依赖关系（底层→上层）：**

```
math_utils  →  ins_solver  →  fusion_task  ──→  nav_controller  →  cpu0_main
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

// pos_std 加 NaN 保护: P 对角和必须为正才开方
float sum_p = g_ins.P[0][0] + g_ins.P[1][1] + g_ins.P[2][2];
g_nav.pos_std = (sum_p > 0.0f) ? sqrtf(sum_p) : 0.0f;
```

---

#### `void fusion_gps_update(void)`

当有新的 GPS NMEA 帧到达时调用。**5层防护确保不会引入 NaN**：

| 步骤 | 检查内容 | 失败处理 |
|------|----------|----------|
| 1 | `gnss_data_parse()` 返回值 | 校验失败→丢弃帧 |
| 2 | `gnss.state == 0` | 定位无效→丢弃帧 |
| 3 | 经纬度范围 (lat∈[15°,55°], lon∈[70°,140°]) | 异常值→丢弃帧 |
| 4 | 基准点自动设置 (首次有效定位) | 自动记录为原点 |
| 5 | NED 坐标 NaN/Inf 检查 | 异常→丢弃帧 |

**调用频率：** GPS 输出速率（约 10Hz）

**GPS 经纬度 → NED 本地坐标系变换：**

```
输入: lat, lon (度), height (m)
基准点: 首次有效 GPS 定位 (自动设置, 无需手动配置)
输出: x_n (北), y_e (东), z_d (下)

纬度差 1° ≈ 111320 m (常数)
经度差 1° ≈ 111320 × cos(LAT0) m (随纬度变化)

x_n = (lat - LAT0) × 111320.0
y_e = (lon - LON0) × 111320.0 × cos(LAT0 × π/180)
z_d = -(height - H0)       // NED: z轴向下为正
```

**重要：** 基准点 `LAT0, LON0, H0` **无需手动设置**。首次收到有效 GPS 定位时自动记录，此后所有 NED 坐标都相对此原点计算。每次上电会重新设置。

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

## 6. nav_controller.h/c — 科目1 航点导航控制器

### 文件：`code/nav_controller.h`、`code/nav_controller.c`

科目1自动驾驶的核心导航模块。使用 INS 融合后的 NED 坐标和航向角进行航点追踪。

**坐标系：** NED (北-东-地)，与 INS 融合输出一致。航向 yaw (rad)，0=北，π/2=东。

---

### 6.1 数据结构

```c
/* 单个航点 */
typedef struct {
    float x, y;              // NED 位置 (m)
    float target_yaw;        // 期望航向 (rad), 用于车库对准
    float arrival_radius;    // 到达判定半径 (m)
    uint8 type;              // 0=普通锥桶, 1=车库入口, 2=车库停靠点
} Waypoint;

/* 导航模式 */
typedef enum {
    NAV_IDLE = 0,            // 空闲
    NAV_TEACHING,            // 教学记录模式
    NAV_RUNNING,             // 自动导航运行中
    NAV_FINISHED,            // 所有航点完成
} NavMode;

/* 全局变量 */
extern Waypoint g_waypoints[MAX_WAYPOINTS];   // 航点数组
extern uint8    g_waypoint_count;              // 已记录航点数
extern uint8    g_current_target;              // 当前追踪的航点索引
extern NavMode  g_nav_mode;                    // 当前导航模式
```

---

### 6.2 航点类型与到达半径

| type | 名称 | arrival_radius | 用途 |
|------|------|----------------|------|
| 0 | 普通锥桶 | 0.8 m | 转弯点，到达后切换航向追踪下一锥桶 |
| 1 | 车库入口 | 0.5 m | 到门前减速，用 `target_yaw` 对准入库方向 |
| 2 | 车库停靠点 | 0.3 m | 最终停车位置，到达后停电机 |

---

### 6.3 函数

#### `uint8 nav_record_waypoint(uint8 type)`

在教学模式下，以当前 INS 融合位置 `(g_nav.x, g_nav.y)` 记录一个航点，同时记录当前航向 `g_nav.yaw` 作为期望朝向。

| 参数 | 类型 | 含义 |
|------|------|------|
| `type` | `uint8` | 航点类型: 0=锥桶, 1=车库入口, 2=停靠点 |

| 返回 | 含义 |
|------|------|
| `1 ~ MAX_WAYPOINTS` | 记录成功，返回当前航点总数 |
| `0` | 失败（航点数组已满） |

**用法示例：**
```c
// 按键中断中:
if (ButtonPushed(KEY2)) { nav_record_waypoint(0); }  // 锥桶
if (ButtonPushed(KEY3)) { nav_record_waypoint(1); }  // 车库入口
if (ButtonPushed(KEY4)) { nav_record_waypoint(2); }  // 停靠点
```

---

#### `void nav_compute_errors(float *distance, float *yaw_error)`

计算当前位置到目标航点的**水平距离**和**航向误差**。

| 参数 | 类型 | 方向 | 含义 |
|------|------|------|------|
| `distance` | `float*` | 输出 | 到目标航点的水平距离 (m) |
| `yaw_error` | `float*` | 输出 | 航向误差 (rad), 归一化到 [-π, π] |

**导航策略：**
- 普通锥桶 (type=0): 期望航向 = 车指向目标的方向 `atan2(dy, dx)`
- 车库入口/停靠点 (type=1/2): 期望航向 = 教学时记录的 `target_yaw`（入库对准方向）

**yaw_error 解读：**
- `> 0` → 目标在车右侧，需右转
- `< 0` → 目标在车左侧，需左转
- `= 0` → 车头正对目标

**用法示例 (PID控制循环):**
```c
float dist, yaw_err;
nav_compute_errors(&dist, &yaw_err);
float speed_cmd = speed_pid(dist);       // 距离 → 速度
float steer_cmd = steer_pid(yaw_err);    // 航向误差 → 舵角
motor_set_speed(speed_cmd);
servo_set_angle(steer_cmd);
```

---

#### `uint8 nav_check_arrival(void)`

检查是否到达当前航点。到达判定：当前位置到航点的距离 < `arrival_radius`。到达后自动切换到下一个航点。

| 返回 | 含义 |
|------|------|
| `0` | 尚未到达，正常追踪 |
| `1` | 进入减速区 (距目标 < 2m)，应降低车速 |
| `2` | 已到达，已自动切换到下一航点 |

**注意：** 当最后一个航点到达后，`g_nav_mode` 自动变为 `NAV_FINISHED`，此时应在主循环中停车。

---

#### `void nav_clear_waypoints(void)`

清空所有航点，重置 `g_waypoint_count=0`, `g_current_target=0`, `g_nav_mode=NAV_IDLE`。

**用法示例：**
```c
// 进入教学模式前:
nav_clear_waypoints();
g_nav_mode = NAV_TEACHING;
```

---

#### `const Waypoint* nav_get_current_target(void)`

获取当前追踪的航点指针。所有航点完成后返回 `NULL`。

---

#### `void nav_debug_print(void)`

通过调试串口打印所有已记录的航点。输出格式：

```
=== Waypoints ===
  #0: (0.00, 0.00) yaw=45.0deg
  #1: (5.23, 3.10) yaw=60.0deg
  #2: (10.50, 7.80) yaw=90.0deg
```

---

### 6.4 教学→自动完整流程

```
┌─────────────────────────────────────────────────────────┐
│ 教学阶段                                                 │
│                                                         │
│  按 K1 → NAV_TEACHING, 清空航点                          │
│  把车推/遥控到每个关键位置, 按 K2/K3/K4 记录航点          │
│  按 K1 → NAV_RUNNING, 出发                               │
│                                                         │
├─────────────────────────────────────────────────────────┤
│ 自动阶段 (每个 100Hz 控制周期)                            │
│                                                         │
│  nav_compute_errors(&dist, &yaw_err)                    │
│  arrival = nav_check_arrival()                          │
│                                                         │
│  if (arrival == 2 && g_nav_mode == NAV_FINISHED)        │
│      motor_stop();                    // 到达终点       │
│  else                                                   │
│      motor_set_speed(speed_pid(dist));                  │
│      servo_set_angle(steer_pid(yaw_err));               │
│                                                         │
│  GPS有效检查:                                           │
│  if (g_nav.pos_std > 3.0f) → 减速等待GPS恢复            │
└─────────────────────────────────────────────────────────┘
```

---

### 6.5 坐标系示意

```
        N (北, x轴)
        ↑
        │
        │  操场
        │  ┌───────────────────┐
        │  │  锥桶2            │
        │  │    ●              │
        │  │        锥桶3      │
        │  │          ●        │
        │  │                   │
        │  │ 锥桶1       车库   │
        │  │   ●   ┌──┐        │
        │  │       │  │ ← 1.5m │
        │  │   起点●  │  │ 2m   │
        │  │       └──┘        │
        │  └───────────────────┘
        │
        └──────────────────────→ E (东, y轴)

  yaw = 0       → 车朝北
  yaw = π/2     → 车朝东
  yaw = ±π      → 车朝南
```

---

## 7. user/cpu0_main.c — 主入口

### 文件：`user/cpu0_main.c`

TC264 CPU0 核主函数。初始化 → 主循环。调度逻辑完全通过标志位驱动（无 RTOS）。

---

### 7.1 全局变量

| 变量 | 类型 | 含义 |
|------|------|------|
| `g_imu_tick` | `volatile uint8` | PIT Ch1 中断置 1，主循环消费后清零。表示 IMU 采样时刻到达。 |
| `g_debug_tick` | `volatile uint8` | PIT Ch0 中断置 1，主循环消费后清零。表示调试输出时刻到达。 |

---

### 7.2 函数

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
┌──────────────────────────────────────────────────────┐
│ 每一轮 (约 1ms, system_delay_ms(1)):                  │
│                                                       │
│ 检查 g_imu_tick:                                      │
│   ├─ imu_task_read()    (读取IMU传感器)                │
│   ├─ fusion_imu_predict() (ESKF预测)                  │
│   └─ if(NAV_RUNNING) {                                │
│         nav_compute_errors()  (航向/距离误差)          │
│         nav_check_arrival()   (到达判定)               │
│         PID → 电机/舵机      (驱动车辆)                │
│       }                                               │
│                                                       │
│ 检查 gnss_flag (GPS新帧):                             │
│   └─ fusion_gps_update()  (ESKF更新+5层NaN防护)       │
│                                                       │
│ 按键处理:                                             │
│   K1: 教学模式/自动运行 切换                           │
│   K2/K3/K4: 教学模式中记录航点                         │
│                                                       │
│ 检查 g_debug_tick (100ms一次):                        │
│   └─ 每10次(即1秒) → fusion_debug_print()             │
│                      nav_debug_print() (教学模式下)    │
└──────────────────────────────────────────────────────┘
```

**时间分析：**

| 操作 | 频率 | 耗时估算 |
|------|------|----------|
| PIT Ch1 中断 | 100Hz | < 1us（仅置标志） |
| IMU 读取 + ESKF 预测 | 100Hz | ~ 250us |
| nav 计算 | 100Hz | ~ 20us |
| GPS 帧处理 + ESKF 更新 | ~10Hz | ~ 150us |
| 调试输出 | 1Hz | ~ 500us（串口阻塞） |
| **CPU 总负载** | — | **< 5% @ 200MHz** |

---

## 8. user/isr.c — 中断服务

### 文件：`user/isr.c`

在 Seekfree 原版 isr.c 基础上修改，将 PIT Ch1 的空白中断改为 `g_imu_tick = 1` 置标志。

---

### 8.1 修改过的中断

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

### 8.2 未使用的中断（保留接口）

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

## 9. 调用关系总图

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
│      │ ├ 1.gnss_data_parse│ ← 检查返回值                        │
│      │ ├ 2.gnss.state检查 │ ← 定位有效?                         │
│      │ ├ 3.lat/lon范围    │ ← 中国境内?                          │
│      │ ├ 4.基准点自动设置 │ ← 首次有效定位→原点                  │
│      │ ├ 5.NaN/Inf检查    │ ← NED坐标异常?                       │
│      │ ├ lat/lon → NED    │ ← 经纬度→本地坐标                     │
│      │ └ ins_update_gps() │ ← 卡尔曼测量更新                     │
│      │   ├ NaN/Inf保护    │                                      │
│      │   ├ 新息门限500m   │                                      │
│      │   ├ 3x3 解析求逆   │                                      │
│      │   ├ K = PH'/S      │                                      │
│      │   ├ 全状态修正     │ ←位置+速度+四元数+零偏                 │
│      │   ├ P = (I-KH)P    │                                      │
│      │   └ P对角钳位≥1e-8 │                                      │
│      └────────────────────┘                                      │
│    }                                                             │
│                                                                  │
│    /* 导航控制 (仅NAV_RUNNING模式) */                             │
│    ┌──────────────────────┐                                      │
│    │ nav_compute_errors() │ ← 距离+航向误差                      │
│    │ nav_check_arrival()  │ ← 到达判定+切换航点                  │
│    │ PID → 电机/舵机      │ ← 车辆运动控制                       │
│    └──────────────────────┘                                      │
│                                                                  │
│    if(g_debug_tick) {          ← PIT Ch0 10Hz 触发               │
│      g_debug_tick=0;                                            │
│      fusion_debug_print();     ← 串口输出导航状态 (1Hz)           │
│      nav_debug_print();        ← 串口输出航点列表 (教学模式)       │
│    }                                                             │
│  }                                                               │
└──────────────────────────────────────────────────────────────────┘
```

---

## 10. NaN 防护机制详解

> **问题现象**: 未接 GPS 时串口输出正常，接 GPS 后全部显示 `nan`。
>
> **根因**: GPS 首次定位时 NMEA 数据异常（lat/lon=0.0 或校验失败），NED 坐标产生百万米级新息 → 协方差矩阵 P 变非正定 → `sqrtf(负数)` → NaN → 永久污染全状态和协方差。

### 10.1 防护总览

| 层次 | 文件 | 位置 | 措施 | 类型 |
|------|------|------|------|------|
| L1 | `fusion_task.c` | `fusion_gps_update()` | 检查 `gnss_data_parse()` 返回值 | 校验失败丢弃 |
| L2 | `fusion_task.c` | `fusion_gps_update()` | 检查 `gnss.state == 0` | 无效定位丢弃 |
| L3 | `fusion_task.c` | `fusion_gps_update()` | lat∈[15,55], lon∈[70,140] | 异常坐标丢弃 |
| L4 | `fusion_task.c` | `fusion_gps_update()` | 基准点自动设置为首次有效定位 | 消除大偏移 |
| L5 | `fusion_task.c` | `fusion_gps_update()` | NED 坐标 `isnan()/isinf()` | NaN输入丢弃 |
| L6 | `ins_solver.c` | `ins_update_gps()` | GPS输入 `isnan()/isinf()` | NaN输入丢弃 |
| L7 | `ins_solver.c` | `ins_update_gps()` | 新息 norm > 500m | 跳变丢弃 |
| L8 | `ins_solver.c` | `ins_update_gps()` | P 对角 ≥ 1e-8 钳位 | 协方差保护 |
| L9 | `fusion_task.c` | `fusion_imu_predict()` | `sum_P > 0 ? sqrt(sum_P) : 0` | sqrt保护 |
| L10 | `ins_solver.c` | `ins_get_attitude()` | R32 clamp [-1,1] | asin域保护 |

### 10.2 GPS 校验失败时的行为

```
GPS帧到达 → gnss_data_parse() 返回 1 (校验失败)
    ↓
L1: parse_ok != 0 → 丢弃帧, g_nav.gps_valid = 0, return
    ↓
ESKF 不更新, 继续纯 IMU 预测 (短时间内漂移可接受)
    ↓
下一帧校验成功 → 正常更新, 滤波器恢复
```

### 10.3 GPS 输出 lat=0.0 时的行为（刚上电未定位）

```
GPS帧: $GNRMC,...,A,0000.0000,N,00000.0000,E,...  ← 状态='A' 但坐标为 0.0
    ↓
L2: gnss.state == 1 → 通过
    ↓
L3: lat=0.0, 不在 [15,55] → 丢弃帧, return
    ↓
滤波器不受影响
```

### 10.4 基准点自动设置

```
首次有效GPS: lat=30.572, lon=104.066 → LAT0=30.572, LON0=104.066, origin_set=1
后续GPS:     lat=30.573, lon=104.067 → x_n=(0.001)*111320=111m, y_e=(0.001)*96300=96m
```

所有 NED 坐标均为相对首次定位的偏移量。每次上电重新设定。

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
| `g_waypoints[]` | `nav_controller.c` | `Waypoint[20]` | 科目1 航点数组 |
| `g_waypoint_count` | `nav_controller.c` | `uint8` | 已记录航点数 |
| `g_current_target` | `nav_controller.c` | `uint8` | 当前追踪航点索引 |
| `g_nav_mode` | `nav_controller.c` | `NavMode` | 导航模式 (0=空闲,1=教学,2=自动,3=完成) |
| `gnss` | Seekfree库 | `gnss_info_struct` | GPS 最新解析数据 |
| `gnss_flag` | Seekfree库 | `uint8` | GPS 新帧到达标志 |
| `gnss.state` | Seekfree库 | `uint8` | GPS 定位有效标志 (1=有效) |
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
| `MeasurementUpdate(error_gps_noise,predP,predErrorState)` | `ins_update_gps(ins, gps_x, gps_y, gps_z)` — 含NaN/Inf/新息门限/P钳位 |
| `QuatNormalize(state(i+1,7:10))` | `quat_normalize(q)` |
| `cnb = quat2cnb(quat)` | `quat_to_rot_matrix(R_nb, q)` |
| `QuatMulitMat(state(i+1,7:10),[1,0.5*...])` | `quat_mul(q_new, q_old, dq)` |
| `acc_noise, gyro_noise, gps_noise` | `g_imu.acc_*, g_imu.gyro_*, fusion_gps_update()` |

---

## 附录 C：nav_controller API 速查

| 函数 | 用途 | 关键参数 |
|------|------|----------|
| `nav_record_waypoint(type)` | 教学记录航点 | type: 0=锥桶, 1=车库入口, 2=停靠点 |
| `nav_compute_errors(&dist, &yaw_err)` | 计算距离和航向误差 | dist(m), yaw_err(rad, [-π,π]) |
| `nav_check_arrival()` | 到达判定+切航点 | 返回0=追踪中, 1=减速区, 2=已到达 |
| `nav_clear_waypoints()` | 重置全部航点 | — |

---

*文档版本: 2.0 · 生成日期: 2026-04-28 · 更新: 新增 nav_controller, NaN防护章节, 基准点自动设置*

