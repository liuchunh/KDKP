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
 * 文件名：[ky_exti.h]
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
#ifndef KEYU_DRIVER_EXTI_H_
#define KEYU_DRIVER_EXTI_H_

#include "Cpu/Std/Ifx_Types.h"
#include "Scu/Std/IfxScuEru.h"

/***************************************************************
 *  ERU通道枚举定义
 **************************************************************/
typedef enum {
    // ERU通道0
    ERU_CH0_REQ0_P15_4   = 0*3 + 1,
    
    // ERU通道1
    ERU_CH1_REQ10_P14_3  = 1*3 + 1,
    
    // ERU通道2
    ERU_CH2_REQ7_P00_4   = 2*3,
    ERU_CH2_REQ14_P02_1  = 2*3 + 1,
    ERU_CH2_REQ2_P10_2   = 2*3 + 2,
    
    // ERU通道3
    ERU_CH3_REQ6_P02_0   = 3*3,
    ERU_CH3_REQ3_P10_3   = 3*3 + 1,
    ERU_CH3_REQ15_P14_1  = 3*3 + 2,
    
    // ERU通道4
    ERU_CH4_REQ13_P15_5  = 4*3,
    ERU_CH4_REQ8_P33_7   = 4*3 + 1,
    
    // ERU通道5
    ERU_CH5_REQ1_P15_8   = 5*3,
    
    // ERU通道6
    ERU_CH6_REQ12_P11_10 = 6*3,
    ERU_CH6_REQ9_P20_0   = 6*3 + 1,
    
    // ERU通道7
    ERU_CH7_REQ16_P15_1  = 7*3,
    ERU_CH7_REQ11_P20_9  = 7*3 + 1,
} EXTI_Pin_t;
/***************************************************************
 *  触发方式枚举定义
 **************************************************************/
typedef enum {
    EXTI_TRIGGER_RISING  = 0,  // 上升沿触发
    EXTI_TRIGGER_FALLING = 1,  // 下降沿触发
    EXTI_TRIGGER_BOTH    = 2,  // 双边沿触发
} EXTI_Trigger_t;
/***************************************************************
 *  函数声明
 **************************************************************/

void exti_init(EXTI_Pin_t exti_pin, EXTI_Trigger_t trigger);    //外部中断初始化
void exti_enable(EXTI_Pin_t exti_pin);                          //使能外部中断
void exti_disable(EXTI_Pin_t exti_pin);                         //禁用外部中断
#endif /* KEYU_DRIVER_EXTI_H_ */
