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
 * 文件名：[ky_exti.c]
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
#include "ky_exti.h"
#include "Scu/Std/IfxScuEru.h"
#include "Src/Std/IfxSrc.h"
#include "Port/Std/IfxPort.h"
#include "Isr.h"

/***************************************************************
 *  @brief     获取EXTI引脚配置
 *  @param     exti_pin    ERU通道及引脚
 *  @return    IfxScu_Req_In*  引脚配置指针
 **************************************************************/
static IfxScu_Req_In* get_exti_pin(EXTI_Pin_t exti_pin)
{
    IfxScu_Req_In *pin_config = NULL;
    
    switch(exti_pin)
    {
        case ERU_CH0_REQ0_P15_4:   pin_config = &IfxScu_REQ0_P15_4_IN;   break;
        case ERU_CH1_REQ10_P14_3:  pin_config = &IfxScu_REQ10_P14_3_IN;  break;
        case ERU_CH2_REQ7_P00_4:   pin_config = &IfxScu_REQ7_P00_4_IN;   break;
        case ERU_CH2_REQ14_P02_1:  pin_config = &IfxScu_REQ14_P02_1_IN;  break;
        case ERU_CH2_REQ2_P10_2:   pin_config = &IfxScu_REQ2_P10_2_IN;   break;
        case ERU_CH3_REQ6_P02_0:   pin_config = &IfxScu_REQ6_P02_0_IN;   break;
        case ERU_CH3_REQ3_P10_3:   pin_config = &IfxScu_REQ3_P10_3_IN;   break;
        case ERU_CH3_REQ15_P14_1:  pin_config = &IfxScu_REQ15_P14_1_IN;  break;
        case ERU_CH4_REQ13_P15_5:  pin_config = &IfxScu_REQ13_P15_5_IN;  break;
        case ERU_CH4_REQ8_P33_7:   pin_config = &IfxScu_REQ8_P33_7_IN;   break;
        case ERU_CH5_REQ1_P15_8:   pin_config = &IfxScu_REQ1_P15_8_IN;   break;
        case ERU_CH6_REQ12_P11_10: pin_config = &IfxScu_REQ12_P11_10_IN; break;
        case ERU_CH6_REQ9_P20_0:   pin_config = &IfxScu_REQ9_P20_0_IN;   break;
        case ERU_CH7_REQ16_P15_1:  pin_config = &IfxScu_REQ16_P15_1_IN;  break;
        case ERU_CH7_REQ11_P20_9:  pin_config = &IfxScu_REQ11_P20_9_IN;  break;
        default: break;
    }
    
    return pin_config;
}

/***************************************************************
 *  @brief     EXTI外部中断初始化
 *  @param     exti_pin    ERU通道及引脚
 *  @param     trigger     触发方式
 *  @return    void
 **************************************************************/
void exti_init(EXTI_Pin_t exti_pin, EXTI_Trigger_t trigger)
{
    boolean interrupt_state = IfxCpu_disableInterrupts();
    
    // 获取引脚配置
    IfxScu_Req_In *reqPin = get_exti_pin(exti_pin);
    if(reqPin == NULL) return;
    
    // 初始化ERU输入引脚
    IfxScuEru_initReqPin(reqPin, IfxPort_InputMode_pullUp);
    
    // 获取输入通道和输出通道
    IfxScuEru_InputChannel inputChannel = (IfxScuEru_InputChannel)reqPin->channelId;
    
    // 修复outputChannel映射：直接根据通道号映射
    // Channel 0 -> OGU 0
    // Channel 1 -> OGU 1
    // Channel 2 -> OGU 2
    // Channel 3 -> OGU 3 (VSYNC)
    IfxScuEru_OutputChannel outputChannel;
    IfxScuEru_InputNodePointer triggerSelect;
    
    // 获取通道索引 (根据枚举定义推导)
    // 简单假定：使用 reqPin 所在的 InputChannel 作为 OutputChannel 的依据
    // P02.0 (REQ6) -> InputChannel 3 -> OGU 3
    
    // 这样比 exti_pin/3 更安全，因为 inputChannel 是从 iLLD 结构体里读出来的
    outputChannel = (IfxScuEru_OutputChannel)inputChannel;
    triggerSelect = (IfxScuEru_InputNodePointer)inputChannel;
    
    // 对于 P02.1 (PCLK -> REQ14 -> InputChannel 2) -> OGU 2
    // 这样也符合逻辑
    
    // 配置触发方式
    switch(trigger)
    {
        case EXTI_TRIGGER_RISING:
            IfxScuEru_disableFallingEdgeDetection(inputChannel);
            IfxScuEru_enableRisingEdgeDetection(inputChannel);
            break;
            
        case EXTI_TRIGGER_FALLING:
            IfxScuEru_enableFallingEdgeDetection(inputChannel);
            IfxScuEru_disableRisingEdgeDetection(inputChannel);
            break;
            
        case EXTI_TRIGGER_BOTH:
            IfxScuEru_enableFallingEdgeDetection(inputChannel);
            IfxScuEru_enableRisingEdgeDetection(inputChannel);
            break;
            
        default:
            break;
    }
    
    // 使能触发脉冲并连接
    IfxScuEru_enableTriggerPulse(inputChannel);
    IfxScuEru_connectTrigger(inputChannel, triggerSelect);
    
    // 配置输出通道用于中断
    IfxScuEru_setFlagPatternDetection(outputChannel, inputChannel, TRUE);
    IfxScuEru_enablePatternDetectionTrigger(outputChannel);
    IfxScuEru_setInterruptGatingPattern(outputChannel, IfxScuEru_InterruptGatingPattern_alwaysActive);
    
    // 配置中断优先级
    volatile Ifx_SRC_SRCR *src = &MODULE_SRC.SCU.SCU.ERU[(int)outputChannel % 4];
    
    // 使用固定优先级60（与ISR_PRIORITY_MT9V034_VSYNC匹配）
    // 注意：这里应该根据具体应用设置不同的优先级
    // 对于MT9V034的VSYNC，使用60
    uint8 priority = 60;
    
    IfxSrc_init(src, IfxSrc_Tos_cpu0, priority);
    IfxSrc_enable(src);
    
    IfxCpu_restoreInterrupts(interrupt_state);
}

/***************************************************************
 *  @brief     使能EXTI中断
 *  @param     exti_pin    ERU通道及引脚
 *  @return    void
 **************************************************************/
void exti_enable(EXTI_Pin_t exti_pin)
{
    IfxScuEru_OutputChannel outputChannel = (IfxScuEru_OutputChannel)(exti_pin/3);
    volatile Ifx_SRC_SRCR *src = &MODULE_SRC.SCU.SCU.ERU[(int)outputChannel % 4];
    IfxSrc_enable(src);
}

/***************************************************************
 *  @brief     禁用EXTI中断
 *  @param     exti_pin    ERU通道及引脚
 *  @return    void
 **************************************************************/
void exti_disable(EXTI_Pin_t exti_pin)
{
    IfxScuEru_OutputChannel outputChannel = (IfxScuEru_OutputChannel)(exti_pin/3);
    volatile Ifx_SRC_SRCR *src = &MODULE_SRC.SCU.SCU.ERU[(int)outputChannel % 4];
    IfxSrc_disable(src);
}
