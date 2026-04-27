/**
 * isr.c - 中断服务程序
 *
 * 在 Seekfree 原版 isr.c 基础上修改:
 *   - 增加 IMU 数据采集 PIT 中断
 *   - 保留 GNSS UART3 中断
 *   - 保留其他外设中断 (摄像头、无线模块等, 使用时可取消注释)
 */

#include "isr_config.h"
#include "isr.h"
#include "zf_device_gnss.h"

/* 外部标志 (cpu0_main.c 中定义) */
extern volatile uint8 g_imu_tick;
extern volatile uint8 g_debug_tick;

/* ========================= PIT 定时器中断 ========================= */

/* PIT Ch0: 调试输出定时器 */
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU60_CH0);
    g_debug_tick = 1;
}

/* PIT Ch1: IMU 采集 + ESKF 预测定时器 (100Hz) */
IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU60_CH1);
    g_imu_tick = 1;
}

/* PIT Ch2/Ch3: 保留, 如需其他定时任务可在此实现 */
IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH0);
}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH1);
}

/* ========================= 外部中断 ========================= */

IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);
    if(exti_flag_get(ERU_CH0_REQ0_P15_4)) {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);
        /* IMU660RC callback — 如使用660系列IMU而非963RA可启用 */
        /* imu660rc_callback(); */
    }
    if(exti_flag_get(ERU_CH4_REQ13_P15_5)) {
        exti_flag_clear(ERU_CH4_REQ13_P15_5);
    }
}

IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);
    if(exti_flag_get(ERU_CH1_REQ10_P14_3)) {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);
        /* tof_module_exti_handler(); */
    }
    if(exti_flag_get(ERU_CH5_REQ1_P15_8)) {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);
    }
}

IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);
    if(exti_flag_get(ERU_CH3_REQ6_P02_0)) {
        exti_flag_clear(ERU_CH3_REQ6_P02_0);
        /* camera_vsync_handler(); */
    }
    if(exti_flag_get(ERU_CH7_REQ16_P15_1)) {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);
    }
}

/* ========================= DMA 中断 ========================= */

IFX_INTERRUPT(dma_ch5_isr, 0, DMA_INT_PRIO)
{
    interrupt_global_enable(0);
    /* camera_dma_handler(); */
}

/* ========================= 串口中断 ========================= */

/* UART0 — 调试串口 */
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);
#if DEBUG_UART_USE_INTERRUPT
    debug_interrupr_handler();
#endif
}

/* UART1 — 摄像头 (可选) */
IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO) { interrupt_global_enable(0); }
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    /* camera_uart_handler(); */
}

/* UART2 — 无线模块 (可选) */
IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO) { interrupt_global_enable(0); }
IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    /* wireless_module_uart_handler(); */
}

/* UART3 — GNSS/GPS */
IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO) { interrupt_global_enable(0); }
IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    gnss_uart_callback();         /* GPS 数据接收回调 */
}

/* 串口错误处理 */
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO) { interrupt_global_enable(0); IfxAsclin_Asc_isrError(&uart0_handle); }
IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO) { interrupt_global_enable(0); IfxAsclin_Asc_isrError(&uart1_handle); }
IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO) { interrupt_global_enable(0); IfxAsclin_Asc_isrError(&uart2_handle); }
IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO) { interrupt_global_enable(0); IfxAsclin_Asc_isrError(&uart3_handle); }
