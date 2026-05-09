#include "zf_common_headfile.h"
#include "motor.h"
#include "encoder.h"
#include "common.h"

#define DUTY_STEP   (100)

void motor_encoder_test(void)
{
    motor1_init();
    motor2_init();
    encoder1_init();
    encoder2_init();

    int16 duty = 0;
    motor1_set_duty(duty);
    motor2_set_duty(duty);

    uart_write_string(DEBUG_UART_INDEX,
        "\r\n==== Dual Motor Manual ====\r\n"
        "M1: P21_2/P21_3  M2: P21_4/P21_5\r\n"
        "+ = +100  - = -100  s = stop\r\n\r\n");

    uint32 tick = 0;
    uint8 rx_buf[32];
    while (1)
    {
        /* 串口接收 */
        uint32 len = debug_read_ring_buffer(rx_buf, sizeof(rx_buf));
        for (uint32 i = 0; i < len; i++)
        {
            uint8 ch = rx_buf[i];
            if (ch == '+' || ch == '=')
            {
                duty += DUTY_STEP;
                if (duty > (int16)MOTOR_MAX_DUTY) duty = (int16)MOTOR_MAX_DUTY;
            }
            else if (ch == '-' || ch == '_')
            {
                duty -= DUTY_STEP;
                if (duty < -(int16)MOTOR_MAX_DUTY) duty = -(int16)MOTOR_MAX_DUTY;
            }
            else if (ch == 's' || ch == 'S')
            {
                duty = 0;
            }
            motor1_set_duty(duty);
            motor2_set_duty(duty);

            uart_write_string(DEBUG_UART_INDEX, "Duty=");
            if (duty < 0) {
                uart_write_byte(DEBUG_UART_INDEX, '-');
                uart_write_integer(DEBUG_UART_INDEX, (uint32)(-duty));
            } else {
                uart_write_integer(DEBUG_UART_INDEX, (uint32)duty);
            }
            uart_write_string(DEBUG_UART_INDEX, "\r\n");
        }

        system_delay_ms(ENCODER_SAMPLE_MS);
        encoder1_update();
        encoder2_update();

        tick++;
        if (tick % 5 == 0)
        {
            int16 spd1 = encoder1_get_speed();
            int16 spd2 = encoder2_get_speed();
            float rpm1 = encoder1_get_rpm();
            float rpm2 = encoder2_get_rpm();

            uart_write_string(DEBUG_UART_INDEX, "D=");
            uart_write_integer(DEBUG_UART_INDEX, (uint32)(duty < 0 ? -duty : duty));
            uart_write_string(DEBUG_UART_INDEX, " M1:spd=");
            if (spd1 < 0) { uart_write_byte(DEBUG_UART_INDEX, '-'); spd1 = -spd1; }
            uart_write_integer(DEBUG_UART_INDEX, (uint32)spd1);
            uart_write_string(DEBUG_UART_INDEX, " rpm=");
            uart_write_float(DEBUG_UART_INDEX, rpm1);
            uart_write_string(DEBUG_UART_INDEX, " M2:spd=");
            if (spd2 < 0) { uart_write_byte(DEBUG_UART_INDEX, '-'); spd2 = -spd2; }
            uart_write_integer(DEBUG_UART_INDEX, (uint32)spd2);
            uart_write_string(DEBUG_UART_INDEX, " rpm=");
            uart_write_float(DEBUG_UART_INDEX, rpm2);
            uart_write_string(DEBUG_UART_INDEX, "\r\n");
        }
    }
}
