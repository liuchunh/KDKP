/*********************************************************************************************************************
 * 文件名称: cpu0_main.c
 * 功能描述: 第21届智能车竞赛 - 卡丁快跑组 - 科目1 自动驾驶 主控制器 (CPU0 核)
 *
 * 整体思路:
 *   采用"教学-回放"方案完成科目1:
 *     阶段1 (教学模式): 手动推车或遥控，依次经过每个锥桶和车库，
 *                       在关键位置按下按键记录 GPS 坐标作为航点。
 *     阶段2 (自动模式): 车模从发车区自动出发，依次导航到每个航点，
 *                       绕完所有锥桶后减速进入车库，完成倒车入库停车。
 *
 * 核心算法:
 *   1. ESKF (误差状态卡尔曼滤波) 融合 IMU963RA + GPS，输出 NED 坐标和航向角
 *   2. 航点导航控制器 (nav_controller) 计算距离误差和航向误差
 *   3. 双环 PID 控制: 速度环(距离→PWM) + 转向环(航向误差→舵机角度)
 *   4. 可选: 1D 卡尔曼滤波平滑 ToF 测距数据
 *
 * 硬件平台: TC264D (英飞凌 TriCore, Seekfree 逐飞开源库)
 * 传感器:   IMU963RA (SPI, 100Hz) + GNSS GN42A (UART3, 10Hz)
 * 执行器:   直流电机 (PWM驱动) + 舵机 (PWM 50Hz)
 *
 * 任务调度:
 *   PIT Ch0 (100ms): 调试输出定时标志
 *   PIT Ch1 (10ms):  IMU 数据采集定时标志 (100Hz)
 *   主循环 (约1ms):  IMU预测 + GPS更新 + 导航PID控制 + 按键处理
 *
 * 修改记录:
 *   2026-05-02  基于 INS_Fusion_Project 整合, 新增教学/自动模式、PID控制、按键逻辑
 ********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "zf_device_gnss.h"
#include "zf_device_imu963ra.h"

/* ---- INS 融合模块头文件 (位于 user/code/ 子目录) ---- */
#include "code/ins/ins_solver.h"      /* ESKF 误差状态卡尔曼滤波核心 */
#include "code/imu_task.h"           /* IMU963RA 数据采集 + 零偏标定 */
#include "code/fusion_task.h"        /* 融合调度层 (IMU预测 + GPS更新) */
#include "code/nav_controller.h"     /* 航点导航控制器 (教学记录 + 距离/航向误差) */
#include "code/pid_controller.h"     /* PID 控制器 (位置式/增量式) */
#include "code/kalman_filter.h"      /* 1D/2D 卡尔曼滤波器 (ToF平滑等) */
#include "code/angle_control.h"

/* =====================================================================
 *  宏定义 — 硬件配置 (根据你的实际接线修改!)
 * ===================================================================== */

/* PIT 定时器通道分配 */
#define IMU_PIT_CHANNEL     (CCU60_CH1)         /* PIT 通道1: 10ms 周期, 用于 IMU 采样 */
#define DEBUG_PIT_CHANNEL   (CCU60_CH0)         /* PIT 通道0: 100ms 周期, 用于调试输出 */

/* 按键引脚定义 (低电平有效, 上拉输入) */
#define KEY1  (P20_6)           /* K1: 教学/自动模式切换 */
#define KEY2  (P20_7)           /* K2: 教学模式下记录锥桶航点 */
#define KEY3  (P11_2)           /* K3: 教学模式下记录车库入口航点 */
#define KEY4  (P11_3)           /* K4: 教学模式下记录车库停靠点 */

/* 电机 PWM 配置 (根据驱动板和接线修改) */
#define MOTOR_PWM_CH        (ATOM0_CH0_P21_2)   /* 电机 PWM 输出引脚 */
#define MOTOR_PWM_FREQ      (10000)              /* 电机 PWM 频率 10kHz (BTN7971B 驱动板) */
#define MOTOR_MAX_DUTY      (8000)               /* 电机最大占空比 (PWM周期为8000) */

