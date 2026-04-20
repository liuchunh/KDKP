/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的显示设备演示 - IPS114屏幕例程
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
 * 文件功能：IPS114 1.14寸屏幕例程 - 显示文字、图形、图像
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
 * 2026年01月10日      毛毛    V3.1     增加IPS114屏幕例程
 *
 * === 代码说明 ===
 * 1. 硬件连接：
 *    - IPS114屏幕: 1.14寸 SPI接口 (分辨率:135x240)
 *    - SPI引脚: MOSI:P15_5, SCLK:P15_3, CS:P15_2, DC:P15_0, RST:P15_1, BLK:P15_4
 * 2. 显示功能演示：
 *    - 文字显示: 中英文字符、整数显示
 *    - 图形绘制: 直线、矩形填充
 *    - 颜色展示: 红绿蓝黄青紫六色
 * 3. 实验现象：
 *    - 屏幕显示动态计数器
 *    - 彩色线条和矩形图形
 *    - Hello World欢迎信息
 */

#include "ky_all.h"
#include "ky_ips.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

// 简单的测试图案数据
const uint8_t test_image[16] = {
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
};

int counter = 0;

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    uart_debug_init();
    /* 初始化开始 */

    ips_init(IPS_TYPE_114);  // IPS114屏幕初始化
    ips_clear(BLACK);

    printf("IPS114屏幕例程\r\n");

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    while(1)
    {
        /* 主循环开始 */

        // 显示标题
        ips_set_color(WHITE, BLACK);
        ips_show_string(5, 5, "IPS114 Test");

        // 显示计数器
        ips_show_string(5, 25, "Count:");
        ips_show_int(50, 25, counter, 5);

        // 绘制图形演示
        ips_draw_line(5, 45, 100, 45, RED);           // 红色横线
        ips_draw_line(5, 55, 100, 55, GREEN);         // 绿色横线
        ips_draw_line(5, 65, 100, 65, BLUE);          // 蓝色横线

        ips_fill_rectangle(5, 75, 20, 90, YELLOW);    // 黄色矩形
        ips_fill_rectangle(25, 75, 40, 90, CYAN);     // 青色矩形
        ips_fill_rectangle(45, 75, 60, 90, MAGENTA);  // 紫色矩形

        // 显示一些文本
        ips_show_string(5, 95, "Hello World!");
        ips_show_string(5, 105, "Keyu TC264DA");

        counter++;
        delay_ms(500);

        /* 主循环结束 */
    }
}
