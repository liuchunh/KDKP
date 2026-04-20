/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的基础功能空库
 * 版权所有：[2025] [北京科宇通博科技有限公司江西分公司]
 * 
 * 许可协议：采用 GNU GPL v3.0 开源许可
 * 您可依据协议进行二次开发、传播，但须保留原始版权信息
 * 协议详情参见：https://www.gnu.org/licenses/gpl-3.0.html
 * 
 * 免责声明：本组件库仅提供技术参考，使用方需自行验证适用性
 * 
 * 协议文件：GPL v3.0 完整文本位于 [Keyu_TC264DA_Open_Source_Library] 目录下
 * 
 * === 文件信息 ===
 * 文件名：Isr.c
 * 文件功能：中断服务程序实现文件 - 中断分发
 * 开发单位：北京科宇通博科技有限公司江西分公司
 * 适用环境：[AURIX™ Development Studio V1.10.28]
  * 官方渠道：
        代码仓库 https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library.git
        淘宝店铺 https://kyznc.taobao.com/
        技术支持 QQ群 974530818
 * 
 * === 修订记录 ===
 * 日期         开发者    变更说明
 * -------------------------------------------
 * 2026年01月07日      毛毛    V3.0
 */

#include "Isr.h"
#include "Isr.h"
#include "ky_timer.h"
#include "ky_uart.h"
#include "ky_dma.h"

extern void timer_interrupt_handler(TIM_Channel_t tim);
/***************************************************************
  *  @brief     定时器0 中断服务函数 - 用于帧率计算
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(PIT0_ISR, 0, ISR_PRIORITY_STM0_CMP0);
void PIT0_ISR(void)
{
    IfxCpu_enableInterrupts();
    IfxStm_clearCompareFlag(&MODULE_STM0, IfxStm_Comparator_0);
    timer_interrupt_handler(TIM_0);
}

/***************************************************************
  *  @brief     定时器1 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(PIT1_ISR, 0, ISR_PRIORITY_STM0_CMP1);
void PIT1_ISR(void)
{
    IfxCpu_enableInterrupts();
    IfxStm_clearCompareFlag(&MODULE_STM0, IfxStm_Comparator_1);
    timer_interrupt_handler(TIM_1);

}


/***************************************************************
  *  @brief     定时器2 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(PIT2_ISR, 0, ISR_PRIORITY_STM1_CMP0);
void PIT2_ISR(void)
{
    IfxCpu_enableInterrupts();
    IfxStm_clearCompareFlag(&MODULE_STM1, IfxStm_Comparator_0);
    timer_interrupt_handler(TIM_2);
}

/***************************************************************
  *  @brief     定时器3 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(PIT3_ISR, 0, ISR_PRIORITY_STM1_CMP1);
void PIT3_ISR(void)
{
    IfxCpu_enableInterrupts();
    IfxStm_clearCompareFlag(&MODULE_STM1, IfxStm_Comparator_1);
    timer_interrupt_handler(TIM_3);
}

/***************************************************************
  *  @brief     GPIO外部中断0 服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(GPIO_EXT_INT0_ISR, 0, ISR_PRIORITY_GPIO_EXT_INT0);
void GPIO_EXT_INT0_ISR(void)
{
    IfxCpu_enableInterrupts();
    
    /* 用户逻辑: 处理外部中断0 */
    /* 建议使用回调机制或外部函数处理业务逻辑 */
}

/***************************************************************
  *  @brief     GPIO外部中断1 服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(GPIO_EXT_INT1_ISR, 0, ISR_PRIORITY_GPIO_EXT_INT1);
void GPIO_EXT_INT1_ISR(void)
{
    IfxCpu_enableInterrupts();
    
    /* 用户逻辑: 处理外部中断1 */
    /* 建议使用回调机制或外部函数处理业务逻辑 */
}

