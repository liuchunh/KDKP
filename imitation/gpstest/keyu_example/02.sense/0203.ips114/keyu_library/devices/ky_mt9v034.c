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
 * 文件名：[ky_mt9v034.c]
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
#include "ky_mt9v034.h"
#include "ky_config.h"

#include "ky_soft_i2c.h"
#include "ky_delay.h"
#include "ky_gpio.h"
#include "Dma/Dma/IfxDma_Dma.h"
#include "Src/Std/IfxSrc.h"
#include "Cpu/Std/IfxCpu.h"
#include "Port/Std/IfxPort.h"
#include "Scu/Std/IfxScuEru.h"
#include "Scu/Std/IfxScuWdt.h" // Needed for WDT functions



#include "IfxScu_PinMap.h"
#include "Isr.h"

uint8 mt9_v034__image[mt9_v034_height][mt9_v034_width];
static volatile uint8 new_frame_ready = 0;
static SoftI2c_Info_t g_mt9v034_i2c;
static IfxDma_Dma g_dmaModule;
static IfxDma_Dma_Channel g_mt9v034DmaChannel;



static uint8 mt9_v034_dma__init(void);
static void configureDataPins(void);



/***************************************************************
  *  @brief     读取MT9V034的寄存器值
  *  @param     reg     [reg description]
  *  @Sample usage:     mt9v034_read_reg(reg);
 **************************************************************/
static void _mt9v034_set_config(void);
static void _mt9v034_set_frame_rate(uint8 fps);
static void _mt9v034_set_auto_gain(uint8 enable);
static void _mt9v034_set_auto_exposure(uint8 enable);


/***************************************************************
  *  @brief     读取MT9V034的寄存器值
  *  @param     reg     [reg description]
  *  @Sample usage:     mt9v034_read_reg(reg);
 **************************************************************/
uint16 mt9v034_read_reg(uint8 reg)
{
    return soft_i2c_read_reg16(&g_mt9v034_i2c, reg);
}

/***************************************************************
  *  @brief     写入MT9V034寄存器
  *  @param     reg     [reg description]
  *  @param     data     [data description]
  *  @Sample usage:     mt9v034_write_reg(reg, data);
 **************************************************************/
void mt9v034_write_reg(uint8 reg, uint16 data)
{
    soft_i2c_write_reg16(&g_mt9v034_i2c, reg, data);
    delay_ms(1);
}

/***************************************************************
  *  @brief     MT9V034设置EXPOSURE
  *  @param     exposure     [exposure description]
  *  @Sample usage:     mt9_v034_set_exposure(exposure);
 **************************************************************/
void mt9_v034_set_exposure(uint16 exposure)
{
    // Update config struct
    mt9v034_config.exposure_time = exposure;
    // Apply (force manual mode if calling this directly? Or stick to config?)
    // STC logic implies we should just set the register if manual, or update target.
    // For compatibility with old API, we write directly to 0x0B
    mt9v034_write_reg(0x0B, exposure);
}


/***************************************************************
  *  @brief     MT9V034设置GAIN
  *  @param     gain     [gain description]
  *  @Sample usage:     mt9_v034_set_gain(gain);
 **************************************************************/
void mt9_v034_set_gain(uint16 gain)
{
    mt9v034_config.gain = (uint8)gain;
    mt9v034_write_reg(0x35, gain);
}



/***************************************************************
  *  @brief     配置MT9V034的数据引脚
  *  @param     None
  *  @Sample usage:     configureDataPins();
 **************************************************************/
static void configureDataPins(void)
{
    volatile uint32 *pdisc = (volatile uint32 *)&MODULE_P00.PDISC.U;
    
    uint16 passwd = IfxScuWdt_getCpuWatchdogPassword();
    IfxScuWdt_clearCpuEndinit(passwd);
    
    *pdisc &= ~(0x000000FF);
    
    IfxScuWdt_setCpuEndinit(passwd);

    for(uint8 i = 0; i < 8; i++)
    {
        gpio_init((GPIO_Pin_t)(GPIO_P00 + i), GPIO_MODE_IN_FLOATING, GPIO_HIGH);
    }
}

// Ported Helper Functions

