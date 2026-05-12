/*
 * motor_test_firmware.c
 *
 * 转向电机硬件验证固件 (无 PID, 纯手动驱动)
 * 用于排查: 电机能不能转? 编码器有没有数?
 *
 * 测试流程:
 *   1. 上电后电机正转 30% 占空比, 持续 2 秒
 *   2. 停 1 秒
 *   3. 电机反转 30% 占空比, 持续 2 秒
 *   4. 停 1 秒
 *   5. 循环
 *
 * 串口每 100ms 输出一行:
 *   pwm=<当前PWM>  enc=<编码器原始计数>  angle=<角度>
 *
 * 用法: 替换 cpu0_main.c 中的 core0_main()
 */

#include "zf_common_headfile.h"
#include "code/angle_control.h"

/* PWM 和编码器引脚 (与 angle_control.h 一致) */
#define TEST_PWM_IN1    ATOM0_CH7_P02_7
#define TEST_PWM_IN2    ATOM0_CH6_P02_6
#define TEST_ENCODER    TIM4_ENCODER
#define TEST_ENC_A      TIM4_ENCODER_CH1_P02_8
#define TEST_ENC_B      TIM4_ENCODER_CH2_P00_9

#define PWM_FREQ        10000
#define PWM_DUTY        3000        /* 30% 占空比 (满量程 10000) */

int core0_main(void)
{
    clock_init();
    debug_init();

    /* ---- 初始化 PWM (两个通道, 初始停) ---- */
    pwm_init(TEST_PWM_IN1, PWM_FREQ, 0);
    pwm_init(TEST_PWM_IN2, PWM_FREQ, 0);

    /* ---- 初始化编码器 ---- */
    encoder_quad_init(TEST_ENCODER, TEST_ENC_A, TEST_ENC_B);

    /* ---- 等待多核同步 ---- */
    cpu_wait_event_ready();

    uart_write_string(DEBUG_UART_INDEX, "=== Motor Test Start ===\r\n");
    uart_write_string(DEBUG_UART_INDEX, "Expect: fwd 2s -> stop 1s -> rev 2s -> stop 1s -> loop\r\n");
    uart_write_string(DEBUG_UART_INDEX, "Format: pwm=<duty> enc=<raw_count> angle=<deg>\r\n\r\n");

    uint32 tick = 0;
    char buf[80];

    while (TRUE)
    {
        /* 每 100ms 输出一次状态 */
        int16 enc_raw = encoder_get_count(TEST_ENCODER);
        float angle = (float)enc_raw * (360.0f / (1024.0f * 600.0f));

        /* 阶段判断: 0~20 正转, 20~30 停, 30~50 反转, 50~60 停, 循环 */
        int phase = tick % 60;

        if (phase < 20) {
            /* 正转 */
            pwm_set_duty(TEST_PWM_IN1, PWM_DUTY);
            pwm_set_duty(TEST_PWM_IN2, 0);
            sprintf(buf, "pwm=+%d  enc=%d  angle=%.2f\r\n", PWM_DUTY, enc_raw, angle);
        } else if (phase < 30) {
            /* 停 */
            pwm_set_duty(TEST_PWM_IN1, 0);
            pwm_set_duty(TEST_PWM_IN2, 0);
            sprintf(buf, "pwm=0    enc=%d  angle=%.2f\r\n", enc_raw, angle);
        } else if (phase < 50) {
            /* 反转 */
            pwm_set_duty(TEST_PWM_IN1, 0);
            pwm_set_duty(TEST_PWM_IN2, PWM_DUTY);
            sprintf(buf, "pwm=-%d  enc=%d  angle=%.2f\r\n", PWM_DUTY, enc_raw, angle);
        } else {
            /* 停 */
            pwm_set_duty(TEST_PWM_IN1, 0);
            pwm_set_duty(TEST_PWM_IN2, 0);
            sprintf(buf, "pwm=0    enc=%d  angle=%.2f\r\n", enc_raw, angle);
        }

        uart_write_string(DEBUG_UART_INDEX, buf);

        tick++;
        system_delay_ms(100);
    }
}
