/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的核心功能演示 - 定时器测试例程
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
 * 文件功能：cpu0主函数，定时器中断测试
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
 * 2026年01月10日      毛毛    V3.1     增加定时器测试
 *
 * === 实验现象 ===
 * 1. 定时器0 (TIM_0): 
 *    - 周期: 100ms
 *    - 功能: 翻转LED1 (P21.4) -> 5Hz闪烁
 * 
 * 2. 定时器1 (TIM_1):
 *    - 周期: 500ms
 *    - 功能: 翻转LED2 (P21.5) -> 1Hz闪烁
 * 
 * 3. 定时器2 (TIM_2):
 *    - 周期: 1000ms (1秒)
 *    - 功能: 串口打印计数
 * 
 * 测试方法:
 * - 观察LED1快闪 (5Hz)
 * - 观察LED2慢闪 (1Hz)
 * - 串口每秒打印一次计数
 */
 
#include "ky_all.h"
#include "ky_timer.h"
#include "ky_gpio.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

// 计数器
volatile uint32_t timer0_count = 0;
volatile uint32_t timer1_count = 0;
volatile uint32_t timer2_count = 0;

// 定时器0回调函数 - 100ms翻转LED1
void timer0_callback(void)
{
    timer0_count++;
    gpio_toggle_level(GPIO_P21_4);
}

// 定时器1回调函数 - 500ms翻转LED2
void timer1_callback(void)
{
    timer1_count++;
    gpio_toggle_level(GPIO_P21_5);
}

// 定时器2回调函数 - 1秒打印计数
void timer2_callback(void)
{
    timer2_count++;
    printf("Timer counts - T0:%d T1:%d T2:%d\n", timer0_count, timer1_count, timer2_count);
}

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    
    /* 初始化开始 */
    uart_debug_init();      // 初始化调试串口
    
    // 初始化LED GPIO
    gpio_init(GPIO_P21_4, GPIO_MODE_OUT_PP, GPIO_LOW);
    gpio_init(GPIO_P21_5, GPIO_MODE_OUT_PP, GPIO_LOW);
    
    // 初始化定时器
    timer_init_ms(TIM_0, 100, timer0_callback);   // 100ms定时器
    timer_init_ms(TIM_1, 500, timer1_callback);   // 500ms定时器
    timer_init_ms(TIM_2, 1000, timer2_callback);  // 1秒定时器

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    printf("Timer Test Started\n");
    printf("TIM_0: 100ms -> LED1 (P21.4) 5Hz\n");
    printf("TIM_1: 500ms -> LED2 (P21.5) 1Hz\n");
    printf("TIM_2: 1000ms -> Print count\n\n");

    while(1)
    {
        /* 主循环 */
    }
}
