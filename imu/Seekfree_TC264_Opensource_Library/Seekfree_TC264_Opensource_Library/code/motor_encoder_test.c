#include "zf_common_headfile.h"
#include "motor.h"
#include "encoder.h"
#include "pid_controller.h"
#include "common.h"
#include <string.h>
#include <stdlib.h>

/* ---- PID 参数 (会被上位机远程更新) ---- */
static float g_kp = 50.0f;
static float g_ki = 2.0f;
static float g_kd = 0.0f;
static float g_setpoint = 60.0f;   /* 目标转速 RPM */

/* ---- 串口命令解析缓冲区 ---- */
static char    g_cmd_buf[80];
static uint8_t g_cmd_len = 0;

/* 输出浮点数 (指定小数位) */
static void uart_print_float(uint8 dec, float val)
{
    if (val < 0) { uart_write_byte(DEBUG_UART_INDEX, '-'); val = -val; }
    int32 integer = (int32)val;
    float frac = val - (float)integer;
    uart_write_integer(DEBUG_UART_INDEX, (uint32)integer);
    uart_write_byte(DEBUG_UART_INDEX, '.');
    for (uint8 i = 0; i < dec; i++) {
        frac *= 10.0f;
        int32 digit = (int32)frac;
        uart_write_byte(DEBUG_UART_INDEX, (uint8)('0' + digit));
        frac -= (float)digit;
    }
}

/* 输出一行 CSV: timestamp_ms,setpoint,input,pwm,error,kp,ki,kd */
static void send_csv(uint32 ts, float setpoint, float input, float pwm,
                     float error, float kp, float ki, float kd)
{
    uart_write_integer(DEBUG_UART_INDEX, ts);
    uart_write_byte(DEBUG_UART_INDEX, ',');
    uart_print_float(2, setpoint);
    uart_write_byte(DEBUG_UART_INDEX, ',');
    uart_print_float(2, input);
    uart_write_byte(DEBUG_UART_INDEX, ',');
    uart_print_float(2, pwm);
    uart_write_byte(DEBUG_UART_INDEX, ',');
    uart_print_float(2, error);
    uart_write_byte(DEBUG_UART_INDEX, ',');
    uart_print_float(3, kp);
    uart_write_byte(DEBUG_UART_INDEX, ',');
    uart_print_float(3, ki);
    uart_write_byte(DEBUG_UART_INDEX, ',');
    uart_print_float(3, kd);
    uart_write_string(DEBUG_UART_INDEX, "\r\n");
}

/* 解析上位机发来的 PID 指令 */
static void process_command(void)
{
    g_cmd_buf[g_cmd_len] = '\0';
    g_cmd_len = 0;

    /* 去掉前后空格 */
    char *cmd = g_cmd_buf;
    while (*cmd == ' ' || *cmd == '\t') cmd++;
    if (*cmd == '\0') return;

    if (strncmp(cmd, "SET ", 4) == 0 || strncmp(cmd, "PID", 3) == 0)
    {
        char *params;
        if (cmd[0] == 'S') params = cmd + 4;
        else params = cmd + 3;
        while (*params == ' ') params++;

        float new_kp = g_kp, new_ki = g_ki, new_kd = g_kd;

        if (strstr(params, "P:") != NULL || strstr(params, "KP:") != NULL ||
            strstr(params, "p:") != NULL)
        {
            /* SET P:1.5 I:0.2 D:0.05 格式 */
            char *p = strstr(params, ":");
            /* 找 P: 或 KP: */
            char *p_pos = strstr(params, "P:");
            char *kp_pos = strstr(params, "KP:");
            if (!p_pos) p_pos = strstr(params, "p:");
            if (!kp_pos) kp_pos = strstr(params, "kp:");
            char *k_pos = kp_pos ? kp_pos : p_pos;
            if (k_pos) {
                int idx = (kp_pos != NULL) ? 3 : 2;
                new_kp = (float)atof(k_pos + idx);
            }

            char *i_pos = strstr(params, "I:");
            if (!i_pos) i_pos = strstr(params, "KI:");
            if (!i_pos) i_pos = strstr(params, "i:");
            if (!i_pos) i_pos = strstr(params, "ki:");
            if (i_pos) {
                int idx = (strstr(params, "KI:") == i_pos || strstr(params, "ki:") == i_pos) ? 3 : 2;
                new_ki = (float)atof(i_pos + idx);
            }

            char *d_pos = strstr(params, "D:");
            if (!d_pos) d_pos = strstr(params, "KD:");
            if (!d_pos) d_pos = strstr(params, "d:");
            if (!d_pos) d_pos = strstr(params, "kd:");
            if (d_pos) {
                int idx = (strstr(params, "KD:") == d_pos || strstr(params, "kd:") == d_pos) ? 3 : 2;
                new_kd = (float)atof(d_pos + idx);
            }
        }
        else
        {
            /* PID 1.5 0.2 0.05 纯数字格式 */
            char *s1 = strchr(params, ' ');
            if (s1) {
                char *s2 = strchr(s1 + 1, ' ');
                if (s2) {
                    new_kp = (float)atof(params);
                    new_ki = (float)atof(s1 + 1);
                    new_kd = (float)atof(s2 + 1);
                }
            }
        }

        /* 参数校验 */
        if (new_kp > 0 && new_kp <= 100 &&
            new_ki >= 0 && new_ki <= 50 &&
            new_kd >= 0 && new_kd <= 50)
        {
            g_kp = new_kp;
            g_ki = new_ki;
            g_kd = new_kd;
            uart_write_string(DEBUG_UART_INDEX, "# PID Updated: P=");
            uart_print_float(3, g_kp);
            uart_write_string(DEBUG_UART_INDEX, " I=");
            uart_print_float(3, g_ki);
            uart_write_string(DEBUG_UART_INDEX, " D=");
            uart_print_float(3, g_kd);
            uart_write_string(DEBUG_UART_INDEX, "\r\n");
        }
        else
        {
            uart_write_string(DEBUG_UART_INDEX, "# ERROR: PID params rejected\r\n");
        }
    }
    else if (strncmp(cmd, "SETPOINT:", 9) == 0 || strncmp(cmd, "setpoint:", 9) == 0)
    {
        float new_sp = (float)atof(cmd + 9);
        if (new_sp > 0 && new_sp <= 500) {
            g_setpoint = new_sp;
            uart_write_string(DEBUG_UART_INDEX, "# Setpoint Updated: ");
            uart_print_float(2, g_setpoint);
            uart_write_string(DEBUG_UART_INDEX, "\r\n");
        }
    }
    else if (strncmp(cmd, "STATUS", 6) == 0)
    {
        uart_write_string(DEBUG_UART_INDEX, "# STATUS: P=");
        uart_print_float(3, g_kp);
        uart_write_string(DEBUG_UART_INDEX, " I=");
        uart_print_float(3, g_ki);
        uart_write_string(DEBUG_UART_INDEX, " D=");
        uart_print_float(3, g_kd);
        uart_write_string(DEBUG_UART_INDEX, " SP=");
        uart_print_float(2, g_setpoint);
        uart_write_string(DEBUG_UART_INDEX, "\r\n");
    }
}

