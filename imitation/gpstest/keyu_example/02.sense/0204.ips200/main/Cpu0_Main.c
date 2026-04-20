/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的显示设备演示 - IPS200屏幕例程
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
 * 文件功能：IPS200 2.0寸屏幕例程 - 显示文字、图形、图像
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
 * 2026年01月10日      毛毛    V3.1     增加IPS200屏幕例程
 *
 * === 代码说明 ===
 * 1. 硬件连接：
 *    - IPS200屏幕: 2.0寸 SPI接口 (分辨率:320x240)
 *    - SPI引脚: MOSI:P15_5, SCLK:P15_3, CS:P15_2, DC:P15_0, RST:P15_1, BLK:P15_4
 * 2. 显示功能演示：
 *    - 文字显示: 中英文字符、整数、浮点数显示
 *    - 图形绘制: 直线、矩形、点
 *    - 颜色展示: RGB全彩色彩
 * 3. 实验现象：
 *    - 屏幕显示动态计数器和浮点数
 *    - 彩色线条和矩形图形演示
 *    - RGB颜色测试显示
 */

#include "ky_all.h"
#include "ky_ips.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

int counter = 0;
float test_float = 123.456f;

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    uart_debug_init();
    /* 初始化开始 */

    ips_init(IPS_TYPE_200);  // IPS200屏幕初始化
    ips_clear(BLACK);

    printf("IPS200屏幕例程\r\n");

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    while(1)
    {
        /* 主循环开始 */

        // 显示标题
        ips_set_color(WHITE, BLACK);
        ips_show_string(10, 10, "IPS200 2.0 Inch Display");
        ips_show_string(10, 30, "Keyu TC264DA Demo");

        // 显示计数器
        ips_show_string(10, 60, "Counter:");
        ips_show_int(80, 60, counter, 5);

        // 显示浮点数
        ips_show_string(10, 80, "Float:");
        ips_show_float(60, 80, test_float, 6, 2);

        // 绘制图形演示
        ips_draw_line(10, 110, 150, 110, RED);          // 红色横线
        ips_draw_line(10, 125, 150, 125, GREEN);        // 绿色横线
        ips_draw_line(10, 140, 150, 140, BLUE);         // 蓝色横线

        ips_fill_rectangle(10, 155, 40, 185, YELLOW);   // 黄色矩形
        ips_fill_rectangle(50, 155, 80, 185, CYAN);     // 青色矩形
        ips_fill_rectangle(90, 155, 120, 185, MAGENTA); // 紫色矩形

        // 画一些点
        for(int i = 0; i < 50; i++)
        {
            ips_draw_point(130 + (i % 20), 155 + (i / 20), WHITE);
        }

        // 显示文本
        ips_show_string(10, 195, "RGB Colors Test:");
        ips_show_string(10, 215, "RED   GREEN  BLUE");

        counter++;
        test_float += 0.001f;
        delay_ms(500);

        /* 主循环结束 */
    }
}
