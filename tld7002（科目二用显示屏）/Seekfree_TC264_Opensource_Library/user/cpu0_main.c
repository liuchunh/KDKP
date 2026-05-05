/*********************************************************************************************************************
* TC264 Opensourec Library （TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
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
* 开发环境          ADS v1.9.4
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#include "servo.h"
#include "screen.h"
#pragma section all "cpu0_dsram"

// **************************** 代码区域 ****************************

// 数字1-6对应的图案表
static const unsigned char pattern_table[6] =
{
    DOT_MATRIX_PATTERN_DOUBLE_FLASH,    // 1 - 双闪
    DOT_MATRIX_PATTERN_TURN_LEFT,       // 2 - 左转
    DOT_MATRIX_PATTERN_TURN_RIGHT,      // 3 - 右转
    DOT_MATRIX_PATTERN_LOW_BEAM,        // 4 - 近光
    DOT_MATRIX_PATTERN_HIGH_BEAM,       // 5 - 远光
    DOT_MATRIX_PATTERN_FOG_LIGHT        // 6 - 雾灯
};

// 当前运行模式：0=空闲，1-6=图案，7=舵机（串口），8=舵机（语音雨刷）
static unsigned char current_mode = 0;
static unsigned char last_voice_cmd = 0;   // 上一次语音命令，用于变化检测

//-------------------------------------------------------------------------------------------------------------------
// 停止所有功能，回到空闲状态
//-------------------------------------------------------------------------------------------------------------------
static void stop_all(void)
{
    if(current_mode >= 1 && current_mode <= 6)
    {
        dot_matrix_screen_set_brightness(0);
        dot_matrix_screen_clear_pattern();
    }
    current_mode = 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 根据 CI1302 语音命令触发对应功能（互斥）
//-------------------------------------------------------------------------------------------------------------------
static void handle_voice_cmd(unsigned char cmd_id)
{
    switch(cmd_id)
    {
        case CI_CMD_TURN_LEFT:
            stop_all();
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_TURN_LEFT);
            current_mode = 2;
            break;

        case CI_CMD_TURN_RIGHT:
            stop_all();
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_TURN_RIGHT);
            current_mode = 3;
            break;

        case CI_CMD_HIGH_BEAM:
            stop_all();
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_HIGH_BEAM);
            current_mode = 5;
            break;

        case CI_CMD_LOW_BEAM:
            stop_all();
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_LOW_BEAM);
            current_mode = 4;
            break;

        case CI_CMD_FOG_LIGHT:
            stop_all();
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_FOG_LIGHT);
            current_mode = 6;
            break;

        case CI_CMD_DOUBLE_FLASH:
            stop_all();
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_DOUBLE_FLASH);
            current_mode = 1;
            break;

        case CI_CMD_WIPER:
            stop_all();
            current_mode = 8;
            uart_write_string(DEBUG_UART_INDEX, "Wiper Servo START\r\n");
            break;

        default:
            break;
    }
}

void pit_interrupt_handler(void)
{
}

int core0_main(void)
{
    uint8 recv_data;
    uint8 idx;
    uint8 voice_cmd;

    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    screen_init();                  // 屏幕初始化

    servo_init();                   // 舵机初始化

    uart_write_string(DEBUG_UART_INDEX, "Cmd: 1-6=pattern, 7=servo, 0=stop\r\n");

    cpu_wait_event_ready();         // 等待所有核心初始化完毕

    while (TRUE)
    {
        screen_poll();              // 屏幕协议解析（CI1302语音）

        // 检测语音命令变化
        voice_cmd = screen_get_last_cmd();
        if(voice_cmd != last_voice_cmd)
        {
            last_voice_cmd = voice_cmd;
            handle_voice_cmd(voice_cmd);
        }

        // 检测串口命令
        if(debug_read_ring_buffer(&recv_data, 1))
        {
            // 数字 1-6：显示对应图案
            if(recv_data >= '1' && recv_data <= '6')
            {
                stop_all();
                idx = recv_data - '1';
                dot_matrix_screen_set_brightness(5000);
                dot_matrix_screen_show_led_pattern(pattern_table[idx]);
                current_mode = recv_data - '0';
                uart_write_string(DEBUG_UART_INDEX, "Pattern ");
                uart_write_byte(DEBUG_UART_INDEX, recv_data);
                uart_write_string(DEBUG_UART_INDEX, "\r\n");
            }
            // 数字 7：舵机开始转动
            else if(recv_data == '7')
            {
                stop_all();
                current_mode = 7;
                uart_write_string(DEBUG_UART_INDEX, "Servo START\r\n");
            }
            // 数字 0：停止所有
            else if(recv_data == '0')
            {
                stop_all();
                uart_write_string(DEBUG_UART_INDEX, "STOP ALL\r\n");
            }
        }

        // 舵机运行时执行摆动（串口7 或 语音雨刷8）
        if(current_mode == 7 || current_mode == 8)
        {
            servo_sweep();
        }

        system_delay_ms(10);
    }
}

#pragma section all restore
