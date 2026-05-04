# 科目1 自动驾驶 — INS 融合定位使用指南

## 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                        科目1 自动驾驶                         │
│                                                             │
│  ┌──────────┐   ┌──────────┐   ┌──────────────────────────┐ │
│  │ IMU963RA │   │ GPS模块   │   │   nav_controller (NEW)   │ │
│  │ (100Hz)  │   │ (10Hz)   │   │   航点记录 + 导航追踪     │ │
│  └────┬─────┘   └────┬─────┘   └────────────┬─────────────┘ │
│       │              │                      │               │
│       └──────┬───────┘                      │               │
│              ↓                              │               │
│  ┌──────────────────────┐                   │               │
│  │    INS Fusion ESKF   │──→ g_nav ─────────┘               │
│  │  (融合定位, 100Hz)    │   x, y, z, yaw, pos_std         │
│  └──────────────────────┘                   │               │
│                                             ↓               │
│                                 ┌──────────────────────┐    │
│                                 │   PID 控制器          │    │
│                                 │   速度PID: dist → PWM │    │
│                                 │   转向PID: yaw_err →  │    │
│                                 │          舵机角度     │    │
│                                 └──────────┬───────────┘    │
│                                            ↓                │
│                                    ┌──────────────┐         │
│                                    │  电机 + 舵机  │         │
│                                    └──────────────┘         │
└─────────────────────────────────────────────────────────────┘
```

## 比赛流程分两步

### 阶段1: 教学 (Teaching) — 记录航点

你**手动推车或遥控**绕完全部锥桶 + 入库，在每个关键位置**按下按键**记录航点。

```
发车区 ──→ 锥桶1 ──→ 锥桶2 ──→ ... ──→ 锥桶N ──→ 车库入口 ──→ 库内停靠点
  K2         K2        K2              K2         K2            K2
```

**操作步骤：**

1. 上电，等待 GPS 定位（串口出现 `NAV: pos=(0.00, 0.00 ...)`  说明基准点已锁定）
2. 把车放到发车区，按 **K1** 进入教学模式
3. 按 **K2** → 记录**起点** (type=0)
4. 推车/遥控到第1个锥桶旁，按 **K2** → 记录锥桶位置
5. 重复...依次记录所有锥桶
6. 到达**车库入口** (门前1米)，按 **K3** → 记录为车库入口 (type=1)
7. 进入车库**停稳位置**，按 **K4** → 记录为最终停靠点 (type=2)
8. 按 **K1** → 退出教学，打印所有航点供检查

记录时串口会输出每个航点坐标，确认所有航点坐标合理。

### 阶段2: 自动运行 (Autonomous) — 航点追踪

把车放回发车区（对准初始方向），按 **K1** 开始自动运行。

**车会自动：**
1. 读取第1个航点 → 计算距离和航向误差 → 驱动电机和舵机追踪
2. 到达第1个航点（距离 < arrival_radius）→ 自动切换到第2个航点
3. 到达车库入口 → 减速，用记录的 yaw 角对齐入库方向
4. 到达停靠点 → 停车

## 核心导航逻辑

```
每一个控制周期 (100Hz, 与 IMU 同步):

  ┌──────────────────────────────────────────────┐
  │ 1. IMU 读取 + ESKF 预测                       │
  │    更新 g_nav.x, g_nav.y, g_nav.yaw          │
  ├──────────────────────────────────────────────┤
  │ 2. nav_compute_errors(&dist, &yaw_err)       │
  │    dist = 当前位置到目标航点的距离              │
  │    yaw_err = (期望航向 - 当前航向), [-π, π]   │
  ├──────────────────────────────────────────────┤
  │ 3. PID 计算                                   │
  │    speed_cmd = speed_pid(dist)                │
  │    steer_cmd = steer_pid(yaw_err)             │
  │    // 越远离目标越快, 越接近越慢                │
  │    // 航向误差越大转向越猛                      │
  ├──────────────────────────────────────────────┤
  │ 4. 到达判定                                   │
  │    if (dist < wp.arrival_radius) → 下一航点   │
  └──────────────────────────────────────────────┘
