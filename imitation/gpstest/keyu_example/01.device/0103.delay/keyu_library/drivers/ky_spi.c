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
 * 文件名：[ky_spi.c]
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
#include "ky_spi.h"
#include "IfxQspi_SpiMaster.h"
#include "IfxQspi.h"
#include <string.h>


SpiHandle_t g_spiHandle[4];
static Ifx_QSPI_BACON g_bacon[4];       // BACON配置数组

// 获取QSPI模块指针
static Ifx_QSPI* spi_get_module(SPI_Index_t spiIndex)
{
    return IfxQspi_getAddress((IfxQspi_Index)spiIndex);
}

// 清除SPI接收FIFO
/***************************************************************
  *  @brief     SPI清除接收FIFO
  *  @param     module     [module description]
  *  @Sample usage:     spi_clear_rx_fifo(module);
 **************************************************************/
static void spi_clear_rx_fifo(Ifx_QSPI *module)
{
    uint32 fifoLevel = module->STATUS.B.RXFIFOLEVEL;
    while(fifoLevel--)
    {
        (void)IfxQspi_readReceiveFifo(module);
    }
}

// 配置SPI引脚复用
static void spi_config_pins(SPI_Index_t spiIndex, SPI_Sclk_Pin_t sclkPin, 
                          SPI_Mosi_Pin_t mtsrPin, SPI_Miso_Pin_t mrstPin,
                          IfxQspi_SpiMaster_Pins *pins)
{
    pins->mrstMode  = IfxPort_InputMode_pullDown; 
    pins->mtsrMode  = IfxPort_OutputMode_pushPull;
    pins->sclkMode  = IfxPort_OutputMode_pushPull;
    pins->pinDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    switch(spiIndex)
    {
        case SPI_0:
            if      (SPI0_SCLK_P20_11 == sclkPin)   pins->sclk = &IfxQspi0_SCLK_P20_11_OUT;
            else if (SPI0_SCLK_P20_13 == sclkPin)   pins->sclk = &IfxQspi0_SCLK_P20_13_OUT;
            
            if      (SPI0_MOSI_P20_12 == mtsrPin)   pins->mtsr = &IfxQspi0_MTSR_P20_12_OUT;
            else if (SPI0_MOSI_P20_14 == mtsrPin)   pins->mtsr = &IfxQspi0_MTSR_P20_14_OUT;
            
            if      (SPI0_MISO_P20_12 == mrstPin)   pins->mrst = &IfxQspi0_MRSTA_P20_12_IN;
            break;

        case SPI_1:
            if      (SPI1_SCLK_P10_2  == sclkPin)   pins->sclk = &IfxQspi1_SCLK_P10_2_OUT;
            else if (SPI1_SCLK_P11_6  == sclkPin)   pins->sclk = &IfxQspi1_SCLK_P11_6_OUT;
            
            if      (SPI1_MOSI_P10_1  == mtsrPin)   pins->mtsr = &IfxQspi1_MTSR_P10_1_OUT;
            else if (SPI1_MOSI_P10_3  == mtsrPin)   pins->mtsr = &IfxQspi1_MTSR_P10_3_OUT;
            else if (SPI1_MOSI_P10_4  == mtsrPin)   pins->mtsr = &IfxQspi1_MTSR_P10_4_OUT;
            else if (SPI1_MOSI_P11_9  == mtsrPin)   pins->mtsr = &IfxQspi1_MTSR_P11_9_OUT;
            
            if      (SPI1_MISO_P10_1  == mrstPin)   pins->mrst = &IfxQspi1_MRSTA_P10_1_IN;
            else if (SPI1_MISO_P11_3  == mrstPin)   pins->mrst = &IfxQspi1_MRSTB_P11_3_IN;
            break;

        case SPI_2:
            if      (SPI2_SCLK_P15_3  == sclkPin)   pins->sclk = &IfxQspi2_SCLK_P15_3_OUT;
            else if (SPI2_SCLK_P15_6  == sclkPin)   pins->sclk = &IfxQspi2_SCLK_P15_6_OUT;
            else if (SPI2_SCLK_P15_8  == sclkPin)   pins->sclk = &IfxQspi2_SCLK_P15_8_OUT;
            else if (SPI2_SCLK_P13_0  == sclkPin)   pins->sclk = &IfxQspi2_SCLKN_P13_0_OUT;
            
            if      (SPI2_MOSI_P15_5  == mtsrPin)   pins->mtsr = &IfxQspi2_MTSR_P15_5_OUT;
            else if (SPI2_MOSI_P15_6  == mtsrPin)   pins->mtsr = &IfxQspi2_MTSR_P15_6_OUT;
            else if (SPI2_MOSI_P13_2  == mtsrPin)   pins->mtsr = &IfxQspi2_MTSRP_P13_3_OUT;
            
            // 只有当 MISO 不是 NONE 时才配置
            if (mrstPin != SPI_MISO_NONE)
            {
                if      (SPI2_MISO_P15_4  == mrstPin)   pins->mrst = &IfxQspi2_MRSTA_P15_4_IN;
                else if (SPI2_MISO_P15_7  == mrstPin)   pins->mrst = &IfxQspi2_MRSTB_P15_7_IN;
                else if (SPI2_MISO_P15_2  == mrstPin)   pins->mrst = &IfxQspi2_MRSTE_P15_2_IN;
                else if (SPI2_MISO_P21_2  == mrstPin)   pins->mrst = &IfxQspi2_MRSTCN_P21_2_IN;
            }
            else
            {
                pins->mrst = NULL_PTR;  // 不配置 MISO 引脚
            }
            break;

        case SPI_3:
            if      (SPI3_SCLK_P02_7  == sclkPin)   pins->sclk = &IfxQspi3_SCLK_P02_7_OUT;
            else if (SPI3_SCLK_P10_8  == sclkPin)   pins->sclk = &IfxQspi3_SCLK_P10_8_OUT;
            else if (SPI3_SCLK_P22_3  == sclkPin)   pins->sclk = &IfxQspi3_SCLK_P22_3_OUT;
            else if (SPI3_SCLK_P33_11 == sclkPin)   pins->sclk = &IfxQspi3_SCLK_P33_11_OUT;
            
            if      (SPI3_MOSI_P02_6  == mtsrPin)   pins->mtsr = &IfxQspi3_MTSR_P02_6_OUT;
            else if (SPI3_MOSI_P10_6  == mtsrPin)   pins->mtsr = &IfxQspi3_MTSR_P10_6_OUT;
            else if (SPI3_MOSI_P33_12 == mtsrPin)   pins->mtsr = &IfxQspi3_MTSR_P33_12_OUT;
            else if (SPI3_MOSI_P22_0  == mtsrPin)   pins->mtsr = &IfxQspi3_MTSR_P22_0_OUT;
            else if (SPI3_MOSI_P22_2  == mtsrPin)   pins->mtsr = &IfxQspi3_MTSRP_P22_3_OUT;
            
            if      (SPI3_MISO_P02_5  == mrstPin)   pins->mrst = &IfxQspi3_MRSTA_P02_5_IN;
            else if (SPI3_MISO_P10_7  == mrstPin)   pins->mrst = &IfxQspi3_MRSTB_P10_7_IN;
            else if (SPI3_MISO_P33_13 == mrstPin)   pins->mrst = &IfxQspi3_MRSTD_P33_13_IN;
            else if (SPI3_MISO_P22_1  == mrstPin)   pins->mrst = &IfxQspi3_MRSTE_P22_1_IN;
            else if (SPI3_MISO_P21_2  == mrstPin)   pins->mrst = &IfxQspi3_MRSTFN_P21_2_IN;
            break;
    }
}

