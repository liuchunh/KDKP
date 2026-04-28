/**
 * cpu0_main.c - INS Fusion 项目主入口
 *
 * 基于 Seekfree TC264 通用开源库模板
 * 实现 GPS + IMU963RA ESKF 融合定位
 *
 * 任务调度:
 *   - PIT0 (1ms):  系统心跳
 *   - PIT1 (10ms): IMU 数据采集 + ESKF 预测 (100Hz)
 *   - 主循环:      GPS 帧处理 + ESKF 更新 (10Hz), 调试输出
 */

#include "zf_common_headfile.h"
#include "zf_device_gnss.h"
#include "zf_device_imu963ra.h"

#include "ins_solver.h"
#include "imu_task.h"
#include "fusion_task.h"

/* PIT 定时器定义 */
#define IMU_PIT_CHANNEL     (CCU60_CH1)        /* PIT 通道1: 10ms IMU 中断 */
#define DEBUG_PIT_CHANNEL   (CCU60_CH0)        /* PIT 通道0: 100ms 调试输出中断 */

/* 全局标志 — ISR 与主循环之间通信 */
volatile uint8 g_imu_tick  = 0;     /* PIT 置位, 主循环清零 */
volatile uint8 g_debug_tick = 0;

#pragma section all "cpu0_dsram"

/**
 * core0_main - TC264 CPU0 主函数
 */
int core0_main(void) {
    /* ---- 系统初始化 ---- */
    clock_init();
    debug_init();

    /* ---- 外设初始化 ---- */
    /* 1. GPS 初始化 (UART3, 115200, 中断接收) */
    gnss_init(GN42A);

    /* 2. PIT 定时器初始化 */
    /* PIT Ch0: 100ms 周期, 调试输出 — pit_ms_init(通道, 毫秒) */
    pit_ms_init(DEBUG_PIT_CHANNEL, 100);
    /* PIT Ch1: 10ms 周期, 即 IMU 读取 + ESKF 预测 100Hz */
    pit_ms_init(IMU_PIT_CHANNEL, 10);

    /* ---- 融合系统初始化 ---- */
    fusion_init();

    /* ---- 等待所有核心就绪 ---- */
    cpu_wait_event_ready();

    uint8 led_state = 0;
    uint32 main_loop_count = 0;

    /* ---- 主循环 ---- */
    while (TRUE) {
        /* --- IMU 预测 (100Hz, 由 PIT 中断触发) --- */
        if (g_imu_tick) {
            g_imu_tick = 0;

            /* 读取 IMU 数据 */
            imu_task_read();

            /* ESKF 预测一步 */
            fusion_imu_predict();
        }

        /* --- GPS 更新 (在新帧到达时) --- */
        if (gnss_flag) {
            fusion_gps_update();

            /* GPS 更新后点亮 LED 指示 */
            led_state ^= 1;
            /* gpio_set_level(LED_PIN, led_state); */
        }

        /* --- 调试输出 (100ms 一次) --- */
        if (g_debug_tick) {
            g_debug_tick = 0;

            /* 每秒输出一次导航状态 */
            if (++main_loop_count >= 10) {
                main_loop_count = 0;
                fusion_debug_print();
            }
        }

        /* --- 用户按键操作 (可选) --- */
        /* 长按 K1: 重新标定 IMU 零偏 */
        /* 长按 K2: 记录当前 GPS 点为基准点 */

        system_delay_ms(1);
    }
}

#pragma section all restore

/* ========================= 以下为兼容旧代码 ========================= */
void IPS200_Show_Init(void) {
    /* IPS200 屏幕初始化 — 如有屏幕可启用 */
}
