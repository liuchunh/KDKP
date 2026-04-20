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
 * 文件名：[ky_sys_clock.h]
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
#ifndef KEYU_LIB_INCLUDE_KEYU_CORE_KEYU_CORE_SYSCLOCK_H_
#define KEYU_LIB_INCLUDE_KEYU_CORE_KEYU_CORE_SYSCLOCK_H_

#include "Cpu/Std/IfxCpu.h"
#include "Scu/Std/IfxScuCcu.h"
#include "Scu/Std/IfxScuWdt.h"

// 定义系统可用的时钟源类型
typedef enum {
    CLOCK_SOURCE_PLL = 0,   // 使用PLL锁相环时钟源（推荐，高性能
    CLOCK_SOURCE_EVR = 1    // 使用EVR备份时钟源（100MHz固定频率
} ClockSource_t;
// 用于标识当前运行的CPU核心
typedef enum {
    KEYU_CPU_CORE_0 = 0,    // CPU核心0
    KEYU_CPU_CORE_1 = 1     // CPU核心1
} KeyuCpuCoreId_t;
void clockInit(ClockSource_t clockSource);                          //时钟初始化
void systemInit(KeyuCpuCoreId_t cpuCoreId);                         //系统初始化
boolean cpuSyncWait(IfxCpu_syncEvent *syncEvent, uint32 timeoutMs); //CPU同步等待
#endif /* KEYU_LIB_INCLUDE_KEYU_CORE_KEYU_CORE_SYSCLOCK_H_ */