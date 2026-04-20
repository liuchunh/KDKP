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
 * 文件名：[ky_delay.c]
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
#include "ky_delay.h"
#include "Stm/Std/IfxStm.h"

/***************************************************************
  *  @brief     毫秒延时函数
  *  @param     ms    延时时间（毫秒）
  *  @note      该函数使用系统定时器（STM）实现精确的毫秒级延时
  *  @note      该函数使用忙等待方式，会占用CPU资源
  *  @Sample usage:     delay_ms(100);  // 延时100ms
  **************************************************************/
void delay_ms(uint16 ms)
{
    sint32 ticks;
    
    ticks = IfxStm_getTicksFromMilliseconds(&MODULE_STM0, ms);
    
    if (ticks > 0)
    {
        IfxStm_waitTicks(&MODULE_STM0, (uint32)ticks);
    }
}

/***************************************************************
  *  @brief     微秒延时函数
  *  @param     us    延时时间（微秒）
  *  @note      使用STM系统定时器实现精确的微秒级延时
  *  @Sample usage:     delay_us(500);  // 延时500us
  **************************************************************/
void delay_us(uint16 us)
{
    sint32 ticks;
    
    ticks = IfxStm_getTicksFromMicroseconds(&MODULE_STM0, us);
    
    if (ticks > 0)
    {
        IfxStm_waitTicks(&MODULE_STM0, (uint32)ticks);
    }
}
