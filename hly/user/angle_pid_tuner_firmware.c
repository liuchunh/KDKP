/*
 * angle_pid_tuner_firmware.c
 *
 * 转向电机 (角度闭环直流电机) PID 自动调参固件
 * 配合 llm-pid-tuner 上位机使用
 *
 * 串口协议 (兼容 llm-pid-tuner):
 *   MCU→PC:  timestamp_ms,setpoint,input,pwm,error,p,i,d
 *   PC→MCU:  SET P:<kp> I:<ki> D:<kd>
 *            SETPOINT:<value>
 *            STATUS
 *            RESET
 *
 * 硬件: TC264 + 角度电机 (ATOM0_CH7/CH6) + 编码器 (TIM4)
 *
 * 使用方法:
 *   1. 将此文件中的 core0_main 替换掉 cpu0_main.c 中的 core0_main
 *   2. 或者新建工程只包含此文件 + angle_control + PID 模块
 *   3. 上位机运行: python tuner.py
 *
 * 注意: 此固件使用 delay 方式控制周期, 不需要修改 isr.c
 *       主循环中每 1ms 轮询一次串口, 保证命令响应灵敏
 */

#include "zf_common_headfile.h"
#include "code/angle_control.h"
#include <stdlib.h>     /* atof */

/* ================================================================
 *  配置
 * ================================================================ */
#define CONTROL_INTERVAL_MS 50      /* 控制周期 50ms (20Hz) */
#define SERIAL_SEND_INTERVAL 3      /* 每3个控制周期发送一次数据 (~150ms) */

/* ================================================================
 *  全局状态
 * ================================================================ */
static float  g_target_angle = 30.0f;   /* 默认目标角度 30° (首次测试建议小角度) */
static uint32 g_timestamp_ms = 0;       /* 毫秒时间戳 */
static uint8  g_serial_send_div = 0;    /* 串口发送分频 */

/* ================================================================
 *  串口命令解析缓冲区
 * ================================================================ */
static char    g_cmd_buf[80];
static uint8   g_cmd_len = 0;
static uint8   g_cmd_overflow = 0;

/* ================================================================
 *  安全限制
 * ================================================================ */
#define KP_MAX  100.0f
#define KI_MAX  50.0f
#define KD_MAX  50.0f
#define TARGET_MAX  360.0f
#define TARGET_MIN -360.0f

/* ================================================================
 *  uart_send_csv() - CSV 格式上报数据
 *  格式: timestamp_ms,setpoint,input,pwm,error,p,i,d
 * ================================================================ */
static void uart_send_csv(void)
{
    char buf[128];
    float error = angle_ctrl.target_angle - angle_ctrl.current_angle;

    sprintf(buf, "%lu,%.2f,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f\r\n",
            (unsigned long)g_timestamp_ms,
            angle_ctrl.target_angle,
            angle_ctrl.current_angle,
            angle_ctrl.pid.Output,
            error,
            angle_ctrl.pid.Kp,
            angle_ctrl.pid.Ki,
            angle_ctrl.pid.Kd);

    uart_write_string(DEBUG_UART_INDEX, buf);
}

/* ================================================================
 *  process_command() - 处理一条完整的串口命令
 * ================================================================ */
static void process_command(char *cmd)
{
    if (cmd[0] == '\0') return;

    /* ---- SET P:... I:... D:... 或 PID ... ---- */
    if (strncmp(cmd, "SET ", 4) == 0 || strncmp(cmd, "PID", 3) == 0)
    {
        char *params = (cmd[0] == 'S') ? cmd + 4 : cmd + 3;
        while (*params == ' ') params++;

        float new_kp = angle_ctrl.pid.Kp;
        float new_ki = angle_ctrl.pid.Ki;
        float new_kd = angle_ctrl.pid.Kd;

        if (strstr(params, ":") != NULL) {
            char *p_pos = strstr(params, "P:");
            char *i_pos = strstr(params, "I:");
            char *d_pos = strstr(params, "D:");
            if (p_pos) new_kp = (float)atof(p_pos + 2);
            if (i_pos) new_ki = (float)atof(i_pos + 2);
            if (d_pos) new_kd = (float)atof(d_pos + 2);
        } else {
            float vals[3];
            int cnt = 0;
            char *tok = strtok(params, " ");
            while (tok && cnt < 3) {
                vals[cnt++] = (float)atof(tok);
                tok = strtok(NULL, " ");
            }
            if (cnt >= 3) {
                new_kp = vals[0];
                new_ki = vals[1];
                new_kd = vals[2];
            } else {
                uart_write_string(DEBUG_UART_INDEX, "# ERROR: PID format invalid\r\n");
                return;
            }
        }

        if (new_kp > 0.0f && new_kp <= KP_MAX &&
            new_ki >= 0.0f && new_ki <= KI_MAX &&
            new_kd >= 0.0f && new_kd <= KD_MAX)
        {
            angle_ctrl.pid.Kp = new_kp;
            angle_ctrl.pid.Ki = new_ki;
            angle_ctrl.pid.Kd = new_kd;
            angle_ctrl.pid.Integral = 0;    /* 清积分, 防饱和冲击 */
            angle_ctrl.pid.LastError = 0;   /* 清历史误差, 防微分突变 */

            char buf[80];
            sprintf(buf, "# PID Updated: P=%.3f I=%.3f D=%.3f\r\n", new_kp, new_ki, new_kd);
            uart_write_string(DEBUG_UART_INDEX, buf);
        } else {
            uart_write_string(DEBUG_UART_INDEX, "# ERROR: PID params out of range\r\n");
        }
    }
    /* ---- SETPOINT:value ---- */
    else if (strncmp(cmd, "SETPOINT", 8) == 0)
    {
        char *colon = strchr(cmd, ':');
        if (colon) {
            float new_target = (float)atof(colon + 1);
            if (new_target >= TARGET_MIN && new_target <= TARGET_MAX) {
                g_target_angle = new_target;
                angle_control_set_target((int32)new_target);
                char buf[48];
                sprintf(buf, "# Setpoint: %.2f\r\n", g_target_angle);
                uart_write_string(DEBUG_UART_INDEX, buf);
            } else {
                uart_write_string(DEBUG_UART_INDEX, "# ERROR: Setpoint out of range\r\n");
            }
        }
    }
    /* ---- STATUS ---- */
    else if (strcmp(cmd, "STATUS") == 0)
    {
        char buf[128];
        sprintf(buf, "# STATUS: Kp=%.3f Ki=%.3f Kd=%.3f Target=%.2f Angle=%.2f\r\n",
                angle_ctrl.pid.Kp, angle_ctrl.pid.Ki, angle_ctrl.pid.Kd,
                angle_ctrl.target_angle, angle_ctrl.current_angle);
        uart_write_string(DEBUG_UART_INDEX, buf);
    }
    /* ---- RESET ---- */
    else if (strcmp(cmd, "RESET") == 0)
    {
        angle_control_reset();
        angle_ctrl.pid.Kp = ANGLE_DEFAULT_KP;
        angle_ctrl.pid.Ki = ANGLE_DEFAULT_KI;
        angle_ctrl.pid.Kd = ANGLE_DEFAULT_KD;
        g_target_angle = 0.0f;
        uart_write_string(DEBUG_UART_INDEX, "# System Reset\r\n");
    }
}

