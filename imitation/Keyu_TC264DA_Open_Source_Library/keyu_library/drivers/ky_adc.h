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
 * 文件名：[ky_adc.h]
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
#ifndef __KY_ADC_H__
#define __KY_ADC_H__

#include "ky_all.h"

#define ADC_SAMPLE_FREQUENCY    10000000 // 默认采样频率 10Mhz

// ADC 引脚枚举 (Adapted for TC264DA)
typedef enum {
    // ADC0 (Group 0)
    ADC0_CH0_A0   = 0x00,
    ADC0_CH1_A1   = 0x01,
    ADC0_CH2_A2   = 0x02,
    ADC0_CH3_A3   = 0x03,
    ADC0_CH4_A4   = 0x04,
    ADC0_CH5_A5   = 0x05,
    ADC0_CH6_A6   = 0x06,
    ADC0_CH7_A7   = 0x07,
    ADC0_CH8_A8   = 0x08,
    ADC0_CH10_A10 = 0x0A,
    ADC0_CH11_A11 = 0x0B,
    ADC0_CH12_A12 = 0x0C,
    ADC0_CH13_A13 = 0x0D,

    // ADC1 (Group 1)
    ADC1_CH0_A16  = 0x10,
    ADC1_CH1_A17  = 0x11,
    ADC1_CH4_A20  = 0x14,
    ADC1_CH5_A21  = 0x15,
    ADC1_CH8_A24  = 0x18,
    ADC1_CH9_A25  = 0x19,

    // ADC2 (Group 2)
    ADC2_CH3_A35  = 0x23,
    ADC2_CH4_A36  = 0x24,
    ADC2_CH5_A37  = 0x25,
    ADC2_CH6_A38  = 0x26,
    ADC2_CH7_A39  = 0x27,
    ADC2_CH10_A44 = 0x2A,
    ADC2_CH11_A45 = 0x2B,
    ADC2_CH12_A46 = 0x2C,
    ADC2_CH13_A47 = 0x2D,
    ADC2_CH14_A48 = 0x2E,
    ADC2_CH15_A49 = 0x2F,
} ADC_Channel_t;
// ADC 分辨率枚举
typedef enum {
    ADC_RES_12BIT = 1, // 右对齐，结果为 0~4095
    ADC_RES_10BIT = 2, // 右对齐，结果为 0~1023
    ADC_RES_8BIT  = 0  // 左对齐，只读取高8位 (0~255)
} ADC_Resolution_t;
void adc_set_mode(ADC_Channel_t ch, ADC_Resolution_t re);   //ADC模式设置
uint16_t adc_convert(ADC_Channel_t ch);                     //ADC转换一次
#endif /* __KY_ADC_H__ */
