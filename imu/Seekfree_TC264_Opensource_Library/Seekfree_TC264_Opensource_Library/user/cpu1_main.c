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

#pragma section all "cpu1_dsram"    /* 以下变量放入 CPU1 的 DSRAM 中 */

/* =====================================================================
 *  功能开关宏定义
 *  根据你的硬件配置启用/禁用各项功能
 * ===================================================================== */

/* 是否启用 IPS200 屏幕显示 */
/* 0 = 禁用 (不接屏幕时) */
/* 1 = 启用 (接了 IPS200 2.0寸 SPI 屏幕时) */
#define ENABLE_IPS200_DISPLAY       (0)

/* 是否启用无线模块数据转发 */
/* 0 = 禁用 */
/* 1 = 启用 (通过 UART2 发送导航状态到上位机/遥控器) */
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
    ips200_set_color(RGB565_BLACK, RGB565_WHITE);   /* 黑底白字 */
    ips200_set_font(IPS200_6X8_FONT);               /* 6x8 小字体 */
    ips200_set_dir(IPS200_PORTAIT);                 /* 竖屏 */
    ips200_init(IPS200_TYPE_SPI);                   /* SPI 通信 */
    ips200_clear();                                  /* 清屏 */

    /* 显示标题 */
    ips200_show_string(0, 0,  "KDKP Subject 1");
    ips200_show_string(0, 12, "INS Fusion Nav");
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
    char buf[42];                              /* 行缓冲区 */
    static uint8 last_mode = 0xFF;             /* 上次模式, 用于变化检测 */
    static uint8 last_target = 0xFF;           /* 上次目标航点 */

    /* ---- 导航模式 ---- */
    const char *mode_names[] = {"IDLE ", "TEACH", "RUN  ", "DONE "};
    uint8 mode = (uint8)g_nav_mode;
    if (mode > 3) mode = 0;
    if (mode != last_mode) {
        last_mode = mode;
        sprintf(buf, "Mode: %s", mode_names[mode]);
        ips200_show_string(0, 28, buf);
    }

    /* ---- NED 位置 ---- */
    sprintf(buf, "X:%7.2f Y:%7.2f", g_nav.x, g_nav.y);
    ips200_show_string(0, 40, buf);

    /* ---- 航向角 ---- */
    float yaw_deg = g_nav.yaw * 180.0f / 3.14159f;
    sprintf(buf, "Yaw: %6.1f deg  ", yaw_deg);
    ips200_show_string(0, 52, buf);

    /* ---- GPS 精度 ---- */
    sprintf(buf, "Std:%5.2fm GPS:%s", g_nav.pos_std, g_nav.gps_valid ? "OK" : "--");
    ips200_show_string(0, 64, buf);

    /* ---- 航点信息 (教学/自动模式) ---- */
    if (g_nav_mode == NAV_TEACHING || g_nav_mode == NAV_RUNNING) {
        sprintf(buf, "WP:%2d/%2d Tgt:%2d  ", g_waypoint_count, 20, g_current_target);
        ips200_show_string(0, 80, buf);

        /* 自动模式: 显示到目标距离 */
        if (g_nav_mode == NAV_RUNNING && g_current_target < g_waypoint_count) {
            float dx = g_waypoints[g_current_target].x - g_nav.x;
            float dy = g_waypoints[g_current_target].y - g_nav.y;
            float dist = sqrtf(dx * dx + dy * dy);
            sprintf(buf, "Dist: %5.2f m    ", dist);
            ips200_show_string(0, 92, buf);
        }
    }
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

    /* 通过 UART2 发送 (无线模块) */
    uart_write_buffer(UART2_INDEX, (uint8 *)buf, (uint32)len);
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
