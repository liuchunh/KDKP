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
 * 文件名：[ky_spi.h]
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
#ifndef __KY_SPI_H__
#define __KY_SPI_H__

#include "Cpu/Std/Ifx_Types.h"
#include "Qspi/SpiMaster/IfxQspi_SpiMaster.h"
#include "ky_typedef.h"

// SPI 索引 (映射到 TC264 QSPI)
typedef enum {
    SPI_0 = 0,      // QSPI0
    SPI_1,          // QSPI1
    SPI_2,          // QSPI2
    SPI_3,          // QSPI3
} SPI_Index_t;
// SPI 模式 (CPOL/CPHA)
typedef enum {
    SPI_MODE0 = 0,     // CPOL=0, CPHA=0 (时钟空闲低，第1边沿采样)
    SPI_MODE1 = 1,     // CPOL=0, CPHA=1 (时钟空闲低，第2边沿采样)
    SPI_MODE2 = 2,     // CPOL=1, CPHA=0 (时钟空闲高，第1边沿采样)
    SPI_MODE3 = 3      // CPOL=1, CPHA=1 (时钟空闲高，第2边沿采样)
} SPI_Mode_t;
// MOSI 引脚选择
typedef enum {
    /* QSPI0 MTSR */
    SPI0_MOSI_P20_12 = 0,
    SPI0_MOSI_P20_14,
    
    /* QSPI1 MTSR */
    SPI1_MOSI_P10_1,
    SPI1_MOSI_P10_3,
    SPI1_MOSI_P10_4,
    SPI1_MOSI_P11_9,
    
    /* QSPI2 MTSR */
    SPI2_MOSI_P15_5,
    SPI2_MOSI_P15_6,
    SPI2_MOSI_P13_2, // LVDS N
    
    /* QSPI3 MTSR */
    SPI3_MOSI_P02_6,
    SPI3_MOSI_P10_6,
    SPI3_MOSI_P33_12,
    SPI3_MOSI_P22_0, // LVDS
    SPI3_MOSI_P22_2  // LVDS N
} SPI_Mosi_Pin_t;
// MISO 引脚选择
typedef enum {
    /* QSPI0 MRST */
    SPI0_MISO_P20_12 = 0,
    
    /* QSPI1 MRST */
    SPI1_MISO_P10_1,
    SPI1_MISO_P11_3,
    
    /* QSPI2 MRST */
    SPI2_MISO_P15_4,
    SPI2_MISO_P15_7,
    SPI2_MISO_P15_2,
    SPI2_MISO_P21_2, // LVDS N
    
    /* QSPI3 MRST */
    SPI3_MISO_P02_5,
    SPI3_MISO_P10_7,
    SPI3_MISO_P33_13,
    SPI3_MISO_P22_1,
    SPI3_MISO_P21_2,  // LVDS N
    
    SPI_MISO_NONE = 0xFF
} SPI_Miso_Pin_t;
// SCLK 引脚选择
typedef enum {
    /* QSPI0 SCLK */
    SPI0_SCLK_P20_11 = 0,
    SPI0_SCLK_P20_13,
    
    /* QSPI1 SCLK */
    SPI1_SCLK_P10_2,
    SPI1_SCLK_P11_6,
    
    /* QSPI2 SCLK */
    SPI2_SCLK_P15_3,
    SPI2_SCLK_P15_6,
    SPI2_SCLK_P15_8,
    SPI2_SCLK_P13_0, // LVDS N
    
    /* QSPI3 SCLK */
    SPI3_SCLK_P02_7,
    SPI3_SCLK_P10_8,
    SPI3_SCLK_P22_3,
    SPI3_SCLK_P33_11
} SPI_Sclk_Pin_t;
typedef struct {
    IfxQspi_SpiMaster spiMaster;
    IfxQspi_SpiMaster_Channel channel;
} SpiHandle_t;
void spi_init(SPI_Index_t spiIndex, SPI_Mode_t mode, uint32_t baud, 
            SPI_Mosi_Pin_t mosiPin, SPI_Miso_Pin_t misoPin, SPI_Sclk_Pin_t sclkPin);    //SPI初始化
// ... existing code ...
uint8_t spi_read_byte(SPI_Index_t spiIndex);                                            //SPI读取字节
void spi_read_byte_array(SPI_Index_t spiIndex, uint8_t *dat, uint32_t length);          //SPI读取字节数组
void spi_write_byte(SPI_Index_t spiIndex, const uint8_t dat);                           //SPI写入字节
void spi_write_byte_array(SPI_Index_t spiIndex, const uint8_t *dat, uint32_t length);   //SPI写入字节数组
uint8_t spi_swap_byte(SPI_Index_t spiIndex, uint8_t dat);                               //SPI交换字节(发送同时接收)
#endif /* __KY_SPI_H__ */