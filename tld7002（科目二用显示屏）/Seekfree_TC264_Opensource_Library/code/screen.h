#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "zf_common_headfile.h"
#include "zf_device_dot_matrix_screen.h"

// 灯光命令码定义
#define SCREEN_CMD_OFF              (0)     // 关闭屏幕
#define SCREEN_CMD_DOUBLE_FLASH     (1)     // 双闪
#define SCREEN_CMD_TURN_LEFT        (2)     // 左转
#define SCREEN_CMD_TURN_RIGHT       (3)     // 右转
#define SCREEN_CMD_LOW_BEAM         (4)     // 近光灯
#define SCREEN_CMD_HIGH_BEAM        (5)     // 远光灯
#define SCREEN_CMD_FOG_LIGHT        (6)     // 雾灯
#define SCREEN_CMD_MAX              (6)

// 初始化屏幕模块（FIFO + 点阵屏 + 自检），在 debug_init() 之后调用
void screen_init(void);

// 主循环中调用，轮询 FIFO 并处理串口命令
void screen_poll(void);

// 串口接收中断处理（在 isr.c 的 uart0_rx_isr 中调用）
void screen_uart_rx_handler(void);

// 获取当前灯光模式命令码
uint8 screen_get_mode(void);

#endif