/***************************************************************
  *  @brief     初始化并配置指定的SPI通道
  *  @param     spiIndex    SPI外设 (SPI_0 ~ SPI_3)
  *  @param     mode        SPI工作模式
  *  @param     baud        SPI通信速率 (Hz)
  *  @param     mosiPin     MOSI引脚
  *  @param     misoPin     MISO引脚
  *  @param     sclkPin     SCLK引脚
 **************************************************************/
void spi_init(SPI_Index_t spiIndex, SPI_Mode_t mode, uint32_t baud, 
            SPI_Mosi_Pin_t mosiPin, SPI_Miso_Pin_t misoPin, SPI_Sclk_Pin_t sclkPin)
{
    IfxQspi_SpiMaster_Config masterConfig;
    IfxQspi_SpiMaster_Pins masterPins;
    IfxQspi_SpiMaster_ChannelConfig chConfig;
    Ifx_QSPI *module = spi_get_module(spiIndex);

    memset(&masterPins, 0, sizeof(IfxQspi_SpiMaster_Pins));
    
    masterPins.sclkMode = IfxPort_OutputMode_pushPull;
    masterPins.mtsrMode = IfxPort_OutputMode_pushPull;
    masterPins.mrstMode = IfxPort_InputMode_pullDown;
    masterPins.pinDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    spi_config_pins(spiIndex, sclkPin, mosiPin, misoPin, &masterPins);

    IfxQspi_SpiMaster_initModuleConfig(&masterConfig, module);
    masterConfig.mode = IfxQspi_Mode_master;
    masterConfig.maximumBaudrate = 50000000; 
    masterConfig.isrProvider = IfxSrc_Tos_cpu0;
    masterConfig.pins = &masterPins;

    IfxQspi_SpiMaster_initModule(&g_spiHandle[spiIndex].spiMaster, &masterConfig);

    IfxQspi_SpiMaster_initChannelConfig(&chConfig, &g_spiHandle[spiIndex].spiMaster);
    chConfig.ch.baudrate = (float32)baud;
    chConfig.ch.channelId = IfxQspi_ChannelId_0;
    chConfig.sls.output.pin = NULL_PTR;          
    
    switch(mode)
    {
        case SPI_MODE0:
            chConfig.ch.mode.clockPolarity = (boolean)IfxQspi_ClockPolarity_idleLow;
            chConfig.ch.mode.shiftClock = (boolean)IfxQspi_ShiftClock_shiftTransmitDataOnTrailingEdge;
            break;
        case SPI_MODE1:
            chConfig.ch.mode.clockPolarity = (boolean)IfxQspi_ClockPolarity_idleLow;
            chConfig.ch.mode.shiftClock = (boolean)IfxQspi_ShiftClock_shiftTransmitDataOnLeadingEdge;
            break;
        case SPI_MODE2:
            chConfig.ch.mode.clockPolarity = (boolean)IfxQspi_ClockPolarity_idleHigh;
            chConfig.ch.mode.shiftClock = (boolean)IfxQspi_ShiftClock_shiftTransmitDataOnTrailingEdge;
            break;
        case SPI_MODE3:
            chConfig.ch.mode.clockPolarity = (boolean)IfxQspi_ClockPolarity_idleHigh;
            chConfig.ch.mode.shiftClock = (boolean)IfxQspi_ShiftClock_shiftTransmitDataOnLeadingEdge;
            break;
    }
    
    chConfig.ch.mode.dataHeading = (boolean)IfxQspi_DataHeading_msbFirst;
    chConfig.ch.mode.dataWidth = 8;
    chConfig.ch.mode.csActiveLevel = (boolean)Ifx_ActiveState_low;
    
    IfxQspi_SpiMaster_initChannel(&g_spiHandle[spiIndex].channel, &chConfig);

    g_bacon[spiIndex].U = 0;
    g_bacon[spiIndex].B.DL = 7;
    g_bacon[spiIndex].B.MSB = 1;
    g_bacon[spiIndex].B.BYTE = 0;
    g_bacon[spiIndex].B.IDLE = 1;
    g_bacon[spiIndex].B.IPRE = 1;
    g_bacon[spiIndex].B.LEAD = 1;
    g_bacon[spiIndex].B.LPRE = 1;
    g_bacon[spiIndex].B.TRAIL = 1;
    g_bacon[spiIndex].B.TPRE = 1;
    g_bacon[spiIndex].B.CS = IfxQspi_ChannelId_0;

    IfxQspi_configPT1Event(module, IfxQspi_PhaseTransitionEvent_endOfFrame);
}

