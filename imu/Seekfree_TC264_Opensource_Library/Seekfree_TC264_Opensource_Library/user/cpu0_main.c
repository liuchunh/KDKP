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
#include "code/motor_encoder_test.h" /* 电机+编码器 测试 */

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
int core0_main(void)
{
    /* ================================================================
     *  第1步: 系统基础初始化
     * ================================================================ */
    clock_init();                   /* 配置系统时钟 (200MHz) */
    debug_init();                   /* 初始化调试串口 (UART0, 115200) */

    /* ================================================================
     *  ★ 电机+编码器测试 (进入后不会返回, 直接在串口显示转速)
     *  测试完成后注释掉此行, 恢复下面的完整导航流程
     * ================================================================ */
    motor_encoder_test();

    /* ================================================================
     *  第2步: 按键 GPIO 初始化
     *  四个按键均为低电平有效, 配置为上拉输入
     *  K1=模式切换  K2=记录锥桶  K3=记录车库入口  K4=记录停靠点
     * ================================================================ */
    gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);

    /* ================================================================
     *  第3步: GPS 模块初始化
     *  GN42A 模块, UART3 接口, 115200 波特率, 中断接收
     *  初始化后 GPS 模块开始输出 NMEA 数据帧
     * ================================================================ */
    gnss_init(GN42A);

    /* ================================================================
     *  第4步: PIT 定时器初始化
     *  Ch0: 100ms 周期 → 调试输出定时 (g_debug_tick)
     *  Ch1: 10ms 周期  → IMU 采样定时 (g_imu_tick, 100Hz)
     * ================================================================ */
    pit_ms_init(DEBUG_PIT_CHANNEL, 100);       /* 100ms */
    pit_ms_init(IMU_PIT_CHANNEL, 10);          /* 10ms = 100Hz */

    /* ================================================================
     *  第5步: INS 融合系统初始化
     *  包含: IMU963RA 传感器初始化 + ESKF 滤波器初始化 + 初始位姿设置
     *  ESKF 使用 15 维误差状态: [δp(3) δv(3) δθ(3) δba(3) δbw(3)]
     * ================================================================ */
    fusion_init();

    /* ================================================================
     *  第6步: 电机/舵机初始化
     * ================================================================ */
    motor_servo_init();

    /* ================================================================
     *  第7步: PID 控制器初始化
     * ================================================================ */
    pid_controllers_init();

    /* ================================================================
     *  第8步: ToF 卡尔曼滤波初始化 (可选)
     * ================================================================ */
    tof_kalman_init();

    /* ================================================================
     *  第9步: IMU 零偏标定
     *  ★★★ 必须在车辆完全静止时进行! ★★★
     *  采集 200 帧 (2秒), 计算陀螺仪和加速度计的零偏均值
     *  后续每次读取 IMU 数据会自动扣除这些零偏
     * ================================================================ */
    uart_write_string(DEBUG_UART_INDEX, "[INIT] IMU calibrating... Keep the car still!\r\n");
    imu_task_calibrate(200);        /* 200帧 x 10ms = 2秒 */
    uart_write_string(DEBUG_UART_INDEX, "[INIT] IMU calibration done.\r\n");

    /* ================================================================
     *  第10步: 等待 CPU1 等其他核心初始化完成
     * ================================================================ */
    cpu_wait_event_ready();

    /* ================================================================
     *  初始化完成, 打印使用说明
     * ================================================================ */
    uart_write_string(DEBUG_UART_INDEX,
        "========================================\r\n"
        "  KDKP Subject 1 - Autonomous Driving\r\n"
        "  ESKF Fusion + Waypoint Navigation\r\n"
        "========================================\r\n"
        "  K1: Switch Teaching/Auto mode\r\n"
        "  K2: Record cone waypoint (teaching)\r\n"
        "  K3: Record garage entrance (teaching)\r\n"
        "  K4: Record parking spot (teaching)\r\n"
        "========================================\r\n");

    /* ================================================================
     *  主循环局部变量
     * ================================================================ */
    uint32 debug_count = 0;         /* 调试输出分频计数器 */
    uint8  led_state = 0;           /* LED 状态 (GPS 更新指示) */

    /* ================================================================
     *  ★★★ 主循环开始 ★★★
     *  循环周期约 1ms, 实际控制逻辑由标志位驱动
     * ================================================================ */
    while (TRUE)
    {

        /* ============================================================
         *  ① IMU 预测 + 导航控制 (100Hz, 由 PIT Ch1 中断触发)
         *
         *  每 10ms:
         *    1) 读取 IMU963RA 加速度计 + 陀螺仪数据
         *    2) 执行 ESKF 预测步骤 (状态外推 + 协方差传播)
         *    3) 如果处于自动模式, 执行导航 PID 控制
         * ============================================================ */
        if (g_imu_tick)
        {
            g_imu_tick = 0;         /* 消费标志 */

            /* 读取 IMU 数据 (加速度 + 角速度, 已扣除零偏) */
            imu_task_read();

            /* ESKF 预测: 用 IMU 数据推算位置/速度/姿态 */
            fusion_imu_predict();

            /* 累计行驶距离计算 */
            if (!g_dist_init) {
                g_prev_x = g_nav.x;
                g_prev_y = g_nav.y;
                g_dist_init = 1;
            } else {
                float dx = g_nav.x - g_prev_x;
                float dy = g_nav.y - g_prev_y;
                g_total_dist += sqrtf(dx*dx + dy*dy);
                g_prev_x = g_nav.x;
                g_prev_y = g_nav.y;
            }

            /* ---- 自动导航控制 (仅在 NAV_RUNNING 模式下执行) ---- */
            if (g_nav_mode == NAV_RUNNING)
            {
                /* 计算到当前目标航点的距离和航向误差 */
                float dist, yaw_err;
                nav_compute_errors(&dist, &yaw_err);

                /* 到达检测: 如果到达当前航点, 自动切换到下一个 */
                uint8 arrival = nav_check_arrival();

                if (g_nav_mode == NAV_FINISHED) {
                    /* 所有航点完成 → 停车, 科目1完成! */
                    motor_stop();
                    servo_set_angle(0.0f);
                    uart_write_string(DEBUG_UART_INDEX,
                        "[NAV] === ALL DONE! Subject 1 completed. ===\r\n");
                } else {
                    /* 判断当前目标是否是车库相关航点 */
                    const Waypoint *wp = nav_get_current_target();
                    uint8 is_garage = (wp && (wp->type == 1 || wp->type == 2)) ? 1 : 0;

                    /* 速度控制: 距离 → 目标速度 → PWM */
                    float target_speed = speed_control(dist, is_garage);
                    motor_set_speed(target_speed);

                    /* 转向控制: 航向误差 → 舵机角度 */
                    float steer_deg = steer_control(yaw_err);
                    servo_set_angle(steer_deg);
                }
            }
            /* 非自动模式: 电机和舵机保持静止 */
        }

        /* ============================================================
         *  ② GPS 更新 (约 10Hz, 有新 NMEA 帧时)
         *
         *  fusion_gps_update() 内部包含 5 层 NaN 防护:
         *    L1: NMEA 校验和检查
         *    L2: GPS 定位有效性检查
         *    L3: 经纬度范围合理性检查 (中国境内)
         *    L4: 基准点自动设置 (首次有效定位为原点)
         *    L5: NED 坐标 NaN/Inf 检查
         *  任何一层失败都会丢弃该帧, ESKF 继续纯 IMU 推算
         * ============================================================ */
        if (gnss_flag)
        {
            fusion_gps_update();    /* GPS 数据 → ESKF 测量更新 */

            /* GPS 更新时翻转状态指示 */
            led_state ^= 1;
        }

        /* ============================================================
         *  ③ 按键扫描处理
         *
         *  K1: 模式切换 (空闲→教学→自动→空闲)
         *      空闲/完成 → 按K1 → 教学模式 (清空旧航点)
         *      教学     → 按K1 → 自动模式 (开始导航)
         *      自动     → 按K1 → 急停回空闲 (安全保护)
         *
         *  K2/K3/K4: 仅在教学模式下有效
         *      K2 → 记录锥桶航点 (type=0, 到达半径0.8m)
         *      K3 → 记录车库入口 (type=1, 到达半径0.5m)
         *      K4 → 记录停靠点   (type=2, 到达半径0.3m)
         * ============================================================ */

        /* ---- K1: 模式切换 ---- */
        if (!gpio_get_level(KEY1))                  /* 按键按下 (低电平) */
        {
            system_delay_ms(20);                    /* 消抖延时 20ms */
            if (!gpio_get_level(KEY1))              /* 再次确认 */
            {
                if (g_nav_mode == NAV_IDLE || g_nav_mode == NAV_FINISHED)
                {
                    /* → 进入教学模式 */
                    nav_clear_waypoints();           /* 清空旧航点 */
                    g_nav_mode = NAV_TEACHING;
                    motor_stop();                    /* 确保电机停止 */
                    servo_set_angle(0.0f);           /* 舵机回中 */
                    uart_write_string(DEBUG_UART_INDEX,
                        "[MODE] >>> TEACHING mode\r\n"
                        "  Drive/push car to each cone, press K2/K3/K4 to record.\r\n");
                }
                else if (g_nav_mode == NAV_TEACHING)
                {
                    /* → 进入自动运行模式 */
                    if (g_waypoint_count > 0) {
                        g_current_target = 0;       /* 从第0个航点开始 */
                        g_nav_mode = NAV_RUNNING;
                        /* 重置 PID 状态 (清除积分和历史误差) */
                        pid_reset(&g_speed_pid_state);
                        pid_reset(&g_steer_pid_state);
                        uart_write_string(DEBUG_UART_INDEX, "[MODE] >>> AUTO RUNNING\r\n");
                        uart_write_string(DEBUG_UART_INDEX, "  Waypoints: ");
                        uart_write_integer(DEBUG_UART_INDEX, g_waypoint_count);
                        uart_write_string(DEBUG_UART_INDEX, "\r\n");
                        nav_debug_print();           /* 打印所有航点坐标 */
                    } else {
                        uart_write_string(DEBUG_UART_INDEX,
                            "[ERR] No waypoints! Press K2/K3/K4 to record first.\r\n");
                    }
                }
                else if (g_nav_mode == NAV_RUNNING)
                {
                    /* → 急停: 回到空闲状态 (安全保护) */
                    g_nav_mode = NAV_IDLE;
                    motor_stop();
                    servo_set_angle(0.0f);
                    uart_write_string(DEBUG_UART_INDEX,
                        "[MODE] EMERGENCY STOP -> IDLE\r\n");
                }

                /* 等待按键释放 (防止长按重复触发) */
                while (!gpio_get_level(KEY1)) { system_delay_ms(10); }
            }
        }

        /* ---- K2: 记录锥桶航点 (仅教学模式) ---- */
        if (g_nav_mode == NAV_TEACHING && !gpio_get_level(KEY2))
        {
            system_delay_ms(20);
            if (!gpio_get_level(KEY2))
            {
                uint8 cnt = nav_record_waypoint(0); /* type=0: 普通锥桶 */
                if (cnt > 0) {
                    uart_write_string(DEBUG_UART_INDEX, "[WP] Cone #");
                    uart_write_integer(DEBUG_UART_INDEX, cnt);
                    uart_write_string(DEBUG_UART_INDEX, " (");
                    uart_write_float(DEBUG_UART_INDEX, g_nav.x);
                    uart_write_string(DEBUG_UART_INDEX, ", ");
                    uart_write_float(DEBUG_UART_INDEX, g_nav.y);
                    uart_write_string(DEBUG_UART_INDEX, ")\r\n");
                } else {
                    uart_write_string(DEBUG_UART_INDEX, "[ERR] Waypoint array full!\r\n");
                }
                while (!gpio_get_level(KEY2)) { system_delay_ms(10); }
            }
        }

        /* ---- K3: 记录车库入口 (仅教学模式) ---- */
        if (g_nav_mode == NAV_TEACHING && !gpio_get_level(KEY3))
        {
            system_delay_ms(20);
            if (!gpio_get_level(KEY3))
            {
                uint8 cnt = nav_record_waypoint(1); /* type=1: 车库入口 */
                if (cnt > 0) {
                    uart_write_string(DEBUG_UART_INDEX, "[WP] Garage entrance #");
                    uart_write_integer(DEBUG_UART_INDEX, cnt);
                    uart_write_string(DEBUG_UART_INDEX, " recorded\r\n");
                }
                while (!gpio_get_level(KEY3)) { system_delay_ms(10); }
            }
        }

        /* ---- K4: 记录车库停靠点 (仅教学模式) ---- */
        if (g_nav_mode == NAV_TEACHING && !gpio_get_level(KEY4))
        {
            system_delay_ms(20);
            if (!gpio_get_level(KEY4))
            {
                uint8 cnt = nav_record_waypoint(2); /* type=2: 停靠点 */
                if (cnt > 0) {
                    uart_write_string(DEBUG_UART_INDEX, "[WP] Parking spot #");
                    uart_write_integer(DEBUG_UART_INDEX, cnt);
                    uart_write_string(DEBUG_UART_INDEX, " recorded\r\n");
                }
                while (!gpio_get_level(KEY4)) { system_delay_ms(10); }
            }
        }

        /* ============================================================
         *  ④ 调试输出 (1Hz, 每秒输出一次)
         *
         *  输出内容:
         *    - NED 位置 (x, y, z)
         *    - 航向角 yaw (度)
         *    - 位置标准差 pos_std (反映 GPS 精度)
         *    - 自动模式下额外输出: 航点编号、距离、航向误差
         *    - 教学模式下额外输出: 已记录航点数
         * ============================================================ */
        if (g_debug_tick)
        {
            g_debug_tick = 0;       /* 消费标志 */
            debug_count++;

            if (debug_count >= 10)  /* 100ms x 10 = 1秒 */
            {
                debug_count = 0;

                /* ---- 导航状态输出 ---- */
                uart_write_string(DEBUG_UART_INDEX, "--- NAV STATUS ---\r\n");

                /* 位置 */
                uart_write_string(DEBUG_UART_INDEX, "  Pos: X=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.x);
                uart_write_string(DEBUG_UART_INDEX, " Y=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.y);
                uart_write_string(DEBUG_UART_INDEX, " Z=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.z);
                uart_write_string(DEBUG_UART_INDEX, "\r\n");

                /* 速度 */
                uart_write_string(DEBUG_UART_INDEX, "  Vel: Vx=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.vx);
                uart_write_string(DEBUG_UART_INDEX, " Vy=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.vy);
                uart_write_string(DEBUG_UART_INDEX, "\r\n");

                /* 航向 */
                float yaw_deg = g_nav.yaw * 180.0f / 3.14159f;
                uart_write_string(DEBUG_UART_INDEX, "  Yaw=");
                uart_write_float(DEBUG_UART_INDEX, yaw_deg);
                uart_write_string(DEBUG_UART_INDEX, " deg\r\n");

                /* GPS 状态 */
                uart_write_string(DEBUG_UART_INDEX, "  GPS: ");
                uart_write_string(DEBUG_UART_INDEX, g_nav.gps_valid ? "OK" : "LOST");
                uart_write_string(DEBUG_UART_INDEX, "  Std=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.pos_std);
                uart_write_string(DEBUG_UART_INDEX, "m\r\n");

                /* 累计行驶距离 */
                uart_write_string(DEBUG_UART_INDEX, "  Dist=");
                uart_write_float(DEBUG_UART_INDEX, g_total_dist);
                uart_write_string(DEBUG_UART_INDEX, "m\r\n");

                /* 误差统计 */
                uart_write_string(DEBUG_UART_INDEX, "  GPS-ESKF: dX=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.gps_dx);
                uart_write_string(DEBUG_UART_INDEX, " dY=");
                uart_write_float(DEBUG_UART_INDEX, g_nav.gps_dy);
                uart_write_string(DEBUG_UART_INDEX, "\r\n");

                /* 航向漂移 */
                float drift_deg = g_nav.yaw_drift * 180.0f / 3.14159f;
                uart_write_string(DEBUG_UART_INDEX, "  YawDrift=");
                uart_write_float(DEBUG_UART_INDEX, drift_deg);
                uart_write_string(DEBUG_UART_INDEX, " deg\r\n");

                /* 零偏估计 */
                uart_write_string(DEBUG_UART_INDEX, "  AccBias: ");
                uart_write_float(DEBUG_UART_INDEX, g_nav.acc_bias[0]);
                uart_write_string(DEBUG_UART_INDEX, " ");
                uart_write_float(DEBUG_UART_INDEX, g_nav.acc_bias[1]);
                uart_write_string(DEBUG_UART_INDEX, " ");
                uart_write_float(DEBUG_UART_INDEX, g_nav.acc_bias[2]);
                uart_write_string(DEBUG_UART_INDEX, "\r\n");

                uart_write_string(DEBUG_UART_INDEX, "  GyroBias: ");
                uart_write_float(DEBUG_UART_INDEX, g_nav.gyro_bias[0]);
                uart_write_string(DEBUG_UART_INDEX, " ");
                uart_write_float(DEBUG_UART_INDEX, g_nav.gyro_bias[1]);
                uart_write_string(DEBUG_UART_INDEX, " ");
                uart_write_float(DEBUG_UART_INDEX, g_nav.gyro_bias[2]);
                uart_write_string(DEBUG_UART_INDEX, "\r\n");

                /* 自动模式: 航点追踪详情 */
                if (g_nav_mode == NAV_RUNNING) {
                    float dist, yaw_err;
                    nav_compute_errors(&dist, &yaw_err);
                    uart_write_string(DEBUG_UART_INDEX, "  WP[");
                    uart_write_integer(DEBUG_UART_INDEX, g_current_target);
                    uart_write_string(DEBUG_UART_INDEX, "/");
                    uart_write_integer(DEBUG_UART_INDEX, g_waypoint_count);
                    uart_write_string(DEBUG_UART_INDEX, "] Dist=");
                    uart_write_float(DEBUG_UART_INDEX, dist);
                    uart_write_string(DEBUG_UART_INDEX, "m YawErr=");
                    uart_write_float(DEBUG_UART_INDEX, yaw_err * 180.0f / 3.14159f);
                    uart_write_string(DEBUG_UART_INDEX, "deg\r\n");
                }

                /* 教学模式 */
                if (g_nav_mode == NAV_TEACHING) {
                    uart_write_string(DEBUG_UART_INDEX, "  [TEACH] Points: ");
                    uart_write_integer(DEBUG_UART_INDEX, g_waypoint_count);
                    uart_write_string(DEBUG_UART_INDEX, "/20\r\n");
                }

                uart_write_string(DEBUG_UART_INDEX, "------------------\r\n");
            }
        }

        /* ============================================================
         *  ⑤ 主循环延时
         *  约 1ms, 实际周期取决于上面各步骤的执行时间
         *  TC264 @200MHz 下, ESKF 预测约 250us, 导航计算约 20us
         * ============================================================ */
        system_delay_ms(1);

    } /* while(TRUE) 主循环结束 */
}

#pragma section all restore

/* 兼容旧代码的屏幕初始化函数 (如有 IPS200 屏幕可在此实现) */
void IPS200_Show_Init(void)
{
    /* 空实现, 如需要可在 cpu1_main.c 中启用屏幕显示 */
}
