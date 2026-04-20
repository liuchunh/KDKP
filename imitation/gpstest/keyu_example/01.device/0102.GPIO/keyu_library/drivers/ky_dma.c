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
 * 文件名：[ky_dma.c]
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
#include "ky_dma.h"
#include "Dma/Dma/IfxDma_Dma.h"
#include "Src/Std/IfxSrc.h"
#include "Cpu/Std/IfxCpu.h"
#include "IfxSrc_reg.h" 

/* DMA回调函数数组 */
static DMA_Callback_t s_DmaTxCallbacks[4] = {0}; 
static DMA_Callback_t s_DmaRxCallbacks[4] = {0}; 
static DMA_Callback_t s_DmaSpiCallbacks[4] = {0}; 

/* DMA模块句柄 */
static IfxDma_Dma g_dmaModule;
static boolean g_dmaInitialized = FALSE;

/* DMA通道句柄 */
static IfxDma_Dma_Channel g_dmaUartTxChannel[4];
static IfxDma_Dma_Channel g_dmaUartRxChannel[4];
static IfxDma_Dma_Channel g_dmaSpiTxChannel[4];
static IfxDma_Dma_Channel g_dmaSpiRxChannel[4];

/* 引用外部UART句柄用于获取寄存器地址 */
extern UartHandle_t g_uartHandle[4];

/* 初始化DMA模块 (内部调用) */
static void dma_internal_init(void)
{
    IfxDma_Dma_Config dmaConfig;
    if (g_dmaInitialized) return;
    
    IfxDma_Dma_initModuleConfig(&dmaConfig, &MODULE_DMA);
    IfxDma_Dma_initModule(&g_dmaModule, &dmaConfig);
    g_dmaInitialized = TRUE;
}

/***************************************************************
  *  @brief     获取UART DMA请求源 (通道号)
  *  @param     uartx     串口索引 (UART_0 ~ UART_3)
  *  @param     isTx      是否为发送DMA
  *  @Sample usage:     dma_get_uart_request_source(UART_1, TRUE);
 **************************************************************/
static IfxDma_ChannelId dma_get_uart_request_source(UART_Index_t uartx, boolean isTx)
{
    if (isTx)
    {
        switch (uartx)
        {
            case UART_0: return IfxDma_ChannelId_0;
            case UART_1: return IfxDma_ChannelId_2;
            case UART_2: return IfxDma_ChannelId_4;
            case UART_3: return IfxDma_ChannelId_6;
            default: return IfxDma_ChannelId_0;
        }
    }
    else
    {
        switch (uartx)
        {
            case UART_0: return IfxDma_ChannelId_1;
            case UART_1: return IfxDma_ChannelId_3;
            case UART_2: return IfxDma_ChannelId_5;
            case UART_3: return IfxDma_ChannelId_7;
            default: return IfxDma_ChannelId_1;
        }
    }
}

/***************************************************************
  *  @brief     获取SPI DMA请求源 (通道号)
  *  @param     spix      SPI索引 (SPI_0 ~ SPI_3)
  *  @param     isTx      是否为发送DMA
  *  @Sample usage:     dma_get_spi_request_source(SPI_1, TRUE);
 **************************************************************/
/***************************************************************
  *  @brief     获取SPI DMA请求源 (通道号)
  *  @param     spix      SPI索引 (SPI_0 ~ SPI_3)
  *  @param     isTx      是否为发送DMA
  *  @Sample usage:     dma_get_spi_request_source(SPI_1, TRUE);
 **************************************************************/
static IfxDma_ChannelId dma_get_spi_request_source(SPI_Index_t spix, boolean isTx)
{
    /* SPI使用通道 8-15 */
    /* SPI0: TX=8, RX=9 */
    /* SPI1: TX=10, RX=11 */
    /* SPI2: TX=12, RX=13 */
    /* SPI3: TX=14, RX=15 */
    uint8 base = 8 + (uint8)spix * 2;
    return (IfxDma_ChannelId)(isTx ? base : base + 1);
}

