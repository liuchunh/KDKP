/*********************************************************************************************************************
 * 文件名称: cpu0_main.c
 * 功能描述: 第21届智能车竞赛 - 卡丁快跑组 - 科目1 自动驾驶 主控制器 (CPU0 核)
 *
 * 整体思路:
 *   采用"打点-导航"方案完成科目1:
 *     阶段1 (打点): 手动推车或遥控，经过关键位置时按 S3 记录 GPS 航点。
 *                   S4/S5 可在空闲时手动控制车速前进。
 *     阶段2 (导航): 记录完航点后，车模自动依次导航到每个航点，
 *                   绕完所有锥桶后减速进入车库，完成倒车入库停车。
 *                   S6 随时刹车并清空航点重置路线。
 *
 * 核心算法:
 *   1. ESKF (误差状态卡尔曼滤波) 融合 IMU963RA + GPS，输出 NED 坐标和航向角
 *   2. 航点导航控制器 (nav_controller) 计算距离误差和航向误差
 *   3. 双环 PID 控制: 速度环(距离→PWM) + 转向环(航向误差→舵机角度)
 *   4. 电机闭环 PID: 前馈+PID, 编码器反馈, 100ms 控制周期
 *   5. 可选: 1D 卡尔曼滤波平滑 ToF 测距数据
 *
 * 硬件平台: TC264D (英飞凌 TriCore, Seekfree 逐飞开源库)
 * 传感器:   IMU963RA (SPI, 100Hz) + GNSS GN42A (UART0, 10Hz)
 * 执行器:   直流电机 (PWM驱动) + 舵机 (PWM 50Hz)
 *
 * 任务调度:
 *   PIT Ch0 (100ms): 调试输出定时标志
 *   PIT Ch1 (10ms):  IMU 数据采集定时标志 (100Hz)
 *   PIT Ch2 (10ms):  编码器速度采样 (电机PID反馈)
 *   主循环 (约1ms):  I   MU预测 + GPS更新 + 导航PID + 电机PID + 按键处理
 *
 * 修改记录:
 *   2026-05-02  基于 INS_Fusion_Project 整合, 新增教学/自动模式、PID控制、按键逻辑
 ********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "zf_device_imu963ra.h"
#include "zf_device_wireless_uart.h"

/* ---- INS 融合模块头文件 (位于 user/code/ 子目录) ---- */
#include "code/ins/ins_solver.h"      /* ESKF 误差状态卡尔曼滤波核心 */
#include "code/imu_task.h"           /* IMU963RA 数据采集 + 零偏标定 */
#include "code/fusion_task.h"        /* 融合调度层 (IMU预测 + GPS更新) */
#include "code/nav_controller.h"     /* 航点导航控制器 (教学记录 + 距离/航向误差) */
#include "code/pid_controller.h"     /* PID 控制器 (位置式/增量式) */
#include "code/kalman_filter.h"      /* 1D/2D 卡尔曼滤波器 (ToF平滑等) */

/* ---- 电机/编码器模块 (PID_Export) ---- */
#include "code/encoder.h"            /* 双编码器驱动 (中断驱动速度采样) */
#include "code/motor.h"              /* 双电机驱动 (DRV8701E DIR+PWM) */
#include "code/motor_pid.h"          /* 前馈+PID 电机速度控制器 */
#include "code/angle_control.h"     /* 转向电机角度控制 (PID+编码器) */
#include "code/subject 2/action_task.h"

/* =====================================================================
 *  宏定义 — 硬件配置 (根据你的实际接线修改!)
 * ===================================================================== */

/* PIT 定时器通道分配 */
#define IMU_PIT_CHANNEL     (CCU60_CH1)         /* PIT 通道1: 10ms 周期, 用于 IMU 采样 */
#define DEBUG_PIT_CHANNEL   (CCU60_CH0)         /* PIT 通道0: 100ms 周期, 用于调试输出 */

/* 纯惯导模式开关: 1=无GPS(仅ESKF推算), 0=GPS+INS融合 */
#define INS_ONLY_MODE       1

/* 按键引脚定义 (低电平有效, 上拉输入) */
#define KEY_MARK     (P20_6)    /* S3: 打点 (记录航点) */
#define KEY_SPEED_UP (P20_7)    /* S4: 加速 */
#define KEY_SLOW_DN  (P11_2)    /* S5: 减速 */
#define KEY_RESET    (P11_3)    /* S6: 刹车 (清空航点+急停) */

/* 舵机 PWM 配置 */
#define SERVO_PWM_CH        (ATOM2_CH4_P33_8)   /* 舵机 PWM 输出引脚 */
#define SERVO_PWM_FREQ      (50)                 /* 舵机 PWM 频率 50Hz (标准舵机) */
#define SERVO_CENTER_US     (750)                /* 舵机中位脉宽 (微秒), 需实测! 常见值 700~800 */
#define SERVO_RANGE_US      (250)                /* 舵机左右最大偏移 (微秒), 即中位±250us */

/* =====================================================================
 *  宏定义 — 导航参数 (根据赛道和车模调试)
 * ===================================================================== */