```

## 关键参数调整

| 参数 | 位置 | 默认值 | 调整建议 |
|------|------|--------|----------|
| 普通到达半径 | `nav_controller.c` | 0.8m | 锥桶间距大则加大，密集则减小 |
| 车库到达半径 | `nav_controller.c` | 0.3m | 车库宽1.5m, 越小越精确但不能太小 |
| 减速距离 | `nav_controller.c` `SLOW_DOWN_DISTANCE` | 2.0m | 车速快则加大 |
| 最大车速 | PID 代码 | — | 规则限速 25km/h, 建议 ≤ 3m/s |
| 车库进场车速 | PID 代码 | — | 建议 ≤ 0.5m/s |

## GPS有效性检查

自动运行过程中，如果 GPS 短暂丢星（操场开阔一般不会，但万一）：

```c
/* 在 PID 控制循环中加入: */
if (g_nav.pos_std > 3.0f) {
    /* 位置不确定度 > 3m, GPS 可能丢星太久, 减速停车等待恢复 */
    speed_cmd = 0.0f;
}
```

`g_nav.pos_std` 是 ESKF 对自身精度的估计：
- `< 0.5m` → GPS 良好，正常导航
- `0.5 ~ 2m` → GPS 精度下降，减速行驶
- `> 3m` → 纯 IMU 推算已不可靠，停车等待

## PID 控制器伪代码

你需要根据实际车辆动力学调整 PID 参数。以下是一个起点框架：

```c
/* 速度控制: 距离 → PWM 占空比 */
float speed_control(float dist_to_target) {
    float max_speed = 2.0f;          // 最大速度 m/s
    float min_speed = 0.3f;          // 最低速度 (防止停不下来)
    float slow_zone = 2.0f;          // 减速区

    float target_speed;
    if (dist_to_target > slow_zone) {
        target_speed = max_speed;
    } else {
        // 距离越近速度越低
        target_speed = min_speed + (max_speed - min_speed) * (dist_to_target / slow_zone);
    }

    // PID 计算 (略, 根据你的编码器反馈实现)
    return target_speed;
}

/* 转向控制: 航向误差 → 舵机角度 */
float steer_control(float yaw_error) {
    // yaw_error: [-π, π], 正值=偏右, 负值=偏左
    float kp = 0.5f;                 // 比例系数, 根据实际调
    float max_steer = 30.0f;         // 最大舵角 (度)

    float steer = kp * yaw_error * 180.0f / M_PI;  // rad → deg

    if (steer >  max_steer) steer =  max_steer;
    if (steer < -max_steer) steer = -max_steer;

    return steer;
}
```

## 文件修改清单

在原工程基础上新增/修改：

```
INS_Fusion_Project/
├── code/
│   ├── nav_controller.h/c       ← 新增: 航点导航控制器
│   ├── [ins/, imu_task, fusion_task 已有]
│   └── pid_controller.h/c       ← 你需要自己实现 (根据你的电机/舵机)
├── user/
│   └── cpu0_main.c              ← 修改: 加入按键→模式切换→导航循环
└── doc/
    └── Subject1_Usage_Guide.md  ← 本文件
```

### cpu0_main.c 需要加入的逻辑:

```c
/* 主循环 while(TRUE) 内部: */

/* --- 按键处理 --- */
if (ButtonPushed(KEY1)) {
    // K1: 模式切换
    if (g_nav_mode == NAV_IDLE || g_nav_mode == NAV_FINISHED) {
        g_nav_mode = NAV_TEACHING;      // 进入教学模式
        nav_clear_waypoints();
    } else if (g_nav_mode == NAV_TEACHING) {
        g_nav_mode = NAV_RUNNING;       // 教学结束, 开始自动运行
        g_current_target = 0;
    }
    system_delay_ms(300);  // 防抖
}

