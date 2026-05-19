/*********************************************************************************************************************
 * 文件名称: cpu1_main.c
 * 功能描述: 第21届智能车竞赛 - 卡丁快跑组 - 科目1 自动驾驶 辅助核心 (CPU1)
 *
 * TC264 是双核处理器:
 *   CPU0 (Core0): 负责主控制逻辑 (传感器融合 + 导航 + 电机控制)
 *   CPU1 (Core1): 负责辅助任务 (屏幕显示 + 无线通信 + 系统监控)
 *
 * 两个核心通过共享全局变量通信:
 *   - g_nav        (NavState):  融合后的位置/速度/姿态
 *   - g_nav_mode   (NavMode):   当前导航模式 (空闲/教学/自动/完成)
 *   - g_waypoints[] (Waypoint): 航点数组
 *   - g_current_target (uint8): 当前目标航点索引
 *   - g_waypoint_count (uint8): 已记录航点数
 *
 * 注意: 无 RTOS 环境下, 共享变量的读写在单次主循环迭代中是"伪原子"的
 *       (CPU0 只在主循环中写, CPU1 只读, 不会出现竞争)
 *
 * 屏幕显示内容:
 *   ┌──────────────────────┐
 *   │ KDKP Subject 1       │  标题
 *   │ Mode: RUN            │  当前模式
 *   │ X: 12.34 Y: -5.67   │  NED 位置 (m)
 *   │ Yaw: 45.3 deg        │  航向角 (度)
 *   │ Std: 0.45m GPS: OK   │  GPS 精度 + 状态
 *   │ WP: 5/8  Tgt: 3      │  航点信息
 *   │ Dist: 2.15 m         │  到目标距离
 *   └──────────────────────┘
 *
 * 修改记录:
 *   2026-05-02  新增屏幕显示 + 无线转发 + 系统监控
 ********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "fusion_task.h"            /* NavState, g_nav */
#include "nav_controller.h"         /* Waypoint, g_nav_mode, g_waypoints 等 */
#include "code/subject 2/action_task.h"
#include <math.h>

extern float g_total_dist;
extern float g_target_speed_mps;
extern float g_actual_speed_mps;
extern volatile uint32 g_wireless_uart3_rx_isr_count;
extern volatile uint32 g_wireless_cmd_rx_count;

#pragma section all "cpu1_dsram"    /* 以下变量放入 CPU1 的 DSRAM 中 */

/* =====================================================================
 *  功能开关宏定义
 *  根据你的硬件配置启用/禁用各项功能
 * ===================================================================== */

/* 是否启用 IPS200 屏幕显示 */
/* 0 = 禁用 (不接屏幕时) */
/* 1 = 启用 (接了 IPS200 2.0寸 SPI 屏幕时) */
#define ENABLE_IPS200_DISPLAY       (1)

/* 是否启用无线模块数据转发 */
/* 0 = 禁用 */
/* 1 = 启用 (通过无线串口发送导航状态到上位机/遥控器) */
#define ENABLE_WIRELESS_FORWARD     (0)

/* 屏幕刷新间隔 (ms), 200ms = 5Hz */
#define SCREEN_REFRESH_INTERVAL_MS  (200)

/* 无线发送间隔 (ms), 500ms = 2Hz */
#define WIRELESS_SEND_INTERVAL_MS   (500)

/* =====================================================================
 *  IPS200 屏幕显示相关函数 (条件编译)
 * ===================================================================== */

#if ENABLE_IPS200_DISPLAY

/**
 * display_init() - 初始化 IPS200 屏幕
 *
 * 配置: 黑底白字, 6x8 小字体, 竖屏显示, SPI 通信
 * 初始化后显示标题
 */
static void display_init(void)
{
    ips200_set_dir(IPS200_PORTAIT);                 /* 竖屏 */
    ips200_set_color(RGB565_RED, RGB565_BLACK);
    ips200_init(IPS200_TYPE_SPI);                   /* SPI 通信 */
    ips200_clear();                                  /* 清屏 */

    ips200_set_font(IPS200_16X16_FONT);
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(0, 0, "ACTION MONITOR ");
    ips200_set_font(IPS200_8X16_FONT);
    ips200_set_color(RGB565_RED, RGB565_BLACK);
    ips200_show_string(0, 24, "DIST:");
    ips200_show_string(0, 48, "ANG :");
    ips200_show_string(0, 72, "WIRE:");
    ips200_show_string(0, 96, "VREL:");
    ips200_show_string(0, 120, "TSPD:");
    ips200_show_string(0, 144, "ASPD:");
    ips200_show_string(0, 168, "CMD :");
}

/**
 * display_update() - 更新屏幕显示内容
 *
 * 显示内容:
 *   第3行: 导航模式 (IDLE/TEACH/RUN/DONE)
 *   第4行: NED 位置 X, Y
 *   第5行: 航向角 Yaw (度)
 *   第6行: GPS 精度 Std + GPS 状态
 *   第7行: 航点编号/总数 + 目标编号 (教学/自动模式)
 *   第8行: 到目标距离 (自动模式)
 *
 * 优化: 仅在数据变化时刷新对应行, 减少屏幕闪烁
 */
