# 电机 PID 控制器 - 比赛工程集成指南

## 文件清单

```
PID_Export/
├── motor_pid.h      # PID 控制器头文件
├── motor_pid.c      # PID 控制器实现
├── encoder.h        # 编码器驱动头文件
├── encoder.c        # 编码器驱动实现 (含中断更新)
├── motor.h          # 电机驱动头文件
├── motor.c          # 电机驱动实现 (DRV8701E)
└── README.md        # 本文档
```

---

## 硬件参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 轮子直径 | 24cm (0.24m) | 驱动轮 |
| 轮周长 | 0.754m | π × 0.24 |
| 编码器 | 1024线 × 4倍频 = 4096脉冲/转 | |
| 减速比 | 2:1 | 编码器转2圈，轮子转1圈 |
| 有效脉冲数 | 8192脉冲/轮子转 | 4096 × 2 |
| 采样周期 | 100ms | 编码器中断10ms，PID计算100ms |

---

## PID 参数

### 空载参数

```c
#define FF_GAIN  10.0f   /* 前馈增益 */
float kp = 8.0f;        /* 比例系数 */
float ki = 0.5f;        /* 积分系数 */
float kd = 0.2f;        /* 微分系数 */
```

**实测性能:**
- 目标250脉冲/100ms → 0.23 m/s (0.83 km/h)
- 启动时间: 0.4-0.5秒
- 稳态误差: ±3-5%
- 超调: 3-5%

### 载人参数

```c
#define FF_GAIN  12.0f   /* 前馈增益 */
float kp = 10.0f;       /* 比例系数 */
float ki = 0.8f;        /* 积分系数 */
float kd = 0.4f;        /* 微分系数 */
```

**实测性能:**
- 目标250脉冲/100ms → 0.23 m/s (0.83 km/h)
- 启动时间: 0.5-1秒
- 稳态误差: ±5-10%
- 超调: 5-10%

---

## 集成步骤

### 1. 复制文件

将以下文件复制到比赛工程的 `user/code/` 目录：
- `motor_pid.h`
- `motor_pid.c`
- `encoder.h`
- `encoder.c`
- `motor.h`
- `motor.c`

### 2. 添加 PIT 中断

在 `isr.c` 中添加编码器更新中断：

```c
/* ---- 编码器中断更新函数 (encoder.c) ---- */
extern void encoder_irq_update(void);

/* PIT Ch2 中断 (CCU61_CH0): 编码器更新 (10ms周期) */
IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH0);
    encoder_irq_update();
}
```

### 3. 初始化

在 `core0_main()` 中添加：

```c
#include "code/motor_pid.h"
#include "code/encoder.h"
#include "code/motor.h"

/* PID 控制器实例 */
MotorPID g_motor_pid;

int core0_main(void)
{
    /* ... 其他初始化 ... */

    /* 电机初始化 */
    motor1_init();
    motor2_init();
    motor1_stop();
    motor2_stop();

    /* 编码器初始化 */
    encoder1_init();
    encoder2_init();

    /* 编码器PIT中断 (10ms) */
    pit_ms_init(CCU61_CH0, 10);

    /* PID初始化 (空载参数) */
    motor_pid_init(&g_motor_pid, 8.0f, 0.5f, 0.2f, 10.0f);

    /* 等待多核同步 */
    cpu_wait_event_ready();

    /* ... 主循环 ... */
}
```

### 4. 主循环调用

```c
/* 主循环 (每10ms执行一次) */
while (1)
{
    system_delay_ms(10);
    tick++;

    /* 累加10ms脉冲到100ms */
    spd_100ms += encoder1_get_speed();

    /* 每100ms执行PID */
    if (tick % 10 == 0)
    {
        /* 设置目标速度 (脉冲数/100ms) */
        motor_pid_set_target(&g_motor_pid, 250.0f);  /* 目标250 = 0.23 m/s */

        /* 执行PID计算 */
        int16 pwm = motor_pid_update(&g_motor_pid, spd_100ms);

        /* 输出到电机 */
        motor1_set_duty(pwm);
        motor2_set_duty(pwm);

        /* 清零累计 */
        spd_100ms = 0;
    }
}
```

---

## 速度对照表

| 目标脉冲 | 速度 m/s | 速度 km/h | 适用场景 |
|----------|----------|-----------|----------|
| 100 | 0.09 | 0.33 | 低速精确控制 |
| 150 | 0.14 | 0.50 | 入库/出库 |
| 200 | 0.18 | 0.67 | 正常行驶 |
| 250 | 0.23 | 0.83 | 快速行驶 |
| 300 | 0.28 | 1.00 | 高速行驶 |
| 400 | 0.37 | 1.33 | 最高速度 |

---

## 调参指南

### 如果速度不稳 (振荡)
1. 减小 `kp` (每次 -2)
2. 增大 `kd` (每次 +0.1)
3. 减小 `ff_gain` (每次 -1)

### 如果响应太慢
1. 增大 `kp` (每次 +2)
2. 增大 `ff_gain` (每次 +1)

### 如果稳态误差大
1. 增大 `ki` (每次 +0.2)
2. 增大 `ff_gain` (每次 +1)

### 如果超调大
1. 增大 `kd` (每次 +0.1)
2. 减小 `kp` (每次 -1)

---

## 安全限制

| 参数 | 值 | 说明 |
|------|-----|------|
| PWM硬限幅 | 9500 (95%) | 绝对不能超过 |
| PWM变化率 | 1000/周期 | 防止PWM突变 |
| 积分限幅 | ±2000 | 防止积分饱和 |
| 积分分离阈值 | 60 | 误差>60时不积分 |

---

## 注意事项

1. **编码器方向**: 如果电机反转，速度会是负数，需要在 `encoder.c` 中调整符号
2. **减速比**: 根据实际车模调整 `GEAR_RATIO`
3. **轮子直径**: 根据实际车模调整 `WHEEL_DIAMETER_M`
4. **PID参数**: 首次使用建议用空载参数测试，再根据载人情况调整

---

## 联系方式

如有问题，请联系原作者。
