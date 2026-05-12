#include "zf_common_headfile.h"
#include "angle_control.h"
#pragma section all "cpu0_dsram"

int core0_main(void)
{
    clock_init();
    debug_init();

    angle_control_init();
    angle_control_set_target(180);

    cpu_wait_event_ready();

    while (TRUE)
    {
        angle_control_update();

        // 串口输出编码器计数和当前角度
        int16 count = encoder_get_count(ANGLE_ENCODER);
        char buf[64];
        sprintf(buf, "count=%d angle=%d\r\n", (int)count, (int)angle_ctrl.current_angle);
        uart_write_string(DEBUG_UART_INDEX, buf);

        system_delay_ms(100);
    }
}
