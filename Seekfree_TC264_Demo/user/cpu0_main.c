#include "zf_common_headfile.h"
#include "angle_control.h"
#pragma section all "cpu0_dsram"

// 串口接收角度输入，返回1表示收到新角度，0表示无输入
uint8 serial_parse_angle(int32 *target_angle)
{
    static char rx_buf[16];
    static uint8 rx_index = 0;
    uint8 buf[16];
    uint32 len = debug_read_ring_buffer(buf, sizeof(buf));

    for (uint32 i = 0; i < len; i++) {
        uint8 ch = buf[i];
        if (ch == '\r' || ch == '\n') {
            if (rx_index > 0) {
                rx_buf[rx_index] = '\0';
                *target_angle = atoi(rx_buf);
                rx_index = 0;
                return 1;
            }
        } else if (rx_index < sizeof(rx_buf) - 1) {
            rx_buf[rx_index++] = ch;
        }
    }
    return 0;
}

int core0_main(void)
{
    clock_init();
    debug_init();

    angle_control_init();
    angle_control_set_target(0);

    pit_init(CCU60_CH0, 10000);  // 10ms 定时中断

    cpu_wait_event_ready();

    uart_write_string(DEBUG_UART_INDEX, "Enter angle:\r\n");

    uint32 last_print = 0;

    while (TRUE)
    {
        // 串口接收角度
        int32 target;
        if (serial_parse_angle(&target)) {
            angle_control_set_target(target);
            char msg[48];
            sprintf(msg, "Target=%ld\r\n", (long)target);
            uart_write_string(DEBUG_UART_INDEX, msg);
        }

        // 每100ms输出当前状态
        if (system_getval_ms() - last_print >= 100) {
            last_print = system_getval_ms();
            char buf[48];
            sprintf(buf, "angle=%d\r\n", (int)angle_ctrl.current_angle);
            uart_write_string(DEBUG_UART_INDEX, buf);
        }

        system_delay_ms(10);
    }
}
