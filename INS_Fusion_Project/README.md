# INS_Fusion_Project — GPS+IMU ESKF 融合定位工程

基于 Seekfree TC264 开源库，移植 MATLAB ESKF 算法到嵌入式平台。

## 工程结构

```
INS_Fusion_Project/
├── user/
│   ├── cpu0_main.c           ← 主入口: 初始化、主循环调度
│   └── isr.c                 ← 中断服务: PIT(IMU预测) + UART(GPS接收)
├── code/
│   ├── ins/
│   │   ├── math_utils.h/c    ← 嵌入式矩阵/向量运算库 (3x3, 4x4, 15x15)
│   │   └── ins_solver.h/c    ← ESKF 卡尔曼滤波核心 (MATLAB移植)
│   ├── imu_task.h/c          ← IMU963RA 驱动封装 + 数据采集 + 零偏标定
│   └── fusion_task.h/c       ← 融合调度: GPS→NED转换, 预测/更新协调
└── doc/
    └── README.md             ← 本文件
```

## 算法架构

```
┌──────────────────────────────────────────────────┐
│                   主循环 (1ms)                     │
│                                                   │
│  PIT中断(10ms)                UART3中断             │
│  ┌─────────────────┐       ┌──────────────┐       │
│  │ imu_task_read()  │       │ GNSS NMEA    │       │
│  │ IMU963RA SPI读取  │       │ $GNRMC,$GNGGA│       │
│  └────────┬────────┘       └──────┬───────┘       │
│           ↓                       ↓               │
│  ┌─────────────────┐       ┌──────────────┐       │
│  │ ins_predict()    │       │ gnss_parse() │       │
│  │ 姿态更新(via四元数)│      │ lat,lon,alt  │       │
│  │ 速度积分+重力补偿  │       │ speed,dir    │       │
│  │ 位置积分          │       └──────┬───────┘       │
│  │ 协方差预测P=FdPFd'│             ↓               │
│  └────────┬────────┘       ┌──────────────┐       │
│           │                │ ins_update_gps│       │
│           │                │ 位置误差→卡尔曼│       │
│           │                │ 修正全状态+协方差│      │
│           └────────┬───────┘               │       │
│                    ↓                       │       │
│           ┌────────────────┐               │       │
│           │  g_nav 导航输出 │               │       │
│           │  x,y,z,yaw,std │               │       │
│           └────────────────┘               │       │
└──────────────────────────────────────────────────┘
```

## ESKF 状态定义

| 全状态 (16维) | 含义 | 误差状态 (15维) | 含义 |
|---|---|---|---|
| state[0:2] | 位置 p (NED) | dx[0:2] | δp |
| state[3:5] | 速度 v (NED) | dx[3:5] | δv |
| state[6:9] | 四元数 q | dx[6:8] | δθ (轴角) |
| state[10:12] | 加计零偏 ba | dx[9:11] | δba |
| state[13:15] | 陀螺零偏 bw | dx[12:14] | δbw |

## 集成到 AURIX Development Studio

### 步骤1: 复制 Seekfree 模板

```
1. 找到安装目录下的模板工程:
   Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/
   复制整个 Seekfree_TC264_Opensource_Library 文件夹到工作目录

2. 重命名为 ins_fusion (或任意工程名)
```

### 步骤2: 添加本工程文件

```
将 INS_Fusion_Project/ 中的文件合并到 ins_fusion/ 对应目录:

ins_fusion/
├── user/
│   ├── cpu0_main.c           ← 用本工程的替换
│   └── isr.c                 ← 用本工程的替换
├── code/
│   ├── ins/                  ← 新增: 复制 ins/ 整个目录
│   ├── imu_task.h/c          ← 新增
│   ├── fusion_task.h/c       ← 新增
│   └── ComOutput.h/c         ← 保留原有的(如需)
└── libraries/
    └── zf_device/            ← 保留: 包含 imu963ra 和 gnss 驱动
```

### 步骤3: AURIX Development Studio 配置

```
1. 打开 ADS, File → Import → Existing Projects
2. 选择 ins_fusion 目录
3. 右键工程 → Properties → C/C++ Build → Settings
   - 添加 include 路径: "${ProjDirPath}/code/ins"
                         "${ProjDirPath}/code"
   - 确认 "libraries/zf_device" 已在 include 路径中
4. 确认工程设置 → CPU: TC264D, 编译器: TASKING TriCore
```

### 步骤4: 调整硬件引脚

根据实际接线修改以下宏定义 (在对应驱动头文件中):

| 引脚 | 宏 | 位置 |
|------|----|------|
| IMU963RA SPI SCK | IMU963RA_SPC_PIN | zf_device_imu963ra.h |
| IMU963RA SPI MOSI | IMU963RA_SDI_PIN | zf_device_imu963ra.h |
| IMU963RA SPI MISO | IMU963RA_SDO_PIN | zf_device_imu963ra.h |
| IMU963RA CS | IMU963RA_CS_PIN | zf_device_imu963ra.h |
| GPS UART RX | GNSS_RX | zf_device_gnss.h |
| GPS UART TX | GNSS_TX | zf_device_gnss.h |