static void _mt9v034_set_config(void)
{
    uint16 i;
    // Soft Reset
    mt9v034_write_reg(0x0C, 0x01);
    delay_ms(50);
    mt9v034_write_reg(0x0C, 0x00);
    delay_ms(50);

    for (i = 0; i < mt9v034_reg_config_table_size; i++)
    {
        mt9v034_write_reg((uint8)mt9v034_reg_config_table[i][0],
                           mt9v034_reg_config_table[i][1]);
        delay_us(10);
    }
}

static void _mt9v034_set_frame_rate(uint8 fps)
{
    float vertical = 0;
    if (fps > 200)
        vertical = 2;
    else if (fps >= 150)
        vertical = -1.06f * fps + 251;
    else if (fps > 100)
        vertical = -2.14f * fps + 413;
    else if (fps >= 71)
        vertical = -4.5017f * fps + 649.17f;
    else if (fps > 50)
        vertical = -8.8517f * fps + 957.58f;
    else if (fps > 20)
        vertical = -31.167f * fps + 2073.3f;
    else
        vertical = -155.0f * fps + 4550;
    mt9v034_write_reg(0x06, (uint16)vertical);
}

static void _mt9v034_set_auto_gain(uint8 enable)
{
    uint16 reg = mt9v034_read_reg(0xAF); /* AEC_AGC_ENABLE */
    uint8 gain = mt9v034_config.gain;

    /* 限制增益范围 [16, 64] */
    if (gain < 16)
        gain = 16;
    if (gain > 64)
        gain = 64;

    if (enable)
    {
        /* 启用 AGC - 设置 AGC 使能位 (bit 1) */
        mt9v034_write_reg(0xAF, reg | 0x02);
        /* 设置最大增益 (MAX_GAIN 0xAB) */
        mt9v034_write_reg(0xAB, gain);
    }
    else
    {
        /* 禁用 AGC - 清除 AGC 使能位 */
        mt9v034_write_reg(0xAF, reg & ~0x02);
        /* 设置固定模拟增益 (ANALOG_GAIN 0x35) */
        mt9v034_write_reg(0x35, gain);
    }
}

static void _mt9v034_set_auto_exposure(uint8 enable)
{
    uint16 reg = mt9v034_read_reg(0xAF); /* AEC_AGC_ENABLE */
    uint16 exp_time = mt9v034_config.exposure_time;
    uint8 brightness = mt9v034_config.brightness;

    /* 计算最大曝光时间限制 */
    uint16 window_height = mt9v034_read_reg(0x03);
    uint16 vertical_blanking = mt9v034_read_reg(0x06);
    uint16 max_exp_time = window_height + vertical_blanking - 1;

    if (max_exp_time > 32767)
        max_exp_time = 32767;
    if (exp_time > max_exp_time)
        exp_time = max_exp_time;

    /* 限制亮度范围 [1, 64] */
    if (brightness < 1)
        brightness = 1;
    if (brightness > 64)
        brightness = 64;

    if (enable)
    {
        /* 启用 AEC - 设置 AEC 使能位 (bit 0) */
        mt9v034_write_reg(0xAF, reg | 0x01);
        /* 设置目标亮度 (AEC/AGC_DESIRED_BIN 0xA5) */
        mt9v034_write_reg(0xA5, brightness);
        /* 设置最大曝光时间 (MAX_EXPOSE 0xAD) */
        mt9v034_write_reg(0xAD, exp_time);
        /* 设置最小曝光时间 (AEC_MIN_EXPOSURE 0xAC) */
        mt9v034_write_reg(0xAC, 0x0001);
        /* 设置像素计数 (PIXEL_COUNT 0xB0) */
        mt9v034_write_reg(0xB0, mt9_v034_width * mt9_v034_height);
    }
    else
    {
        /* 禁用 AEC - 清除 AEC 使能位 */
        mt9v034_write_reg(0xAF, reg & ~0x01);
        /* 设置固定曝光时间 (TOTAL_SHUTTER_WIDTH 0x0B) */
        mt9v034_write_reg(0x0B, exp_time);
    }
}

