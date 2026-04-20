/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的核心功能演示 - UART串口测试例程
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
 * 文件功能：cpu0主函数，UART串口通信测试
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
 * 2026年01月10日      毛毛    V3.1     增加UART串口测试
 *
 * === 实验现象 ===
 * 1. UART3配置:
 *    - TX: P15.6
 *    - RX: P15.7
 *    - 波特率: 115200
 * 
 * 2. 功能:
 *    - 持续发送: "kykj" 字符串 (每500ms发送一次)
 *    - 接收显示: 接收到的数据会显示在IPS屏幕和调试串口
 * 
 * 3. 测试方法:
 *    - 使用USB转TTL模块连接P15.6(TX)和P15.7(RX)
 *    - 串口助手会收到"kykj"字符串
 *    - 从串口助手发送数据,会在屏幕和调试串口显示
 */
 
#include "ky_all.h"
#include "ky_uart.h"
#include "ky_ips.h"
#include "ky_delay.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

// 接收缓冲区
#define RX_DISPLAY_SIZE 32
char rxDisplayBuffer[RX_DISPLAY_SIZE] = {0};
uint8_t rxDisplayIndex = 0;

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    
    /* 初始化开始 */
    uart_debug_init();      // 初始化调试串口 (UART0)
    ips_init(IPS_TYPE_200); // 初始化屏幕
    
    // 初始化UART3 (P15.6 TX, P15.7 RX)
    uart_init(UART_3, 115200, UART3_RXA_P15_7, UART3_TX_P15_6);

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    // 显示标题
    ips_show_string(10, 20, "UART3 Test");
    ips_show_string(10, 40, "TX:P15.6 RX:P15.7");
    ips_show_string(10, 60, "Received:");

    printf("UART3 Test Started\n");
    printf("TX: P15.6, RX: P15.7\n");
    printf("Baudrate: 115200\n\n");

    while(1)
    {
        /* 主循环开始 */
        
        // 持续发送 "kykj"
        uart_send_string(UART_3, "kykj\r\n");
        
        // 检查是否有接收数据
        uint8_t receivedByte;
        while (uart_read_byte(UART_3, &receivedByte))
        {
            // 通过调试串口打印
            printf("Received: 0x%02X ('%c')\n", receivedByte, 
                   (receivedByte >= 32 && receivedByte < 127) ? receivedByte : '.');
            
            // 添加到显示缓冲区
            if (receivedByte >= 32 && receivedByte < 127)  // 可打印字符
            {
                rxDisplayBuffer[rxDisplayIndex++] = receivedByte;
                rxDisplayBuffer[rxDisplayIndex] = '\0';
                
                // 缓冲区满则清空
                if (rxDisplayIndex >= RX_DISPLAY_SIZE - 1)
                {
                    rxDisplayIndex = 0;
                }
            }
            else if (receivedByte == '\r' || receivedByte == '\n')
            {
                // 换行符,清空显示缓冲区
                rxDisplayIndex = 0;
                rxDisplayBuffer[0] = '\0';
            }
            
            // 更新屏幕显示
            ips_show_string(10, 80, "                    "); // 清空旧内容
            ips_show_string(10, 80, rxDisplayBuffer);
        }
        
        delay_ms(500);  // 每500ms发送一次
        /* 主循环结束 */
    }
}
