/*********************************************************************************
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 版权所有：[2025] [北京科宇通博科技有限公司]
 *
 * 许可协议：采用 GNU GPL v3.0 开源许可
 * 您可依据协议进行二次开发、传播，但须保留原始版权信息
 * 协议详情参见：https://www.gnu.org/licenses/gpl-3.0.html
 *
 * 免责声明：本组件库仅提供技术参考，使用方需自行验证适用性
 *
 * 协议文件：GPL v3.0 完整文本位于根目录下
 *
 * === 文件信息 ===
 * 文件名：[ky_uart.c]
 * 开发单位：北京科宇通博科技有限公司
 * 适用环境：[AURIX TC264DA]
 * 官方渠道：
 *   - 代码仓库：[https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library]
 *   - 淘宝店铺：https://kyznc.taobao.com/
 *   - 技术支持：QQ群 974530818
 *
 * === 修订记录 ===
 * 日期       |  开发者  | 变更说明
 * -----------|----------|----------------------
 * 2026.01.07 |   毛毛   | V3.0
 *********************************************************************************/
#include "ky_uart.h"
#include "Asclin/Asc/IfxAsclin_Asc.h"
#include "Asclin/Std/IfxAsclin.h"
#include "_PinMap/IfxAsclin_PinMap.h"
#include "IfxCpu_Irq.h"
#include "Isr.h"
#include <string.h>
#include <stdio.h>


UartHandle_t g_uartHandle[4];       // UART句柄数组

/***************************************************************
  *  @brief     获取ASCLIN模块指针
  *  @param     uartIndex    UART索引
  *  @return    Ifx_ASCLIN*  ASCLIN模块指针
  **************************************************************/
static Ifx_ASCLIN* uart_get_module(UART_Index_t uartIndex)
{
    switch (uartIndex)
    {
        case UART_0: return &MODULE_ASCLIN0;
        case UART_1: return &MODULE_ASCLIN1;
        case UART_2: return &MODULE_ASCLIN2;
        case UART_3: return &MODULE_ASCLIN3;
        default: return &MODULE_ASCLIN0;
    }
}

/***************************************************************
  *  @brief     根据RX引脚选择获取RX引脚映射
  *  @param     rxPin                 RX引脚选择
  *  @return    IfxAsclin_Rx_In*      RX引脚映射指针
  **************************************************************/
static const IfxAsclin_Rx_In* uart_get_rx_pin(UART_Rx_Pin_t rxPin)
{
    switch (rxPin)
    {
        /* ASCLIN0 */
        case UART0_RXA_P14_1: return &IfxAsclin0_RXA_P14_1_IN;
        case UART0_RXB_P15_3: return &IfxAsclin0_RXB_P15_3_IN;
        
        /* ASCLIN1 */
        case UART1_RXA_P15_1: return &IfxAsclin1_RXA_P15_1_IN;
        case UART1_RXB_P15_5: return &IfxAsclin1_RXB_P15_5_IN;
        case UART1_RXC_P20_9: return &IfxAsclin1_RXC_P20_9_IN;
        case UART1_RXD_P14_8: return &IfxAsclin1_RXD_P14_8_IN;
        case UART1_RXE_P11_10: return &IfxAsclin1_RXE_P11_10_IN;
        case UART1_RXF_P33_13: return &IfxAsclin1_RXF_P33_13_IN;
        case UART1_RXG_P02_3: return &IfxAsclin1_RXG_P02_3_IN;
        
        /* ASCLIN2 */
        case UART2_RXA_P14_3: return &IfxAsclin2_RXA_P14_3_IN;
        case UART2_RXB_P02_1: return &IfxAsclin2_RXB_P02_1_IN;
        case UART2_RXD_P10_6: return &IfxAsclin2_RXD_P10_6_IN;
        case UART2_RXE_P33_8: return &IfxAsclin2_RXE_P33_8_IN;
        case UART2_RXG_P02_0: return &IfxAsclin2_RXG_P02_0_IN;
        
        /* ASCLIN3 */
        case UART3_RXA_P15_7: return &IfxAsclin3_RXA_P15_7_IN;
        case UART3_RXC_P20_3: return &IfxAsclin3_RXC_P20_3_IN;
        case UART3_RXD_P32_2: return &IfxAsclin3_RXD_P32_2_IN;
        case UART3_RXE_P00_1: return &IfxAsclin3_RXE_P00_1_IN;
        case UART3_RXF_P21_6: return &IfxAsclin3_RXF_P21_6_IN;
        
        default: return NULL_PTR;
    }
}

/***************************************************************
  *  @brief     根据TX引脚选择获取TX引脚映射
  *  @param     txPin                 TX引脚选择
  *  @return    IfxAsclin_Tx_Out*     TX引脚映射指针
  **************************************************************/