/***************************************************************
  *  @brief     通过SPI发送一个字节
  *  @param     spiIndex    SPI外设
  *  @param     dat         要发送的数据
 **************************************************************/
void spi_write_byte(SPI_Index_t spiIndex, const uint8_t dat)
{
    Ifx_QSPI *module = spi_get_module(spiIndex);

    IfxQspi_writeBasicConfigurationEndStream(module, g_bacon[spiIndex].U);
    IfxQspi_writeTransmitFifo(module, dat);

    while(module->STATUS.B.TXFIFOLEVEL != 0);
    while(module->STATUS.B.PT1F == 0);

    IfxQspi_clearAllEventFlags(module);
    spi_clear_rx_fifo(module);
}

/***************************************************************
  *  @brief     通过SPI连续发送一个字节数组
  *  @param     spiIndex    SPI外设
  *  @param     dat         发送缓冲区的指针
  *  @param     length      发送长度
 **************************************************************/
void spi_write_byte_array(SPI_Index_t spiIndex, const uint8_t *dat, uint32_t length)
{
    Ifx_QSPI *module = spi_get_module(spiIndex);
    uint32 i;

    spi_clear_rx_fifo(module);

    for(i = 0; i < length; i++)
    {
        // 1. 发送配置字 (BACON)
        while(module->STATUS.B.TXFIFOLEVEL >= 4); // 等待至少1个空位
        
        if(i == length - 1)
        {
            IfxQspi_writeBasicConfigurationEndStream(module, g_bacon[spiIndex].U);
        }
        else
        {
            IfxQspi_writeBasicConfigurationBeginStream(module, g_bacon[spiIndex].U);
        }

        // 2. 发送数据 (DATA)
        while(module->STATUS.B.TXFIFOLEVEL >= 4); // 再次等待至少1个空位，防止连续写入导致溢出
        IfxQspi_writeTransmitFifo(module, dat[i]);
    }

    while(module->STATUS.B.PT1F == 0);
    IfxQspi_clearAllEventFlags(module);
    spi_clear_rx_fifo(module);
}