/* ASCLIN0 (UART0) */
/***************************************************************
  *  @brief     UART0 发送中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN0_TX_ISR, 0, ISR_PRIORITY_UART0_TX);
void ASCLIN0_TX_ISR(void)
{
    IfxAsclin_Asc_isrTransmit(&g_uartHandle[UART_0].asc);
}

/***************************************************************
  *  @brief     UART0 接收中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN0_RX_ISR, 0, ISR_PRIORITY_UART0_RX);
void ASCLIN0_RX_ISR(void)
{
    uart_process_isr(UART_0);
}

/***************************************************************
  *  @brief     UART0 错误中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN0_ERR_ISR, 0, ISR_PRIORITY_UART0_ERR);
void ASCLIN0_ERR_ISR(void)
{
    IfxAsclin_Asc_isrError(&g_uartHandle[UART_0].asc);
}

/* ASCLIN1 (UART1) */
/***************************************************************
  *  @brief     UART1 发送中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN1_TX_ISR, 0, ISR_PRIORITY_UART1_TX);
void ASCLIN1_TX_ISR(void)
{
    IfxAsclin_Asc_isrTransmit(&g_uartHandle[UART_1].asc);
}

/***************************************************************
  *  @brief     UART1 接收中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN1_RX_ISR, 0, ISR_PRIORITY_UART1_RX);
void ASCLIN1_RX_ISR(void)
{
    uart_process_isr(UART_1);
}

/***************************************************************
  *  @brief     UART1 错误中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN1_ERR_ISR, 0, ISR_PRIORITY_UART1_ERR);
void ASCLIN1_ERR_ISR(void)
{
    IfxAsclin_Asc_isrError(&g_uartHandle[UART_1].asc);
}

/* ASCLIN2 (UART2) */
/***************************************************************
  *  @brief     UART2 发送中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN2_TX_ISR, 0, ISR_PRIORITY_UART2_TX);
void ASCLIN2_TX_ISR(void)
{
    IfxAsclin_Asc_isrTransmit(&g_uartHandle[UART_2].asc);
}

/***************************************************************
  *  @brief     UART2 接收中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN2_RX_ISR, 0, ISR_PRIORITY_UART2_RX);
void ASCLIN2_RX_ISR(void)
{
    uart_process_isr(UART_2);
}

/***************************************************************
  *  @brief     UART2 错误中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN2_ERR_ISR, 0, ISR_PRIORITY_UART2_ERR);
void ASCLIN2_ERR_ISR(void)
{
    IfxAsclin_Asc_isrError(&g_uartHandle[UART_2].asc);
}

/* ASCLIN3 (UART3) */
/***************************************************************
  *  @brief     UART3 发送中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN3_TX_ISR, 0, ISR_PRIORITY_UART3_TX);
void ASCLIN3_TX_ISR(void)
{
    IfxAsclin_Asc_isrTransmit(&g_uartHandle[UART_3].asc);
}

/***************************************************************
  *  @brief     UART3 接收中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN3_RX_ISR, 0, ISR_PRIORITY_UART3_RX);
void ASCLIN3_RX_ISR(void)
{
    uart_process_isr(UART_3);
}

/***************************************************************
  *  @brief     UART3 错误中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(ASCLIN3_ERR_ISR, 0, ISR_PRIORITY_UART3_ERR);
void ASCLIN3_ERR_ISR(void)
{
    IfxAsclin_Asc_isrError(&g_uartHandle[UART_3].asc);
}

/***************************************************************
  *  @brief     DMA通道0 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH0_ISR, 0, ISR_PRIORITY_DMA_CH0);
void DMA_CH0_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_UART_TX, UART_0);
}

/***************************************************************
  *  @brief     DMA通道1 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH1_ISR, 0, ISR_PRIORITY_DMA_CH1);
void DMA_CH1_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_UART_RX, UART_0);
}

/***************************************************************
  *  @brief     DMA通道2 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH2_ISR, 0, ISR_PRIORITY_DMA_CH2);
void DMA_CH2_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_UART_TX, UART_1);
}

/***************************************************************
  *  @brief     DMA通道3 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH3_ISR, 0, ISR_PRIORITY_DMA_CH3);
void DMA_CH3_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_UART_RX, UART_1);
}

/***************************************************************
  *  @brief     DMA通道4 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH4_ISR, 0, ISR_PRIORITY_DMA_CH4);
void DMA_CH4_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_UART_TX, UART_2);
}

/***************************************************************
  *  @brief     DMA通道5 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
// 冲突：DMA CH5 已被相机占用
// IFX_INTERRUPT(DMA_CH5_ISR, 0, ISR_PRIORITY_DMA_CH5);
// void DMA_CH5_ISR(void)
// {
//     IfxCpu_enableInterrupts();
//     dma_process_isr(DMA_TYPE_UART_RX, UART_2);
// }

/***************************************************************
  *  @brief     DMA通道6 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
/* 注释掉 DMA_CH6 (UART3 TX) - 解决与普通UART中断的冲突 */
// IFX_INTERRUPT(DMA_CH6_ISR, 0, ISR_PRIORITY_DMA_CH6);
// void DMA_CH6_ISR(void)
// {
//     IfxCpu_enableInterrupts();
//     dma_process_isr(DMA_TYPE_UART_TX, UART_3);
// }

/***************************************************************
  *  @brief     DMA通道7 中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
/* 注释掉 DMA_CH7 (UART3 RX) - 解决与普通UART中断的冲突 (关键!) */
// IFX_INTERRUPT(DMA_CH7_ISR, 0, ISR_PRIORITY_DMA_CH7);
// void DMA_CH7_ISR(void)
// {
//     IfxCpu_enableInterrupts();
//     dma_process_isr(DMA_TYPE_UART_RX, UART_3);
// }

