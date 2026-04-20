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
 * 文件名：[ky_adc.c]
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
#include "ky_adc.h"
#include "Vadc/Adc/IfxVadc_Adc.h"


static uint8 s_AdcResolution[50];               // 存储每个通道的分辨率配置，用于结果换算
static boolean s_vadcInitialized = FALSE;       // 模块初始化标志
static IfxVadc_Adc s_vadcHandle;                // VADC模块句柄

/***************************************************************
  *  @brief     配置并初始化ADC模块
  *  @param     ch      ADC通道选择
  *  @param     re      ADC分辨率
  *  @Sample usage:     adc_set_mode(ADC0_CH0_A0, ADC_RES_12BIT);
 **************************************************************/
static boolean s_groupInitialized[IFXVADC_NUM_ADC_GROUPS] = {FALSE}; // 记录每个组是否已初始化

void adc_set_mode(ADC_Channel_t ch, ADC_Resolution_t re)
{
    IfxVadc_Adc_Config adcConfig;
    IfxVadc_Adc_GroupConfig adcGroupConfig;
    IfxVadc_Adc_ChannelConfig adcChannelConfig;
    IfxVadc_Adc_Group adcGroup;
    IfxVadc_Adc_Channel adcChannel;
    
    uint8 groupId = (uint8)(ch >> 4); 
    uint8 channelId = (uint8)(ch & 0x0F);

    if (!s_vadcInitialized)
    {
        IfxVadc_Adc_initModuleConfig(&adcConfig, &MODULE_VADC);
        
        adcConfig.globalInputClass[0].resolution = IfxVadc_ChannelResolution_12bit;
        adcConfig.globalInputClass[0].sampleTime = 1.0f / ADC_SAMPLE_FREQUENCY;
        
        IfxVadc_Adc_initModule(&s_vadcHandle, &adcConfig);
        s_vadcInitialized = TRUE;
    }

    if (!s_groupInitialized[groupId])
    {
        IfxVadc_Adc_initGroupConfig(&adcGroupConfig, &s_vadcHandle);
        adcGroupConfig.groupId = (IfxVadc_GroupId)groupId;
        adcGroupConfig.master = adcGroupConfig.groupId;
        
        adcGroupConfig.arbiter.requestSlotBackgroundScanEnabled = TRUE;
        adcGroupConfig.backgroundScanRequest.autoBackgroundScanEnabled = TRUE;
        adcGroupConfig.backgroundScanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

        // Class 0 -> 8位
        adcGroupConfig.inputClass[0].resolution = IfxVadc_ChannelResolution_8bit;
        adcGroupConfig.inputClass[0].sampleTime = 1.0f / ADC_SAMPLE_FREQUENCY;
        
        // Class 1 -> 10位
        adcGroupConfig.inputClass[1].resolution = IfxVadc_ChannelResolution_10bit;
        adcGroupConfig.inputClass[1].sampleTime = 1.0f / ADC_SAMPLE_FREQUENCY;

        IfxVadc_Adc_initGroup(&adcGroup, &adcGroupConfig);
        s_groupInitialized[groupId] = TRUE;
    }
    else
    {
        adcGroup.module = s_vadcHandle;
        adcGroup.group = &MODULE_VADC.G[groupId];
        adcGroup.groupId = (IfxVadc_GroupId)groupId;
    }

    // 3. 通道配置
    IfxVadc_Adc_initChannelConfig(&adcChannelConfig, &adcGroup);
    adcChannelConfig.channelId = (IfxVadc_ChannelId)channelId;
    adcChannelConfig.resultRegister = (IfxVadc_ChannelResult)channelId; 
    adcChannelConfig.rightAlignedStorage = TRUE; 
    adcChannelConfig.backgroundChannel = TRUE; 

    switch (re)
    {
        case ADC_RES_8BIT:
            // 使用 Group Class 0 (已配置为 8-bit)
            adcChannelConfig.inputClass = IfxVadc_InputClasses_group0;
            s_AdcResolution[ch] = 8;
            break;
            
        case ADC_RES_10BIT:
            // 使用 Group Class 1 (已配置为 10-bit)
            adcChannelConfig.inputClass = IfxVadc_InputClasses_group1;
            s_AdcResolution[ch] = 10;
            break;
            
        case ADC_RES_12BIT:
        default:
            // 使用 Global Class 0 (已配置为 12-bit)
            adcChannelConfig.inputClass = IfxVadc_InputClasses_global0;
            s_AdcResolution[ch] = 12;
            break;
    }

    IfxVadc_Adc_initChannel(&adcChannel, &adcChannelConfig);

    unsigned channels = (1 << channelId);
    unsigned mask = channels; 
    
    IfxVadc_Adc_setBackgroundScan(&s_vadcHandle, &adcGroup, channels, mask);

    IfxVadc_Adc_startBackgroundScan(&s_vadcHandle);
}

/***************************************************************
  *  @brief     启动一次ADC转换并返回结果
  *  @param     ch      需要进行转换的ADC通道
  *  @return    uint16_t 返回ADC转换结果
  *  @Sample usage:     uint16_t adcValue = adc_convert(ADC0_CH0_A0);
 **************************************************************/
uint16_t adc_convert(ADC_Channel_t ch)
{
    Ifx_VADC_RES result;
    uint8 groupId = (uint8)(ch >> 4);
    uint8 channelId = (uint8)(ch & 0x0F);
    
    do
    {
        result = IfxVadc_getResult(&MODULE_VADC.G[groupId], channelId);
    } while(!result.B.VF);

    return (uint16_t)result.B.RESULT;
}