if (g_nav_mode == NAV_TEACHING) {
    if (ButtonPushed(KEY2)) {
        nav_record_waypoint(0);         // 记录普通航点
        system_delay_ms(300);
    }
    if (ButtonPushed(KEY3)) {
        nav_record_waypoint(1);         // 记录车库入口
        system_delay_ms(300);
    }
    if (ButtonPushed(KEY4)) {
        nav_record_waypoint(2);         // 记录库内停靠点
        system_delay_ms(300);
    }
}

/* --- IMU 预测 (100Hz) --- */
if (g_imu_tick) {
    g_imu_tick = 0;
    imu_task_read();
    fusion_imu_predict();

    /* 自动模式下运行导航 */
    if (g_nav_mode == NAV_RUNNING) {
        float dist, yaw_err;
        nav_compute_errors(&dist, &yaw_err);

        uint8 arrival = nav_check_arrival();
        if (arrival == 2 && g_nav_mode == NAV_FINISHED) {
            motor_stop();               // 到达终点, 停车
        } else {
            float speed = speed_control(dist);
            float steer = steer_control(yaw_err);
            motor_set_speed(speed);     // 你的电机驱动函数
            servo_set_angle(steer);     // 你的舵机驱动函数
        }
    }
}

/* --- GPS 更新 --- */
if (gnss_flag) {
    fusion_gps_update();
}

/* --- 调试输出 (1Hz) --- */
if (g_debug_tick) {
    // ... 原有输出 ...
    if (g_nav_mode == NAV_RUNNING) {
        float dist, yaw_err;
        nav_compute_errors(&dist, &yaw_err);
        uart_write_printf(DEBUG_UART_INDEX,
            "WP[%d/%d] dist=%.2f yaw_err=%.1f\n",
            g_current_target, g_waypoint_count, dist, yaw_err * 180.0f / M_PI);
    }
}
```

## 坐标系说明

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

## 车库入库策略

车库入口宽 1.5m，深 2m。入库方案：

```
车库入口────→ 库内停靠点
   ●              ●
  (type=1)      (type=2)

教学阶段:
  记录车库入口时, 车头对准车库门方向 → target_yaw 被记录
  记录停靠点时, 车在库内最终位置 → x,y 被记录

自动阶段:
  到达车库入口(type=1)航点时:
    - 期望航向 = target_yaw (记录的入库方向, 不是车指向目标)
    - 车减速到最低速度
    - 航向对准后直行入库
  到达停靠点(type=2)航点时:
    - 速度归零 → 停车 → 科目完成
```

### 纯 GPS 和 INS 融合在入库时的区别

```
纯 GPS (10Hz, 噪声大):          INS 融合 (100Hz, 平滑):
  位置抖动 ±0.5~1m                位置平滑, 航向精确
  入库时可能撞门框                可以精确对准车库中线

  GPS轨迹: ╱╲╱╲╱╲              融合轨迹: ────────
          车库 ╲╱╲╱╲╱                   车库 │
                                               │
```

这就是为什么 INS 融合对科目1有价值——**入库阶段 GPS 噪声 (0.5~1m) 对于 1.5m 的门来说太大了**，而融合后的位置和航向更平滑精确。

## 常见问题

**Q: 为什么融合后的位置和纯GPS位置有一个固定的偏移？**
A: GPS→NED 变换的基准点是首次收到的有效 GPS 坐标。航点也是在同一个 NED 坐标系下记录的，所以偏移不影响导航(所有坐标都是相对的)。每次上电基准点会重新设置。

**Q: 自动运行时车跑偏了怎么办？**
A: 
1. 检查 `yaw_error` 串口输出是否合理（车偏左时 yaw_error 应该是正的）
2. 调整转向 PID 的 Kp
3. 如果航向角漂移严重（>几度/分钟），说明陀螺 z 轴零偏标定不准 → 在 `imu_task_calibrate(300)` 标定更长时间

**Q: 入库时方向对不准怎么办？**
A: 车库入口航点 (type=1) 和停靠点 (type=2) 的 `target_yaw` 是记录时车头的朝向。确保教学记录时车头**精确对准车库中线**。
