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
 * 文件名：[ky_encoder.c]
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
#include "ky_encoder.h"

static uint8 s_EncoderMode[ENCODER_TIM6 + 1] = {0};

/***************************************************************
  *  @brief     验证引脚配置是否有效
  *  @param     encoderIndex    编码器索引
  *  @param     pinA            A相引脚
  *  @param     pinB            B相引脚
  *  @return    uint8           1=有效, 0=无效
  *  @Sample usage:     uint8 valid = enc_validate_pin_config(ENCODER_TIM2, TIM2_ENC_A_P00_7, TIM2_ENC_B_P00_8);
 **************************************************************/
static uint8 enc_validate_pin_config(Encoder_Index_t encoderIndex, Encoder_A_Pin_t pinA, Encoder_B_Pin_t pinB)
{
    switch(encoderIndex)
    {
        case ENCODER_TIM2:
            if ((pinA == TIM2_ENC_A_P00_7 && pinB == TIM2_ENC_B_P00_8) ||
                (pinA == TIM2_ENC_A_P00_7 && pinB == TIM2_ENC_B_P33_6) ||
                (pinA == TIM2_ENC_A_P33_7 && pinB == TIM2_ENC_B_P00_8) ||
                (pinA == TIM2_ENC_A_P33_7 && pinB == TIM2_ENC_B_P33_6))
                return 1;
            break;
            
        case ENCODER_TIM3:
            if (pinA == TIM3_ENC_A_P02_6 && pinB == TIM3_ENC_B_P02_7)
                return 1;
            break;
            
        case ENCODER_TIM4:
            if ((pinA == TIM4_ENC_A_P02_8 && pinB == TIM4_ENC_B_P00_9) ||
                (pinA == TIM4_ENC_A_P02_8 && pinB == TIM4_ENC_B_P33_5))
                return 1;
            break;
            
        case ENCODER_TIM5:
            if ((pinA == TIM5_ENC_A_P21_7 && pinB == TIM5_ENC_B_P21_6) ||
                (pinA == TIM5_ENC_A_P21_7 && pinB == TIM5_ENC_B_P10_1) ||
                (pinA == TIM5_ENC_A_P10_3 && pinB == TIM5_ENC_B_P21_6) ||
                (pinA == TIM5_ENC_A_P10_3 && pinB == TIM5_ENC_B_P10_1))
                return 1;
            break;
            
        case ENCODER_TIM6:
            if ((pinA == TIM6_ENC_A_P20_3 && pinB == TIM6_ENC_B_P20_0) ||
                (pinA == TIM6_ENC_A_P10_2 && pinB == TIM6_ENC_B_P20_0))
                return 1;
            break;
    }
    return 0;
}

/***************************************************************
  *  @brief     配置引脚复用
  *  @param     encoderIndex    编码器索引
  *  @param     pinA            A相引脚
  *  @param     pinB            B相引脚
  *  @Sample usage:     enc_config_pin_mapping(ENCODER_TIM2, TIM2_ENC_A_P00_7, TIM2_ENC_B_P00_8);
 **************************************************************/
static void enc_config_pin_mapping(Encoder_Index_t encoderIndex, Encoder_A_Pin_t pinA, Encoder_B_Pin_t pinB)
{
    IfxGpt12_TxIn_In  *ch1 = NULL;
    IfxGpt12_TxEud_In *ch2 = NULL;

    switch(encoderIndex)
    {
        case ENCODER_TIM2:
            if (pinA == TIM2_ENC_A_P00_7)       ch1 = &IfxGpt120_T2INA_P00_7_IN;
            else if (pinA == TIM2_ENC_A_P33_7)  ch1 = &IfxGpt120_T2INB_P33_7_IN;
            
            if (pinB == TIM2_ENC_B_P00_8)       ch2 = &IfxGpt120_T2EUDA_P00_8_IN;
            else if (pinB == TIM2_ENC_B_P33_6)  ch2 = &IfxGpt120_T2EUDB_P33_6_IN;
            break;

        case ENCODER_TIM3:
            if (pinA == TIM3_ENC_A_P02_6)       ch1 = &IfxGpt120_T3INA_P02_6_IN;
            if (pinB == TIM3_ENC_B_P02_7)       ch2 = &IfxGpt120_T3EUDA_P02_7_IN;
            break;

        case ENCODER_TIM4:
            if (pinA == TIM4_ENC_A_P02_8)       ch1 = &IfxGpt120_T4INA_P02_8_IN;
            
            if (pinB == TIM4_ENC_B_P00_9)       ch2 = &IfxGpt120_T4EUDA_P00_9_IN;
            else if (pinB == TIM4_ENC_B_P33_5)  ch2 = &IfxGpt120_T4EUDB_P33_5_IN;
            break;

        case ENCODER_TIM5:
            if (pinA == TIM5_ENC_A_P21_7)       ch1 = &IfxGpt120_T5INA_P21_7_IN;
            else if (pinA == TIM5_ENC_A_P10_3)  ch1 = &IfxGpt120_T5INB_P10_3_IN;
            
            if (pinB == TIM5_ENC_B_P21_6)       ch2 = &IfxGpt120_T5EUDA_P21_6_IN;
            else if (pinB == TIM5_ENC_B_P10_1)  ch2 = &IfxGpt120_T5EUDB_P10_1_IN;
            break;

        case ENCODER_TIM6:
            if (pinA == TIM6_ENC_A_P20_3)       ch1 = &IfxGpt120_T6INA_P20_3_IN;
            else if (pinA == TIM6_ENC_A_P10_2)  ch1 = &IfxGpt120_T6INB_P10_2_IN;
            
            if (pinB == TIM6_ENC_B_P20_0)       ch2 = &IfxGpt120_T6EUDA_P20_0_IN;
            break;
    }

#pragma warning 507
    if (ch1 != NULL)
        IfxGpt12_initTxInPinWithPadLevel(ch1, IfxPort_InputMode_pullUp, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    if (ch2 != NULL)
        IfxGpt12_initTxEudInPinWithPadLevel(ch2, IfxPort_InputMode_pullUp, IfxPort_PadDriver_cmosAutomotiveSpeed1);
#pragma warning default
}

/***************************************************************
  *  @brief     启用GPT12模块并配置预分频
  *  @Sample usage:     enc_enable_module();
 **************************************************************/
static void enc_enable_module(void)
{
    IfxGpt12_enableModule(&MODULE_GPT120);
    IfxGpt12_setGpt1BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt1BlockPrescaler_4);
    IfxGpt12_setGpt2BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt2BlockPrescaler_4);
}

