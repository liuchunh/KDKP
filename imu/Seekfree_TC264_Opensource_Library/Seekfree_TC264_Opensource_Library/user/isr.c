/*********************************************************************************************************************
 * 文件名称: isr.c
 * 功能描述: 中断服务程序 - 科目1 自动驾驶
 *
 * 本文件定义了 TC264 所有中断的服务函数 (ISR)
 * 基于 Seekfree 逐飞开源库的 isr.c 模板修改
 *
 * 中断分配:
 *   ┌─────────────────┬──────────┬──────────────────────────────┐
 *   │ 中断源           │ 频率/触发 │ 用途                         │
 *   ├─────────────────┼──────────┼──────────────────────────────┤
 *   │ PIT Ch0 (CCU60) │ 100ms    │ 调试输出定时 (g_debug_tick)   │
 *   │ PIT Ch1 (CCU60) │ 10ms     │ IMU 采样定时 (g_imu_tick)     │
 *   │ PIT Ch2 (CCU61) │ 保留     │ —                            │
 *   │ PIT Ch3 (CCU61) │ 保留     │ —                            │
 *   │ ERU Ch0         │ 外部触发  │ IMU660RC INT (如换用660系列)  │
 *   │ ERU Ch1         │ 外部触发  │ ToF 测距模块 INT              │
 *   │ ERU Ch3         │ 外部触发  │ 摄像头 VSYNC                  │
 *   │ DMA Ch5         │ 传输完成  │ 摄像头 DMA                    │
 *   │ UART0 RX        │ 数据到达  │ 调试串口接收                   │
 *   │ UART1 RX        │ 数据到达  │ 预留 (摄像头/其他)             │
 *   │ UART2 RX        │ 数据到达  │ 无线模块接收                   │
 *   │ UART3 RX        │ 数据到达  │ GPS/GNSS 数据接收             │
 *   └─────────────────┴──────────┴──────────────────────────────┘
 *
 * 注意事项:
 *   1. TC264 默认不支持中断嵌套, 如需嵌套需在 ISR 中手动调用 interrupt_global_enable(0)
 *   2. ISR 中应尽量少做工作, 只置标志位, 实际处理在主循环中完成
 *   3. 未使用的中断保留了接口, 需要时取消注释即可
 *
 * 修改记录:
 *   2026-05-02  基于 INS_Fusion_Project isr.c, 整合 IMU+GPS+按键
 ********************************************************************************************************************/

#include "isr_config.h"
#include "isr.h"
#include "zf_device_gnss.h"         /* GPS/GNSS 模块回调函数 */

/* ---- 从 cpu0_main.c 引用的全局标志 ---- */
extern volatile uint8 g_imu_tick;       /* IMU 采样定时标志 */
extern volatile uint8 g_debug_tick;     /* 调试输出定时标志 */

/* ---- 编码器中断更新函数 (encoder.c) ---- */
extern void encoder_irq_update(void);

/* =====================================================================
 *  PIT 定时器中断
 *  PIT (Programmable Interrupt Timer) 是 TC264 的硬件定时器
 *  每个通道独立计数, 到达设定周期后触发中断
 * ===================================================================== */

/**
 * PIT Ch0 中断: 调试输出定时器 (100ms 周期, 10Hz)
 *
 * 作用: 每 100ms 置 g_debug_tick = 1, 主循环检测到后输出调试信息
 * 实际调试输出频率 = 100ms x 10 (分频) = 1Hz (每秒一次)
 */
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);         /* 允许中断嵌套 */
    pit_clear_flag(CCU60_CH0);          /* 清除中断标志 (必须! 否则会反复触发) */
    g_debug_tick = 1;                   /* 置标志, 主循环消费 */
}

/**
 * PIT Ch1 中断: IMU 采样定时器 (10ms 周期, 100Hz)
 *
 * 作用: 每 10ms 置 g_imu_tick = 1, 主循环检测到后:
 *   1) 调用 imu_task_read() 读取 IMU963RA 数据
 *   2) 调用 fusion_imu_predict() 执行 ESKF 预测
 *   3) 如在自动模式, 执行导航 PID 控制
 *
 * 为什么不在 ISR 中直接读 IMU?
 *   因为 IMU 读取 (SPI 通信) + ESKF 计算耗时约 250us,
 *   在 ISR 中执行太久会阻塞其他中断. 所以只置标志, 主循环处理.
 */
IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU60_CH1);
    g_imu_tick = 1;                     /* 置标志, 主循环消费 */
}

/**
 * PIT Ch2 中断 (CCU61_CH0): 编码器更新 (10ms周期)
 * 自动读取编码器脉冲数并清零
 */
IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH0);
    encoder_irq_update();
}

/**
 * PIT Ch3 中断 (CCU61_CH1): 保留
 */
IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH1);
    /* 用户代码区 */
}

/* =====================================================================
 *  外部中断 (ERU)
 *  ERU (External Request Unit) 用于响应外部引脚的电平变化
 *  常用于传感器的 INT (中断) 引脚
 * ===================================================================== */

