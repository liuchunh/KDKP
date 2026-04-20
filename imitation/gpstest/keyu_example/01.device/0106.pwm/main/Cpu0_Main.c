/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的核心功能演示 - PWM电机控制测试例程
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
 * 文件功能：cpu0主函数，PWM电机控制测试
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
 * 2026年01月10日      毛毛    V3.1     增加PWM电机控制测试
 *
 * === 实验现象 ===
 * 1. 电机1 (Motor1): 
 *    - PWM_A: P21.2
 *    - PWM_B: P21.3
 * 2. 电机2 (Motor2):
 *    - PWM_A: P21.4
 *    - PWM_B: P21.5
 * 
 * PWM频率: 10kHz
 * 
 * 运行模式:
 * - 正转: PWM_A输出占空比, PWM_B为0
 * - 反转: PWM_A为0, PWM_B输出占空比
 * 
 * 测试流程:
 * 1. 正转加速: 占空比从0增加到5000
 * 2. 正转减速: 占空比从5000减少到0
 * 3. 反转加速: 占空比从0增加到5000
 * 4. 反转减速: 占空比从5000减少到0
 * 5. 循环
 */
 
#include "ky_all.h"
#include "ky_pwm.h"
#include "ky_delay.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

// 电机控制函数
// duty: 0~5000为正转, 0~-5000为反转
void motor_control(PwmChannel_t pwm_a, PwmChannel_t pwm_b, int16_t duty)
{
    if (duty >= 0)
    {
        // 正转: A相输出PWM, B相为0
        pwm_set_duty(pwm_a, duty);
        pwm_set_duty(pwm_b, 0);
    }
    else
    {
        // 反转: A相为0, B相输出PWM
        pwm_set_duty(pwm_a, 0);
        pwm_set_duty(pwm_b, -duty);
    }
}

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    
    /* 初始化开始 */
    uart_debug_init();      // 初始化串口调试
    
    // 初始化电机1的PWM (P21.2和P21.3)
    pwm_init(PWM_P21_2, 10000, 0);  // 频率10kHz, 初始占空比0
    pwm_init(PWM_P21_3, 10000, 0);
    
    // 初始化电机2的PWM (P21.4和P21.5)
    pwm_init(PWM_P21_4, 10000, 0);  // 频率10kHz, 初始占空比0
    pwm_init(PWM_P21_5, 10000, 0);

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    int16_t duty = 0;
    int8_t direction = 1;  // 1: 正向增加, -1: 反向减少

    while(1)
    {
        /* 主循环开始 */
        
        // 阶段1: 正转加速 (0 -> 5000)
        for (duty = 0; duty <= 5000; duty += 50)
        {
            motor_control(PWM_P21_2, PWM_P21_3, duty);
            motor_control(PWM_P21_4, PWM_P21_5, duty);
            printf("Forward: %d\n", duty);
            delay_ms(10);
        }
        
        // 阶段2: 正转减速 (5000 -> 0)
        for (duty = 5000; duty >= 0; duty -= 50)
        {
            motor_control(PWM_P21_2, PWM_P21_3, duty);
            motor_control(PWM_P21_4, PWM_P21_5, duty);
            printf("Forward: %d\n", duty);
            delay_ms(10);
        }
        
        // 阶段3: 反转加速 (0 -> -5000)
        for (duty = 0; duty >= -5000; duty -= 50)
        {
            motor_control(PWM_P21_2, PWM_P21_3, duty);
            motor_control(PWM_P21_4, PWM_P21_5, duty);
            printf("Reverse: %d\n", duty);
            delay_ms(10);
        }
        
        // 阶段4: 反转减速 (-5000 -> 0)
        for (duty = -5000; duty <= 0; duty += 50)
        {
            motor_control(PWM_P21_2, PWM_P21_3, duty);
            motor_control(PWM_P21_4, PWM_P21_5, duty);
            printf("Reverse: %d\n", duty);
            delay_ms(10);
        }
        
        /* 主循环结束 */
    }
}
