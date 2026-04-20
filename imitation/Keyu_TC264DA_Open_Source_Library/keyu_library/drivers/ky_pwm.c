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
 * 文件名：[ky_pwm.c]
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
#include "ky_pwm.h"
#include "Gtm/Tom/Pwm/IfxGtm_Tom_Pwm.h"

#define PWM_MAX_CHANNELS PWM_CHANNEL_MAX

typedef struct
{
    IfxGtm_Tom_Pwm_Driver driver;
    boolean initialized;
} PwmRuntime_t;

static PwmRuntime_t g_pwmRuntime[PWM_MAX_CHANNELS];

static const IfxGtm_Tom_ToutMap* g_pwmPinMap[PWM_MAX_CHANNELS] = 
{
    // --- Port 00 ---
    &IfxGtm_TOM0_8_TOUT9_P00_0_OUT,  // PWM_P00_0
    &IfxGtm_TOM0_9_TOUT10_P00_1_OUT, // PWM_P00_1
    &IfxGtm_TOM0_9_TOUT11_P00_2_OUT, // PWM_P00_2
    &IfxGtm_TOM1_2_TOUT12_P00_3_OUT, // PWM_P00_3
    &IfxGtm_TOM1_3_TOUT13_P00_4_OUT, // PWM_P00_4
    &IfxGtm_TOM1_4_TOUT14_P00_5_OUT, // PWM_P00_5
    &IfxGtm_TOM1_5_TOUT15_P00_6_OUT, // PWM_P00_6
    &IfxGtm_TOM1_6_TOUT16_P00_7_OUT, // PWM_P00_7
    &IfxGtm_TOM1_7_TOUT17_P00_8_OUT, // PWM_P00_8
    &IfxGtm_TOM1_0_TOUT18_P00_9_OUT, // PWM_P00_9
    &IfxGtm_TOM1_1_TOUT19_P00_10_OUT,// PWM_P00_10
    &IfxGtm_TOM1_2_TOUT20_P00_11_OUT,// PWM_P00_11
    &IfxGtm_TOM1_3_TOUT21_P00_12_OUT,// PWM_P00_12
    // --- Port 02 ---
    &IfxGtm_TOM1_8_TOUT0_P02_0_OUT,  // PWM_P02_0
    &IfxGtm_TOM1_9_TOUT1_P02_1_OUT,  // PWM_P02_1
    &IfxGtm_TOM1_10_TOUT2_P02_2_OUT, // PWM_P02_2
    &IfxGtm_TOM1_11_TOUT3_P02_3_OUT, // PWM_P02_3
    &IfxGtm_TOM1_12_TOUT4_P02_4_OUT, // PWM_P02_4
    &IfxGtm_TOM1_13_TOUT5_P02_5_OUT, // PWM_P02_5
    &IfxGtm_TOM1_14_TOUT6_P02_6_OUT, // PWM_P02_6
    &IfxGtm_TOM1_15_TOUT7_P02_7_OUT, // PWM_P02_7
    &IfxGtm_TOM0_8_TOUT8_P02_8_OUT,  // PWM_P02_8
    // --- Port 10 ---
    &IfxGtm_TOM1_12_TOUT102_P10_0_OUT, // PWM_P10_0
    &IfxGtm_TOM1_9_TOUT103_P10_1_OUT,  // PWM_P10_1
    &IfxGtm_TOM1_10_TOUT104_P10_2_OUT, // PWM_P10_2
    &IfxGtm_TOM1_11_TOUT105_P10_3_OUT, // PWM_P10_3
    &IfxGtm_TOM1_6_TOUT106_P10_4_OUT,  // PWM_P10_4
    &IfxGtm_TOM1_10_TOUT107_P10_5_OUT, // PWM_P10_5
    &IfxGtm_TOM1_11_TOUT108_P10_6_OUT, // PWM_P10_6
    &IfxGtm_TOM1_8_TOUT109_P10_7_OUT,  // PWM_P10_7
    &IfxGtm_TOM1_13_TOUT110_P10_8_OUT, // PWM_P10_8
    // --- Port 11 ---
    &IfxGtm_TOM1_1_TOUT95_P11_2_OUT,   // PWM_P11_2
    &IfxGtm_TOM1_2_TOUT96_P11_3_OUT,   // PWM_P11_3
    &IfxGtm_TOM1_3_TOUT97_P11_6_OUT,   // PWM_P11_6
    &IfxGtm_TOM1_4_TOUT98_P11_9_OUT,   // PWM_P11_9
    &IfxGtm_TOM1_5_TOUT99_P11_10_OUT,  // PWM_P11_10
    &IfxGtm_TOM1_6_TOUT100_P11_11_OUT, // PWM_P11_11
    &IfxGtm_TOM1_7_TOUT101_P11_12_OUT, // PWM_P11_12
    // --- Port 13 ---
    &IfxGtm_TOM1_5_TOUT91_P13_0_OUT, // PWM_P13_0
    &IfxGtm_TOM1_6_TOUT92_P13_1_OUT, // PWM_P13_1
    &IfxGtm_TOM1_7_TOUT93_P13_2_OUT, // PWM_P13_2
    &IfxGtm_TOM1_0_TOUT94_P13_3_OUT, // PWM_P13_3
    // --- Port 14 ---
    &IfxGtm_TOM1_3_TOUT80_P14_0_OUT, // PWM_P14_0
    &IfxGtm_TOM1_4_TOUT81_P14_1_OUT, // PWM_P14_1
    &IfxGtm_TOM1_5_TOUT82_P14_2_OUT, // PWM_P14_2
    &IfxGtm_TOM1_6_TOUT83_P14_3_OUT, // PWM_P14_3
    &IfxGtm_TOM1_7_TOUT84_P14_4_OUT, // PWM_P14_4
    &IfxGtm_TOM1_0_TOUT85_P14_5_OUT, // PWM_P14_5
    &IfxGtm_TOM1_1_TOUT86_P14_6_OUT, // PWM_P14_6
    &IfxGtm_TOM1_0_TOUT87_P14_7_OUT, // PWM_P14_7
    &IfxGtm_TOM1_2_TOUT88_P14_8_OUT, // PWM_P14_8
    &IfxGtm_TOM1_3_TOUT89_P14_9_OUT, // PWM_P14_9
    &IfxGtm_TOM1_4_TOUT90_P14_10_OUT,// PWM_P14_10
    // --- Port 15 ---
    &IfxGtm_TOM1_3_TOUT71_P15_0_OUT, // PWM_P15_0
    &IfxGtm_TOM1_4_TOUT72_P15_1_OUT, // PWM_P15_1
    &IfxGtm_TOM1_5_TOUT73_P15_2_OUT, // PWM_P15_2
    &IfxGtm_TOM1_6_TOUT74_P15_3_OUT, // PWM_P15_3
    &IfxGtm_TOM1_7_TOUT75_P15_4_OUT, // PWM_P15_4
    &IfxGtm_TOM1_0_TOUT76_P15_5_OUT, // PWM_P15_5
    &IfxGtm_TOM1_0_TOUT77_P15_6_OUT, // PWM_P15_6
    &IfxGtm_TOM1_1_TOUT78_P15_7_OUT, // PWM_P15_7
    &IfxGtm_TOM1_2_TOUT79_P15_8_OUT, // PWM_P15_8
    // --- Port 20 ---
    &IfxGtm_TOM1_6_TOUT59_P20_0_OUT, // PWM_P20_0
    &IfxGtm_TOM1_11_TOUT60_P20_1_OUT,// PWM_P20_1
    &IfxGtm_TOM1_12_TOUT61_P20_3_OUT,// PWM_P20_3
    &IfxGtm_TOM1_10_TOUT62_P20_6_OUT,// PWM_P20_6
    &IfxGtm_TOM1_11_TOUT63_P20_7_OUT,// PWM_P20_7
    &IfxGtm_TOM1_7_TOUT64_P20_8_OUT, // PWM_P20_8
    &IfxGtm_TOM1_13_TOUT65_P20_9_OUT,// PWM_P20_9
    &IfxGtm_TOM1_14_TOUT66_P20_10_OUT,// PWM_P20_10
    &IfxGtm_TOM1_15_TOUT67_P20_11_OUT,// PWM_P20_11
    &IfxGtm_TOM1_0_TOUT68_P20_12_OUT,// PWM_P20_12
    &IfxGtm_TOM1_1_TOUT69_P20_13_OUT,// PWM_P20_13
    &IfxGtm_TOM1_2_TOUT70_P20_14_OUT,// PWM_P20_14
    // --- Port 21 ---
    &IfxGtm_TOM1_8_TOUT51_P21_0_OUT, // PWM_P21_0
    &IfxGtm_TOM1_9_TOUT52_P21_1_OUT, // PWM_P21_1
    &IfxGtm_TOM1_0_TOUT53_P21_2_OUT, // PWM_P21_2
    &IfxGtm_TOM1_1_TOUT54_P21_3_OUT, // PWM_P21_3
    &IfxGtm_TOM1_2_TOUT55_P21_4_OUT, // PWM_P21_4
    &IfxGtm_TOM1_3_TOUT56_P21_5_OUT, // PWM_P21_5
    &IfxGtm_TOM1_4_TOUT57_P21_6_OUT, // PWM_P21_6
    &IfxGtm_TOM1_5_TOUT58_P21_7_OUT, // PWM_P21_7
    // --- Port 22 ---
    &IfxGtm_TOM1_1_TOUT47_P22_0_OUT, // PWM_P22_0
    &IfxGtm_TOM1_0_TOUT48_P22_1_OUT, // PWM_P22_1
    &IfxGtm_TOM0_11_TOUT49_P22_2_OUT,// PWM_P22_2
    &IfxGtm_TOM0_12_TOUT50_P22_3_OUT,// PWM_P22_3
    // --- Port 23 ---
    &IfxGtm_TOM1_5_TOUT41_P23_0_OUT, // PWM_P23_0
    &IfxGtm_TOM0_6_TOUT42_P23_1_OUT, // PWM_P23_1
    &IfxGtm_TOM1_6_TOUT43_P23_2_OUT, // PWM_P23_2
    &IfxGtm_TOM1_7_TOUT44_P23_3_OUT, // PWM_P23_3
    &IfxGtm_TOM1_7_TOUT45_P23_4_OUT, // PWM_P23_4
    &IfxGtm_TOM1_2_TOUT46_P23_5_OUT, // PWM_P23_5
    // --- Port 32 ---
    &IfxGtm_TOM1_14_TOUT36_P32_0_OUT,// PWM_P32_0
    &IfxGtm_TOM1_3_TOUT38_P32_2_OUT, // PWM_P32_2
    &IfxGtm_TOM1_4_TOUT39_P32_3_OUT, // PWM_P32_3
    &IfxGtm_TOM1_5_TOUT40_P32_4_OUT, // PWM_P32_4
    // --- Port 33 ---
    &IfxGtm_TOM1_4_TOUT22_P33_0_OUT, // PWM_P33_0
    &IfxGtm_TOM1_5_TOUT23_P33_1_OUT, // PWM_P33_1
    &IfxGtm_TOM1_6_TOUT24_P33_2_OUT, // PWM_P33_2
    &IfxGtm_TOM1_7_TOUT25_P33_3_OUT, // PWM_P33_3
    &IfxGtm_TOM1_0_TOUT26_P33_4_OUT, // PWM_P33_4
    &IfxGtm_TOM1_1_TOUT27_P33_5_OUT, // PWM_P33_5
    &IfxGtm_TOM1_2_TOUT28_P33_6_OUT, // PWM_P33_6
    &IfxGtm_TOM1_3_TOUT29_P33_7_OUT, // PWM_P33_7
    &IfxGtm_TOM1_4_TOUT30_P33_8_OUT, // PWM_P33_8
    &IfxGtm_TOM0_1_TOUT31_P33_9_OUT, // PWM_P33_9
    &IfxGtm_TOM1_0_TOUT32_P33_10_OUT,// PWM_P33_10
    &IfxGtm_TOM1_2_TOUT33_P33_11_OUT,// PWM_P33_11
    &IfxGtm_TOM1_12_TOUT34_P33_12_OUT,// PWM_P33_12
    &IfxGtm_TOM1_13_TOUT35_P33_13_OUT,// PWM_P33_13
};


