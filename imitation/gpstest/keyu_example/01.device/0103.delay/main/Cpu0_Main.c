/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的核心功能演示 - 延时函数测试例程
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
 * 文件功能：cpu0主函数，测试 delay_ms 和 delay_us
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
 * 2026年01月10日      毛毛    V3.1     增加延时测试
 *
 * === 实验现象 ===
 * LED1 (P21.4): 使用 delay_ms(100) 翻转 -> 周期 200ms (5Hz)
 * LED2 (P21.5): 使用 delay_us(500000) 翻转 -> 周期 1000ms (1Hz)
 * 注意：由于这是阻塞延时测试，两个LED会交替动作，
 *       逻辑上是: LED1翻转 -> 100ms -> LED1翻转 -> 100ms ... 
 *       1. LED1 翻转
 *       2. 延时 200ms
 *       3. LED2 翻转
 *       4. 延时 800ms
 *       
 *       这样 LED1 每 1秒闪烁一次
 *       LED1 亮 -> 延时 500ms -> LED1 灭
 *       LED2 亮 -> 延时 500ms (使用us) -> LED2 灭
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

    while(1)
    {
        /* 主循环开始 */
        
        // 阶段1: 测试 delay_ms
        // LED1 (P21.4) 亮
        gpio_set_level(GPIO_P21_4, GPIO_HIGH);
        delay_ms(500); // 延时 500ms
        
        // LED1 (P21.4) 灭
        gpio_set_level(GPIO_P21_4, GPIO_LOW);
        
        // 阶段2: 测试 delay_us
        // LED2 (P21.5) 亮
        gpio_set_level(GPIO_P21_5, GPIO_HIGH);
        delay_us(500000); // 延时 500,000us = 500ms
        
        // LED2 (P21.5) 灭
        gpio_set_level(GPIO_P21_5, GPIO_LOW);

        // 总周期约为 1秒 (500ms + 500ms)
        // 现象：LED1 亮500ms -> LED1 灭 & LED2 亮500ms -> LED2 灭 -> 循环
        
        /* 主循环结束 */
    }
}
