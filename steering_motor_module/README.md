# 转向电机角度控制模块 (Steering Angle Control Module)

## 概述

本模块实现基于角度控制的**转向电机 TC264D 单片机**的转向角度控制模块。

### 控制系统架构

```
UART 输入 (或未来 GPS 航向偏差)
        |
        v
angle_control_set_target()  -->  target_angle (限制在 ±60°)
        |
        v  (每 10ms PIT 中断)
angle_control_update()
    |
    +--> encoder_get_accumulated_count()  (TIM4 编码器, 溢出累积)
    +--> 角度 = count * 360 / (1024 * 300)
    +--> PID_Compute(target_angle, current_angle)
    +--> 死区 (|error| < 0.1° => output = 0)
    +--> angle_motor_set_pwm(pid_output)
            |
            +--> ATOM0_CH7_P02_7 (forward) / ATOM0_CH6_P02_6 (reverse)
            +--> H 桥驱动直流电机 + 300:1 减速箱
```

## 文件列表

| 文件 | 说明 |
|------|------|
| `angle_control.h` | 转向角度控制头文件，包含引脚定义及宏定义 |
| `angle_control.c` | 转向角度控制实现，包含 PID 计算、PWM 输出 |
| `PID.h` | 位置式 PID 控制算法头文件 |
| `PID.c` | 位置式 PID 控制算法实现 |
| `integration_isr_snippet.c` | 中断服务代码插件示例 |
| `integration_main_snippet.c` | 主函数代码插件示例 |

## 硬件配置

### PWM 引脚

| 信号 | 宏 | 物理引脚 | GTM 通道 |
|------|------|------|---------|
| Forward PWM | `ANGLE_PWM_IN1` | P02_7 | ATOM0_CH7 |
| Reverse PWM | `ANGLE_PWM_IN2` | P02_6 | ATOM0_CH6 |

- PWM 频率: **10,000 Hz** (10 kHz)
- PWM 占空比范围: 0 ~ 10000 (对应 0% ~ 100%)
- H 桥方向控制: IN1 高 + IN2 低 = 正转; IN1 低 + IN2 高 = 反转; 均低为刵车

### 编码器引脚

| 信号 | 宏 | 物理引脚 | GPT12 定时器 |
|------|------|------|-----------|
| Encoder A (CH1) | `ANGLE_ENCODER_A_PIN` | P02_8 | TIM4 CH1 |
| Encoder B (CH2) | `ANGLE_ENCODER_B_PIN` | P00_9 | TIM4 CH2 |

- 编码器模式: **四倍频模式** (`encoder_quad_init`)，双边沿计数
- 硬件 PPR: 1024 (quad 编码器驱动已除4，有效计数 256/转)
- 减速比: **300:1** (电机轴到末端输出)
- 每脉冲角度: `360.0 / (1024 * 300)` ≈ 0.00117 度/脉冲

### 控制循环时序

- PIT 中断: CCU6_0 通道 0, 周期 **10ms** (100 Hz 控制频率)
- 中断优先级: 30, 由 CPU0 服务

## 控制参数

| 参数 | 值 | 说明 |
|------|------|------|
| `ANGLE_PPR` | 1024 | 编码器每转脉冲数 (硬件) |
| `ANGLE_GEAR_RATIO` | 300 | 减速比 |
| `ANGLE_MAX_DEGREE` | 60 | 最大目标角度 (度) |
| `ANGLE_MIN_DEGREE` | -60 | 最小目标角度 (度) |
| `ANGLE_DEFAULT_KP` | 200.0f | 比例系数 Kp |
| `ANGLE_DEFAULT_KI` | 4.0f | 积分系数 Ki |
| `ANGLE_DEFAULT_KD` | 10.0f | 微分系数 Kd |
| `ANGLE_OUTPUT_MAX` | 10000 | PWM 最大占空比 |
| `ANGLE_DEAD_BAND` | 0.1f | 死区 (度) |
| `IntegralMax` | 2000 | 积分限幅 (PID_Init 设置) |

## API 接口

| 函数 | 说明 |
|------|------|
| `angle_control_init()` | 初始化角度控制模块，配置 PWM、编码器、PID |
| `angle_control_update()` | 控制器函数，每 10ms PIT 中断调用一次 |
| `angle_control_set_target(int32)` | 设置目标角度，自动限制在 ±60° |
| `angle_control_rotate_relative(int32)` | 在当前角度基础上旋转指定角度 |
| `angle_motor_set_pwm(int32)` | 设置电机 PWM 占空比 |
| `angle_control_get_current_angle()` | 获取当前角度 |
| `angle_control_reset()` | 重置 PID 状态，关闭电机 |
| `PID_Init(...)` | 初始化 PID 结构体 |
| `PID_Compute(...)` | 计算 PID 输出 |
| `PID_Reset(...)` | 重置 PID 状态 |

## 集成指南

### 1. 复制文件

将 `angle_control.h`, `angle_control.c`, `PID.h`, `PID.c` 复制到目标工程的 `code/` 目录

### 2. 添加头文件引用

在总头文件 `zf_common_headfile.h` 中添加:

```c
#include "PID.h"
#include "angle_control.h"
```

### 3. 插入中断服务 (isr.c)

在 `isr.c` 头部添加:

```c
#include "angle_control.h"
```

在 CCU6_0 通道0 中断服务函数中调用:

```c
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    static uint8 in_update = 0;
    if (in_update) return;
    in_update = 1;
    pit_clear_flag(CCU60_CH0);
    angle_control_update();
    in_update = 0;
}
```

### 4. 初始化代码 (cpu0_main.c)

```c
#include "angle_control.h"

int core0_main(void)
{
    clock_init();
    debug_init();

    // 转向控制初始化
    angle_control_init();
    angle_control_set_target(0);

    // 10ms 定时中断 (100Hz)
    pit_init(CCU60_CH0, 10000);

    cpu_wait_event_ready();

    while (TRUE)
    {
        // 修改 target 即可控制转向
        // angle_control_set_target(new_target);

        system_delay_ms(10);
    }
}
```

### 5. 中断优先级 (isr_config.h)

确保定义了 CCU6_0 通道0 中断优先级:

```c
#define CCU6_0_CH0_INT_SERVICE  IfxSrc_Tos_cpu0
#define CCU6_0_CH0_ISR_PRIORITY 30
```

## 依赖关系

| 模块 | 依赖 |
|------|------|
| angle_control.c | PID.h/PID.c (PID 算法) |
| angle_control.c | zf_driver_pwm.h (PWM 驱动) |
| angle_control.c | zf_driver_encoder.h (编码器驱动) |
| angle_control.c | zf_common_headfile.h (类型和定义) |
| isr.c | angle_control.h (PIT 中断调用) |
| cpu0_main.c | angle_control.h (初始化和目标设置) |

## 注意事项

- 转向电机是**直流电机** (H 桥驱动)，不是船机舵机
- 本模块共享 `PID.c` 模块，但本身使用 **TIM4** 编码器和 **ATOM0_CH6/CH7** PWM，与驱动电机 (`motor.c`) 不冲突
- 如需修改硬件配置，修改 `angle_control.h` 中的宏定义即可
- 通过串口发送数字 + 回车可设置目标角度，例如: `10\n` 设置 10°
