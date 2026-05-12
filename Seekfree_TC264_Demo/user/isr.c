/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官�?SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软�?* 您可以根据自由软件基金会发布�?GPL（GNU General Public License，即 GNU通用公共许可证）的条�?* �?GPL 的第3版（�?GPL3.0）或（您选择的）任何后来的版本，重新发布�?或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参�?GPL
*
* 您应该在收到本开源库的同时收到一�?GPL 的副�?* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明�?* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版�?* 许可申明英文版在 libraries/doc 文件夹下�?GPL3_permission_statement.txt 文件�?* 许可证副本在 libraries 文件夹下 即该文件夹下�?LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明�?*
* 文件名称          isr
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环�?         ADS v1.10.2
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作�?               备注
* 2022-09-15       pudding            first version
*********************************************************************************************************************/

#include "isr_config.h"
#include "isr.h"
#include "motor_pid.h"
#include "angle_control.h"

// TC 系列中断默认不支持中断嵌套，如需�?ISR 内允许更高优先级中断打断当前中断�?// 需要在中断函数开头调�?interrupt_global_enable(0)�?// 一般不建议简单地直接调用 interrupt_global_disable() / interrupt_global_enable() 成对控制�?// 这里沿用官方库推荐写法�?
// **************************** PIT 中断 ****************************
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    static uint8 in_update = 0;
    if (in_update) return;                          // 防止重入
    in_update = 1;

    pit_clear_flag(CCU60_CH0);                      // 清除 PIT 中断标志

    //motor_pid_update();
    angle_control_update();

    in_update = 0;
}
IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    pit_clear_flag(CCU60_CH1);                      // 清除 PIT 中断标志
}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    pit_clear_flag(CCU61_CH0);                      // 清除 PIT 中断标志
}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    pit_clear_flag(CCU61_CH1);                      // 清除 PIT 中断标志
}
// **************************** PIT 中断 ****************************


// **************************** 外部中断 ****************************
IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套

    if(exti_flag_get(ERU_CH0_REQ0_P15_4))           // ERU 通道 0
    {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);

        imu660rc_callback();                        // IMU660RC �?INT 中断处理
    }

    if(exti_flag_get(ERU_CH4_REQ13_P15_5))          // ERU 通道 4
    {
        exti_flag_clear(ERU_CH4_REQ13_P15_5);
    }
}

IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套

    if(exti_flag_get(ERU_CH1_REQ10_P14_3))          // ERU 通道 1
    {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);

        tof_module_exti_handler();                  // ToF 模块 INT 中断处理
    }

    if(exti_flag_get(ERU_CH5_REQ1_P15_8))           // ERU 通道 5
    {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);
    }
}

// 摄像�?PCLK 默认占用外部中断通道 2，因此这里保留逐飞库原始模板并注释掉�?// IFX_INTERRUPT(exti_ch2_ch6_isr, 0, EXTI_CH2_CH6_INT_PRIO)
// {
//     interrupt_global_enable(0);                  // 允许中断嵌套
//     if(exti_flag_get(ERU_CH2_REQ7_P00_4))        // ERU 通道 2
//     {
//         exti_flag_clear(ERU_CH2_REQ7_P00_4);
//     }
//     if(exti_flag_get(ERU_CH6_REQ9_P20_0))        // ERU 通道 6
//     {
//         exti_flag_clear(ERU_CH6_REQ9_P20_0);
//     }
// }

IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    if(exti_flag_get(ERU_CH3_REQ6_P02_0))           // ERU 通道 3
    {
        exti_flag_clear(ERU_CH3_REQ6_P02_0);
        camera_vsync_handler();                     // 摄像头场同步中断处理
    }
    if(exti_flag_get(ERU_CH7_REQ16_P15_1))          // ERU 通道 7
    {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);
    }
}
// **************************** 外部中断 ****************************


// **************************** DMA 中断 ****************************
IFX_INTERRUPT(dma_ch5_isr, 0, DMA_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    camera_dma_handler();                           // 摄像�?DMA 搬运完成处理
}
// **************************** DMA 中断 ****************************


// **************************** 串口中断 ****************************
// 串口 0 默认作为调试串口
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
}

IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套

#if DEBUG_UART_USE_INTERRUPT
    debug_interrupr_handler();                      // 调试串口接收中断处理
#endif
}


// ���� 1 Ĭ����������ͷ
IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO)
{
}

IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    camera_uart_handler();                          // ����ͷ���ڽ��մ���
}
// 串口 2 默认连接无线模块
IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
}

IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    wireless_module_uart_handler();                 // 无线模块串口接收处理
}

// 串口 3 默认连接 GNSS / GPS 模块
IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
}

IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    gnss_uart_callback();                           // GNSS 串口接收回调
}

// 串口错误中断
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    IfxAsclin_Asc_isrError(&uart0_handle);
}

IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    IfxAsclin_Asc_isrError(&uart1_handle);
}

IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    IfxAsclin_Asc_isrError(&uart2_handle);
}

IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 允许中断嵌套
    IfxAsclin_Asc_isrError(&uart3_handle);
}
// **************************** 串口中断 ****************************
