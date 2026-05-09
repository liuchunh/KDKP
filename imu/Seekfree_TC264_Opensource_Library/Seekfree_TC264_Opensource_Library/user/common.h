/*********************************************************************************************************************
 * 文件名称: common.h
 * 功能描述: 公共类型定义和头文件包含 - 桥接 Seekfree 库
 *
 * 本文件是 kalman_filter、pid_controller 等模块的公共依赖
 * 将 Seekfree 库的基本类型 (uint8, uint16, float 等) 暴露给自定义模块
 *
 * 为什么需要这个文件?
 *   kalman_filter.h 和 pid_controller.h 是独立模块, 不直接依赖 Seekfree 库
 *   通过 common.h 间接引入类型定义, 保持模块的独立性
 ********************************************************************************************************************/

#ifndef COMMON_H_
#define COMMON_H_

/* ---- 引入 Seekfree 库 (包含所有基本类型定义) ---- */
#include "zf_common_headfile.h"

/* ---- 数学常量 (部分编译器的 math.h 中未定义) ---- */
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#ifndef M_PI_F
#define M_PI_F          3.1415926535898f
#endif

#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880
#endif

/* ---- 布尔类型 (C89 没有 bool) ---- */
#ifndef TRUE
#define TRUE            (1)
#endif

#ifndef FALSE
#define FALSE           (0)
#endif

/* ---- 自定义串口工具函数 (Seekfree 库未提供) ---- */
#include "zf_driver_uart.h"

/* 通过串口输出整数 */
void uart_write_integer(uart_index_enum uart_n, uint32 num);

/* 通过串口输出浮点数 (保留2位小数) */
void uart_write_float(uart_index_enum uart_n, float num);

#endif /* COMMON_H_ */
