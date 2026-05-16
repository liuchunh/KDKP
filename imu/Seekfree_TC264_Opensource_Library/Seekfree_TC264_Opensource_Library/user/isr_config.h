/*********************************************************************************************************************
 * 文件名称: isr_config.h
 * 功能描述: 中断优先级和中断服务分配配置 - 科目1 自动驾驶
 *
 * 本文件定义了所有中断的:
 *   - INT_SERVICE: 中断由谁响应 (IfxSrc_Tos_cpu0 / IfxSrc_Tos_cpu1 / IfxSrc_Tos_dma)
 *   - ISR_PRIORITY: 中断优先级 (1-255, 数值越大优先级越高)
 *
 * TC264 中断系统说明:
 *   - 优先级范围: 0~255, 0=不响应中断, 255=最高优先级
 *   - INT_SERVICE 指定中断由 CPU0、CPU1 还是 DMA 引擎处理
 *   - 若 INT_SERVICE 设为 IfxSrc_Tos_dma, 则优先级范围限制为 0-47
 *
 * 优先级分配原则:
 *   - GPS 接收 (UART3 RX): 优先级 200, 最高优先级, NMEA 帧有时间窗口要求
 *   - IMU 采样 (PIT Ch1): 优先级 150, 确保 10ms 采样周期稳定
 *   - IMU 外部中断 (ERU): 优先级 140
 *   - 调试输出 (PIT Ch0): 优先级 50, 非紧急
 *   - 摄像头 DMA: 优先级 120 (如不使用摄像头可忽略)
 *   - 串口发送: 优先级 60, 低于接收
 *   - 串口错误: 优先级 40, 最低
 *
 * 注意: 所有中断默认由 CPU0 响应, 修改后需要重新编译整个工程
 ********************************************************************************************************************/

#ifndef _isr_config_h
#define _isr_config_h

/* =====================================================================
 *  PIT 定时器中断配置
 *
 *  CCU6_0_CH0: 100ms 调试输出定时
 *  CCU6_0_CH1: 10ms  IMU 采样定时 ★★★
 *  CCU6_1_CH0: 保留
 *  CCU6_1_CH1: 保留
 * ===================================================================== */

/* PIT Ch0: 调试输出 100ms */
#define CCU6_0_CH0_INT_SERVICE      IfxSrc_Tos_cpu0     /* 由 CPU0 响应 */
#define CCU6_0_CH0_ISR_PRIORITY     (50)                /* 优先级 50 */

/* PIT Ch1: IMU 采样 10ms ★★★ */
#define CCU6_0_CH1_INT_SERVICE      IfxSrc_Tos_cpu0     /* 由 CPU0 响应 */
#define CCU6_0_CH1_ISR_PRIORITY     (150)               /* 优先级 150, 较高 */

/* PIT Ch2: 保留 */
#define CCU6_1_CH0_INT_SERVICE      IfxSrc_Tos_cpu0
#define CCU6_1_CH0_ISR_PRIORITY     (30)

/* PIT Ch3: 保留 */
#define CCU6_1_CH1_INT_SERVICE      IfxSrc_Tos_cpu0
#define CCU6_1_CH1_ISR_PRIORITY     (30)

/* =====================================================================
 *  外部中断 (ERU) 配置
 *
 *  Ch0/Ch4: IMU660RC / IMU963RA 外部中断
 *  Ch1/Ch5: ToF 外部中断 (如使用)
 *  Ch2/Ch6: 摄像头 PCLK (如使用 DMA 触发)
 *  Ch3/Ch7: 摄像头 VSYNC
 * ===================================================================== */

/* ERU Ch0/Ch4: IMU 外部中断 */
#define EXTI_CH0_CH4_INT_SERVICE    IfxSrc_Tos_cpu0
#define EXTI_CH0_CH4_INT_PRIO       (140)               /* 优先级 140 */

/* ERU Ch1/Ch5: ToF 外部中断 */
#define EXTI_CH1_CH5_INT_SERVICE    IfxSrc_Tos_cpu0
#define EXTI_CH1_CH5_INT_PRIO       (100)

/* ERU Ch2/Ch6: 摄像头 PCLK / DMA 触发 */
#define EXTI_CH2_CH6_INT_SERVICE    IfxSrc_Tos_dma      /* 由 DMA 引擎处理 */
#define EXTI_CH2_CH6_INT_PRIO       (5)                 /* DMA 模式下范围 0-47 */

/* ERU Ch3/Ch7: 摄像头 VSYNC */
#define EXTI_CH3_CH7_INT_SERVICE    IfxSrc_Tos_cpu0
#define EXTI_CH3_CH7_INT_PRIO       (120)

/* =====================================================================
 *  DMA 中断配置
 *
 *  Seekfree 库的 zf_driver_dma.c 使用 DMA_INT_PRIO_1 / DMA_INT_SERVICE_1
 *  和 DMA_INT_PRIO_2 / DMA_INT_SERVICE_2
 *  同时保留不带编号的 DMA_INT_PRIO / DMA_INT_SERVICE 以兼容其他引用
 * ===================================================================== */

/* DMA 通道 1 (摄像头 DMA 传输) */
#define DMA_INT_SERVICE             IfxSrc_Tos_cpu0
#define DMA_INT_PRIO                (120)

#define DMA_INT_SERVICE_1           IfxSrc_Tos_cpu0
#define DMA_INT_PRIO_1              (120)

/* DMA 通道 2 (备用) */
#define DMA_INT_SERVICE_2           IfxSrc_Tos_cpu0
#define DMA_INT_PRIO_2              (100)

/* =====================================================================
 *  串口中断配置
 *
 *  UART0: GPS/GNSS 模块 ★★★
 *  UART1: 预留
 *  UART2: 无线模块 (蓝牙/WiFi)
 *  UART3: 调试串口 (printf 重定向)
 *
 *  每个串口有 TX(发送)、RX(接收)、ER(错误) 三个中断
 *  所有串口由 CPU0 响应
 * ===================================================================== */

/* UART0: GPS/GNSS ★★★ */
#define UART0_INT_SERVICE           IfxSrc_Tos_cpu0
#define UART0_TX_INT_PRIO           (60)
#define UART0_RX_INT_PRIO           (200)               /* ★GPS接收★ 最高优先级 */
#define UART0_ER_INT_PRIO           (40)

/* UART1: 预留 */
#define UART1_INT_SERVICE           IfxSrc_Tos_cpu0
#define UART1_TX_INT_PRIO           (60)
#define UART1_RX_INT_PRIO           (80)
#define UART1_ER_INT_PRIO           (40)

/* UART2: 无线模块 */
#define UART2_INT_SERVICE           IfxSrc_Tos_cpu0
#define UART2_TX_INT_PRIO           (60)
#define UART2_RX_INT_PRIO           (80)
#define UART2_ER_INT_PRIO           (40)

/* UART3: 调试串口 */
#define UART3_INT_SERVICE           IfxSrc_Tos_cpu0
#define UART3_TX_INT_PRIO           (60)
#define UART3_RX_INT_PRIO           (80)
#define UART3_ER_INT_PRIO           (40)

#endif /* _isr_config_h */