/***************************************************************
  *  @brief     获取ASCLIN SRC指针
  *  @param     uartx     串口索引
  *  @param     isTx      是否为发送
 **************************************************************/
static volatile Ifx_SRC_SRCR* dma_get_asclin_src(UART_Index_t uartx, boolean isTx)
{
    if (isTx)
    {
        return &MODULE_SRC.ASCLIN.ASCLIN[(uint8)uartx].TX;
    }
    else
    {
        return &MODULE_SRC.ASCLIN.ASCLIN[(uint8)uartx].RX;
    }
}

/***************************************************************
  *  @brief     获取QSPI SRC指针
  *  @param     spix      SPI索引
  *  @param     isTx      是否为发送
 **************************************************************/
static volatile Ifx_SRC_SRCR* dma_get_qspi_src(SPI_Index_t spix, boolean isTx)
{
    if (isTx)
    {
        return &MODULE_SRC.QSPI.QSPI[(uint8)spix].TX;
    }
    else
    {
        return &MODULE_SRC.QSPI.QSPI[(uint8)spix].RX;
    }
}

/***************************************************************
  *  @brief     注册 DMA 完成回调函数
  *  @param     type    外设类型
  *  @param     idx     外设索引
  *  @param     cb      回调函数 (NULL 则取消注册)
  *  @Sample usage:     dma_set_callback(DMA_TYPE_UART_TX, UART_1, MyTxDoneCallback);
  **************************************************************/
void dma_set_callback(DMA_Type_t type, uint8 idx, DMA_Callback_t cb)
{
    if (idx >= 4) return;

    switch (type)
    {
        case DMA_TYPE_UART_TX:
            s_DmaTxCallbacks[idx] = cb;
            break;
        case DMA_TYPE_UART_RX:
            s_DmaRxCallbacks[idx] = cb;
            break;
        case DMA_TYPE_SPI:
            s_DmaSpiCallbacks[idx] = cb;
            break;
        default:
            break;
    }
}

/***************************************************************
  *  @brief     在中断中调用的处理函数
  *  @param     type    外设类型
  *  @param     idx     索引
  *  @Sample usage:     dma_process_isr(DMA_TYPE_UART_TX, UART_1);
  **************************************************************/
void dma_process_isr(DMA_Type_t type, uint8 idx)
{
    volatile Ifx_SRC_SRCR *src;
    IfxDma_ChannelId dmaId;

    if (idx >= 4) return;
    
    switch (type)
    {
        case DMA_TYPE_UART_TX:
            dmaId = dma_get_uart_request_source((UART_Index_t)idx, TRUE);
            src = IfxDma_getSrcPointer(&MODULE_DMA, dmaId);
            IfxSrc_clearRequest(src);
            
            if (s_DmaTxCallbacks[idx])
            {
                s_DmaTxCallbacks[idx](idx);
            }
            break;
            
        case DMA_TYPE_UART_RX:
            dmaId = dma_get_uart_request_source((UART_Index_t)idx, FALSE);
            src = IfxDma_getSrcPointer(&MODULE_DMA, dmaId);
            IfxSrc_clearRequest(src);
            
            if (s_DmaRxCallbacks[idx])
            {
                s_DmaRxCallbacks[idx](idx);
            }
            break;
            
        case DMA_TYPE_SPI:
            dmaId = dma_get_spi_request_source((SPI_Index_t)idx, FALSE);
            src = IfxDma_getSrcPointer(&MODULE_DMA, dmaId);
            IfxSrc_clearRequest(src);
            
            dmaId = dma_get_spi_request_source((SPI_Index_t)idx, TRUE);
            src = IfxDma_getSrcPointer(&MODULE_DMA, dmaId);
            IfxSrc_clearRequest(src);

            if (s_DmaSpiCallbacks[idx])
            {
                s_DmaSpiCallbacks[idx](idx);
            }
            break;
    }
}