static const IfxAsclin_Tx_Out* uart_get_tx_pin(UART_Tx_Pin_t txPin)
{
    switch (txPin)
    {
        /* ASCLIN0 */
        case UART0_TX_P14_0: return &IfxAsclin0_TX_P14_0_OUT;
        case UART0_TX_P14_1: return &IfxAsclin0_TX_P14_1_OUT;
        case UART0_TX_P15_2: return &IfxAsclin0_TX_P15_2_OUT;
        case UART0_TX_P15_3: return &IfxAsclin0_TX_P15_3_OUT;
        
        /* ASCLIN1 */
        case UART1_TX_P02_2: return &IfxAsclin1_TX_P02_2_OUT;
        case UART1_TX_P11_12: return &IfxAsclin1_TX_P11_12_OUT;
        case UART1_TX_P14_10: return &IfxAsclin1_TX_P14_10_OUT;
        case UART1_TX_P15_0: return &IfxAsclin1_TX_P15_0_OUT;
        case UART1_TX_P15_1: return &IfxAsclin1_TX_P15_1_OUT;
        case UART1_TX_P15_4: return &IfxAsclin1_TX_P15_4_OUT;
        case UART1_TX_P15_5: return &IfxAsclin1_TX_P15_5_OUT;
        case UART1_TX_P20_10: return &IfxAsclin1_TX_P20_10_OUT;
        case UART1_TX_P33_12: return &IfxAsclin1_TX_P33_12_OUT;
        case UART1_TX_P33_13: return &IfxAsclin1_TX_P33_13_OUT;
        
        /* ASCLIN2 */
        case UART2_TX_P02_0: return &IfxAsclin2_TX_P02_0_OUT;
        case UART2_TX_P10_5: return &IfxAsclin2_TX_P10_5_OUT;
        case UART2_TX_P14_2: return &IfxAsclin2_TX_P14_2_OUT;
        case UART2_TX_P14_3: return &IfxAsclin2_TX_P14_3_OUT;
        case UART2_TX_P33_8: return &IfxAsclin2_TX_P33_8_OUT;
        case UART2_TX_P33_9: return &IfxAsclin2_TX_P33_9_OUT;
        
        /* ASCLIN3 */
        case UART3_TX_P00_0: return &IfxAsclin3_TX_P00_0_OUT;
        case UART3_TX_P00_1: return &IfxAsclin3_TX_P00_1_OUT;
        case UART3_TX_P15_6: return &IfxAsclin3_TX_P15_6_OUT;
        case UART3_TX_P15_7: return &IfxAsclin3_TX_P15_7_OUT;
        case UART3_TX_P20_0: return &IfxAsclin3_TX_P20_0_OUT;
        case UART3_TX_P20_3: return &IfxAsclin3_TX_P20_3_OUT;
        case UART3_TX_P21_7: return &IfxAsclin3_TX_P21_7_OUT;
        case UART3_TX_P32_2: return &IfxAsclin3_TX_P32_2_OUT;
        case UART3_TX_P32_3: return &IfxAsclin3_TX_P32_3_OUT;
        
        default: return NULL_PTR;
    }
}

/***************************************************************
 *  @brief     初始化指定的UART端口
 *  @param     uartIndex    UART索引
 *  @param     baudRate     波特率
 *  @param     rxPin        RX引脚选择
 *  @param     txPin        TX引脚选择
 *  @note      必须在调用此函数前确保全局中断已使能，或在初始化后使能。
 **************************************************************/
