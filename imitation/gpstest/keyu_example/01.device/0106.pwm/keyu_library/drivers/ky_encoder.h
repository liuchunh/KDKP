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
 * 文件名：[ky_encoder.h]
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
#ifndef __KY_ENCODER_H__
#define __KY_ENCODER_H__

#include "ky_all.h"
#include "IfxGpt12.h"

typedef enum {
    ENCODER_TIM2 = 0,
    ENCODER_TIM3 = 1,
    ENCODER_TIM4 = 2,
    ENCODER_TIM5 = 3,
    ENCODER_TIM6 = 4
} Encoder_Index_t;
typedef enum {
    // TIM2 A相引脚
    TIM2_ENC_A_P00_7 = 0,
    TIM2_ENC_A_P33_7,

    // TIM3 A相引脚
    TIM3_ENC_A_P02_6,

    // TIM4 A相引脚
    TIM4_ENC_A_P02_8,

    // TIM5 A相引脚
    TIM5_ENC_A_P21_7,
    TIM5_ENC_A_P10_3,

    // TIM6 A相引脚
    TIM6_ENC_A_P20_3,
    TIM6_ENC_A_P10_2
} Encoder_A_Pin_t;
typedef enum {
    // TIM2 B相引脚
    TIM2_ENC_B_P00_8 = 0,
    TIM2_ENC_B_P33_6,

    // TIM3 B相引脚
    TIM3_ENC_B_P02_7,

    // TIM4 B相引脚
    TIM4_ENC_B_P00_9,
    TIM4_ENC_B_P33_5,

    // TIM5 B相引脚
    TIM5_ENC_B_P21_6,
    TIM5_ENC_B_P10_1,

    // TIM6 B相引脚
    TIM6_ENC_B_P20_0
} Encoder_B_Pin_t;
void encoder_init_quad(Encoder_Index_t encoderIndex, Encoder_A_Pin_t aPin, Encoder_B_Pin_t bPin);       //正交编码器初始化
void encoder_init_dir(Encoder_Index_t encoderIndex, Encoder_A_Pin_t pulsePin, Encoder_B_Pin_t dirPin);  //方向编码器初始化
int16 encoder_get_count(Encoder_Index_t encoderIndex);                                                  //获取编码器计数
void encoder_clear_count(Encoder_Index_t encoderIndex);                                                 //清除编码器计数
#endif /* __KY_ENCODER_H__ */
