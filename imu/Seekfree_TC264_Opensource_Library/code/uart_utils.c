/**
 * uart_utils.c - 自定义串口工具函数实现
 *
 * Seekfree 库只提供 uart_write_string, 不提供整数/浮点输出
 * 这里实现 uart_write_integer 和 uart_write_float
 *
 * 适配 TC264 (TriCore) 平台
 */

#include "common.h"
#include <stdio.h>

/* 整数输出 */
void uart_write_integer(uart_index_enum uart_n, uint32 num) {
    char buf[12];
    int i = 0;

    if (num == 0) {
        buf[i++] = '0';
    } else {
        /* 反转数字 */
        char tmp[12];
        int j = 0;
        while (num > 0) {
            tmp[j++] = '0' + (char)(num % 10);
            num /= 10;
        }
        /* 反转 */
        while (j > 0) {
            buf[i++] = tmp[--j];
        }
    }
    buf[i] = '\0';
    uart_write_string(uart_n, buf);
}

/* 浮点数输出 (保留2位小数) */
void uart_write_float(uart_index_enum uart_n, float num) {
    /* 处理负数 */
    if (num < 0.0f) {
        uart_write_byte(uart_n, '-');
        num = -num;
    }

    /* 整数部分 */
    uint32 int_part = (uint32)num;
    float frac = num - (float)int_part;

    /* 输出整数部分 */
    uart_write_integer(uart_n, int_part);

    /* 小数点 */
    uart_write_byte(uart_n, '.');

    /* 小数部分 (2位) */
    uint32 frac_part = (uint32)(frac * 100.0f + 0.5f);
    if (frac_part < 10) {
        uart_write_byte(uart_n, '0');
    }
    uart_write_integer(uart_n, frac_part);
}