void uart_init(UART_Index_t uartIndex, uint32 baudRate, UART_Rx_Pin_t rxPin, UART_Tx_Pin_t txPin)
{
    IfxAsclin_Asc_Config ascConfig;

    boolean interrupt_state = IfxCpu_disableInterrupts();
    
    Ifx_ASCLIN *asclin = uart_get_module(uartIndex);
    
    IfxAsclin_Asc_initModuleConfig(&ascConfig, asclin);
    
    g_uartHandle[uartIndex].rxReadIndex = 0;
    g_uartHandle[uartIndex].rxWriteIndex = 0;
    g_uartHandle[uartIndex].rxCallback = NULL_PTR;
    
    ascConfig.txBuffer = &g_uartHandle[uartIndex].txBuffer[0];
    ascConfig.txBufferSize = UART_TX_BUFFER_SIZE;
    ascConfig.rxBuffer = &g_uartHandle[uartIndex].rxBuffer[0];
    ascConfig.rxBufferSize = UART_RX_BUFFER_SIZE;
    
    ascConfig.clockSource = IfxAsclin_ClockSource_ascFastClock;
    ascConfig.baudrate.prescaler = 4;
    ascConfig.baudrate.baudrate = (float32)baudRate;
    ascConfig.baudrate.oversampling = IfxAsclin_OversamplingFactor_8;
    
    const IfxAsclin_Rx_In *rxPinMap = uart_get_rx_pin(rxPin);
    const IfxAsclin_Tx_Out *txPinMap = uart_get_tx_pin(txPin);
    
    IfxAsclin_Asc_Pins pins;
    memset(&pins, 0, sizeof(IfxAsclin_Asc_Pins));
    
    pins.cts = NULL_PTR;
    pins.rts = NULL_PTR;
    
    if (rxPinMap != NULL_PTR)
    {
        pins.rx = rxPinMap;
        pins.rxMode = IfxPort_InputMode_pullUp;
    }
    
    if (txPinMap != NULL_PTR)
    {
        pins.tx = txPinMap;
        pins.txMode = IfxPort_OutputMode_pushPull;
    }
    
    pins.pinDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    ascConfig.pins = &pins;
    
    ascConfig.frame.frameMode = IfxAsclin_FrameMode_asc;
    ascConfig.frame.dataLength = IfxAsclin_DataLength_8;
    ascConfig.frame.stopBit = IfxAsclin_StopBit_1;
    ascConfig.frame.shiftDir = IfxAsclin_ShiftDirection_lsbFirst;
    ascConfig.frame.parityBit = FALSE;
    
    ascConfig.rxBuffer = &g_uartHandle[uartIndex].rxBuffer[0];
    ascConfig.rxBufferSize = UART_RX_BUFFER_SIZE;
    
    switch (uartIndex)
    {
        case UART_0:
            ascConfig.interrupt.txPriority = ISR_PRIORITY_UART0_TX;
            ascConfig.interrupt.rxPriority = ISR_PRIORITY_UART0_RX;
            ascConfig.interrupt.erPriority = ISR_PRIORITY_UART0_ERR;
            break;
        case UART_1:
            ascConfig.interrupt.txPriority = ISR_PRIORITY_UART1_TX;
            ascConfig.interrupt.rxPriority = ISR_PRIORITY_UART1_RX;
            ascConfig.interrupt.erPriority = ISR_PRIORITY_UART1_ERR;
            break;
        case UART_2:
            ascConfig.interrupt.txPriority = ISR_PRIORITY_UART2_TX;
            ascConfig.interrupt.rxPriority = ISR_PRIORITY_UART2_RX;
            ascConfig.interrupt.erPriority = ISR_PRIORITY_UART2_ERR;
            break;
        case UART_3:
            ascConfig.interrupt.txPriority = ISR_PRIORITY_UART3_TX;
            ascConfig.interrupt.rxPriority = ISR_PRIORITY_UART3_RX;
            ascConfig.interrupt.erPriority = ISR_PRIORITY_UART3_ERR;
            break;
    }
    ascConfig.interrupt.typeOfService = IfxSrc_Tos_cpu0;
    
    IfxAsclin_Asc_initModule(&g_uartHandle[uartIndex].asc, &ascConfig);
    
    Ifx_ASCLIN *asclinSFR = g_uartHandle[uartIndex].asc.asclin;
    volatile Ifx_SRC_SRCR *src = IfxAsclin_getSrcPointerRx(asclinSFR);
    IfxAsclin_enableRxFifoFillLevelFlag(asclinSFR, TRUE);
    IfxSrc_enable(src);

    IfxCpu_restoreInterrupts(interrupt_state);
}

/***************************************************************
 *  @brief     通过UART发送字符串
 *  @param     uartIndex    UART索引
 *  @param     str          要发送的字符串
 *  @note      阻塞式发送一个以null结尾的字符串。
 **************************************************************/
void uart_send_string(UART_Index_t uartIndex, const char *str)
{
    if (str != NULL_PTR)
    {
        uint16 length = (uint16)strlen(str);
        uart_send_buffer(uartIndex, (const uint8 *)str, length);
    }
}

/***************************************************************
 *  @brief     通过UART发送一个字节
 *  @param     uartIndex    UART索引
 *  @param     dat          要发送的字节数据
 *  @note      阻塞式发送单个字节。
 **************************************************************/
void uart_send_byte(UART_Index_t uartIndex, uint8 dat)
{
    uart_send_buffer(uartIndex, &dat, 1);
}

/***************************************************************
 *  @brief     通过UART发送指定长度的缓冲区
 *  @param     uartIndex    UART索引
 *  @param     buffer       数据缓冲区指针
 *  @param     length       要发送的数据字节数
 **************************************************************/
void uart_send_buffer(UART_Index_t uartIndex, const uint8 *buffer, uint16 length)
{
    if (buffer != NULL_PTR && length > 0)
    {
        Ifx_SizeT count = (Ifx_SizeT)length;
        IfxAsclin_Asc_write(&g_uartHandle[uartIndex].asc, (void*)buffer, &count, TIME_INFINITE);
    }
}