static void display_update(void)
{
    float relative_speed = sqrtf(g_nav.vx * g_nav.vx + g_nav.vy * g_nav.vy);

    ips200_set_font(IPS200_16X16_FONT);
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(0, 0, "ACTION MONITOR ");

    ips200_set_font(IPS200_8X16_FONT);
    ips200_set_color(RGB565_RED, RGB565_BLACK);

    ips200_show_string(0, 24, "DIST:");
    ips200_show_string(40, 24, "                ");
    ips200_show_float(40, 24, g_total_dist, 4, 2);
    ips200_show_string(96, 24, "m");

    ips200_show_string(0, 48, "ANG :");
    ips200_show_string(40, 48, "                ");
    ips200_show_float(40, 48, action_task_get_turn_angle_deg(), 3, 1);
    ips200_show_string(96, 48, "deg");

    ips200_show_string(0, 72, "WIRE:");
    ips200_show_string(40, 72, "                ");
    ips200_show_uint(40, 72, g_wireless_uart3_rx_isr_count, 5);

    ips200_show_string(0, 96, "VREL:");
    ips200_show_string(40, 96, "                ");
    ips200_show_float(40, 96, relative_speed, 3, 2);
    ips200_show_string(96, 96, "m/s");

    ips200_show_string(0, 120, "TSPD:");
    ips200_show_string(40, 120, "                ");
    ips200_show_float(40, 120, g_target_speed_mps, 2, 2);
    ips200_show_string(96, 120, "m/s");

    ips200_show_string(0, 144, "ASPD:");
    ips200_show_string(40, 144, "                ");
    ips200_show_float(40, 144, g_actual_speed_mps, 2, 2);
    ips200_show_string(96, 144, "m/s");

    ips200_show_string(0, 168, "CMD :");
    ips200_show_string(40, 168, "                ");
    ips200_show_uint(40, 168, g_wireless_cmd_rx_count, 5);
}

#endif /* ENABLE_IPS200_DISPLAY */

/* =====================================================================
 *  无线模块数据转发函数 (条件编译)
 * ===================================================================== */

#if ENABLE_WIRELESS_FORWARD

/**
 * wireless_send_status() - 通过无线模块发送当前导航状态
 *
 * 发送协议: CSV 文本格式
 *   NAV,x,y,yaw_deg,pos_std,mode,target,count\r\n
 *
 * 示例:
 *   NAV,12.34,-5.67,45.3,0.45,2,3,8\r\n
 *
 * 上位机/遥控器接收到后可实时显示车辆位置和状态
 */
static void wireless_send_status(void)
{
    char buf[80];
    int len = sprintf(buf, "NAV,%.2f,%.2f,%.1f,%.2f,%d,%d,%d\r\n",
                      g_nav.x,
                      g_nav.y,
                      g_nav.yaw * 180.0f / 3.14159f,
                      g_nav.pos_std,
                      (int)g_nav_mode,
                      g_current_target,
                      g_waypoint_count);

    /* 通过无线模块发送 */
    wireless_uart_send_buffer((uint8 *)buf, (uint32)len);
}

#endif /* ENABLE_WIRELESS_FORWARD */

/* =====================================================================
 *  core1_main() - CPU1 主函数
 *
 *  初始化:
 *    1. 关闭看门狗
 *    2. 开启全局中断
 *    3. 初始化屏幕 (如启用)
 *    4. 等待 CPU0 初始化完成
 *
 *  主循环 (10Hz):
 *    1. 屏幕刷新 (每 200ms)
 *    2. 无线发送 (每 500ms)
 *    3. 系统监控 (可扩展)
 * ===================================================================== */
void core1_main(void)
{
    /* ================================================================
     *  初始化
     * ================================================================ */
    disable_Watchdog();                     /* 关闭看门狗 (CPU1 需要手动关闭) */
    interrupt_global_enable(0);             /* 开启全局中断 */

    /* ---- 屏幕初始化 ---- */
#if ENABLE_IPS200_DISPLAY
    display_init();
#endif

    /* ---- 等待 CPU0 初始化完成 ---- */
    /* cpu_wait_event_ready() 会阻塞, 直到所有核心都调用了这个函数 */
    /* 这确保了 CPU0 的传感器初始化、PID 初始化等在主循环开始前完成 */
    cpu_wait_event_ready();

    /* ================================================================
     *  主循环变量
     * ================================================================ */
    uint32 loop_count = 0;                  /* 循环计数器 (用于分频) */

    /* ================================================================
     *  主循环 (约 10ms 周期, 即 100Hz)
     * ================================================================ */
    while (TRUE)
    {
        /* ============================================================
         *  屏幕显示刷新 (每 SCREEN_REFRESH_INTERVAL_MS 毫秒)
         *  默认 200ms = 5Hz, 避免刷新太快导致屏幕闪烁
         * ============================================================ */
#if ENABLE_IPS200_DISPLAY
        if ((loop_count % (SCREEN_REFRESH_INTERVAL_MS / 10)) == 0)
        {
            display_update();
        }
#endif

        /* ============================================================
         *  无线数据转发 (每 WIRELESS_SEND_INTERVAL_MS 毫秒)
         *  默认 500ms = 2Hz, 节省无线带宽
         * ============================================================ */
#if ENABLE_WIRELESS_FORWARD
        if ((loop_count % (WIRELESS_SEND_INTERVAL_MS / 10)) == 0)
        {
            wireless_send_status();
        }
#endif

        /* ============================================================
         *  系统健康监控 (可扩展)
         *
         *  可以在这里添加:
         *    - 电池电压检测 (ADC 读取, 低于阈值报警)
         *    - CPU0 心跳检测 (检查 g_imu_tick 是否正常翻转)
         *    - 传感器通信超时检测
         *    - 温度监控
         * ============================================================ */

        /* ---- 循环计数器递增 ---- */
        loop_count++;

        /* ---- 主循环延时 10ms ---- */
        system_delay_ms(10);

    } /* while(TRUE) */
}

#pragma section all restore