默认配置: IMU963RA 使用 SPI0 (P20_11 SCK, P20_14 MOSI, P20_12 MISO, P20_13 CS)
          GPS 使用 UART3 (P15_6 RX, P15_7 TX)

### 步骤5: 标定参数

编译下载后, 需要通过调试串口和实车测试调整以下参数:

#### a) IMU 零偏标定
```c
// 在 cpu0_main.c 的初始化阶段调用:
imu_task_calibrate(100);  // 静止采集100帧, 自动计算零偏均值
```

#### b) GPS 基准点设置
```c
// 在 fusion_task.c 中修改基准点经纬度:
static const double LAT0 = 30.xxxxx;   // 赛场发车区纬度
static const double LON0 = 104.xxxxx;  // 赛场发车区经度
```

#### c) 噪声协方差调参
```c
// 在 fusion_task.c 的 g_Qc_diag[] 和 g_Rc_diag[] 中调整
// 加大 Qc → 更信任测量值
// 加大 Rc → 更信任模型预测
```

## 数据流示意

```
IMU963RA (SPI, 100Hz)                GPS (UART3, 10Hz)
     │                                     │
     ├─ acc_x, acc_y, acc_z (g)           ├─ $GNRMC → lat, lon, speed, dir
     ├─ gyro_x, gyro_y, gyro_z (deg/s)    ├─ $GNGGA → alt, satellites
     └─ mag_x, mag_y, mag_z (Gauss)       └─ $GNTHS → antenna_heading
           │                                     │
           ↓                                     ↓
     imu_task_read()                      gnss_uart_callback()
     (PIT中断 10ms)                       (UART3 RX中断)
           │                                     │
           ↓                                     ↓
     ins_predict()                        gnss_data_parse()
     - 欧拉积分                           - NMEA解析
     - 四元数更新                          - 校验和
     - P = Fd*P*Fd' + Qd                         │
           │                                     ↓
           │                              fusion_gps_update()
           │                              - lat/lon → NED x,y,z
           │                              - ins_update_gps()
           │                              - 卡尔曼增益 K
           │                              - 全状态修正
           │                              - P = (I-KH)*P
           └──────────────┬──────────────────┘
                          ↓
                    g_nav (融合后位置/速度/姿态)
                          │
                          ↓
                    控制算法 (PID/MPC)
```

## 调试方法

### 串口输出格式
```
NAV: pos=(12.34, -5.67, 0.12) yaw=45.3deg std=0.45m
```

### 关键变量监控 (通过调试器或上位机)

| 变量 | 含义 | 期望值 |
|------|------|--------|
| g_nav.x, g_nav.y | NED位置 | 与实际轨迹一致 |
| g_nav.yaw (deg) | 航向角 | 0=北, 90=东 |
| g_nav.pos_std | 位置不确定度 | 收敛后应 < 0.5m |
| g_imu.acc_z | Z轴加速度 | 静止时 ≈ -9.8 m/s² |
| gnss.state | GPS定位状态 | 1=有效, 0=无效 |
| gnss.satellite_used | 可见卫星数 | 室外 > 6 |

## MATLAB 对照表

| MATLAB 函数/变量 | C 对应 |
|---|---|
| `main.m` 仿真数据生成 | 不需要 — 使用真实传感器 |
| `InsSolver.imu2state()` | `fusion_imu_predict()` + `fusion_gps_update()` |
| `InsSolver.statePrediction()` | `ins_predict()` — 用欧拉积分代替ODE45 |
| `InsSolver.MeasurementUpdate()` | `ins_update_gps()` |
| `InsSolver.getExponentMatFd()` | `build_Fd()` |
| `AttitudeBase.quat2cnb()` | `quat_to_rot_matrix()` |
| `AttitudeBase.QuatMulitMat()` | `quat_mul()` |
| `state(7:10)` 四元数 | `ins->state[6:9]` |
| `Qc0, Rc0` | `g_Qc_diag[], g_Rc_diag[]` |

## 性能预估 (TC264 @ 200MHz)

| 运算 | 耗时 | 频率 |
|------|------|------|
| IMU SPI读取 | ~50us | 100Hz |
| ins_predict() (含Fd构建) | ~200us | 100Hz |
| ins_update_gps() (3x3求逆) | ~100us | 10Hz |
| 总CPU负载 | < 3% | — |

## 传感器型号

| 传感器 | 型号 | 接口 | 驱动文件 |
|--------|------|------|----------|
| 9轴IMU | IMU963RA (LSM6DSO + LIS2MDL) | SPI (默认) / IIC | zf_device_imu963ra.h/c |
| GPS | TAU1201 / GN42A 双频模块 | UART3, 115200bps | zf_device_gnss.h/c |

## 许可证

- Seekfree 库部分: GPL 3.0 (见 libraries/doc/LICENSE)
- INS 算法移植部分: 基于 MATLAB 仿真工程, 仅供学习和比赛使用