/* 舵机 PWM 配置 */
#define SERVO_PWM_CH        (ATOM0_CH1_P21_3)   /* 舵机 PWM 输出引脚 */
#define SERVO_PWM_FREQ      (50)                 /* 舵机 PWM 频率 50Hz (标准舵机) */
#define SERVO_CENTER_US     (750)                /* 舵机中位脉宽 (微秒), 需实测! 常见值 700~800 */
#define SERVO_RANGE_US      (250)                /* 舵机左右最大偏移 (微秒), 即中位±250us */

/* =====================================================================
 *  宏定义 — 导航参数 (根据赛道和车模调试)
 * ===================================================================== */

#define MAX_SPEED_MPS       (2.5f)      /* 最大车速 m/s (约9km/h, 规则限速25km/h, 建议不超过3m/s) */
#define MIN_SPEED_MPS       (0.3f)      /* 最低车速 m/s (防止低速时电机堵转停不下来) */
#define GARAGE_SPEED_MPS    (0.5f)      /* 入库阶段车速 m/s (慢速精确入库) */
#define SLOW_DOWN_DIST      (2.0f)      /* 开始减速的距离阈值 (m) */
#define GPS_STD_WARN        (1.0f)      /* GPS 精度警告阈值 (m), 超过此值减速 */
#define GPS_STD_STOP        (3.0f)      /* GPS 精度停车阈值 (m), 超过此值停车等待 */

/* ToF 卡尔曼滤波参数 (可选, 用于平滑 ToF 测距噪声) */
#define TOF_KF_Q            (0.5f)      /* 过程噪声: 越大滤波器越相信测量值 (响应快, 平滑差) */
#define TOF_KF_R            (2.0f)      /* 测量噪声: 越大滤波器越相信预测值 (响应慢, 平滑好) */

/* =====================================================================
 *  全局变量 — ISR 与主循环之间通信用
 * ===================================================================== */

//typedef unsigned char uint8;
//typedef unsigned short uint16;
//typedef unsigned long uint32;

volatile uint8 g_imu_tick   = 0;        /* PIT Ch1 中断置1, 主循环消费后清0. 表示10ms到了 */
volatile uint8 g_debug_tick = 0;        /* PIT Ch0 中断置1, 主循环消费后清0. 表示100ms到了 */

/* =====================================================================
 *  模块级变量
 * ===================================================================== */

/* ---- PID 控制器实例 (速度环 + 转向环) ---- */
static PidParams g_speed_pid_params;    /* 速度 PID 参数 */
static PidState  g_speed_pid_state;     /* 速度 PID 状态 (积分、上次误差等) */
static PidParams g_steer_pid_params;    /* 转向 PID 参数 */
static PidState  g_steer_pid_state;     /* 转向 PID 状态 */

/* ---- ToF 卡尔曼滤波器 (可选) ---- */
static KF1D_State  g_tof_kf_state;     /* 1D KF 状态 */
static KF1D_Params g_tof_kf_params;    /* 1D KF 参数 */

#pragma section all "cpu0_dsram"        /* 以下变量放入 CPU0 的 DSRAM 中, 访问速度快 */

/* ---- 累计误差统计变量 ---- */
static float g_total_dist   = 0.0f;    /* 累计行驶距离 (m) */
static float g_prev_x       = 0.0f;    /* 上次位置 X */
static float g_prev_y       = 0.0f;    /* 上次位置 Y */
static uint8 g_dist_init    = 0;       /* 距离计算已初始化 */

/* =====================================================================
 *  电机/舵机 底层驱动函数
 *  ★★★ 你需要根据实际硬件接线和驱动板修改这些函数! ★★★
 * ===================================================================== */

/**
 * motor_servo_init() - 初始化电机和舵机的 PWM 输出
 *
 * 舵机: 50Hz 标准 PWM, 中位脉宽 SERVO_CENTER_US 微秒
 * 电机: 10kHz PWM (BTN7971B 驱动板), 初始占空比 0 (停止)
 *
 * 调用时机: core0_main() 初始化阶段调用一次
 */
static void motor_servo_init(void)
{
    /* 舵机初始化: 50Hz, 初始中位 */
    pwm_init(SERVO_PWM_CH, SERVO_PWM_FREQ, SERVO_CENTER_US);

    /* 电机初始化: 10kHz, 初始停止 (占空比0) */
    pwm_init(MOTOR_PWM_CH, MOTOR_PWM_FREQ, 0);

    /* 确保初始状态安全 */
    pwm_set_duty(SERVO_PWM_CH, SERVO_CENTER_US);
    pwm_set_duty(MOTOR_PWM_CH, 0);
}

