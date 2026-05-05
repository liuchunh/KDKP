#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "zf_common_headfile.h"
#include "zf_device_dot_matrix_screen.h"

// ===================== CI1302 协议 CMD_ID 定义 =====================
// 系统类 (ID 1~3)
#define CI_CMD_WAKEUP               (0x01)  // 唤醒词
#define CI_CMD_WELCOME              (0x02)  // 欢迎语
#define CI_CMD_SLEEP                (0x03)  // 休息语

// 灯光类 (ID 4~10)
#define CI_CMD_TURN_LEFT            (0x04)  // 打开左转向灯
#define CI_CMD_TURN_RIGHT           (0x05)  // 打开右转向灯
#define CI_CMD_HIGH_BEAM            (0x06)  // 打开远光灯
#define CI_CMD_LOW_BEAM             (0x07)  // 打开近光灯
#define CI_CMD_FOG_LIGHT            (0x08)  // 打开雾灯
#define CI_CMD_DOUBLE_FLASH         (0x09)  // 打开双闪灯
#define CI_CMD_INTERIOR_LIGHT       (0x0A)  // 打开车内照明灯

// 设备控制类 (ID 11)
#define CI_CMD_WIPER                (0x0B)  // 打开雨刷器

// 鸣笛类 (ID 12~20)
#define CI_CMD_HORN_1S              (0x0C)  // 鸣笛1秒钟
#define CI_CMD_HORN_2S              (0x0D)  // 鸣笛两秒钟
#define CI_CMD_HORN_3S              (0x0E)  // 鸣笛三秒钟
#define CI_CMD_HORN_2X              (0x0F)  // 鸣笛两声
#define CI_CMD_HORN_3X              (0x10)  // 鸣笛三声
#define CI_CMD_HORN_4X              (0x11)  // 鸣笛四声
#define CI_CMD_HORN_LONG_SHORT      (0x12)  // 长短鸣笛
#define CI_CMD_HORN_RAPID           (0x13)  // 急促鸣笛
#define CI_CMD_HORN_ALARM           (0x14)  // 警报鸣笛

// 通过门洞类 (ID 21~25)
#define CI_CMD_GATE1_LEFT           (0x15)  // 通过门洞1左侧
#define CI_CMD_GATE1                (0x16)  // 通过门洞1
#define CI_CMD_GATE2                (0x17)  // 通过门洞2
#define CI_CMD_GATE3                (0x18)  // 通过门洞3
#define CI_CMD_GATE3_RIGHT          (0x19)  // 通过门洞3右侧

// 门洞返回类 (ID 26~30)
#define CI_CMD_GATE1_RIGHT_BACK     (0x1A)  // 门洞1右侧返回
#define CI_CMD_GATE1_BACK           (0x1B)  // 门洞1返回
#define CI_CMD_GATE2_BACK           (0x1C)  // 门洞2返回
#define CI_CMD_GATE3_BACK           (0x1D)  // 门洞3返回
#define CI_CMD_GATE3_LEFT_BACK      (0x1E)  // 门洞3左侧返回

// 行驶控制类 (ID 31~38)
#define CI_CMD_FORWARD_10M          (0x1F)  // 前行10米
#define CI_CMD_BACKWARD_10M         (0x20)  // 后退10米
#define CI_CMD_SNAKE_FORWARD        (0x21)  // 蛇形前进10米
#define CI_CMD_SNAKE_BACKWARD       (0x22)  // 蛇形后退10米
#define CI_CMD_CCW_CIRCLE           (0x23)  // 逆时针转一圈
#define CI_CMD_CW_CIRCLE            (0x24)  // 顺时针转一圈
#define CI_CMD_TURN_LEFT_DRIVE      (0x25)  // 左转
#define CI_CMD_TURN_RIGHT_DRIVE     (0x26)  // 右转

#define CI_CMD_MAX                  (0x26)

// 初始化屏幕模块（FIFO + 点阵屏 + 自检），在 debug_init() 之后调用
void screen_init(void);

// 主循环中调用，解析 CI1302 协议帧并控制屏幕
void screen_poll(void);

// 串口接收中断处理（在 isr.c 的 uart0_rx_isr 中调用）
void screen_uart_rx_handler(void);

// 获取最近一次收到的 CI1302 命令 ID
uint8 screen_get_last_cmd(void);

#endif