void mt9v034_apply_config(void)
{
    _mt9v034_set_config();
    _mt9v034_set_frame_rate(mt9v034_config.fps);
    _mt9v034_set_auto_gain(mt9v034_config.auto_exposure);
    _mt9v034_set_auto_exposure(mt9v034_config.auto_exposure);
}


/***************************************************************
  *  @brief     MT9V034初始化
  *  @param     None
  *  @Sample usage:     mt9_v034_init();
 **************************************************************/
uint8 mt9_v034_init(void)
{
    uint16 chip_id = 0;

    soft_i2c_init(&g_mt9v034_i2c, 
                 mt9_v034_iic_addr, 
                 mt9_v034_iic_delay, 
                 mt9_v034_iic_scl, 
                 mt9_v034_iic_sda);
    delay_ms(50);
    
    chip_id = soft_i2c_read_reg16(&g_mt9v034_i2c, 0x00);
    
    if(chip_id != 0x1324) return mt9_v034_err_id;
    
    mt9v034_apply_config();
    
    uint16 read_mode = soft_i2c_read_reg16(&g_mt9v034_i2c, 0x0D);
    
    if(read_mode != 0x033A)
    {
        return (uint8)(read_mode >> 8); 
    }
    
    configureDataPins(); 

    mt9_v034_dma__init();
    
    return mt9_v034_err_none;
}

/***************************************************************
  *  @brief     MT9V034DMA初始化
  *  @param     None
  *  @Sample usage:     mt9_v034_dma__init();
 **************************************************************/
static uint8 mt9_v034_dma__init(void)
{
    IfxScu_Req_In *reqPin = &IfxScu_REQ6_P02_0_IN; 
    IfxScuEru_initReqPin(reqPin, IfxPort_InputMode_pullUp);

    IfxScuEru_InputChannel inputChannel = IfxScuEru_InputChannel_3;
    IfxScuEru_InputNodePointer triggerSelect = IfxScuEru_InputNodePointer_3;
    
    IfxScuEru_disableFallingEdgeDetection(inputChannel);
    IfxScuEru_disableRisingEdgeDetection(inputChannel);
    IfxScuEru_enableFallingEdgeDetection(inputChannel);
    IfxScuEru_enableAutoClear(inputChannel);
    IfxScuEru_enableTriggerPulse(inputChannel);

    IfxScuEru_connectTrigger(inputChannel, triggerSelect);
    
    IfxScuEru_OutputChannel outputChannel = IfxScuEru_OutputChannel_3;
    IfxScuEru_setFlagPatternDetection(outputChannel, inputChannel, FALSE);
    IfxScuEru_disablePatternDetectionTrigger(outputChannel);
    IfxScuEru_setInterruptGatingPattern(outputChannel, IfxScuEru_InterruptGatingPattern_alwaysActive);
    
    IfxScu_Req_In *pclk_pin = &IfxScu_REQ14_P02_1_IN;
    IfxScuEru_initReqPin(pclk_pin, IfxPort_InputMode_noPullDevice);
    
    IfxScuEru_InputChannel pclkInputChannel = (IfxScuEru_InputChannel)pclk_pin->channelId;
    
    IfxScuEru_InputNodePointer pclkTriggerSelect = IfxScuEru_InputNodePointer_2;
    IfxScuEru_OutputChannel pclkOutputChannel = IfxScuEru_OutputChannel_2;

    IfxScuEru_disableFallingEdgeDetection(pclkInputChannel);
    IfxScuEru_disableRisingEdgeDetection(pclkInputChannel);

    IfxScuEru_enableFallingEdgeDetection(pclkInputChannel);
    IfxScuEru_enableTriggerPulse(pclkInputChannel);
    IfxScuEru_enableAutoClear(pclkInputChannel);
    IfxScuEru_connectTrigger(pclkInputChannel, pclkTriggerSelect); 
    
    IfxScuEru_setFlagPatternDetection(pclkOutputChannel, pclkInputChannel, FALSE);
    
    IfxScuEru_setInterruptGatingPattern(pclkOutputChannel, IfxScuEru_InterruptGatingPattern_alwaysActive);
    IfxScuEru_enablePatternDetectionTrigger(pclkOutputChannel); 
    
    IfxDma_Dma_Config dmaConfig;
    IfxDma_Dma_initModuleConfig(&dmaConfig, &MODULE_DMA);
    IfxDma_Dma_initModule(&g_dmaModule, &dmaConfig);
    
    IfxDma_Dma_ChannelConfig channelConfig;
    IfxDma_Dma_initChannelConfig(&channelConfig, &g_dmaModule);
    
    channelConfig.channelId = mt9_v034_dma_channel;
    channelConfig.transferCount = mt9_v034_image_size;
    channelConfig.moveSize = IfxDma_ChannelMoveSize_8bit;
    
    channelConfig.sourceAddress = (uint32)mt9_v034_data_add;
    channelConfig.sourceCircularBufferEnabled = TRUE; 
    channelConfig.sourceAddressCircularRange = IfxDma_ChannelIncrementCircular_none;
    
    channelConfig.destinationAddress = IFXCPU_GLB_ADDR_DSPR(IfxCpu_getCoreId(), (uint32)&mt9_v034__image[0][0]);
    channelConfig.destinationCircularBufferEnabled = FALSE;
    channelConfig.destinationAddressIncrementStep = IfxDma_ChannelIncrementStep_1;
    
    channelConfig.destinationAddressIncrementDirection = IfxDma_ChannelIncrementDirection_positive;
    
    channelConfig.shadowControl = IfxDma_ChannelShadow_none;
    channelConfig.operationMode = IfxDma_ChannelOperationMode_continuous;
    
    channelConfig.hardwareRequestEnabled = TRUE;
    channelConfig.requestMode = IfxDma_ChannelRequestMode_oneTransferPerRequest; 
    
    channelConfig.channelInterruptEnabled = TRUE;
    channelConfig.channelInterruptControl = IfxDma_ChannelInterruptControl_thresholdLimitMatch;
    channelConfig.channelInterruptPriority = ISR_PRIORITY_DMA_CH5;
    channelConfig.channelInterruptTypeOfService = IfxSrc_Tos_cpu0;
    
    IfxDma_Dma_initChannel(&g_mt9v034DmaChannel, &channelConfig);
    
    IfxDma_disableChannelTransaction(&MODULE_DMA, mt9_v034_dma_channel);
    
    volatile Ifx_SRC_SRCR *src_eru2 = &SRC_SCU_SCU_ERU2; 
    IfxSrc_init(src_eru2, IfxSrc_Tos_dma, mt9_v034_dma_channel);
    IfxSrc_enable(src_eru2); 

    volatile Ifx_SRC_SRCR *src_eru3 = &SRC_SCU_SCU_ERU3;
    IfxSrc_init(src_eru3, IfxSrc_Tos_cpu0, ISR_PRIORITY_MT9V034_VSYNC);
    IfxSrc_enable(src_eru3);
    
    return 0;
}