/***************************************************************
  *  @brief     启动 UART DMA 发送
  *  @param     idx         串口索引 (UART_0 ~ UART_3)
  *  @param     txBuffer    发送数据缓冲区地址
  *  @param     size        发送字节数
  *  @Sample usage:     dma_uart_start_tx(UART_1, buffer, 10);
  **************************************************************/
void dma_uart_start_tx(UART_Index_t idx, uint8 *txBuffer, uint16 size)
{
    IfxDma_Dma_ChannelConfig channelConfig;
    volatile Ifx_SRC_SRCR *uartSrc; 
    
    if (size == 0 || txBuffer == NULL) return;
    dma_internal_init();

    IfxDma_Dma_initChannelConfig(&channelConfig, &g_dmaModule);
    channelConfig.channelId = dma_get_uart_request_source(idx, TRUE);
    channelConfig.sourceAddress = (uint32)txBuffer;
    channelConfig.destinationAddress = (uint32)&(g_uartHandle[idx].asc.asclin->TXDATA.U);
    channelConfig.transferCount = size;
    channelConfig.moveSize = IfxDma_ChannelMoveSize_8bit;
    channelConfig.requestMode = IfxDma_ChannelRequestMode_oneTransferPerRequest;
    channelConfig.hardwareRequestEnabled = TRUE; 
    
    uartSrc = dma_get_asclin_src(idx, TRUE);
    IfxSrc_init(uartSrc, IfxSrc_Tos_dma, channelConfig.channelId);
    IfxSrc_enable(uartSrc);

    IfxDma_Dma_initChannel(&g_dmaUartTxChannel[idx], &channelConfig);
    IfxDma_Dma_startChannelTransaction(&g_dmaUartTxChannel[idx]);
}

/***************************************************************
  *  @brief     配置 UART DMA 接收
  *  @param     idx         串口索引 (UART_0 ~ UART_3)
  *  @param     rxBuffer    接收数据缓冲区地址
  *  @param     size        缓冲区最大接收长度
  *  @Sample usage:     dma_uart_start_rx(UART_1, rxBuf, 100);
  **************************************************************/
void dma_uart_start_rx(UART_Index_t idx, uint8 *rxBuffer, uint16 size)
{
    IfxDma_Dma_ChannelConfig channelConfig;
    volatile Ifx_SRC_SRCR *uartSrc;

    if (size == 0 || rxBuffer == NULL) return;
    dma_internal_init();

    IfxDma_Dma_initChannelConfig(&channelConfig, &g_dmaModule);
    channelConfig.channelId = dma_get_uart_request_source(idx, FALSE);
    channelConfig.sourceAddress = (uint32)&(g_uartHandle[idx].asc.asclin->RXDATA.U);
    channelConfig.destinationAddress = (uint32)rxBuffer;
    channelConfig.transferCount = size;
    channelConfig.moveSize = IfxDma_ChannelMoveSize_8bit;
    channelConfig.requestMode = IfxDma_ChannelRequestMode_oneTransferPerRequest;
    channelConfig.hardwareRequestEnabled = TRUE;

    uartSrc = dma_get_asclin_src(idx, FALSE);
    IfxSrc_init(uartSrc, IfxSrc_Tos_dma, channelConfig.channelId);
    IfxSrc_enable(uartSrc);

    IfxDma_Dma_initChannel(&g_dmaUartRxChannel[idx], &channelConfig);
    IfxDma_Dma_startChannelTransaction(&g_dmaUartRxChannel[idx]);
}

/***************************************************************
  *  @brief     禁止/停止指定的 DMA 传输
  *  @param     type      外设类型 (DMA_TYPE_UART_TX, etc.)
  *  @param     idx       外设索引
  *  @Sample usage:     dma_disable(DMA_TYPE_UART_TX, UART_1);
  **************************************************************/
