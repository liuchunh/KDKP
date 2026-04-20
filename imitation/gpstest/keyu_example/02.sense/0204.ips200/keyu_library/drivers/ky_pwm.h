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
 * 文件名：[ky_pwm.h]
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
#ifndef KEYU_LIB_INCLUDE_KEYU_DRIVERS_KEYU_DRIVER_PWM_H_
#define KEYU_LIB_INCLUDE_KEYU_DRIVERS_KEYU_DRIVER_PWM_H_

#include "Ifx_Types.h"
#include "_PinMap/IfxGtm_PinMap.h"

#define PWM_DUTY_MAX 10000

typedef enum {
    // --- Port 00 ---
    PWM_P00_0, PWM_P00_1, PWM_P00_2, PWM_P00_3, PWM_P00_4,
    PWM_P00_5, PWM_P00_6, PWM_P00_7, PWM_P00_8, PWM_P00_9,
    PWM_P00_10, PWM_P00_11, PWM_P00_12,

    // --- Port 02 ---
    PWM_P02_0, PWM_P02_1, PWM_P02_2, PWM_P02_3, PWM_P02_4,
    PWM_P02_5, PWM_P02_6, PWM_P02_7, PWM_P02_8,

    // --- Port 10 ---
    PWM_P10_0, PWM_P10_1, PWM_P10_2, PWM_P10_3, PWM_P10_4,
    PWM_P10_5, PWM_P10_6, PWM_P10_7, PWM_P10_8,

    // --- Port 11 ---
    PWM_P11_2, PWM_P11_3, PWM_P11_6, PWM_P11_9, PWM_P11_10,
    PWM_P11_11, PWM_P11_12,

    // --- Port 13 ---
    PWM_P13_0, PWM_P13_1, PWM_P13_2, PWM_P13_3,

    // --- Port 14 ---
    PWM_P14_0, PWM_P14_1, PWM_P14_2, PWM_P14_3, PWM_P14_4,
    PWM_P14_5, PWM_P14_6, PWM_P14_7, PWM_P14_8, PWM_P14_9,
    PWM_P14_10,

    // --- Port 15 ---
    PWM_P15_0, PWM_P15_1, PWM_P15_2, PWM_P15_3, PWM_P15_4,
    PWM_P15_5, PWM_P15_6, PWM_P15_7, PWM_P15_8,

    // --- Port 20 ---
    PWM_P20_0, PWM_P20_1, PWM_P20_3, PWM_P20_6, PWM_P20_7,
    PWM_P20_8, PWM_P20_9, PWM_P20_10, PWM_P20_11, PWM_P20_12,
    PWM_P20_13, PWM_P20_14,

    // --- Port 21 ---
    PWM_P21_0, PWM_P21_1, PWM_P21_2, PWM_P21_3, PWM_P21_4,
    PWM_P21_5, PWM_P21_6, PWM_P21_7,

    // --- Port 22 ---
    PWM_P22_0, PWM_P22_1, PWM_P22_2, PWM_P22_3,

    // --- Port 23 ---
    PWM_P23_0, PWM_P23_1, PWM_P23_2, PWM_P23_3, PWM_P23_4,
    PWM_P23_5,

    // --- Port 32 ---
    PWM_P32_0, PWM_P32_2, PWM_P32_3, PWM_P32_4,

    // --- Port 33 ---
    PWM_P33_0, PWM_P33_1, PWM_P33_2, PWM_P33_3, PWM_P33_4,
    PWM_P33_5, PWM_P33_6, PWM_P33_7, PWM_P33_8, PWM_P33_9,
    PWM_P33_10, PWM_P33_11, PWM_P33_12, PWM_P33_13,

    PWM_CHANNEL_MAX
} PwmChannel_t;
void pwm_init(PwmChannel_t channel, uint32 frequency, uint16 initialDuty);  //PWM初始化
void pwm_set_duty(PwmChannel_t channel, uint16 duty);                       //设置PWM占空比
#endif /* KEYU_LIB_INCLUDE_KEYU_DRIVERS_KEYU_DRIVER_PWM_H_ */
