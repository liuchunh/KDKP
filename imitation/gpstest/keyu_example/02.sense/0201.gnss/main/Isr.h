/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的基础功能空库
 * 版权所有：[2025] [北京科宇通博科技有限公司江西分公司]
 * 
 * 许可协议：采用 GNU GPL v3.0 开源许可
 * 您可依据协议进行二次开发、传播，但须保留原始版权信息
 * 协议详情参见：https://www.gnu.org/licenses/gpl-3.0.html
 * 
 * 免责声明：本组件库仅提供技术参考，使用方需自行验证适用性
 * 
 * 协议文件：GPL v3.0 完整文本位于 [Keyu_TC264DA_Open_Source_Library] 目录下
 * 
 * === 文件信息 ===
 * 文件名：Isr.h
 * 文件功能：中断服务程序头文件 - 定义中断优先级
 * 开发单位：北京科宇通博科技有限公司江西分公司
 * 适用环境：[AURIX™ Development Studio V1.10.28]
  * 官方渠道：
        代码仓库 https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library.git 
        淘宝店铺 https://kyznc.taobao.com/ 
        技术支持 QQ群 974530818
 * 
 * === 修订记录 ===
 * 日期         开发者    变更说明
 * -------------------------------------------
 * 2026年01月07日      毛毛    V3.0
 */

#ifndef ISR_H_
#define ISR_H_

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include "Cpu/Std/Ifx_Types.h"


/******************************************************************************/
/*-----------------------Interrupt Priority Definitions-----------------------*/
/******************************************************************************/

/*
 * AURIX™ TC264DA 中断优先级说明：
 * - 优先级范围：1-255（0保留给NMI）
 * - 数值越大，优先级越高
 * - iLLD库本身不定义具体的优先级值，必须由用户在应用层定义
 * - 建议分组：
 *   1-63:   低优先级（非关键任务）
 *   64-127: 中优先级（一般任务）
 *   128-191:高优先级（关键任务）
 *   192-255:最高优先级（紧急任务）
 */

/* 系统定时器中断优先级 */
#define ISR_PRIORITY_STM0_CMP0          40  
#define ISR_PRIORITY_STM0_CMP1          41  
#define ISR_PRIORITY_STM1_CMP0          45  
#define ISR_PRIORITY_STM1_CMP1          46  
#define ISR_PRIORITY_STM2_CMP0          42  
#define ISR_PRIORITY_STM2_CMP1          43  

/* MT9V03X Camera */
#define ISR_PRIORITY_MT9V034_VSYNC      200  // VSYNC中断优先级 - 提高到最高
#define ISR_PRIORITY_MT9V03X_DMA        208  

/* GPIO外部中断 */
#define ISR_PRIORITY_GPIO_EXT_INT0      100 
#define ISR_PRIORITY_GPIO_EXT_INT1      101 

/* UART通信中断优先级 */
#define ISR_PRIORITY_UART0_RX           80
#define ISR_PRIORITY_UART0_TX           81
#define ISR_PRIORITY_UART0_ERR          82

#define ISR_PRIORITY_UART1_RX           83
#define ISR_PRIORITY_UART1_TX           84
#define ISR_PRIORITY_UART1_ERR          85

#define ISR_PRIORITY_UART2_RX           86
#define ISR_PRIORITY_UART2_TX           87
#define ISR_PRIORITY_UART2_ERR          88

#define ISR_PRIORITY_UART3_RX           150  // 提高优先级,确保GPS数据及时接收
#define ISR_PRIORITY_UART3_TX           90
#define ISR_PRIORITY_UART3_ERR          91

/* DMA传输中断优先级 */
#define ISR_PRIORITY_DMA_CH0            50
#define ISR_PRIORITY_DMA_CH1            51
#define ISR_PRIORITY_DMA_CH2            52
#define ISR_PRIORITY_DMA_CH3            53
#define ISR_PRIORITY_DMA_CH4            54
#define ISR_PRIORITY_DMA_CH5            55
#define ISR_PRIORITY_DMA_CH6            56
#define ISR_PRIORITY_DMA_CH7            57
#define ISR_PRIORITY_DMA_CH8            58
#define ISR_PRIORITY_DMA_CH9            59
#define ISR_PRIORITY_DMA_CH10           60
#define ISR_PRIORITY_DMA_CH11           61
#define ISR_PRIORITY_DMA_CH12           62
#define ISR_PRIORITY_DMA_CH13           63
#define ISR_PRIORITY_DMA_CH14           64
#define ISR_PRIORITY_DMA_CH15           65

/* ADC转换中断优先级 */
#define ISR_PRIORITY_ADC_RESULT         90

/* CAN通信中断优先级 */
#define ISR_PRIORITY_CAN_RX             110
#define ISR_PRIORITY_CAN_TX             105

/* 编码器中断优先级 */
#define ISR_PRIORITY_ENCODER_T2         130
#define ISR_PRIORITY_ENCODER_T3         131
#define ISR_PRIORITY_ENCODER_T4         132
#define ISR_PRIORITY_ENCODER_T5         133
#define ISR_PRIORITY_ENCODER_T6         134

/* I2C中断优先级 */
#define ISR_PRIORITY_I2C_ERR            140
#define ISR_PRIORITY_I2C_P              141



/* PWM中断优先级（最高优先级） */
#define ISR_PRIORITY_PWM_PERIOD         150

#endif /* ISR_H_ */
