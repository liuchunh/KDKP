/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的核心功能演示 - ADC采集例程
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
 * 文件功能：cpu0主函数，主要用于初始化及主循环
 * 开发单位：北京科宇通博科技有限公司江西分公司
 * 适用环境：[AURIX™ Development Studio V1.10.28]
  * 官方渠道：
        代码仓库 https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library.git
        淘宝店铺 https://kyznc.taobao.com/
        技术支持 QQ群 974530818
 * 
 * === 修订记录 ===
 * 日期         开发者    变更说明
 * -------------------------------------------
 * 2026年01月07日      毛毛    V3.0
 * 2026年01月10日      毛毛    V3.1     增加ADC例程
 *
 * === 代码说明 ===
 * 1.ADC通道配置：
 *    - AN1: 8位分辨率
 *    - AN2: 10位分辨率
 *    - AN3: 12位分辨率
 *    - AN4: 12位分辨率
 * 2. 数据输出方式：
 *    - 方式A：板载屏幕显示 (已适配科宇标准 IPS/TFT 屏幕接口)
 *    - 方式B：串口调试助手
 *    - 端口：UART0 (通常对应 USB 转串口)
 *    - 波特率：115200 (8-N-1)
 *    - 输出格式：文本打印 / 虚拟示波器格式
 * 3. 实验现象：
 *  旋转电位器或接入传感器，屏幕/串口数值随电压变化而变化
 */
 
#include "ky_all.h"
#include "ky_adc.h"
#include "ky_ips.h"
#include "ky_delay.h"

// ADC通道宏定义
#define ADC_CH_AN1  ADC0_CH0_A0
#define ADC_CH_AN2  ADC0_CH1_A1
#define ADC_CH_AN3  ADC0_CH2_A2
#define ADC_CH_AN4  ADC0_CH3_A3

IfxCpu_syncEvent cpuSyncEvent = 0;

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    uart_debug_init();  
    /* 初始化开始 */
    ips_init(IPS_TYPE_200);
    // ADC初始化配置

    // AN1 - 8位
    adc_set_mode(ADC_CH_AN1, ADC_RES_8BIT);
    // AN2 - 10位
    adc_set_mode(ADC_CH_AN2, ADC_RES_10BIT);
    // AN3 - 12位
    adc_set_mode(ADC_CH_AN3, ADC_RES_12BIT);
    // AN4 - 12位
    adc_set_mode(ADC_CH_AN4, ADC_RES_12BIT);

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    while(1)
    {
        /* 主循环开始 */
        // ADC转换读取
        uint16_t val_an1 = adc_convert(ADC_CH_AN1);
        uint16_t val_an2 = adc_convert(ADC_CH_AN2);
        uint16_t val_an3 = adc_convert(ADC_CH_AN3);
        uint16_t val_an4 = adc_convert(ADC_CH_AN4);

        // 串口打印
        printf("AN1: %d, AN2: %d, AN3: %d, AN4: %d\n", val_an1, val_an2, val_an3, val_an4);

        // IPS屏幕显示
        // 显示AN1 (8-bit, max 255)
        ips_show_string(10, 40, "AN1 (8bit): ");
        ips_show_int(120, 40, val_an1, 4);

        // 显示AN2 (10-bit, max 1023)
        ips_show_string(10, 60, "AN2 (10bit):");
        ips_show_int(120, 60, val_an2, 4);

        // 显示AN3 (12-bit, max 4095)
        ips_show_string(10, 80, "AN3 (12bit):");
        ips_show_int(120, 80, val_an3, 4);

        // 显示AN4 (12-bit, max 4095)
        ips_show_string(10, 100,"AN4 (12bit):");
        ips_show_int(120, 100, val_an4, 4);

        delay_ms(100);
        /* 主循环结束 */
    }
}
