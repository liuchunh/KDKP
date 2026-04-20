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
 * 文件名：[ky_timer.c]
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
#include "ky_timer.h"
#include "Stm/Std/IfxStm.h"
#include "Src/Std/IfxSrc.h"


TimerCallback_t TimerCallbacks[5] = {0};        // 全局回调函数数组
static uint32 g_TimerCompareValues[5] = {0};    // 比较值存储，用于在中断中更新下次比较值

// 内部帮助函数：获取STM模块指针和比较器配置
/***************************************************************
  *  @brief     定时器获取STMINFO
  *  @param     tim     [tim description]
  *  @param     stm     [stm description]
  *  @param     comparator     [comparator description]
  *  @Sample usage:     timer_get_stm_info(tim, stm, comparator);
 **************************************************************/
static void timer_get_stm_info(TIM_Channel_t tim, Ifx_STM **stm, IfxStm_Comparator *comparator)
{
    switch (tim)
    {
        case TIM_0:
            *stm = &MODULE_STM0;
            *comparator = IfxStm_Comparator_0;
            break;
        case TIM_1:
            *stm = &MODULE_STM0;
            *comparator = IfxStm_Comparator_1;
            break;
        case TIM_2:
            *stm = &MODULE_STM1;
            *comparator = IfxStm_Comparator_0;
            break;
        case TIM_3:
            *stm = &MODULE_STM1;
            *comparator = IfxStm_Comparator_1;
            break;
        default:
            *stm = &MODULE_STM0;
            *comparator = IfxStm_Comparator_0;
            break;
    }
}

/***************************************************************
 *  @brief     定时器初始化
 *  @param     tim       选择定时器 (TIM_0 ~ TIM_4)
 *  @param     time_us   定时时间，单位微秒(us)。
 *  @param     cb        中断回调函数
 **************************************************************/
void timer_init(TIM_Channel_t tim, uint32_t time_us, TimerCallback_t cb)
{
    Ifx_STM *stm;
    IfxStm_Comparator comparator;
    IfxStm_CompareConfig stmConfig;
    uint32 ticks;

    if (tim >= TIM_4) return;

    // 注册回调
    TimerCallbacks[tim] = cb;

    // 获取STM模块信息
    timer_get_stm_info(tim, &stm, &comparator);

    // 计算 ticks:  time_us * (freq / 1000000)
    float32 stmFreq = IfxStm_getFrequency(stm);
    ticks = (uint32)(stmFreq * (float32)time_us / 1000000.0f);
    g_TimerCompareValues[tim] = ticks;

    IfxStm_initCompareConfig(&stmConfig);
    stmConfig.comparator = comparator;
    stmConfig.ticks = ticks;
    stmConfig.typeOfService = IfxSrc_Tos_cpu0;

    switch (tim)
    {
        case TIM_0: 
            stmConfig.comparatorInterrupt = IfxStm_ComparatorInterrupt_ir0;
            stmConfig.triggerPriority = 40; 
            break;
        case TIM_1: 
            stmConfig.comparatorInterrupt = IfxStm_ComparatorInterrupt_ir1;
            stmConfig.triggerPriority = 41; 
            break;
        case TIM_2: 
            stmConfig.comparatorInterrupt = IfxStm_ComparatorInterrupt_ir0;
            stmConfig.triggerPriority = 42; 
            break;
        case TIM_3: 
            stmConfig.comparatorInterrupt = IfxStm_ComparatorInterrupt_ir1;
            stmConfig.triggerPriority = 43; 
            break;
        default:
            return;
    }

    IfxStm_initCompare(stm, &stmConfig);
}

// 内部函数：需要在 Isr.c 的中断中调用此处理函数
/***************************************************************
  *  @brief     定时器INTERRUPTHANDLER
  *  @param     tim     [tim description]
  *  @Sample usage:     timer_interrupt_handler(tim);
 **************************************************************/
void timer_interrupt_handler(TIM_Channel_t tim)
{
    Ifx_STM *stm;
    IfxStm_Comparator comparator;
    
    timer_get_stm_info(tim, &stm, &comparator);
    
    IfxStm_updateCompare(stm, comparator, IfxStm_getLower(stm) + g_TimerCompareValues[tim]);

    if (TimerCallbacks[tim] != NULL_PTR)
    {
        TimerCallbacks[tim]();
    }
}