/**
 * ERU Ch0/Ch4 中断
 *
 * Ch0 (P15_4): IMU660RC 的 INT 引脚
 *   如果你使用的是 IMU660RC 而非 IMU963RA, 取消注释 imu660rc_callback()
 *   IMU963RA 使用 SPI 主动读取, 不需要外部中断
 *
 * Ch4 (P15_5): 保留
 */
IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);

    if (exti_flag_get(ERU_CH0_REQ0_P15_4)) {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);
        /* ---- IMU660RC 外部中断回调 ---- */
        /* 如使用 IMU660RC (而非 IMU963RA), 取消下面的注释: */
        /* imu660rc_callback(); */
    }

    if (exti_flag_get(ERU_CH4_REQ13_P15_5)) {
        exti_flag_clear(ERU_CH4_REQ13_P15_5);
        /* 用户代码区 */
    }
}

/**
 * ERU Ch1/Ch5 中断
 *
 * Ch1 (P14_3): ToF 测距模块的 INT 引脚
 *   ToF 模块测量完成后触发中断, 通知 CPU 读取距离数据
 *   如使用 ToF 模块, 取消注释 tof_module_exti_handler()
 *
 * Ch5 (P15_8): 保留
 */
IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);

    if (exti_flag_get(ERU_CH1_REQ10_P14_3)) {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);
        /* ---- ToF 测距模块中断回调 ---- */
        /* 如使用 ToF 模块, 取消下面的注释: */
        /* tof_module_exti_handler(); */
    }

    if (exti_flag_get(ERU_CH5_REQ1_P15_8)) {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);
        /* 用户代码区 */
    }
}

/**
 * ERU Ch3/Ch7 中断
 *
 * Ch3 (P02_0): 摄像头 VSYNC (场同步) 信号
 *   摄像头每输出一帧图像, VSYNC 引脚产生一个脉冲
 *   如使用摄像头, 取消注释 camera_vsync_handler()
 *
 * Ch7 (P15_1): 保留
 */
IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);

    if (exti_flag_get(ERU_CH3_REQ6_P02_0)) {
        exti_flag_clear(ERU_CH3_REQ6_P02_0);
        /* ---- 摄像头 VSYNC 回调 ---- */
        /* 如使用摄像头, 取消下面的注释: */
        /* camera_vsync_handler(); */
    }

    if (exti_flag_get(ERU_CH7_REQ16_P15_1)) {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);
        /* 用户代码区 */
    }
}

/* =====================================================================
 *  DMA 中断
 *  DMA (Direct Memory Access) 用于高速数据传输, 不占用 CPU
 *  摄像头图像数据通过 DMA 从外设搬运到内存
 * ===================================================================== */

/**
 * DMA Ch5 中断: 摄像头 DMA 传输完成
 * 一帧图像数据全部搬运到内存后触发
 * 如使用摄像头, 取消注释 camera_dma_handler()
 */
IFX_INTERRUPT(dma_ch5_isr, 0, DMA_INT_PRIO)
{
    interrupt_global_enable(0);
    /* ---- 摄像头 DMA 回调 ---- */
    /* 如使用摄像头, 取消下面的注释: */
    /* camera_dma_handler(); */
}

/* =====================================================================
 *  串口中断 (UART)
 *  TC264 有 4 组 UART (UART0~UART3), 每组有 TX/RX/Error 三个中断
 * ===================================================================== */

/* ---- UART0: GPS/GNSS 模块 ---- */

/* UART0 发送完成中断 (一般不需要处理) */
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);
    /* 发送完成, 无需处理 */
}

/**
 * UART0 接收中断: GPS/GNSS 数据接收
 *
 * GPS 模块通过 UART0 (P14_0/P14_1) 以 115200 波特率发送 NMEA 数据帧
 * gnss_uart_callback() 逐字节接收, 组帧, 校验
 */
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    gnss_uart_callback();               /* GPS 数据接收回调 */
}

/* ---- UART1: 预留 (摄像头串口/其他) ---- */

IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}

/**
 * UART1 接收中断
 * 默认连接摄像头串口, 如使用摄像头取消注释
 */
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    /* ---- 摄像头串口回调 ---- */
    /* camera_uart_handler(); */
}

/* ---- UART2: 无线模块 ---- */

IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}

/**
 * UART2 接收中断: 无线模块数据接收
 * 如使用无线模块 (如蓝牙、WiFi), 取消注释
 */
IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    /* ---- 无线模块回调 ---- */
    /* wireless_module_uart_handler(); */
}

/* ---- UART3: 调试串口 ---- */

IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}

/**
 * UART3 接收中断: 调试串口数据接收
 *
 * 调试串口通过 UART3 (P15_6/P15_7) 输出导航状态等信息
 */
IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);
#if DEBUG_UART_USE_INTERRUPT
    debug_interrupr_handler();          /* Seekfree 库的调试串口接收处理 */
#endif
}

/* =====================================================================
 *  串口错误处理中断
 *  当串口发生帧错误、校验错误、溢出等异常时触发
 *  一般只需调用 Seekfree 库的错误处理函数即可
 * ===================================================================== */

IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart0_handle);
}

IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart1_handle);
}

IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart2_handle);
}

IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart3_handle);
}