/***************************************************************
 *  @brief     从UART接收一个字节（非阻塞）
 *  @param     uartIndex    UART索引
 *  @param     data         用于存储读取数据的指针
 *  @return    boolean      TRUE: 成功读取到数据; FALSE: 缓冲区为空
 **************************************************************/
boolean uart_read_byte(UART_Index_t uartIndex, uint8 *data)
{
    if (data == NULL_PTR)
    {
        return FALSE;
    }
    Ifx_SizeT count = 1;
    if (IfxAsclin_Asc_read(&g_uartHandle[uartIndex].asc, data, &count, TIME_NULL))
    {
        return TRUE;
    }
    
    return FALSE;
}

/***************************************************************
 *  @brief     查询接收缓冲区中的可用字节数
 *  @param     uartIndex    UART索引
 *  @return    uint16       当前接收缓冲区中等待读取的字节数
 **************************************************************/
uint16 uart_available(UART_Index_t uartIndex)
{
    return (uint16)IfxAsclin_Asc_getReadCount(&g_uartHandle[uartIndex].asc);
}

/***************************************************************
 *  @brief     清空接收缓冲区
 *  @param     uartIndex    UART索引
 *  @note      丢弃接收缓冲区中的所有现有数据。
 **************************************************************/
void uart_flush_rx(UART_Index_t uartIndex)
{
    IfxAsclin_Asc_clearRx(&g_uartHandle[uartIndex].asc);
}

/***************************************************************
 *  @brief     注册UART接收回调函数
 *  @param     uartIndex    UART索引
 *  @param     cb           回调函数指针 (参数为接收到的字节)
 *  @note      当接收到新字节时，将在中断上下文中调用此回调函数。
 **************************************************************/
void uart_set_callback(UART_Index_t uartIndex, UART_Callback_t cb)
{
    g_uartHandle[uartIndex].rxCallback = cb;
}

/***************************************************************
 *  @brief     UART中断处理函数
 *  @param     uartIndex    UART索引
 *  @note      此函数应在项目的 Isr.c 中对应的 ASCLIN RX 中断服务例程中调用。
 **************************************************************/
void uart_process_isr(UART_Index_t uartIndex)
{
    IfxAsclin_Asc_isrReceive(&g_uartHandle[uartIndex].asc);

    UART_Callback_t cb = g_uartHandle[uartIndex].rxCallback;
    if (cb != NULL_PTR)
    {
        uint8 dat;
        while (uart_read_byte(uartIndex, &dat))
        {
            cb(dat);
        }
    }
}

/***************************************************************
 *  @brief     发送一个字节（用于printf重定向）
 *  @param     c            要发送的字符
 *  @return    int          返回发送的字符
 *  @note      重写标准库的weak函数
 **************************************************************/
int _putchar(int c)
{
    uint8 byte = (uint8)c;
    uart_send_buffer(DEBUG_UART_INDEX, &byte, 1);
    return c;
}

/***************************************************************
 *  @brief     发送一个字节（用于标准库文件流重定向）
 *  @param     c            要发送的字符
 *  @param     stream       文件流指针
 *  @return    int          返回发送的字符
 **************************************************************/
int fputc(int c, FILE *stream)
{
    (void)stream;
    uint8 byte = (uint8)c;
    uart_send_buffer(DEBUG_UART_INDEX, &byte, 1);
    return c;
}


/***************************************************************
 *  @brief     初始化调试串口
 *  @note      使用默认配置初始化 DEBUG_UART_INDEX
 **************************************************************/
void uart_debug_init(void)
{
    uart_init(DEBUG_UART_INDEX, DEBUG_UART_BAUDRATE, DEBUG_UART_RX_PIN, DEBUG_UART_TX_PIN);
}

/***************************************************************
 *  @brief     裸机 UART 发送（直接操作寄存器，用于调试）
 *  @param     byte  要发送的字节
 *  @return    void
 *  @note      绕过所有驱动层，直接写 ASCLIN0 寄存器
 **************************************************************/
void uart_send_byte_bare(uint8 byte)
{
    Ifx_ASCLIN *asclin = &MODULE_ASCLIN0;
    
    // 等待发送 FIFO 不满
    while(asclin->TXFIFOCON.B.FILL >= 16);
    
    // 写入数据到 TXDATA 寄存器
    asclin->TXDATA.U = byte;
    
    // 等待发送完成
    while(asclin->TXFIFOCON.B.FILL > 0);
}

/***************************************************************
 *  @brief     裸机 UART 发送字符串
 *  @param     str  要发送的字符串
 *  @return    void
 **************************************************************/
void uart_send_string_bare(const char *str)
{
    while(*str)
    {
        uart_send_byte_bare((uint8)*str);
        str++;
    }
}