void dma_disable(DMA_Type_t type, uint8 idx)
{
    IfxDma_ChannelId dmaId;
    if (idx >= 4) return;

    switch (type)
    {
        case DMA_TYPE_UART_TX:
            dmaId = dma_get_uart_request_source((UART_Index_t)idx, TRUE);
            break;
        case DMA_TYPE_UART_RX:
            dmaId = dma_get_uart_request_source((UART_Index_t)idx, FALSE);
            break;
        case DMA_TYPE_SPI:
            {
                IfxDma_ChannelId txId = dma_get_spi_request_source((SPI_Index_t)idx, TRUE);
                IfxDma_ChannelId rxId = dma_get_spi_request_source((SPI_Index_t)idx, FALSE);
                IfxDma_disableChannelTransaction(&MODULE_DMA, txId);
                IfxDma_disableChannelTransaction(&MODULE_DMA, rxId);
            }
            return; /* Done for SPI */
        default:
            return;
    }
    
    IfxDma_disableChannelTransaction(&MODULE_DMA, dmaId);
}

/***************************************************************
  *  @brief     启动 SPI DMA 传输 (全双工)
  *  @param     idx         SPI 索引 (SPI_0 ~ SPI_3)
  *  @param     txBuffer    发送缓冲区地址 (若为 NULL 则不开启发送DMA)
  *  @param     rxBuffer    接收缓冲区地址 (若为 NULL 则不开启接收DMA)
  *  @param     size        传输字节数
  *  @Sample usage:     dma_spi_transceive(SPI_1, tx, rx, 10);
  **************************************************************/
void dma_spi_transceive(SPI_Index_t idx, uint8 *txBuffer, uint8 *rxBuffer, uint16 size)
{
    Ifx_QSPI *qspiModule;
    IfxDma_Dma_ChannelConfig txConfig, rxConfig;
    volatile Ifx_SRC_SRCR *qspiSrc;
    
    if (size == 0) return;
    dma_internal_init();
    
    qspiModule = IfxQspi_getAddress((IfxQspi_Index)idx);

    if (rxBuffer != NULL)
    {
        IfxDma_Dma_initChannelConfig(&rxConfig, &g_dmaModule);
        rxConfig.channelId = dma_get_spi_request_source(idx, FALSE);
        rxConfig.sourceAddress = (uint32)&qspiModule->RXEXIT.U;
        rxConfig.destinationAddress = (uint32)rxBuffer;
        rxConfig.transferCount = size;
        rxConfig.moveSize = IfxDma_ChannelMoveSize_8bit;
        rxConfig.requestMode = IfxDma_ChannelRequestMode_oneTransferPerRequest;
        rxConfig.hardwareRequestEnabled = TRUE;
        
        qspiSrc = dma_get_qspi_src(idx, FALSE);
        IfxSrc_init(qspiSrc, IfxSrc_Tos_dma, rxConfig.channelId);
        IfxSrc_enable(qspiSrc);

        IfxDma_Dma_initChannel(&g_dmaSpiRxChannel[idx], &rxConfig);
        IfxDma_Dma_startChannelTransaction(&g_dmaSpiRxChannel[idx]);
    }

    if (txBuffer != NULL)
    {
        IfxDma_Dma_initChannelConfig(&txConfig, &g_dmaModule);
        txConfig.channelId = dma_get_spi_request_source(idx, TRUE);
        txConfig.sourceAddress = (uint32)txBuffer;
        txConfig.destinationAddress = (uint32)&qspiModule->DATAENTRY[0].U;
        txConfig.transferCount = size;
        txConfig.moveSize = IfxDma_ChannelMoveSize_8bit;
        txConfig.requestMode = IfxDma_ChannelRequestMode_oneTransferPerRequest;
        txConfig.hardwareRequestEnabled = TRUE;

        qspiSrc = dma_get_qspi_src(idx, TRUE);
        IfxSrc_init(qspiSrc, IfxSrc_Tos_dma, txConfig.channelId);
        IfxSrc_enable(qspiSrc);

        IfxDma_Dma_initChannel(&g_dmaSpiTxChannel[idx], &txConfig);
        IfxDma_Dma_startChannelTransaction(&g_dmaSpiTxChannel[idx]);
    }
}