/***************************************************************
  *  @brief     配置定时器为正交编码器模式
  *  @param     encoderIndex    编码器索引
  *  @Sample usage:     enc_config_quad_mode(ENCODER_TIM2);
 **************************************************************/
static void enc_config_quad_mode(Encoder_Index_t encoderIndex)
{
    switch(encoderIndex)
    {
        case ENCODER_TIM2:
            IfxGpt12_T2_setCounterInputMode(&MODULE_GPT120, IfxGpt12_IncrementalInterfaceInputMode_bothEdgesTxINOrTxEUD);
            IfxGpt12_T2_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T2_setMode(&MODULE_GPT120, IfxGpt12_Mode_incrementalInterfaceEdgeDetection);
            IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM3:
            IfxGpt12_T3_setCounterInputMode(&MODULE_GPT120, IfxGpt12_IncrementalInterfaceInputMode_bothEdgesTxINOrTxEUD);
            IfxGpt12_T3_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T3_setMode(&MODULE_GPT120, IfxGpt12_Mode_incrementalInterfaceEdgeDetection);
            IfxGpt12_T3_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM4:
            IfxGpt12_T4_setCounterInputMode(&MODULE_GPT120, IfxGpt12_IncrementalInterfaceInputMode_bothEdgesTxINOrTxEUD);
            IfxGpt12_T4_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T4_setMode(&MODULE_GPT120, IfxGpt12_Mode_incrementalInterfaceEdgeDetection);
            IfxGpt12_T4_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM5:
            IfxGpt12_T5_setCounterInputMode(&MODULE_GPT120, IfxGpt12_IncrementalInterfaceInputMode_bothEdgesTxINOrTxEUD);
            IfxGpt12_T5_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T5_setMode(&MODULE_GPT120, IfxGpt12_Mode_incrementalInterfaceEdgeDetection);
            IfxGpt12_T5_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM6:
            IfxGpt12_T6_setCounterInputMode(&MODULE_GPT120, IfxGpt12_IncrementalInterfaceInputMode_bothEdgesTxINOrTxEUD);
            IfxGpt12_T6_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T6_setMode(&MODULE_GPT120, IfxGpt12_Mode_incrementalInterfaceEdgeDetection);
            IfxGpt12_T6_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;
    }
}

/***************************************************************
  *  @brief     配置定时器为脉冲+方向模式
  *  @param     encoderIndex    编码器索引
  *  @Sample usage:     enc_config_dir_mode(ENCODER_TIM2);
 **************************************************************/
static void enc_config_dir_mode(Encoder_Index_t encoderIndex)
{
    switch(encoderIndex)
    {
        case ENCODER_TIM2:
            IfxGpt12_T2_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T2_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T2_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM3:
            IfxGpt12_T3_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T3_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T3_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T3_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM4:
            IfxGpt12_T4_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T4_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T4_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T4_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM5:
            IfxGpt12_T5_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T5_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T5_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T5_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case ENCODER_TIM6:
            IfxGpt12_T6_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T6_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T6_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T6_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;
    }
}