void motor_encoder_test(void)
{
    motor1_init();
    motor2_init();
    encoder1_init();
    encoder2_init();

    float dt = (float)ENCODER_SAMPLE_MS / 1000.0f;
    float out_max = (float)MOTOR_MAX_DUTY;

    PidParams pid_params;
    PidState  pid1_state, pid2_state;
    pid_init(&pid_params, g_kp, g_ki, g_kd, dt, 0, out_max, PID_POSITION);
    pid_set_integral_limit(&pid_params, out_max * 0.5f);
    pid_reset(&pid1_state);
    pid_reset(&pid2_state);

    /* 打印头信息 (tuner 会忽略 # 开头的行) */
    uart_write_string(DEBUG_UART_INDEX, "# LLM PID Tuner - TC264 Motor Control\r\n");
    uart_write_string(DEBUG_UART_INDEX, "# Format: timestamp_ms,setpoint,input,pwm,error,kp,ki,kd\r\n");
    uart_write_string(DEBUG_UART_INDEX, "# Commands: SET P:x I:x D:x | PID x x x | SETPOINT:x | STATUS\r\n");

    uint32 tick = 0;
    uint32 ts = 0;
    uint8 rx_buf[128];

    while (1)
    {
        system_delay_ms(ENCODER_SAMPLE_MS);
        encoder1_update();
        encoder2_update();
        ts += ENCODER_SAMPLE_MS;

        /* 接收上位机指令 */
        uint32 len = debug_read_ring_buffer(rx_buf, sizeof(rx_buf));
        for (uint32 i = 0; i < len; i++)
        {
            char c = (char)rx_buf[i];
            if (c == '\r') continue;
            if (c != '\n') {
                if (g_cmd_len < sizeof(g_cmd_buf) - 1) {
                    g_cmd_buf[g_cmd_len++] = c;
                }
                continue;
            }
            /* 收到换行, 处理命令 */
            process_command();
            /* 更新 PID 参数 */
            pid_params.kp = g_kp;
            pid_params.ki = g_ki;
            pid_params.kd = g_kd;
            pid_reset(&pid1_state);
            pid_reset(&pid2_state);
        }

        /* PID 控制: 用电机2的编码器做闭环 (主电机) */
        float actual = encoder2_get_rpm();
        float error = g_setpoint - actual;
        float duty_f = pid_compute(&pid_params, &pid2_state, g_setpoint, actual);

        motor2_set_duty((int16)duty_f);
        /* 电机1 跟随电机2 */
        motor1_set_duty((int16)duty_f);

        /* 每个周期都输出 CSV (100ms 一行) */
        tick++;
        send_csv(ts, g_setpoint, actual, duty_f, error, g_kp, g_ki, g_kd);
    }
}