/**
 * motor_set_speed() - 设置电机转速
 * @speed_mps: 目标速度 (m/s), 正值前进, <=0 停车
 *
 * 实现原理: 将目标速度线性映射到 PWM 占空比
 *   占空比 = (speed_mps / MAX_SPEED_MPS) * MOTOR_MAX_DUTY
 *
 * 注意: 这是开环控制. 如有编码器可改为闭环 (编码器反馈→PID→PWM)
 */
static void motor_set_speed(float speed_mps)
{
    if (speed_mps <= 0.0f) {
        pwm_set_duty(MOTOR_PWM_CH, 0);         /* 停车 */
        return;
    }

    /* 速度 → 占空比 线性映射 */
    float duty_f = (speed_mps / MAX_SPEED_MPS) * (float)MOTOR_MAX_DUTY;

    /* 限幅: 0 ~ MOTOR_MAX_DUTY */
    if (duty_f > (float)MOTOR_MAX_DUTY) duty_f = (float)MOTOR_MAX_DUTY;
    if (duty_f < 0.0f) duty_f = 0.0f;

    pwm_set_duty(MOTOR_PWM_CH, (uint32)duty_f);
}

/**
 * motor_stop() - 紧急停车, 电机占空比归零
 */
static void motor_stop(void)
{
    pwm_set_duty(MOTOR_PWM_CH, 0);
}

/**
 * servo_set_angle() - 设置舵机转向角度
 * @angle_deg: 目标角度 (度), 正值=右转, 负值=左转, 0=直行
 *
 * 映射关系:
 *   -30° → 脉宽 (SERVO_CENTER_US - SERVO_RANGE_US) 微秒 (最左)
 *     0° → 脉宽 SERVO_CENTER_US 微秒 (中位)
 *   +30° → 脉宽 (SERVO_CENTER_US + SERVO_RANGE_US) 微秒 (最右)
 *
 * 注意: 角度限幅在 ±30°, 防止舵机过转损坏
 */
static void servo_set_angle(float angle_deg)
{
    /* 角度限幅 ±30° */
    if (angle_deg >  30.0f) angle_deg =  30.0f;
    if (angle_deg < -30.0f) angle_deg = -30.0f;

    /* 角度 → 脉宽(μs) 线性映射 */
    float pulse_us = (float)SERVO_CENTER_US
                   + (angle_deg / 30.0f) * (float)SERVO_RANGE_US;

    /* 脉宽限幅 */
    uint32 pw = (uint32)pulse_us;
    uint32 pw_min = (uint32)(SERVO_CENTER_US - SERVO_RANGE_US);
    uint32 pw_max = (uint32)(SERVO_CENTER_US + SERVO_RANGE_US);
    if (pw < pw_min) pw = pw_min;
    if (pw > pw_max) pw = pw_max;

    pwm_set_duty(SERVO_PWM_CH, pw);
}

/* =====================================================================
 *  PID 控制上层封装
 * ===================================================================== */

/**
 * speed_control() - 速度控制器
 * @dist_to_target: 到当前目标航点的水平距离 (m)
 * @is_garage:      当前目标是否是车库相关航点 (1=是, 0=否)
 *
 * 返回: 目标速度 (m/s)
 *
 * 控制策略:
 *   距离 > 2m   → 全速行驶 (MAX_SPEED_MPS)
 *   0.3m < 距离 < 2m → 线性减速 (越近越慢)
 *   距离 < 0.3m → 最低速度 (MIN_SPEED_MPS)
 *   入库阶段    → 限速 GARAGE_SPEED_MPS
 *   GPS精度差   → 减速或停车
 */