/***************************************************************
  *  @brief     通过SPI接收一个字节
  *  @param     spiIndex    SPI外设
  *  @return    uint8_t     接收到的数据
 **************************************************************/
uint8_t spi_read_byte(SPI_Index_t spiIndex)
{
    Ifx_QSPI *module = spi_get_module(spiIndex);

    spi_clear_rx_fifo(module);

    IfxQspi_writeBasicConfigurationEndStream(module, g_bacon[spiIndex].U);
    IfxQspi_writeTransmitFifo(module, 0x00);

    while(module->STATUS.B.RXFIFOLEVEL == 0);
    while(module->STATUS.B.PT1F == 0);

    IfxQspi_clearAllEventFlags(module);
    return (uint8)IfxQspi_readReceiveFifo(module);
}

/***************************************************************
  *  @brief     通过SPI连续接收多年字节
  *  @param     spiIndex    SPI外设
  *  @param     dat         接收缓冲区的指针
  *  @param     length      接收长度
 **************************************************************/
void spi_read_byte_array(SPI_Index_t spiIndex, uint8_t *dat, uint32_t length)
{
    Ifx_QSPI *module = spi_get_module(spiIndex);
    uint32 i;

    spi_clear_rx_fifo(module);

    for(i = 0; i < length; i++)
    {
        while(module->STATUS.B.TXFIFOLEVEL >= 4);

        if(i == length - 1)
        {
            IfxQspi_writeBasicConfigurationEndStream(module, g_bacon[spiIndex].U);
        }
        else
        {
            IfxQspi_writeBasicConfigurationBeginStream(module, g_bacon[spiIndex].U);
        }

        IfxQspi_writeTransmitFifo(module, 0x00);

        while(module->STATUS.B.RXFIFOLEVEL == 0);
        dat[i] = (uint8)IfxQspi_readReceiveFifo(module);
    }

    while(module->STATUS.B.PT1F == 0);
    IfxQspi_clearAllEventFlags(module);
}

/***************************************************************
  *  @brief     通过SPI交换一个字节(发送数据的同时接收数据)
  *  @param     spiIndex    SPI外设
  *  @param     dat         要发送的数据
  *  @return    uint8_t     接收到的数据
 **************************************************************/
uint8_t spi_swap_byte(SPI_Index_t spiIndex, uint8_t dat)
{
    Ifx_QSPI *module = spi_get_module(spiIndex);

    spi_clear_rx_fifo(module);

    IfxQspi_writeBasicConfigurationEndStream(module, g_bacon[spiIndex].U);
    IfxQspi_writeTransmitFifo(module, dat);

    while(module->STATUS.B.RXFIFOLEVEL == 0);
    while(module->STATUS.B.PT1F == 0);

    IfxQspi_clearAllEventFlags(module);
    return (uint8)IfxQspi_readReceiveFifo(module);
}