#define MAX_SPEED_MPS       (2.5f)      /* 最大车速 m/s (约9km/h, 规则限速25km/h, 建议不超过3m/s) */
#define MIN_SPEED_MPS       (0.3f)      /* 最低车速 m/s (防止低速时电机堵转停不下来) */
#define NAV_MAX_SPEED_MPS   (2.0f)
#define NAV_NEAR_SPEED_MPS  (0.8f)
#define MANUAL_SPEED_STEP   (0.1f)
#define MANUAL_MAX_SPEED_MPS (10.0f)
#define MANUAL_MIN_SPEED_MPS (-5.0f)
#define GARAGE_SPEED_MPS    (0.5f)      /* 入库阶段车速 m/s (慢速精确入库) */
#define SLOW_DOWN_DIST      (2.0f)      /* 开始减速的距离阈值 (m) */
#define GPS_STD_WARN        (1.0f)      /* GPS 精度警告阈值 (m), 超过此值减速 */
#define GPS_STD_STOP        (3.0f)      /* GPS 精度停车阈值 (m), 超过此值停车等待 */
#define STANLEY_K           (2.5f)      /* Stanley 控制器横向偏差增益, 越大修正越强 */
#define REQUIRED_WAYPOINT_COUNT (4)

/* ToF 卡尔曼滤波参数 (可选, 用于平滑 ToF 测距噪声) */
#define TOF_KF_Q            (0.5f)      /* 过程噪声: 越大滤波器越相信测量值 (响应快, 平滑差) */
#define TOF_KF_R            (2.0f)      /* 测量噪声: 越大滤波器越相信预测值 (响应慢, 平滑好) */

/* =====================================================================
 *  全局变量 — ISR 与主循环之间通信用
 * ===================================================================== */

volatile uint8 g_imu_tick   = 0;        /* PIT Ch1 中断置1, 主循环消费后清0. 表示10ms到了 */
volatile uint8 g_debug_tick = 0;        /* PIT Ch0 中断置1, 主循环消费后清0. 表示100ms到了 */
volatile uint32 g_wireless_uart3_rx_isr_count = 0;
volatile uint32 g_wireless_cmd_rx_count = 0;

/* =====================================================================
 *  电机 PID 控制器实例 (PID_Export 模块)
 * ===================================================================== */
static MotorPID g_motor_pid;            /* 电机速度 PID 控制器 */
static int32    g_encoder_100ms = 0;    /* 100ms 累计编码器脉冲 */
static uint32   g_encoder_tick_count = 0; /* 编码器中断计数 (10ms 一级) */

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
float g_total_dist   = 0.0f;    /* 累计行驶距离 (m) */
float g_target_speed_mps = 0.0f;
float g_actual_speed_mps = 0.0f;
static float g_speed_limit  = 0.0f; /* 用户设定最大运行速度 */
static float g_manual_speed = 0.0f;    /* 手动驾驶目标速度 */

/* ---- 卡死检测 ---- */
#define STUCK_CHECK_SEC     30         /* 卡死判定时间 (秒) */
#define STUCK_MIN_DIST      1.0f      /* 最小行驶距离 (m), 低于此值判定卡死 */
static float g_stuck_snap_dist = 0.0f; /* 卡死检测距离快照 */
static uint32 g_stuck_timer    = 0;    /* 卡死检测计时器 (秒) */

/* =====================================================================
 *  舵机底层驱动函数
 * ===================================================================== */

/**
 * servo_init() - 初始化舵机 PWM 输出
 * 舵机: 50Hz 标准 PWM, 中位脉宽 SERVO_CENTER_US 微秒
 */
static void servo_init(void)
{
    pwm_init(SERVO_PWM_CH, SERVO_PWM_FREQ, SERVO_CENTER_US);
    pwm_set_duty(SERVO_PWM_CH, SERVO_CENTER_US);
}

/**
 * motor_control_set_speed() - 设置目标速度 (闭环 PID 控制)
 * @speed_mps: 目标速度 (m/s), 正值前进, 负值后退, 0 停车
 *
 * 实现: 将 m/s 转换为脉冲数/100ms, 设置 PID 目标
 * 转换公式: pulses_per_100ms = speed_mps * EFFECTIVE_PPR / WHEEL_CIRCUM_M * SAMPLE_SEC
 *
 * 注意: 编码器方向已在 encoder.c 中通过取反校正, 正脉冲=前进
 *       倒车时传入负速度, 转换为负脉冲, motor_pid 会输出负 PWM
 */
static void motor_control_set_speed(float speed_mps)
{
    if (speed_mps == 0.0f) {
        g_target_speed_mps = 0.0f;
        motor_pid_stop(&g_motor_pid);
        motor1_stop();
        motor2_stop();
        return;
    }

    /* m/s → 脉冲数/100ms (支持正负) */
    g_target_speed_mps = speed_mps;
    float target_pulses = speed_mps * (float)EFFECTIVE_PPR / WHEEL_CIRCUM_M * SAMPLE_SEC;
    motor_pid_set_target(&g_motor_pid, target_pulses);
}

/**
 * motor_control_stop() - 紧急停车
 */
static void motor_control_stop(void)
{
    g_target_speed_mps = 0.0f;
    motor_pid_stop(&g_motor_pid);
    motor1_stop();
    motor2_stop();
}

static float action_get_total_distance(void)
{
    return g_total_dist;
}

static float action_get_gyro_z(void)
{
    return g_imu.gyro_z;
}

/**
 * motor_control_update() - 电机 PID 更新 (每 100ms 调用一次)
 * 读取编码器累计脉冲, 执行 PID 计算, 输出到电机
 */