static float speed_control(float dist_to_target, uint8 is_garage)
{
    /* 入库限速 */
    float max_speed = is_garage ? GARAGE_SPEED_MPS : MAX_SPEED_MPS;
    float target_speed;

    if (dist_to_target > SLOW_DOWN_DIST) {
        /* 远离目标: 全速 */
        target_speed = max_speed;
    } else if (dist_to_target > 0.3f) {
        /* 减速区: 线性插值减速 */
        target_speed = MIN_SPEED_MPS
                     + (max_speed - MIN_SPEED_MPS) * (dist_to_target / SLOW_DOWN_DIST);
    } else {
        /* 极近距离: 最低速度 */
        target_speed = MIN_SPEED_MPS;
    }

    /* ---- GPS 精度保护 ---- */
    /* g_nav.pos_std 是 ESKF 估计的位置标准差, 反映 GPS 精度 */
    if (g_nav.pos_std > GPS_STD_STOP) {
        /* GPS 信号严重劣化 (可能丢星太久), 停车等待恢复 */
        target_speed = 0.0f;
    } else if (g_nav.pos_std > GPS_STD_WARN) {
        /* GPS 精度下降, 降速行驶 */
        target_speed *= 0.5f;
    }

    return target_speed;
}

/**
 * steer_control() - 转向控制器
 * @yaw_error: 航向误差 (rad), 正值=目标在车右侧, 负值=在左侧
 *
 * 返回: 舵机角度 (度)
 *
 * 实现: 使用带微分滤波的 PID 控制器
 *   输入: yaw_error (rad)
 *   输出: 舵机角度 (度), 限幅 ±30°
 *   alpha=0.2 的一阶低通滤波微分项, 减少传感器噪声引起的舵机抖动
 */
static float steer_control(float yaw_error)
{
    /* PID 计算: target=0 (期望航向误差为0), current=-yaw_error */
    /* 注意取负号: yaw_error>0 表示目标在右侧, 舵机正值=右转 */
    float steer_deg = pid_compute_filtered(
        &g_steer_pid_params, &g_steer_pid_state,
        0.0f,           /* target: 期望航向误差为 0 */
        -yaw_error,     /* current: 取负使误差方向与舵机方向一致 */
        0.2f            /* alpha: 微分滤波系数, 0.1~0.3 之间 */
    );

    /* 限幅 ±30° (安全保护) */
    if (steer_deg >  30.0f) steer_deg =  30.0f;
    if (steer_deg < -30.0f) steer_deg = -30.0f;

    return steer_deg;
}

/* =====================================================================
 *  初始化函数
 * ===================================================================== */

/**
 * pid_controllers_init() - 初始化速度和转向的 PID 控制器
 *
 * 速度 PID 参数说明:
 *   Kp=1.0   距离误差直接映射为速度, 增益适中
 *   Ki=0.05  小积分消除稳态误差 (如坡道上速度不足)
 *   Kd=0.1   微分抑制超调
 *   输出范围 [0, MAX_SPEED_MPS]
 *   死区 0.1m: 距离<10cm 时不调速 (避免频繁加减速)
 *
 * 转向 PID 参数说明:
 *   Kp=15.0  航向误差(rad)→舵角(deg), 1rad≈57°, 需较大增益
 *   Ki=0.0   转向一般不用积分项 (防止积分饱和导致转弯过度)
 *   Kd=2.0   微分抑制转向超调
 *   输出范围 [-30°, +30°]
 *   死区 0.02rad≈1.1°: 航向误差<1° 时不转向 (避免舵机抖动)
 *
 * 注意: 这些参数只是初始值, 需要在实车上反复调试!
 */
static void pid_controllers_init(void)
{
    /* ---- 速度 PID ---- */
    pid_init(&g_speed_pid_params,
             1.0f,               /* Kp: 比例系数 */
             0.05f,              /* Ki: 积分系数 */
             0.1f,               /* Kd: 微分系数 */
             0.01f,              /* dt: 控制周期 10ms */
             0.0f,               /* output_min: 最低速度 0 */
             MAX_SPEED_MPS,      /* output_max: 最高速度 */
             PID_POSITION);      /* 位置式 PID */
    pid_set_dead_zone(&g_speed_pid_params, 0.1f);  /* 死区 10cm */
    pid_reset(&g_speed_pid_state);

    /* ---- 转向 PID ---- */
    pid_init(&g_steer_pid_params,
             15.0f,              /* Kp: 比例系数 (rad→deg 需要较大增益) */
             0.0f,               /* Ki: 积分系数 (转向不用积分) */
             2.0f,               /* Kd: 微分系数 (抑制超调) */
             0.01f,              /* dt: 控制周期 10ms */
             -30.0f,             /* output_min: 最大左转 -30° */
              30.0f,             /* output_max: 最大右转 +30° */
             PID_POSITION);      /* 位置式 PID */
    pid_set_dead_zone(&g_steer_pid_params, 0.02f);  /* 死区 ~1° */
    pid_reset(&g_steer_pid_state);
}

