/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的核心功能演示 - 方向编码器测试例程
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
 * 文件功能：cpu0主函数，方向编码器读取测试
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
 * 2026年01月10日      毛毛    V3.1     增加 3路 方向编码器测试
 *
 * === 实验现象 ===
 * 1. 编码器 1 (TIM6): 
 *    - A相/Pulse: P20.3
 *    - B相/Dir:   P20.0
 * 2. 编码器 2 (TIM2):
 *    - A相/Pulse: P33.7
 *    - B相/Dir:   P33.6
 * 3. 编码器 3 (TIM4):
 *    - A相/Pulse: P02.8
 *    - B相/Dir:   P00.9
 * 
 * 旋转任意编码器，屏幕/串口显示对应计数值
 */
 
#include "ky_all.h"
#include "ky_encoder.h"
#include "ky_ips.h"
#include "ky_delay.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    
    /* 初始化开始 */
    uart_debug_init();      // 初始化串口调试
    ips_init(IPS_TYPE_200); // 初始化屏幕
    
    // 初始化编码器 1 (TIM6)
    // A相=P20.3, B相=P20.0 (用户指定: 20.0, 20.3)
    encoder_init_dir(ENCODER_TIM6, TIM6_ENC_A_P20_3, TIM6_ENC_B_P20_0);

    // 初始化编码器 2 (TIM2)
    // A相=P33.7, B相=P33.6
    encoder_init_dir(ENCODER_TIM2, TIM2_ENC_A_P33_7, TIM2_ENC_B_P33_6);

    // 初始化编码器 3 (TIM4)
    // A相=P02.8, B相=P00.9 (用户指定: 00.9, 02.8)
    encoder_init_dir(ENCODER_TIM4, TIM4_ENC_A_P02_8, TIM4_ENC_B_P00_9);

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    while(1)
    {
        /* 主循环开始 */
        
        // 获取编码器计数
        int16_t count1 = encoder_get_count(ENCODER_TIM6);
        int16_t count2 = encoder_get_count(ENCODER_TIM2);
        int16_t count3 = encoder_get_count(ENCODER_TIM4);
        
        // 串口打印
        printf("Enc1: %d, Enc2: %d, Enc3: %d\n", count1, count2, count3);
        
        // IPS显示
        ips_show_string(10, 20, "Dir Encoder Test");
        
        ips_show_string(10, 40, "Enc1 (TIM6):"); // 20.3, 20.0
        ips_show_int(120, 40, count1, 5);
        
        ips_show_string(10, 60, "Enc2 (TIM2):"); // 33.7, 33.6
        ips_show_int(120, 60, count2, 5);

        ips_show_string(10, 80, "Enc3 (TIM4):"); // 02.8, 00.9
        ips_show_int(120, 80, count3, 5);
        
        delay_ms(100);
        /* 主循环结束 */
    }
}
