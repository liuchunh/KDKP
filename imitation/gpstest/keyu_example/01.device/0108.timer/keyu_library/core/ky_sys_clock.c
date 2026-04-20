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
 * 文件名：[ky_sys_clock.c]
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
#include "ky_sys_clock.h"

/***************************************************************
  *  @brief     系统时钟初始化函数
  *  @param     clockSource    时钟源选择（PLL或EVR）
  *  @note      根据选择的时钟源初始化系统时钟配置
  *  @Sample usage:     clockInit(CLOCK_SOURCE_PLL);
  **************************************************************/
void clockInit(ClockSource_t clockSource)
{
    IfxScuCcu_Config clockConfig = IfxScuCcu_defaultClockConfig;
    
    if (clockSource == CLOCK_SOURCE_PLL)
    {

    }
    else if (clockSource == CLOCK_SOURCE_EVR)
    {

    }
    
    (void)IfxScuCcu_init(&clockConfig);
    
}

/***************************************************************
  *  @brief     系统初始化函数
  *  @param     cpuCoreId    CPU核心ID
  *  @note      根据CPU核心ID执行相应的初始化操作
  *  @Sample usage:     systemInit(KEYU_CPU_CORE_0);
  **************************************************************/
void systemInit(KeyuCpuCoreId_t cpuCoreId)
{
    IfxCpu_enableInterrupts();
    
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    
    if (cpuCoreId == KEYU_CPU_CORE_0)
    {
        IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
    }
}

/***************************************************************
  *  @brief     CPU同步等待函数
  *  @param     syncEvent    同步事件指针
  *  @param     timeoutMs    超时时间（毫秒）
  *  @return    boolean      TRUE：同步成功，FALSE：同步失败或超时
  *  @note      实现多核CPU同步机制
  *  @Sample usage:     cpuSyncWait(&syncEvent, 1000);
  **************************************************************/
boolean cpuSyncWait(IfxCpu_syncEvent *syncEvent, uint32 timeoutMs)
{
    boolean syncResult;
    
    if (syncEvent == NULL_PTR)
    {
        return FALSE;
    }
    
    IfxCpu_emitEvent(syncEvent);

    syncResult = IfxCpu_waitEvent(syncEvent, timeoutMs);
    
    return syncResult;
}