/**
 * tof_kalman_init() - 初始化 ToF 测距的 1D 卡尔曼滤波器 (可选)
 *
 * 用途: ToF 传感器测距数据会有噪声和跳变, 用 1D KF 平滑:
 *   状态模型: x(k) = x(k-1) + w  (匀速模型, w~N(0,q))
 *   观测模型: z(k) = x(k) + v     (v~N(0,r))
 *
 * 参数含义:
 *   q (过程噪声) 越大 → 滤波器越相信测量值 → 响应快但平滑差
 *   r (测量噪声) 越大 → 滤波器越相信预测值 → 响应慢但平滑好
 *   q/r 比值决定滤波器的"信任分配"
 */
static void tof_kalman_init(void)
{
    g_tof_kf_params.q = TOF_KF_Q;      /* 过程噪声 */
    g_tof_kf_params.r = TOF_KF_R;      /* 测量噪声 */
    kf1d_init(&g_tof_kf_state, &g_tof_kf_params, 0.0f, 100.0f);
    /* 初始状态 x0=0, 初始协方差 p0=100 (表示初始时很不确定) */
}

/* =====================================================================
 *  core0_main() - CPU0 主函数
 *
 *  初始化顺序:
 *    1. 系统时钟 + 调试串口
 *    2. 按键 GPIO
 *    3. GPS 模块 (UART3)
 *    4. PIT 定时器 (调试100ms + IMU10ms)
 *    5. 融合系统 (IMU963RA + ESKF)
 *    6. 电机/舵机 PWM
 *    7. PID 控制器
 *    8. ToF 卡尔曼滤波 (可选)
 *    9. IMU 零偏标定 (静止2秒)
 *   10. 等待多核同步
 *
 *  主循环:
 *    每约1ms 循环一次, 依次:
 *    ① 检查 IMU tick → 读IMU + ESKF预测 + 导航PID控制
 *    ② 检查 GPS 新帧 → ESKF 更新
 *    ③ 按键扫描 → 模式切换/航点记录
 *    ④ 调试输出 (1Hz)
 * ===================================================================== */
/**
 * core0_main() - 串口角度控制
 *
 * 串口助手发送整数角度值 (如 "30", "-45"), 电机旋转到对应角度
 * 正数 = 右转, 负数 = 左转
 *
 * 串口配置: 115200 波特率, 8N1, 文本模式
 *
 * 使用示例:
 *   发送 "30"   → 右转 30°
 *   发送 "-45"  → 左转 45°
 *   发送 "0"    → 回到零位
 *   发送 "r"    → 重置 (重新记录零位)
 */