/***************************************************************
  *  @brief     DMA通道8 中断服务函数 (SPI0 TX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH8_ISR, 0, ISR_PRIORITY_DMA_CH8);
void DMA_CH8_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_0);
}

/***************************************************************
  *  @brief     DMA通道9 中断服务函数 (SPI0 RX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH9_ISR, 0, ISR_PRIORITY_DMA_CH9);
void DMA_CH9_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_0);
}

/***************************************************************
  *  @brief     DMA通道10 中断服务函数 (SPI1 TX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH10_ISR, 0, ISR_PRIORITY_DMA_CH10);
void DMA_CH10_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_1);
}

/***************************************************************
  *  @brief     DMA通道11 中断服务函数 (SPI1 RX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH11_ISR, 0, ISR_PRIORITY_DMA_CH11);
void DMA_CH11_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_1);
}

/***************************************************************
  *  @brief     DMA通道12 中断服务函数 (SPI2 TX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH12_ISR, 0, ISR_PRIORITY_DMA_CH12);
void DMA_CH12_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_2);
}

/***************************************************************
  *  @brief     DMA通道13 中断服务函数 (SPI2 RX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH13_ISR, 0, ISR_PRIORITY_DMA_CH13);
void DMA_CH13_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_2);
}

/***************************************************************
  *  @brief     DMA通道14 中断服务函数 (SPI3 TX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH14_ISR, 0, ISR_PRIORITY_DMA_CH14);
void DMA_CH14_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_3);
}

/***************************************************************
  *  @brief     DMA通道15 中断服务函数 (SPI3 RX)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(DMA_CH15_ISR, 0, ISR_PRIORITY_DMA_CH15);
void DMA_CH15_ISR(void)
{
    IfxCpu_enableInterrupts();
    dma_process_isr(DMA_TYPE_SPI, SPI_3);
}

/***************************************************************
  *  @brief     I2C 错误中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(I2C_ERR_ISR, 0, ISR_PRIORITY_I2C_ERR);
void I2C_ERR_ISR(void)
{
    IfxCpu_enableInterrupts();
    /* I2C错误处理逻辑 */
}

/***************************************************************
  *  @brief     I2C 协议中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(I2C_P_ISR, 0, ISR_PRIORITY_I2C_P);
void I2C_P_ISR(void)
{
    IfxCpu_enableInterrupts();
    /* I2C传输完成/协议事件处理 */
}

// /***************************************************************
//   *  @brief     QSPI0 发送中断服务函数
//   *  @param     None
//   *  @Sample usage:     (Hardware Interrupt)
//  **************************************************************/
// IFX_INTERRUPT(QSPI0_TX_ISR, 0, ISR_PRIORITY_QSPI0_TX);
// void QSPI0_TX_ISR(void)
// {
//     IfxCpu_enableInterrupts();
//     /* QSPI发送完成处理 */
// }

// /***************************************************************
//   *  @brief     QSPI0 接收中断服务函数
//   *  @param     None
//   *  @Sample usage:     (Hardware Interrupt)
//  **************************************************************/
// IFX_INTERRUPT(QSPI0_RX_ISR, 0, ISR_PRIORITY_QSPI0_RX);
// void QSPI0_RX_ISR(void)
// {
//     IfxCpu_enableInterrupts();
//     /* QSPI接收完成处理 */
// }

// /***************************************************************
//   *  @brief     QSPI0 错误中断服务函数
//   *  @param     None
//   *  @Sample usage:     (Hardware Interrupt)
//  **************************************************************/
// IFX_INTERRUPT(QSPI0_ERR_ISR, 0, ISR_PRIORITY_QSPI0_ERR);
// void QSPI0_ERR_ISR(void)
// {
//     IfxCpu_enableInterrupts();
//     /* QSPI错误处理 */
// }

/***************************************************************
  *  @brief     MT9V034 VSYNC中断服务函数
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(MT9V034_VSYNC_ISR, 0, ISR_PRIORITY_MT9V034_VSYNC);

// 调试：ISR内部计数（全局变量，可在Main中访问）
volatile uint32 isr_debug_count = 0;

void MT9V034_VSYNC_ISR(void)
{
    IfxCpu_enableInterrupts();
    
    // 调试：直接在ISR中增加计数
    isr_debug_count++;
    
    // 清除ERU中断标志 (OutputChannel 3)
    IfxScuEru_clearEventFlag(IfxScuEru_OutputChannel_3);
    
    // 调用VSYNC处理函数
    extern void mt9_v034_vsync_handler(void);
    mt9_v034_vsync_handler();
}

/***************************************************************
  *  @brief     MT9V034 DMA中断服务函数 (DMA通道5)
  *  @param     None
  *  @Sample usage:     (Hardware Interrupt)
 **************************************************************/
IFX_INTERRUPT(MT9V034_DMA_ISR, 0, ISR_PRIORITY_DMA_CH5);
void MT9V034_DMA_ISR(void)
{
    IfxCpu_enableInterrupts();
    
    // 调用DMA处理函数
    extern void mt9_v034_dma_handler(void);
    mt9_v034_dma_handler();
}