/***************************************************************
  *  @brief     MT9V034获取图像
  *  @param     None
  *  @Sample usage:     mt9_v034_get_image();
 **************************************************************/
uint8 mt9_v034_get_image(void)
{
    if(!new_frame_ready)
    {
        return 1;
    }
    
    new_frame_ready = 0;
    return 0; 
}


/***************************************************************
  *  @brief     MT9V034VSYNCHANDLER
  *  @param     None
  *  @Sample usage:     mt9_v034_vsync_handler();
 **************************************************************/
void mt9_v034_vsync_handler(void)
{
    volatile Ifx_DMA_CH *dma_ch = &MODULE_DMA.CH[mt9_v034_dma_channel];
    dma_ch->DADR.U = IFXCPU_GLB_ADDR_DSPR(IfxCpu_getCoreId(), (uint32)&mt9_v034__image[0][0]);
    IfxDma_enableChannelTransaction(&MODULE_DMA, mt9_v034_dma_channel);
}


/***************************************************************
  *  @brief     MT9V034DMAHANDLER
  *  @param     None
  *  @Sample usage:     mt9_v034_dma_handler();
 **************************************************************/
void mt9_v034_dma_handler(void)
{

    new_frame_ready = 1; 

    volatile Ifx_DMA_CH *dma_ch = &MODULE_DMA.CH[mt9_v034_dma_channel];
    dma_ch->CHCSR.B.SCH = 0;
}
