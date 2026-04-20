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
 * 文件名：[ky_timer.h]
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
#ifndef _KEYU_DRIVER_TIMER_H_
#define _KEYU_DRIVER_TIMER_H_

#include "Cpu/Std/Ifx_Types.h"
#include "ky_typedef.h"

// 定时器通道枚举
typedef enum {
    TIM_0 = 0,          // STM0 比较器0
    TIM_1,              // STM0 比较器1
    TIM_2,              // STM1 比较器0
    TIM_3,              // STM1 比较器1
    TIM_4,              // 备用
} TIM_Channel_t;
typedef void (*TimerCallback_t)(void);
extern TimerCallback_t TimerCallbacks[5];
void timer_init(TIM_Channel_t tim, uint32_t time_us, TimerCallback_t cb);   //定时器初始化(微秒)
#define timer_init_ms(tim, time_ms, cb) timer_init(tim, time_ms * 1000, cb)// 宏定义：初始化毫秒级定时器

#endif