/* ================================================================
 *  poll_serial() - 非阻塞串口命令接收
 * ================================================================ */
static void poll_serial(void)
{
    uint8 ch;
    while (uart_query_byte(DEBUG_UART_INDEX, &ch))
    {
        char c = (char)ch;
        if (c == '\r') continue;
        if (c != '\n') {
            if (!g_cmd_overflow) {
                if (g_cmd_len < sizeof(g_cmd_buf) - 1) {
                    g_cmd_buf[g_cmd_len++] = c;
                } else {
                    g_cmd_overflow = 1;
                }
            }
            continue;
        }

        if (g_cmd_overflow) {
            g_cmd_len = 0;
            g_cmd_overflow = 0;
            uart_write_string(DEBUG_UART_INDEX, "# ERROR: command too long\r\n");
            continue;
        }

        g_cmd_buf[g_cmd_len] = '\0';
        g_cmd_len = 0;
        process_command(g_cmd_buf);
    }
}

/* ================================================================
 *  core0_main() - 主函数
 *
 *  使用 delay 方式控制 50ms 周期, 不依赖 PIT 中断
 *  每 1ms 轮询一次串口, 保证命令响应及时
 * ================================================================ */
int core0_main(void)
{
    /* ---- 系统初始化 ---- */
    clock_init();
    debug_init();

    /* ---- 角度控制模块初始化 ---- */
    angle_control_init();

    /* ---- 设置初始 PID 参数 (预调值, 可通过串口在线修改) ---- */
    angle_ctrl.pid.Kp = 15.0f;     /* 比例: 太大会振荡, 太小响应慢 */
    angle_ctrl.pid.Ki = 0.3f;      /* 积分: 消除稳态误差, 太大会超调 */
    angle_ctrl.pid.Kd = 3.0f;      /* 微分: 抑制振荡, 太大响应迟钝 */
    angle_ctrl.pid.Integral = 0;
    angle_ctrl.pid.LastError = 0;

    angle_control_set_target((int32)g_target_angle);

    /* ---- 启动信息 (# 开头, 上位机会忽略) ---- */
    uart_write_string(DEBUG_UART_INDEX, "# ========================================\r\n");
    uart_write_string(DEBUG_UART_INDEX, "# Angle Motor PID Tuner Firmware v1.0\r\n");
    uart_write_string(DEBUG_UART_INDEX, "# Format: timestamp_ms,setpoint,input,pwm,error,p,i,d\r\n");
    uart_write_string(DEBUG_UART_INDEX, "# Commands: SET P: I: D: | SETPOINT: | STATUS | RESET\r\n");
    uart_write_string(DEBUG_UART_INDEX, "# ========================================\r\n");

    /* ---- 等待多核同步 ---- */
    cpu_wait_event_ready();

    /* ---- 主循环 ---- */
    uint8 tick_count = 0;       /* 1ms 计数器, 到 50ms 执行一次控制 */

    while (TRUE)
    {
        /* 1. 非阻塞轮询串口命令 (每 1ms 检查一次) */
        poll_serial();

        /* 2. 每 1ms 自增一次计数器 */
        tick_count++;

        /* 3. 到达 50ms 控制周期 */
        if (tick_count >= CONTROL_INTERVAL_MS)
        {
            tick_count = 0;

            /* 更新目标角度 */
            angle_control_set_target((int32)g_target_angle);

            /* 执行一次 PID 控制 (读编码器 → PID → 驱动电机) */
            angle_control_update();

            /* 时间戳累加 */
            g_timestamp_ms += CONTROL_INTERVAL_MS;

            /* 4. 按分频周期上报 CSV 数据 */
            g_serial_send_div++;
            if (g_serial_send_div >= SERIAL_SEND_INTERVAL)
            {
                g_serial_send_div = 0;
                uart_send_csv();
            }
        }

        /* 5. 1ms 延时 (保证循环周期稳定) */
        system_delay_ms(1);
    }
}