static void motor_control_update(void)
{
    int16 pwm = motor_pid_update(&g_motor_pid, g_encoder_100ms);
    motor1_set_duty(pwm);
    motor2_set_duty(pwm);
    g_encoder_100ms = 0;
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

static void steer_motor_set_angle(float angle_deg)
{
    if (angle_deg >  (float)ANGLE_MAX_DEGREE) angle_deg =  (float)ANGLE_MAX_DEGREE;
    if (angle_deg <  (float)ANGLE_MIN_DEGREE) angle_deg =  (float)ANGLE_MIN_DEGREE;
    angle_control_set_target((int32)angle_deg);
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
    /* 导航模式下速度上限: 不低于 MIN_SPEED_MPS (防止堵转), 不超过 g_speed_limit */
    float nav_speed_limit = g_speed_limit;
    if (nav_speed_limit < NAV_NEAR_SPEED_MPS) nav_speed_limit = NAV_NEAR_SPEED_MPS;
    if (nav_speed_limit > NAV_MAX_SPEED_MPS) nav_speed_limit = NAV_MAX_SPEED_MPS;

    /* 入库限速 */
    float max_speed = is_garage ? (nav_speed_limit > GARAGE_SPEED_MPS ? GARAGE_SPEED_MPS : nav_speed_limit) : nav_speed_limit;
    float target_speed;

    /* ---- 弯道前瞻减速 ---- */
    float turn_deg = nav_get_turn_angle();
    if (turn_deg > 60.0f) {
        /* 急弯: 大幅限速 */
        float turn_limit = NAV_NEAR_SPEED_MPS;
        if (max_speed > turn_limit) max_speed = turn_limit;
    } else if (turn_deg > 30.0f) {
        /* 中等弯道: 适度限速 */
        float turn_limit = NAV_NEAR_SPEED_MPS + 0.5f;
        if (max_speed > turn_limit) max_speed = turn_limit;
    }

    if (dist_to_target > SLOW_DOWN_DIST) {
        /* 远离目标: 全速 (已受弯道限速) */
        target_speed = max_speed;
    } else if (dist_to_target > 0.3f) {
        /* 减速区: 线性插值减速 */
        target_speed = NAV_NEAR_SPEED_MPS
                     + (max_speed - NAV_NEAR_SPEED_MPS) * (dist_to_target / SLOW_DOWN_DIST);
    } else {
        /* 极近距离: 最低速度 */
        target_speed = NAV_NEAR_SPEED_MPS;
    }

    /* ---- GPS 精度保护 (纯惯导模式下跳过, 因 pos_std 无 GPS 修正会无限增长) ---- */
#if !INS_ONLY_MODE
    if (g_nav.pos_std > GPS_STD_STOP) {
        /* GPS 信号严重劣化 (可能丢星太久), 停车等待恢复 */
        target_speed = 0.0f;
    } else if (g_nav.pos_std > GPS_STD_WARN) {
        /* GPS 精度下降, 降速行驶 */
        target_speed *= 0.5f;
    }
#endif

    return target_speed;
}

/**
 * steer_control() - Stanley 转向控制器
 * @yaw_error: 航向误差 (rad), 正值=目标在车右侧
 *
 * 返回: 舵机角度 (度)
 *
 * Stanley 控制器同时考虑航向误差和横向偏移:
 *   steer = heading_error - atan2(k * cross_track_error, speed)
 *
 * - 航向误差: 车头朝向与路径方向的偏差
 * - 横向偏差: 车到路径连线的垂直距离 (正=在路径右侧)
 * - speed: 当前车速, 低速时衰减横向修正防止震荡
 */
static float steer_control(float yaw_error)
{
    (void)yaw_error;    /* Stanley 内部自行计算航向误差 */
    float cte, path_yaw;
    nav_compute_crosstrack(&cte, &path_yaw);

    /* 航向误差: 路径方向 - 车头朝向 */
    float heading_err = path_yaw - g_nav.yaw;
    while (heading_err >  (float)M_PI) heading_err -= 2.0f * (float)M_PI;
    while (heading_err < -(float)M_PI) heading_err += 2.0f * (float)M_PI;

    /* 横向修正: atan2(k * cte, speed), 低速时衰减 */
    /* encoder1_get_speed() 返回 10ms 脉冲数, ×10 得 100ms 脉冲数 */
    float speed = fabsf(pulses_to_mps((int32)encoder1_get_speed() * 10));
    if (speed < 0.1f) speed = 0.1f;
    float cte_term = atan2f(STANLEY_K * cte, speed);

    /* Stanley: steer = heading_err - cte_term */
    float steer_rad = heading_err - cte_term;
    float steer_deg = steer_rad * 180.0f / (float)M_PI;

    /* 限幅 ±30° */
    if (steer_deg >  30.0f) steer_deg =  30.0f;
    if (steer_deg < -30.0f) steer_deg = -30.0f;

    return steer_deg;
}

/* =====================================================================
 *  串口角度指令解析 (调试用)
 *  从调试串口读取 "10\n" 这样的字符串, 解析为目标角度值
 *  返回 1 表示成功解析到一个角度, 0 表示暂无数据
 * ===================================================================== */
static uint8 serial_parse_angle(int32 *target_angle)
{
#if DEBUG_UART_USE_INTERRUPT
    static char rx_buf[16];
    static uint8 rx_index = 0;
    uint8 buf[16];
    uint32 len = debug_read_ring_buffer(buf, sizeof(buf));

    for (uint32 i = 0; i < len; i++) {
        uint8 ch = buf[i];
        if (ch == '\r' || ch == '\n') {
            if (rx_index > 0) {
                rx_buf[rx_index] = '\0';
                *target_angle = atoi(rx_buf);
                rx_index = 0;
                return 1;
            }
        } else if (rx_index < sizeof(rx_buf) - 1) {
            rx_buf[rx_index++] = ch;
        }
    }
    return 0;
#else
    (void)target_angle;
    return 0;
#endif
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
    debug_init();                   /* DEBUG UART0: P14_0(TX) / P14_1(RX) */
    wireless_uart_init();
    uart_write_string(DEBUG_UART_INDEX, "\r\n[DEBUG] UART0 P14_0/P14_1 115200 OK\r\n");
    uart_write_string(DEBUG_UART_INDEX, "[WIRELESS] UART3 P15_6/P15_7 115200 OK\r\n");

    /* ================================================================
     *  第2步: 按键 GPIO 初始化
     *  四个按键均为低电平有效, 配置为上拉输入
     *  S3=打点  S4=加速  S5=减速  S6=刹车
     * ================================================================ */
    gpio_init(KEY_MARK,     GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY_SPEED_UP, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY_SLOW_DN,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY_RESET,    GPI, GPIO_HIGH, GPI_PULL_UP);
    uart_write_string(DEBUG_UART_INDEX, "[INIT] GPIO buttons OK (S3=P20_6 S4=P20_7 S5=P11_2 S6=P11_3)\r\n");
    /* 打印按键初始电平, 用于验证接线 (正常应全为1=高电平=释放) */
    uart_write_string(DEBUG_UART_INDEX, "  Button levels: S3=");
    uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_MARK));
    uart_write_string(DEBUG_UART_INDEX, " S4=");
    uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_SPEED_UP));
    uart_write_string(DEBUG_UART_INDEX, " S5=");
    uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_SLOW_DN));
    uart_write_string(DEBUG_UART_INDEX, " S6=");
    uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_RESET));
    uart_write_string(DEBUG_UART_INDEX, " (expect all 1)\r\n");

    /* ================================================================
     *  第3步: GPS 模块初始化
     *  GN42A 模块, UART3 接口, 9600 波特率, P15_6(TX)/P15_7(RX)
     *  初始化后 GPS 模块开始输出 NMEA 数据帧
     * ================================================================ */
    uart_write_string(DEBUG_UART_INDEX, "[INIT] GPS SKIPPED. UART3 is used by wireless uart.\r\n");

    /* ================================================================
     *  第4步: PIT 定时器初始化
     *  Ch0: 100ms 周期 → 调试输出定时 (g_debug_tick)
     *  Ch1: 10ms 周期  → IMU 采样定时 (g_imu_tick, 100Hz)
     * ================================================================ */
    pit_ms_init(DEBUG_PIT_CHANNEL, 100);       /* 100ms */
    pit_ms_init(IMU_PIT_CHANNEL, 10);          /* 10ms = 100Hz */
    uart_write_string(DEBUG_UART_INDEX, "[INIT] PIT timers OK (Ch0=100ms Ch1=10ms)\r\n");

    /* ================================================================
     *  第5步: INS 融合系统初始化
     *  包含: IMU963RA 传感器初始化 + ESKF 滤波器初始化 + 初始位姿设置
     *  ESKF 使用 15 维误差状态: [δp(3) δv(3) δθ(3) δba(3) δbw(3)]
     * ================================================================ */
    uart_write_string(DEBUG_UART_INDEX, "[INIT] INS fusion init (IMU963RA + ESKF)...\r\n");
    fusion_init();
    uart_write_string(DEBUG_UART_INDEX, "[INIT] INS fusion init done.\r\n");

    /* ================================================================
     *  第6步: 电机/舵机/编码器初始化
     * ================================================================ */
    uart_write_string(DEBUG_UART_INDEX, "[INIT] Servo + Motor + Encoder init...\r\n");
    servo_init();                       /* 舵机 PWM 50Hz */
    motor1_init();                      /* 电机1 DRV8701E (DIR+PWM 17kHz) */
    motor2_init();                      /* 电机2 DRV8701E (DIR+PWM 17kHz) */
    motor1_stop();                      /* 确保初始停止 */
    motor2_stop();
    encoder1_init();                    /* 编码器1 正交解码 */
    encoder2_init();                    /* 编码器2 正交解码 */
    pit_ms_init(CCU61_CH0, 10);        /* 编码器采样 PIT 10ms (由 isr.c 处理) */
    angle_control_init();               /* 转向电机初始化 (PWM+编码器+PID) */
    angle_control_set_target(0);        /* 初始目标角度 0° */
    uart_write_string(DEBUG_UART_INDEX, "[INIT] Servo(P33_8 50Hz) + Motor1(P21_2/P21_3) + Motor2(P21_4/P21_5) OK\r\n");
    uart_write_string(DEBUG_UART_INDEX, "[INIT] Encoder1 + Encoder2 + PIT sampling OK\r\n");
    uart_write_string(DEBUG_UART_INDEX, "[INIT] Angle motor (TIM4 P02_8/P00_9 + ATOM0_CH6/CH7) OK\r\n");

    /* ================================================================
     *  第7步: PID 控制器初始化
     * ================================================================ */
    pid_controllers_init();             /* 导航 PID (速度环 + 转向环) */
    motor_pid_init(&g_motor_pid, 10.0f, 0.3f, 0.8f, 13.0f);  /* 电机 PID (载人参数 2026-05-16) */
    {
        ActionTaskDriver action_driver;
        action_driver.set_speed = motor_control_set_speed;
        action_driver.stop = motor_control_stop;
        action_driver.set_steer = steer_motor_set_angle;
        action_driver.get_distance = action_get_total_distance;
        action_driver.get_gyro_z = action_get_gyro_z;
        action_task_init(&action_driver);
    }
    uart_write_string(DEBUG_UART_INDEX, "[INIT] PID controllers OK (speed Kp=1.0 Ki=0.05 Kd=0.1, steer Kp=15 Kd=2, motor Kp=10 Ki=0.3 Kd=0.8 FF=13)\r\n");

    /* ================================================================
     *  第8步: ToF 卡尔曼滤波初始化 (可选)
     * ================================================================ */
    tof_kalman_init();
    uart_write_string(DEBUG_UART_INDEX, "[INIT] ToF Kalman filter OK (q=0.5 r=2.0)\r\n");

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
    uart_write_string(DEBUG_UART_INDEX, "[INIT] Waiting for CPU1 sync...\r\n");
    cpu_wait_event_ready();
    uart_write_string(DEBUG_UART_INDEX, "[INIT] CPU1 ready.\r\n");

    /* ================================================================
     *  初始化完成, 打印使用说明
     * ================================================================ */
    uart_write_string(DEBUG_UART_INDEX,
        "========================================\r\n"
        "  KDKP Action Task Control\r\n"
#if INS_ONLY_MODE
        "  *** INS-ONLY MODE (no GPS) ***\r\n"
#else
        "  ESKF Fusion + Waypoint Navigation\r\n"
#endif
        "========================================\r\n"
        "  UART3 115200 command:\r\n"
        "  1: forward 10m\r\n"
        "  2: backward 10m\r\n"
        "  3: snake forward 10m\r\n"
        "  4: snake backward 10m\r\n"
        "  5: counterclockwise circle\r\n"
        "  6: clockwise circle\r\n"
        "  7: left right-angle turn\r\n"
        "  8: right right-angle turn\r\n"
        "  9: straight\r\n"
        "  S4: Speed up (加速)\r\n"
        "  S5: Slow down (减速)\r\n"
        "  S6: Brake + Reset (刹车重置)\r\n"
        "========================================\r\n");

    /* ================================================================
     *  主循环局部变量
     * ================================================================ */
    uint32 debug_count = 0;         /* 调试输出分频计数器 */
    uint32 imu_tick_count = 0;      /* IMU tick 计数 (用于验证 100Hz 采样) */

    /* ---- 串口命令解析缓冲区 ---- */
    uint8  cmd_byte = 0;            /* 命令接收字节 */

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
            imu_tick_count++;       /* 统计 IMU tick 数 */

            /* 读取 IMU 数据 (加速度 + 角速度, 已扣除零偏) */
            imu_task_read();

            /* ESKF 预测: 用 IMU 数据推算位置/速度/姿态 */
            fusion_imu_predict();

            action_task_update();
        }

        /* ============================================================
         *  ①-2 电机 PID 控制 (100ms 周期, 基于编码器反馈)
         *
         *  编码器由 PIT Ch2 中断 (10ms) 采样, 每次更新 g_encoder1_speed
         *  这里每 10ms 累加一次, 每 100ms 执行一次 PID 计算
         * ============================================================ */
        {
            static uint8 encoder_div = 0;
            static uint32 last_encoder_sample_count = 0;

            if(last_encoder_sample_count != g_encoder_sample_count)
            {
                last_encoder_sample_count = g_encoder_sample_count;

                int16 encoder1_pulses_10ms = encoder1_get_speed();
                g_actual_speed_mps = pulses_to_mps((int32)encoder1_pulses_10ms * 10);
                g_encoder_100ms += encoder1_pulses_10ms;  /* 累加 10ms 脉冲 */
                g_total_dist += fabsf((float)encoder1_pulses_10ms) / (float)EFFECTIVE_PPR * WHEEL_CIRCUM_M;
                encoder_div++;

                if (encoder_div >= 10)  /* 10ms × 10 = 100ms */
                {
                    encoder_div = 0;
                    if (g_motor_pid.enabled) {
                        motor_control_update();  /* PID 计算 + 输出到电机 */
                    }
                }
            }
        }

        if (wireless_uart_read_buffer(&cmd_byte, 1) > 0)
        {
            g_wireless_cmd_rx_count++;
            uart_write_string(DEBUG_UART_INDEX, "[WIRELESS RX] byte=");
            uart_write_integer(DEBUG_UART_INDEX, cmd_byte);
            uart_write_string(DEBUG_UART_INDEX, "\r\n");
            ActionTaskId task_id = action_task_from_command(cmd_byte);
            if (task_id != ACTION_TASK_NONE) {
                g_manual_speed = 0.0f;
                g_speed_limit = 0.0f;
                g_nav_mode = NAV_IDLE;
                if (action_task_start(task_id)) {
                    wireless_uart_send_string("[ACTION] start ");
                    wireless_uart_send_string(action_task_get_name(task_id));
                    wireless_uart_send_string("\r\n");
                    uart_write_string(DEBUG_UART_INDEX, "[ACTION] start ");
                    uart_write_string(DEBUG_UART_INDEX, action_task_get_name(task_id));
                    uart_write_string(DEBUG_UART_INDEX, "\r\n");
                }
            } else if (cmd_byte == '0') {
                action_task_stop();
                wireless_uart_send_string("[ACTION] stop\r\n");
                uart_write_string(DEBUG_UART_INDEX, "[ACTION] stop\r\n");
            }
        }

        /* ============================================================
         *  ③ 按键扫描处理
         *
         *  S3 (打点):  短按 = 记录航点, 长按(>1s) = 启动/停止自动导航
         *  S4 (加速):  目标速度 +0.2 m/s, 空闲时同时驱动电机
         *  S5 (减速):  目标速度 -0.1 m/s, 空闲时可减速到 -5 m/s (倒车)
         *  S6 (刹车):  清空所有航点 + 紧急停车 + 回到空闲
         * ============================================================ */

        /* ---- S3: 短按打点 / 长按启停导航 ---- */
        if (!gpio_get_level(KEY_MARK))
        {
            system_delay_ms(20);                    /* 消抖 */
            if (!gpio_get_level(KEY_MARK))
            {
                uint32 hold_ms = 0;
                while (!gpio_get_level(KEY_MARK))   /* 等待释放 */
                {
                    system_delay_ms(10);
                    hold_ms += 10;
                }

                if (hold_ms >= 1000)
                {
                    /* ---- 长按: 切换导航模式 ---- */
                    if (g_nav_mode == NAV_IDLE && g_waypoint_count == REQUIRED_WAYPOINT_COUNT)
                    {
                        /* 空闲 + 有航点 → 启动自动导航 */
                        g_current_target = 0;
                        g_nav_mode = NAV_RUNNING;
                        pid_reset(&g_speed_pid_state);
                        pid_reset(&g_steer_pid_state);
                        g_stuck_timer = 0;
                        g_stuck_snap_dist = g_total_dist;
                        uart_write_string(DEBUG_UART_INDEX,
                            "[S3] >>> AUTO NAVIGATION START\r\n"
                            "  Waypoints: ");
                        uart_write_integer(DEBUG_UART_INDEX, g_waypoint_count);
                        uart_write_string(DEBUG_UART_INDEX, "\r\n");
                        nav_debug_print();
                    }
                    else if (g_nav_mode == NAV_RUNNING || g_nav_mode == NAV_PARKING)
                    {
                        /* 导航中 → 停止导航, 回空闲 */
                        g_nav_mode = NAV_IDLE;
                        motor_control_stop();
                        steer_motor_set_angle(0.0f);
                        uart_write_string(DEBUG_UART_INDEX,
                            "[S3] >>> NAVIGATION STOPPED\r\n");
                    }
                    else if (g_nav_mode == NAV_IDLE && g_waypoint_count == 0)
                    {
                        uart_write_string(DEBUG_UART_INDEX,
                            "[S3] No waypoints! Press S3 briefly to record first.\r\n");
                    }
                    else if (g_nav_mode == NAV_IDLE && g_waypoint_count < REQUIRED_WAYPOINT_COUNT)
                    {
                        uart_write_string(DEBUG_UART_INDEX,
                            "[S3] Need 4 waypoints: start, cone-in, cone-out, finish.\r\n");
                    }
                }
                else
                {
                    /* ---- 短按: 记录航点 ---- */
                    uint8 cnt = 0;
                    if (g_waypoint_count < REQUIRED_WAYPOINT_COUNT) {
                        cnt = nav_record_waypoint(0); /* type=0: 普通航点 */
                    }
                    if (cnt > 0) {
                        uart_write_string(DEBUG_UART_INDEX, "[S3] Waypoint #");
                        uart_write_integer(DEBUG_UART_INDEX, cnt);
                        uart_write_string(DEBUG_UART_INDEX, " (");
                        uart_write_float(DEBUG_UART_INDEX, g_nav.x);
                        uart_write_string(DEBUG_UART_INDEX, ", ");
                        uart_write_float(DEBUG_UART_INDEX, g_nav.y);
                        uart_write_string(DEBUG_UART_INDEX, ")\r\n");
                    } else {
                        uart_write_string(DEBUG_UART_INDEX, "[S3] 4 waypoints already recorded.\r\n");
                    }
                }
            }
        }

        /* ---- S4: 加速 ---- */
        if (!gpio_get_level(KEY_SPEED_UP))
        {
            system_delay_ms(20);
            if (!gpio_get_level(KEY_SPEED_UP))
            {
                g_speed_limit += MANUAL_SPEED_STEP;
                if (g_nav_mode == NAV_IDLE && g_speed_limit > MANUAL_MAX_SPEED_MPS) g_speed_limit = MANUAL_MAX_SPEED_MPS;
                uart_write_string(DEBUG_UART_INDEX, "[S4] Speed limit = ");
                uart_write_float(DEBUG_UART_INDEX, g_speed_limit);
                uart_write_string(DEBUG_UART_INDEX, " m/s\r\n");

                /* 空闲模式下直接驱动电机 (手动前进) */
                if (g_nav_mode == NAV_IDLE) {
                    g_manual_speed = g_speed_limit;
                    motor_control_set_speed(g_manual_speed);
                }
                while (!gpio_get_level(KEY_SPEED_UP)) { system_delay_ms(10); }
            }
        }

        /* ---- S5: 减速 ---- */
        if (!gpio_get_level(KEY_SLOW_DN))
        {
            system_delay_ms(20);
            if (!gpio_get_level(KEY_SLOW_DN))
            {
                g_speed_limit -= MANUAL_SPEED_STEP;
                if (g_nav_mode == NAV_IDLE) {
                    if (g_speed_limit < MANUAL_MIN_SPEED_MPS) g_speed_limit = MANUAL_MIN_SPEED_MPS;
                } else if (g_speed_limit < 0.0f) {
                    g_speed_limit = 0.0f;
                }
                uart_write_string(DEBUG_UART_INDEX, "[S5] Speed limit = ");
                uart_write_float(DEBUG_UART_INDEX, g_speed_limit);
                uart_write_string(DEBUG_UART_INDEX, " m/s\r\n");

                /* NAV_IDLE: allow negative manual speed for reverse. */
                if (g_nav_mode == NAV_IDLE) {
                    g_manual_speed = g_speed_limit;
                    if (g_manual_speed == 0.0f) {
                        motor_control_stop();
                    } else {
                        motor_control_set_speed(g_manual_speed);
                    }
                }
                while (!gpio_get_level(KEY_SLOW_DN)) { system_delay_ms(10); }
            }
        }

        /* ---- S6: 刹车 (清空航点 + 急停) ---- */
        if (!gpio_get_level(KEY_RESET))
        {
            system_delay_ms(20);
            if (!gpio_get_level(KEY_RESET))
            {
                action_task_stop();
                nav_clear_waypoints();      /* 清空所有航点 */
                g_manual_speed = 0.0f;      /* 清除手动速度 */
                g_speed_limit = 0.0f;  /* 重置速度限制 */
                g_nav_mode = NAV_IDLE;      /* 回到空闲模式 */
                motor_control_stop();       /* 电机停止 */
                steer_motor_set_angle(0.0f);      /* 舵机回中 */
                uart_write_string(DEBUG_UART_INDEX,
                    "[S6] BRAKE! Waypoints cleared, route reset.\r\n");
                while (!gpio_get_level(KEY_RESET)) { system_delay_ms(10); }
            }
        }

        /* 原调试串口命令接收已关闭 */

        /* ============================================================
         *  ④ 调试输出 (1Hz, 每秒输出一次)
         *
         *  输出内容:
         *    - NED 位置 (x, y, z)
         *    - 航向角 yaw (度)
         *    - 位置标准差 pos_std (反映 GPS 精度)
         *    - 自动模式下额外输出: 航点编号、距离、航向误差
         *    - 空闲模式下额外输出: 已记录航点数和速度限制
         * ============================================================ */
        if (g_debug_tick)
        {
            g_debug_tick = 0;       /* 消费标志 */
            debug_count++;

            if (debug_count >= 10)  /* 100ms x 10 = 1秒 */
            {
                debug_count = 0;

                /* ---- 卡死检测 (仅在自动/倒车模式下) ---- */
                if (g_nav_mode == NAV_RUNNING || g_nav_mode == NAV_PARKING) {
                    g_stuck_timer++;
                    if (g_stuck_timer >= STUCK_CHECK_SEC) {
                        g_stuck_timer = 0;
                        float dist_delta = g_total_dist - g_stuck_snap_dist;
                        if (dist_delta < STUCK_MIN_DIST) {
                            /* 30秒内行驶不到1m, 判定卡死 */
                            g_nav_mode = NAV_IDLE;
                            motor_control_stop();
                            steer_motor_set_angle(0.0f);
                            uart_write_string(DEBUG_UART_INDEX,
                                "[SAFETY] STUCK! No movement for 30s. STOP.\r\n");
                        }
                        g_stuck_snap_dist = g_total_dist;
                    }
                } else {
                    g_stuck_timer = 0;
                    g_stuck_snap_dist = g_total_dist;
                }

                /* ---- 导航状态输出 ---- */
                uart_write_string(DEBUG_UART_INDEX, "--- NAV STATUS ---\r\n");

                /* 导航模式 */
                uart_write_string(DEBUG_UART_INDEX, "  Mode: ");
                if (g_nav_mode == NAV_IDLE)      uart_write_string(DEBUG_UART_INDEX, "IDLE");
                if (g_nav_mode == NAV_RUNNING)   uart_write_string(DEBUG_UART_INDEX, "RUNNING");
                if (g_nav_mode == NAV_PARKING)   uart_write_string(DEBUG_UART_INDEX, "PARKING");
                if (g_nav_mode == NAV_FINISHED)  uart_write_string(DEBUG_UART_INDEX, "FINISHED");
                uart_write_string(DEBUG_UART_INDEX, "\r\n");

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

                /* 累计行驶距离 */
                uart_write_string(DEBUG_UART_INDEX, "  Dist=");
                uart_write_float(DEBUG_UART_INDEX, g_total_dist);
                uart_write_string(DEBUG_UART_INDEX, "m\r\n");

                uart_write_string(DEBUG_UART_INDEX, "  Wireless: uart3_isr=");
                uart_write_integer(DEBUG_UART_INDEX, g_wireless_uart3_rx_isr_count);
                uart_write_string(DEBUG_UART_INDEX, " main_rx=");
                uart_write_integer(DEBUG_UART_INDEX, g_wireless_cmd_rx_count);
                uart_write_string(DEBUG_UART_INDEX, "\r\n");

                fusion_debug_mag();

                /* 自动模式: 输出航点追踪详情 */
                if (g_nav_mode == NAV_RUNNING) {
                    float dist, yaw_err;
                    nav_compute_errors(&dist, &yaw_err);
                    uart_write_string(DEBUG_UART_INDEX, "  WP[");
                    uart_write_integer(DEBUG_UART_INDEX, g_current_target);
                    uart_write_string(DEBUG_UART_INDEX, "/");
                    uart_write_integer(DEBUG_UART_INDEX, g_waypoint_count);
                    uart_write_string(DEBUG_UART_INDEX, "] dist=");
                    uart_write_float(DEBUG_UART_INDEX, dist);
                    uart_write_string(DEBUG_UART_INDEX, "m yaw=");
                    uart_write_float(DEBUG_UART_INDEX, yaw_err * 180.0f / 3.14159f);
                    uart_write_string(DEBUG_UART_INDEX, "deg\r\n");
                }

                /* 空闲模式: 显示已记录航点数和速度限制 */
                if (g_nav_mode == NAV_IDLE) {
                    uart_write_string(DEBUG_UART_INDEX, "  Waypoints: ");
                    uart_write_integer(DEBUG_UART_INDEX, g_waypoint_count);
                    uart_write_string(DEBUG_UART_INDEX, "/20  SpeedLimit=");
                    uart_write_float(DEBUG_UART_INDEX, g_speed_limit);
                    uart_write_string(DEBUG_UART_INDEX, "m/s\r\n");
                }

                /* 倒车入库状态 */
                if (g_nav_mode == NAV_PARKING) {
                    uint8 ps = nav_get_park_state();
                    uart_write_string(DEBUG_UART_INDEX, "  [PARK] state=");
                    uart_write_integer(DEBUG_UART_INDEX, ps);
                    if (ps == 0) uart_write_string(DEBUG_UART_INDEX, " (STOP)");
                    else if (ps == 1) uart_write_string(DEBUG_UART_INDEX, " (REVERSE)");
                    else uart_write_string(DEBUG_UART_INDEX, " (DONE)");
                    uart_write_string(DEBUG_UART_INDEX, "\r\n");
                }

                /* 电机 PID 状态 */
                uart_write_string(DEBUG_UART_INDEX, "  Motor: tgt=");
                uart_write_float(DEBUG_UART_INDEX, g_motor_pid.target);
                uart_write_string(DEBUG_UART_INDEX, " fb=");
                uart_write_float(DEBUG_UART_INDEX, (float)encoder1_get_speed());
                uart_write_string(DEBUG_UART_INDEX, " pwm=");
                uart_write_float(DEBUG_UART_INDEX, (float)g_motor_pid.output);
                uart_write_string(DEBUG_UART_INDEX, " spd=");
                uart_write_float(DEBUG_UART_INDEX, pulses_to_mps(encoder1_get_speed()));
                uart_write_string(DEBUG_UART_INDEX, "m/s\r\n");

                /* 转向电机角度 */
                uart_write_string(DEBUG_UART_INDEX, "  Angle: cur=");
                uart_write_integer(DEBUG_UART_INDEX, angle_control_get_current_angle());
                uart_write_string(DEBUG_UART_INDEX, " deg\r\n");

                /* ---- IMU 采样率 (应约 100/s) ---- */
                uart_write_string(DEBUG_UART_INDEX, "  IMU: ticks/s=");
                uart_write_integer(DEBUG_UART_INDEX, imu_tick_count);
                uart_write_string(DEBUG_UART_INDEX, " (expect ~100)\r\n");
                imu_tick_count = 0;     /* 重置计数器 */

                /* ---- 编码器原始值 ---- */
                uart_write_string(DEBUG_UART_INDEX, "  Enc: enc1=");
                uart_write_integer(DEBUG_UART_INDEX, encoder1_get_speed());
                uart_write_string(DEBUG_UART_INDEX, " enc2=");
                uart_write_integer(DEBUG_UART_INDEX, encoder2_get_speed());
                uart_write_string(DEBUG_UART_INDEX, " (10ms pulses)\r\n");

                /* ---- 按键 GPIO 电平 (检测是否有按键卡住) ---- */
                uart_write_string(DEBUG_UART_INDEX, "  BTN: S3=");
                uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_MARK));
                uart_write_string(DEBUG_UART_INDEX, " S4=");
                uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_SPEED_UP));
                uart_write_string(DEBUG_UART_INDEX, " S5=");
                uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_SLOW_DN));
                uart_write_string(DEBUG_UART_INDEX, " S6=");
                uart_write_integer(DEBUG_UART_INDEX, gpio_get_level(KEY_RESET));
                uart_write_string(DEBUG_UART_INDEX, " (1=released 0=pressed)\r\n");

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
