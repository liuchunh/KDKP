/*************************************************************
 * 示例: 电机PID控制 (可直接复制到比赛工程)
 *************************************************************/

#include "zf_common_headfile.h"
#include "code/motor_pid.h"
#include "code/encoder.h"
#include "code/motor.h"

#pragma section all "cpu0_dsram"

/* PID 控制器实例 */
MotorPID g_motor_pid;

/* 编码器累计脉冲数 */
static int32 spd_100ms = 0;

/* 编码器更新标志 (由PIT中断设置) */
static volatile uint8 g_encoder_tick = 0;

/* 编码器PIT中断 (10ms) */
void encoder_pit_isr(void)
{
    encoder_irq_update();
    g_encoder_tick = 1;
}

int core0_main(void)
{
    clock_init();
    debug_init();

    /* 电机初始化 */
    motor1_init();
    motor2_init();
    motor1_stop();
    motor2_stop();

    /* 编码器初始化 */
    encoder1_init();
    encoder2_init();

    /* 编码器PIT中断 (10ms) */
    pit_ms_init(CCU61_CH0, 10);

    /* PID初始化 (空载参数) */
    motor_pid_init(&g_motor_pid, 8.0f, 0.5f, 0.2f, 10.0f);

    /* 等待多核同步 */
    cpu_wait_event_ready();

    /* 打印信息 */
    uart_write_string(DEBUG_UART_INDEX, "\r\n=== Motor PID Example ===\r\n");

    uint32 tick = 0;
    char buf[128];

    /* 主循环 */
    while (1)
    {
        /* 等待编码器更新 */
        if (g_encoder_tick) {
            g_encoder_tick = 0;

            /* 累加10ms脉冲 */
            spd_100ms += encoder1_get_speed();
            tick++;

            /* 每100ms执行PID */
            if (tick % 10 == 0)
            {
                /* 设置目标速度 (250脉冲/100ms = 0.23 m/s) */
                motor_pid_set_target(&g_motor_pid, 250.0f);

                /* 执行PID计算 */
                int16 pwm = motor_pid_update(&g_motor_pid, spd_100ms);

                /* 输出到电机 */
                motor1_set_duty(pwm);
                motor2_set_duty(pwm);

                /* 打印状态 */
                float spd_mps = pulses_to_mps(spd_100ms);
                float spd_kmh = mps_to_kmh(spd_mps);
                sprintf(buf, "spd=%d(%.2fm/s %.1fkm/h) pwm=%d\r\n",
                        (int)spd_100ms, spd_mps, spd_kmh, (int)pwm);
                uart_write_string(DEBUG_UART_INDEX, buf);

                /* 清零累计 */
                spd_100ms = 0;
            }
        }

        /* 其他任务... */
        system_delay_ms(1);
    }
}
