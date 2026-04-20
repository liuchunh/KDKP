/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          cpu0_main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.10.2
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#include "zf_device_gnss.h"
#include <stdbool.h>
#include "ComOutput.h"
#include "setPoint.h"

#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设

// typedef unsigned short uint16;
//
//#define KEY1                    (P20_6)
//#define KEY2                    (P20_7)
//#define KEY3                    (P11_2)
//#define KEY4                    (P11_3)

void IPS200_Show_Init(void)
{
    ips200_set_color(RGB565_BLACK, RGB565_WHITE);    //设置为白底黑字
    ips200_set_font(IPS200_6X8_FONT);    //设置字体大小为 6 * 8
    ips200_set_dir(IPS200_PORTAIT);    //设置显示方向为竖向显示
    ips200_init(IPS200_TYPE_SPI);    //设置通信模式为SPI通信
}

#define MAX_POINTS 30

GnssData Point[MAX_POINTS];
int index = 0;

// **********************+****** 代码区域 ****************************
int core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    // 此处编写用户代码

     gnss_init(GN42A);
    //    bool parse = false;
    //    ips200_init(IPS200_TYPE_PARALLEL8);
    //
    //    ips200_clear();
    //     IPS200_Show_Init();
    uint16 count = 0;

    gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY1 输入 默认高电平 上拉输入
    gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY2 输入 默认高电平 上拉输入
    gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY3 输入 默认高电平 上拉输入
    gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY4 输入 默认高电平 上拉输入
    
    // 此处编写用户代码 例如外设初始化代码等
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    int index = 0;
    while (TRUE)
    {
        // 此处编写需要循环执行的代码
//        if (index > 150)
//            index = 0;
//        index ++;
//        uart_write_printf(DEBUG_UART_INDEX, "Current Index = %d, Great!\n", index);
//        system_delay_ms(500);

        // TODO: 消抖
        if ((ButtonPushed(KEY1) || ButtonPushed(KEY2) || ButtonPushed(KEY3) || ButtonPushed(KEY4))){
            // 按下后 取 20 个数据 取平均
            SetPoint(Point);

            system_delay_ms(500);
        }
        system_delay_ms(20);

//        if (gnss_flag){
//            gnss_flag = 0;
//            gnss_data_parse();
//        }
//        else{
//            uart_write_string(DEBUG_UART_INDEX, "GPS data invalid! Point array is disabled...\n");
//            system_delay_ms(500);
//            continue;
//        }
//

//        uart_write_string(DEBUG_UART_INDEX, "I am still working!\n");
//        system_delay_ms(50);

//        if (!gpio_get_level(KEY1)){
//            // P20_6 被按下
//            if (index >= MAX_POINTS){
//                index = 0;
//                uart_write_string(DEBUG_UART_INDEX, "Point array overflow! Current count: ");
//                uart_write_integer(DEBUG_UART_INDEX, index);
//                uart_write_string(DEBUG_UART_INDEX, "\n");
//            }
//            Point[index].longitude = gnss.longitude;
//            Point[index].latitude = gnss.latitude;
//            Point[index].speed = gnss.speed;
//            Point[index].direction = gnss.direction;
//            index ++;
//        }


//         if (gnss_flag) {
//             gnss_flag = 0;
//             count = 0;
//             gnss_data_parse();
//
//             uart_write_float(DEBUG_UART_INDEX, gnss.direction);
//             uart_write_string(DEBUG_UART_INDEX, "Direction Got!\n");
//         }
//         else {
//             count++;
//             if (count % 10 == 0) {
//                 uart_write_string(DEBUG_UART_INDEX, "Current Count: ");
//                 uart_write_integer(DEBUG_UART_INDEX, count);
//                 uart_write_string(DEBUG_UART_INDEX, ", Wait for data...\n");
//             }
//         }

        // 此处编写需要循环执行的代码
    }
}

#pragma section all restore
// **************************** 代码区域 ****************************