/***************************************************************
  *  @brief     初始化PWM通道
  *  @param     channel       PWM通道选择
  *  @param     frequency     PWM频率（Hz）
  *  @param     initialDuty   初始占空比（0-10000，对应0%-100%）
  *  @note      此函数会配置GTM TOM模块并启动PWM输出
  
  *  @Sample usage:     pwm_init(PWM_P00_0, 10000, 5000);  // 10kHz, 50%占空比
  **************************************************************/
void pwm_init(PwmChannel_t channel, uint32 frequency, uint16 initialDuty)
{
    if (channel >= PWM_MAX_CHANNELS)
    {
        return;
    }

    Ifx_GTM *gtm = &MODULE_GTM;
    IfxGtm_Tom_Pwm_Config tomConfig;
    static boolean gtmClockInitialized = FALSE;
    
    if (!gtmClockInitialized)
    {
        if (!IfxGtm_isEnabled(gtm))
        {
            IfxGtm_enable(gtm);
        }
        
        float32 sysClkFreq = (float32)IfxScuCcu_getSpbFrequency();
        IfxGtm_Cmu_setClkFrequency(gtm, IfxGtm_Cmu_Clk_0, sysClkFreq);
        IfxGtm_Cmu_enableClocks(gtm, IFXGTM_CMU_CLKEN_CLK0);
        
        IfxGtm_Cmu_enableClocks(gtm, IFXGTM_CMU_CLKEN_FXCLK);
        
        gtmClockInitialized = TRUE;
    }

    IfxGtm_Tom_Pwm_initConfig(&tomConfig, gtm);

    tomConfig.tomChannel = g_pwmPinMap[channel]->channel;
    tomConfig.tom        = g_pwmPinMap[channel]->tom;

    float32 clk0Freq = IfxGtm_Cmu_getClkFrequency(gtm, IfxGtm_Cmu_Clk_0, TRUE);
    float32 gtmFreq;

    if (frequency < 1526)
    {
        tomConfig.clock = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk2;
        gtmFreq = clk0Freq / 256.0f; 
    }
    else
    {
        tomConfig.clock = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
        gtmFreq = clk0Freq;
    }
    
    uint32 periodTicks = (uint32)(gtmFreq / frequency);
    
    if (periodTicks > 0xFFFF)
    {
        periodTicks = 0xFFFF;
    }
    tomConfig.period = (uint16)periodTicks;

    uint32 dutyTicks = (periodTicks * initialDuty) / PWM_DUTY_MAX;
    tomConfig.dutyCycle = (uint16)dutyTicks;

    tomConfig.pin.outputPin = g_pwmPinMap[channel];
    tomConfig.pin.outputMode = IfxPort_OutputMode_pushPull;
    tomConfig.pin.padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    
    tomConfig.synchronousUpdateEnabled = TRUE;

    IfxGtm_Tom_Pwm_init(&g_pwmRuntime[channel].driver, &tomConfig);

    IfxGtm_Tom_Pwm_start(&g_pwmRuntime[channel].driver, TRUE);

    g_pwmRuntime[channel].initialized = TRUE;
}

/***************************************************************
  *  @brief     设置PWM占空比
  *  @param     channel    PWM通道选择
  *  @param     duty       占空比（0-10000，对应0%-100%）
  *  @note      此函数会立即更新PWM输出的占空比
  *  @Sample usage:     pwm_set_duty(PWM_P00_0, 7500);  // 设置为75%占空比
  **************************************************************/
void pwm_set_duty(PwmChannel_t channel, uint16 duty)
{
    if (channel >= PWM_MAX_CHANNELS || !g_pwmRuntime[channel].initialized)
    {
        return;
    }
    
    Ifx_GTM_TOM_CH *tomCh = IfxGtm_Tom_Ch_getChannelPointer(
        g_pwmRuntime[channel].driver.tom, 
        g_pwmRuntime[channel].driver.tomChannel
    );
    
    uint16 periodTicks = tomCh->SR0.B.SR0; 
    
    uint32 dutyTicks = ((uint32)periodTicks * duty) / PWM_DUTY_MAX;
    
    tomCh->SR1.U = (uint32)dutyTicks;
}
