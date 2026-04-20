/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的核心功能演示 - GPIO LED闪烁例程
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
 * 文件名：Cpu0_Main.c
 * 文件功能：cpu0主函数，LED闪烁测试
 * 开发单位：北京科宇通博科技有限公司江西分公司
 * 适用环境：[AURIX™ Development Studio V1.10.28]
 * 官方渠道：
 *         代码仓库 https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library.git
 *         淘宝店铺 https://kyznc.taobao.com/
 *         技术支持 QQ群 974530818
 * 
 * === 修订记录 ===
 * 日期         开发者    变更说明
 * -------------------------------------------
 * 2025年12月25日      毛毛    V3.0     
 * 2026年01月10日      毛毛    V3.1     简单的GPIO闪烁测试
 *
 * === 实验现象 ===
 * LED1 (P21.4): 10Hz 快闪 (每50ms翻转)
 * LED2 (P21.5): 2Hz  慢闪 (每250ms翻转)
 */
 
#include "ky_all.h"
#include "ky_gpio.h"
#include "ky_delay.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    
    /* 初始化开始 */
    // 初始化 P21.4 和 P21.5 为推挽输出，默认低电平
    gpio_init(GPIO_P21_4, GPIO_MODE_OUT_PP, GPIO_LOW);
    gpio_init(GPIO_P21_5, GPIO_MODE_OUT_PP, GPIO_LOW);

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    uint8_t count = 0;

    while(1)
    {
        /* 主循环开始 */
        
        // P21.4: 10Hz -> 周期100ms (50ms亮, 50ms灭) -> 每50ms翻转一次
        gpio_toggle_level(GPIO_P21_4);

        // P21.5: 2Hz -> 周期500ms (250ms亮, 250ms灭) -> 每250ms翻转一次 (5 * 50ms)
        count++;
        if (count >= 5)
        {
            gpio_toggle_level(GPIO_P21_5);
            count = 0;
        }

        delay_ms(50); // 基准延时 50ms
        /* 主循环结束 */
    }
}