int core0_main(void)
{
    /* 所有变量声明必须在函数顶部 (TASKING C89 要求) */
    char rx_buf[16];
    unsigned char rx_len;
    unsigned char rx_byte;
    char tx_buf[80];
    unsigned long loop_cnt;
    long target;
    unsigned char i;
    unsigned char neg;
    long cal_target;

    clock_init();
    debug_init();

    angle_control_init();

    cpu_wait_event_ready();

    uart_write_string(DEBUG_UART_INDEX, "=============================\r\n");
    uart_write_string(DEBUG_UART_INDEX, "  Angle Motor Control\r\n");
    uart_write_string(DEBUG_UART_INDEX, "=============================\r\n");
    uart_write_string(DEBUG_UART_INDEX, "Commands:\r\n");
    uart_write_string(DEBUG_UART_INDEX, "  -10~10 : set target angle\r\n");
    uart_write_string(DEBUG_UART_INDEX, "  r      : reset zero position\r\n");
    uart_write_string(DEBUG_UART_INDEX, "  c<deg> : calibrate (e.g. c10)\r\n");
    uart_write_string(DEBUG_UART_INDEX, "  Step1: send r to set zero\r\n");
    uart_write_string(DEBUG_UART_INDEX, "  Step2: turn wheel to known angle\r\n");
    uart_write_string(DEBUG_UART_INDEX, "  Step3: send c10 (actual 10 deg)\r\n");
    uart_write_string(DEBUG_UART_INDEX, "-----------------------------\r\n");

    rx_len = 0;
    loop_cnt = 0;

    while (TRUE)
    {
        /* ---- 1. 轮询串口接收 (从 debug FIFO 读取) ---- */
        if (debug_read_ring_buffer(&rx_byte, 1) > 0)
        {
            if (rx_byte == '\r' || rx_byte == '\n')
            {
                if (rx_len > 0)
                {
                    rx_buf[rx_len] = '\0';

                    /* 'r' = 重置零位 */
                    if (rx_buf[0] == 'r' || rx_buf[0] == 'R')
                    {
                        angle_control_reset();
                        uart_write_string(DEBUG_UART_INDEX, "[OK] Zero reset done.\r\n");
                    }
                    /* 'c' = 校准 (格式: c10, c-10) */
                    else if (rx_buf[0] == 'c' || rx_buf[0] == 'C')
                    {
                        cal_target = 0;
                        i = 1;
                        neg = 0;

                        if (rx_len > 1 && rx_buf[1] == '-') {
                            neg = 1;
                            i = 2;
                        } else if (rx_len > 1 && rx_buf[1] == '+') {
                            i = 2;
                        }

                        for (; i < rx_len; i++)
                        {
                            if (rx_buf[i] >= '0' && rx_buf[i] <= '9')
                            {
                                cal_target = cal_target * 10 + (long)(rx_buf[i] - '0');
                            }
                            else
                            {
                                cal_target = -9999;
                                break;
                            }
                        }

                        if (cal_target == -9999 || cal_target == 0)
                        {
                            uart_write_string(DEBUG_UART_INDEX, "[ERR] Use: c10 or c-10\r\n");
                        }
                        else
                        {
                            if (neg) cal_target = -cal_target;
                            angle_control_calibrate((float)cal_target);
                            sprintf(tx_buf, "[OK] Calibrated with %d deg\r\n", (int)cal_target);
                            uart_write_string(DEBUG_UART_INDEX, tx_buf);
                        }
                    }
                    else
                    {
                        /* 解析角度整数 (支持负号) */
                        target = 0;
                        i = 0;
                        neg = 0;

                        if (rx_buf[0] == '-') {
                            neg = 1;
                            i = 1;
                        } else if (rx_buf[0] == '+') {
                            i = 1;
                        }

                        for (; i < rx_len; i++)
                        {
                            if (rx_buf[i] >= '0' && rx_buf[i] <= '9')
                            {
                                target = target * 10 + (long)(rx_buf[i] - '0');
                            }
                            else
                            {
                                target = -9999;
                                break;
                            }
                        }

                        if (target == -9999)
                        {
                            uart_write_string(DEBUG_UART_INDEX, "[ERR] Invalid input!\r\n");
                        }
                        else
                        {
                            if (neg) target = -target;
                            if (target > 10) target = 10;
                            if (target < -10) target = -10;

                            angle_control_set_target(target);

                            sprintf(tx_buf, "[OK] Target = %d deg\r\n", (int)target);
                            uart_write_string(DEBUG_UART_INDEX, tx_buf);
                        }
                    }

                    rx_len = 0;
                }
            }
            else
            {
                if (rx_len < 15)
                {
                    rx_buf[rx_len++] = (char)rx_byte;
                }
            }
        }

        /* ---- 2. 执行角度 PID 控制 ---- */
        angle_control_update();

        /* ---- 3. 定期输出当前状态 (约每1秒一次) ---- */
        loop_cnt++;
        if (loop_cnt % 100 == 0)
        {
            sprintf(tx_buf, "cur:%.1f  tgt:%.1f  enc:%d\r\n",
                    (float)(angle_ctrl.current_angle * 100.0f),
                    (float)(angle_ctrl.target_angle * 100.0f),
                    (int)encoder_get_count(ANGLE_ENCODER));
            uart_write_string(DEBUG_UART_INDEX, tx_buf);
        }

        /* ---- 4. 控制周期 10ms ---- */
        system_delay_ms(10);
    }
}

#pragma section all restore

/* 兼容旧代码的屏幕初始化函数 (如有 IPS200 屏幕可在此实现) */
void IPS200_Show_Init(void)
{
    /* 空实现, 如需要可在 cpu1_main.c 中启用屏幕显示 */
}
