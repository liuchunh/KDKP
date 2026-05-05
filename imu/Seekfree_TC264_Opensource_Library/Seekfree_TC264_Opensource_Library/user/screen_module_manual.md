# 点阵屏串口控制模块 使用手册

## 一、模块概述

本模块封装了 TLD7002 点阵屏的 6 种车灯显示模式，通过 UART 串口接收命令码实现灯光切换。

**所属文件：**
- `code/screen.h` — 头文件（命令码定义 + 函数声明）
- `code/screen.c` — 实现文件（串口收发、命令解析、屏幕控制）

**依赖：**
- TC264 开源库（zf_common_headfile.h）
- 点阵屏驱动（zf_device_dot_matrix_screen.h / .c）
- TLD7002 驱动（zf_device_tld7002.h / .c）
- 中断配置（isr_config.h）

---

## 二、硬件接线

| TLD7002驱动模块 | 单片机管脚 |
|-----------------|-----------|
| SR0 ~ SR6 | 查看 zf_device_dot_matrix_screen.h 中对应宏定义 |
| SYNC | 查看 zf_device_dot_matrix_screen.h 中 DOT_MATRIX_SCREEN_SYNC_PIN |
| RX | 查看 zf_device_tld7002.h 中 TLD7002_UART_RX |
| HSLIL | 查看 zf_device_tld7002.h 中 TLD7002_UART_HLSIL |
| HSLIH | 悬空 |
| GPIN0 | 查看 zf_device_tld7002.h 中 TLD7002_GPIN0_PIN |
| GPIN1 | 悬空 |
| VCC | 6-10V 电源 |
| 3V3 | 3.3V 电源 |
| GND | 电源地 |

串口默认使用 UART0（P14_0 TX / P14_1 RX），波特率 115200。

---

## 三、命令码定义

| 宏定义 | 值 | 串口发送 | 功能 |
|--------|------|---------|------|
| SCREEN_CMD_OFF | 0x00 | 文本 `0` 或 HEX `00` | 关闭屏幕 |
| SCREEN_CMD_DOUBLE_FLASH | 0x01 | 文本 `1` 或 HEX `01` | 双闪 |
| SCREEN_CMD_TURN_LEFT | 0x02 | 文本 `2` 或 HEX `02` | 左转灯 |
| SCREEN_CMD_TURN_RIGHT | 0x03 | 文本 `3` 或 HEX `03` | 右转灯 |
| SCREEN_CMD_LOW_BEAM | 0x04 | 文本 `4` 或 HEX `04` | 近光灯 |
| SCREEN_CMD_HIGH_BEAM | 0x05 | 文本 `5` 或 HEX `05` | 远光灯 |
| SCREEN_CMD_FOG_LIGHT | 0x06 | 文本 `6` 或 HEX `06` | 雾灯 |

---

## 四、API 函数说明

### 4.1 screen_init()

```c
void screen_init(void);
```

**功能：** 初始化屏幕模块，包含以下操作：
- 初始化 64 字节 FIFO 环形缓冲区
- 初始化 UART0（115200 波特率）并开启接收中断
- 初始化 TLD7002 点阵屏驱动
- 串口输出 "Screen Ready." 提示信息

**调用时机：** 在 `clock_init()` 和 `debug_init()` 之后、`cpu_wait_event_ready()` 之前调用一次。

---

### 4.2 screen_poll()

```c
void screen_poll(void);
```

**功能：** 轮询 FIFO 缓冲区，读取串口接收到的命令字节，解析并控制屏幕显示。

**工作流程：**
1. 检查 FIFO 中是否有数据
2. 有数据则批量读出
3. 逐字节解析命令码（同时支持文本和 HEX 两种格式）
4. 调用底层驱动函数切换灯光图案
5. 串口回显当前模式 "Mode set: X"

**调用时机：** 在主循环 `while(TRUE)` 中周期性调用，建议间隔 10ms。

---

### 4.3 uart_rx_interrupt_handler()

```c
void uart_rx_interrupt_handler(void);
```

**功能：** 从 UART 硬件寄存器读取 1 字节数据并写入 FIFO 缓冲区。

**调用时机：** 由 `isr.c` 中的 `uart0_rx_isr` 中断自动调用，**用户不需要手动调用此函数**。

**注意：** 此函数必须在 `debug_interrupr_handler()` 之前调用，否则数据会被 debug 模块先读走。

---

### 4.4 screen_get_mode()

```c
uint8 screen_get_mode(void);
```

**功能：** 返回当前灯光模式的命令码（0x00 ~ 0x06）。

**使用场景：** 当其他模块需要知道当前屏幕显示的是哪种灯光模式时调用。

---

## 五、快速上手

### 5.1 最小主函数示例

```c
#include "zf_common_headfile.h"
#include "screen.h"

void pit_interrupt_handler(void) {}

int core0_main(void)
{
    clock_init();
    debug_init();
    screen_init();                  // 初始化屏幕模块

    cpu_wait_event_ready();

    while (TRUE)
    {
        screen_poll();              // 轮询串口命令
        system_delay_ms(10);
    }
}
```

### 5.2 isr.c 中断配置（必须）

确保 `isr.c` 中的 `uart0_rx_isr` 包含以下调用：

```c
#include "screen.h"

IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    uart_rx_interrupt_handler();        // 必须在 debug_handler 之前

#if DEBUG_UART_USE_INTERRUPT
    debug_interrupr_handler();
#endif
}
```

### 5.3 串口助手配置

- **波特率：** 115200
- **数据位：** 8
- **停止位：** 1
- **校验位：** 无
- **发送模式：** 文本模式（直接输入数字 0-6）或 HEX 模式（发送 00-06）

---

## 六、数据流

```
串口助手输入 '3'
    → UART0 硬件收到 0x33（ASCII '3'）
    → 触发 uart0_rx_isr 中断
    → uart_rx_interrupt_handler() 读取字节写入 FIFO
    → 主循环 screen_poll() 读出 FIFO 数据
    → 解析 '3' → cmd = 3 → 右转灯
    → dot_matrix_screen_set_brightness(5000)
    → dot_matrix_screen_show_led_pattern(右转图案)
    → SYNC 外部中断驱动扫描刷新 → 屏幕显示右转图案
    → 串口回显 "Mode set: 3"
```

---

## 七、常见问题

### Q1: 串口助手收不到 "Screen Ready."
- 检查串口波特率是否为 115200
- 检查 COM 口是否选对
- 检查 USB 线连接，尝试重新上电或按复位键

### Q2: 串口回显正确但屏幕不亮
- 检查 TLD7002 模块供电（VCC 需要 6-10V）
- 检查 3V3 供电（同步电路供电）
- 检查 SR0-SR6 和 SYNC 接线是否正确

### Q3: 发送命令后显示 "Invalid cmd."
- 文本模式下只能发送单个字符 `0` 到 `6`
- 不要发送 "01" 这样的多字符文本，直接发送 `1` 即可
- HEX 模式下发送 `00` 到 `06`

### Q4: 编译报 uart0_rx_isr 重复定义
- 确保 `cpu0_main.c` 中没有定义 `uart0_rx_isr`
- 该中断只在 `isr.c` 中定义，通过调用 `uart_rx_interrupt_handler()` 来处理数据