/***************************************************************
  *  @brief     读取定时器计数值
  *  @param     encoderIndex    编码器索引
  *  @return    uint16          计数值
  *  @Sample usage:     uint16 count = enc_read_counter(ENCODER_TIM2);
 **************************************************************/
static uint16 enc_read_counter(Encoder_Index_t encoderIndex)
{
    uint16 count = 0;
    
    switch(encoderIndex)
    {
        case ENCODER_TIM2: count = IfxGpt12_T2_getTimerValue(&MODULE_GPT120); break;
        case ENCODER_TIM3: count = IfxGpt12_T3_getTimerValue(&MODULE_GPT120); break;
        case ENCODER_TIM4: count = IfxGpt12_T4_getTimerValue(&MODULE_GPT120); break;
        case ENCODER_TIM5: count = IfxGpt12_T5_getTimerValue(&MODULE_GPT120); break;
        case ENCODER_TIM6: count = IfxGpt12_T6_getTimerValue(&MODULE_GPT120); break;
        default: count = 0; break;
    }
    
    return count;
}

/***************************************************************
  *  @brief     写入定时器计数值
  *  @param     encoderIndex    编码器索引
  *  @param     value           计数值
  *  @Sample usage:     enc_write_counter(ENCODER_TIM2, 0);
 **************************************************************/
static void enc_write_counter(Encoder_Index_t encoderIndex, uint16 value)
{
    switch(encoderIndex)
    {
        case ENCODER_TIM2: IfxGpt12_T2_setTimerValue(&MODULE_GPT120, value); break;
        case ENCODER_TIM3: IfxGpt12_T3_setTimerValue(&MODULE_GPT120, value); break;
        case ENCODER_TIM4: IfxGpt12_T4_setTimerValue(&MODULE_GPT120, value); break;
        case ENCODER_TIM5: IfxGpt12_T5_setTimerValue(&MODULE_GPT120, value); break;
        case ENCODER_TIM6: IfxGpt12_T6_setTimerValue(&MODULE_GPT120, value); break;
    }
}

/***************************************************************
  *  @brief     初始化为正交编码器模式
  *  @param     encoderIndex    使用的定时器组
  *  @param     aPin            A相引脚
  *  @param     bPin            B相引脚
  *  @Sample usage:     encoder_init_quad(ENCODER_TIM2, TIM2_ENC_A_P00_7, TIM2_ENC_B_P00_8);
 **************************************************************/
void encoder_init_quad(Encoder_Index_t encoderIndex, Encoder_A_Pin_t aPin, Encoder_B_Pin_t bPin)
{
    if (!enc_validate_pin_config(encoderIndex, aPin, bPin))
        return;
    
    enc_enable_module();
    enc_config_pin_mapping(encoderIndex, aPin, bPin);
    enc_config_quad_mode(encoderIndex);
    
    s_EncoderMode[encoderIndex] = 0;
}

/***************************************************************
  *  @brief     初始化为脉冲+方向模式
  *  @param     encoderIndex    使用的定时器组
  *  @param     pulsePin        脉冲输入引脚 (A相)
  *  @param     dirPin          方向控制引脚 (B相)
  *  @Sample usage:     encoder_init_dir(ENCODER_TIM2, TIM2_ENC_A_P00_7, TIM2_ENC_B_P00_8);
 **************************************************************/
void encoder_init_dir(Encoder_Index_t encoderIndex, Encoder_A_Pin_t pulsePin, Encoder_B_Pin_t dirPin)
{
    if (!enc_validate_pin_config(encoderIndex, pulsePin, dirPin))
        return;
    
    enc_enable_module();
    enc_config_pin_mapping(encoderIndex, pulsePin, dirPin);
    enc_config_dir_mode(encoderIndex);

    s_EncoderMode[encoderIndex] = 1;
}

/***************************************************************
  *  @brief     获取指定编码器的累计脉冲计数值
  *  @param     encoderIndex    要读取的编码器索引
  *  @return    int16           带符号的计数值
  *  @Sample usage:     int16 count = encoder_get_count(ENCODER_TIM2);
 **************************************************************/
int16 encoder_get_count(Encoder_Index_t encoderIndex)
{
    int16 count = 0;
    
    count = (int16)enc_read_counter(encoderIndex);
    
    //如果编码器数值太大下面的注释掉的（因为4倍频所以要除以4）
    // 正交模式下会得到4倍频的计数,数值更大更灵敏
    if (s_EncoderMode[encoderIndex] == 0)
    {
        count = count / 4;
    }
    
    return count;
}

/***************************************************************
  *  @brief     清除(复位)指定编码器的脉冲计数值
  *  @param     encoderIndex    要清零的编码器索引
  *  @Sample usage:     encoder_clear_count(ENCODER_TIM2);
 **************************************************************/
void encoder_clear_count(Encoder_Index_t encoderIndex)
{
    enc_write_counter(encoderIndex, 0);
}
