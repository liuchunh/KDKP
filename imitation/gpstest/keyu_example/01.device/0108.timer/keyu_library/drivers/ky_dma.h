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
 * 文件名：[ky_dma.h]
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
#ifndef __KY_DMA_H__
#define __KY_DMA_H__

#include "ky_all.h"
#include "ky_uart.h"
#include "ky_spi.h"

/******************************************************************************/
/*-----------------------------Type Definitions-------------------------------*/
/******************************************************************************/

typedef enum {
    DMA_TYPE_UART_TX = 0,
    DMA_TYPE_UART_RX,
    DMA_TYPE_SPI
} DMA_Type_t;
typedef void (*DMA_Callback_t)(uint8 idx);
void dma_uart_start_tx(UART_Index_t idx, uint8 *txBuffer, uint16 size);            //启动UART DMA发送
void dma_uart_start_rx(UART_Index_t idx, uint8 *rxBuffer, uint16 size);            //启动UART DMA接收
void dma_spi_transceive(SPI_Index_t idx, uint8 *txBuffer, uint8 *rxBuffer, uint16 size);  //SPI DMA收发
void dma_disable(DMA_Type_t type, uint8 idx);                                      //禁用DMA通道
void dma_set_callback(DMA_Type_t type, uint8 idx, DMA_Callback_t cb);              //设置DMA回调函数
void dma_process_isr(DMA_Type_t type, uint8 idx);                                  //DMA中断处理
#endif /* __KY_DMA_H__ */