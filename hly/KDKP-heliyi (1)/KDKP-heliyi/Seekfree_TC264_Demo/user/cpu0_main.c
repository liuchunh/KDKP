#include "zf_common_headfile.h"
#include "angle_control.h"
#pragma section all "cpu0_dsram"

static unsigned long loop_tick = 0;

int core0_main(void)
{
    clock_init();
    debug_init();

    angle_control_init();
    angle_control_set_target(180);

    cpu_wait_event_ready();

    // CSV header
    uart_write_string(DEBUG_UART_INDEX, "timestamp_ms,setpoint,input,pwm,error,p,i,d,speed_rpm\r\n");

    while (TRUE)
    {
        angle_control_update();

        // CSV data: timestamp_ms,setpoint,input,pwm,error,p,i,d,speed_rpm
        char buf[160];
        sprintf(buf, "%lu,%.2f,%.2f,%ld,%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
            (unsigned long)(loop_tick * 100),
            (double)angle_ctrl.target_angle,
            (double)angle_ctrl.current_angle,
            (long)angle_ctrl.pwm_output,
            (double)angle_ctrl.pid.Error,
            (double)angle_ctrl.pid.Kp,
            (double)angle_ctrl.pid.Ki,
            (double)angle_ctrl.pid.Kd,
            (double)angle_ctrl.speed_rpm);
        uart_write_string(DEBUG_UART_INDEX, buf);

        loop_tick++;
        system_delay_ms(100);
    }
}
