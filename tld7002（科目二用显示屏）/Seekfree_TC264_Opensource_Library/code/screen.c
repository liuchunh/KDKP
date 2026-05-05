#include "screen.h"

#pragma section all "cpu0_dsram"

// ===================== 串口 FIFO =====================
static uint8  rx_buf[64];
static uint8  rx_tmp[64];
static fifo_struct rx_fifo;

// ===================== 协议解析状态机 =====================
// 帧格式：[6B 79] [00] [81] [CMD_ID] [00] [CHK] [FB]  共 8 字节
#define FRAME_LEN       8
#define FRAME_HEAD0     0x6B
#define FRAME_HEAD1_RX  0x79    // CI1302 → TC264
#define FRAME_HEAD1_TX  0x6A    // TC264 → CI1302
#define FRAME_MSG_RX    0x81
#define FRAME_MSG_TX    0x82
#define FRAME_TAIL      0xFB

typedef enum
{
    STATE_IDLE = 0,     // 等待帧头第一字节 0x6B
    STATE_HEAD1,        // 等待帧头第二字节 0x79
    STATE_DATA          // 接收剩余 6 字节
} parse_state_t;

static parse_state_t parse_state = STATE_IDLE;
static uint8 frame_buf[FRAME_LEN];
static uint8 frame_pos;

static uint8 last_cmd_id = 0;   // 最近收到的命令 ID

#pragma section all restore

// ===================== 内部函数声明 =====================
static void ci1302_send_response(uint8 cmd_id);
static void ci1302_execute_cmd(uint8 cmd_id);
static void parse_byte(uint8 byte);

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      发送应答帧给 CI1302
//  帧格式        6B 6A 00 82 CMD_ID 00 CHK FB
//-------------------------------------------------------------------------------------------------------------------
static void ci1302_send_response(uint8 cmd_id)
{
    uint8 resp[FRAME_LEN];
    resp[0] = FRAME_HEAD0;
    resp[1] = FRAME_HEAD1_TX;
    resp[2] = 0x00;
    resp[3] = FRAME_MSG_TX;
    resp[4] = cmd_id;
    resp[5] = 0x00;
    resp[6] = (resp[0] + resp[1] + resp[2] + resp[3] + resp[4] + resp[5]) & 0xFF;
    resp[7] = FRAME_TAIL;

    uint8 i;
    for(i = 0; i < FRAME_LEN; i++)
    {
        uart_write_byte(DEBUG_UART_INDEX, resp[i]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      根据 CMD_ID 执行对应屏幕动作
//-------------------------------------------------------------------------------------------------------------------
static void ci1302_execute_cmd(uint8 cmd_id)
{
    last_cmd_id = cmd_id;

    switch(cmd_id)
    {
        // 灯光类：控制点阵屏显示对应图案
        case CI_CMD_TURN_LEFT:
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_TURN_LEFT);
            break;

        case CI_CMD_TURN_RIGHT:
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_TURN_RIGHT);
            break;

        case CI_CMD_HIGH_BEAM:
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_HIGH_BEAM);
            break;

        case CI_CMD_LOW_BEAM:
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_LOW_BEAM);
            break;

        case CI_CMD_FOG_LIGHT:
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_FOG_LIGHT);
            break;

        case CI_CMD_DOUBLE_FLASH:
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_DOUBLE_FLASH);
            break;

        case CI_CMD_INTERIOR_LIGHT:
            dot_matrix_screen_set_brightness(10000);
            dot_matrix_screen_show_string("***");
            break;

        // 系统类：无屏幕动作
        case CI_CMD_WAKEUP:
        case CI_CMD_WELCOME:
        case CI_CMD_SLEEP:
            break;

        // 其他类（鸣笛、门洞、行驶）：暂无屏幕动作，后续扩展
        default:
            break;
    }

    // 串口调试输出
    uart_write_string(DEBUG_UART_INDEX, "[CI1302] CMD_ID=0x");
    uart_write_byte(DEBUG_UART_INDEX, "0123456789ABCDEF"[(cmd_id >> 4) & 0x0F]);
    uart_write_byte(DEBUG_UART_INDEX, "0123456789ABCDEF"[cmd_id & 0x0F]);
    uart_write_string(DEBUG_UART_INDEX, "\r\n");
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      协议解析状态机，逐字节输入
//-------------------------------------------------------------------------------------------------------------------
static void parse_byte(uint8 byte)
{
    switch(parse_state)
    {
        case STATE_IDLE:
            if(byte == FRAME_HEAD0)
            {
                frame_buf[0] = byte;
                frame_pos = 1;
                parse_state = STATE_HEAD1;
            }
            break;

        case STATE_HEAD1:
            if(byte == FRAME_HEAD1_RX)
            {
                frame_buf[1] = byte;
                frame_pos = 2;
                parse_state = STATE_DATA;
            }
            else
            {
                parse_state = STATE_IDLE;    // 帧头不匹配，重新等待
            }
            break;

        case STATE_DATA:
            frame_buf[frame_pos++] = byte;
            if(frame_pos >= FRAME_LEN)
            {
                // 收齐 8 字节，验证帧
                // 验证帧尾
                if(frame_buf[7] != FRAME_TAIL)
                {
                    uart_write_string(DEBUG_UART_INDEX, "[ERR] Bad tail\r\n");
                    parse_state = STATE_IDLE;
                    break;
                }
                // 验证消息类型
                if(frame_buf[3] != FRAME_MSG_RX)
                {
                    uart_write_string(DEBUG_UART_INDEX, "[ERR] Bad msg type\r\n");
                    parse_state = STATE_IDLE;
                    break;
                }
                // 验证校验和
                uint8 chk = 0;
                uint8 k;
                for(k = 0; k < 6; k++)
                {
                    chk += frame_buf[k];
                }
                if(chk != frame_buf[6])
                {
                    uart_write_string(DEBUG_UART_INDEX, "[ERR] Bad checksum\r\n");
                    parse_state = STATE_IDLE;
                    break;
                }
                // 帧有效，提取 CMD_ID
                uint8 cmd_id = frame_buf[4];

                // 发送应答帧
                ci1302_send_response(cmd_id);

                // 执行命令
                ci1302_execute_cmd(cmd_id);

                parse_state = STATE_IDLE;
            }
            break;

        default:
            parse_state = STATE_IDLE;
            break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      初始化屏幕模块（FIFO + 点阵屏 + 自检）
//-------------------------------------------------------------------------------------------------------------------
void screen_init(void)
{
    fifo_init(&rx_fifo, FIFO_DATA_8BIT, rx_buf, 64);

    gpio_init(P11_2, GPO, 1, GPO_PUSH_PULL);

    system_delay_ms(100);
    dot_matrix_screen_init();

    // 开机自检：双闪3秒
    uart_write_string(DEBUG_UART_INDEX, "\r\n[DIAG] Self-test: DoubleFlash ON\r\n");
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_DOUBLE_FLASH);
    system_delay_ms(3000);
    dot_matrix_screen_set_brightness(0);
    dot_matrix_screen_clear_pattern();
    uart_write_string(DEBUG_UART_INDEX, "[DIAG] Self-test: OFF\r\n");

    uart_write_string(DEBUG_UART_INDEX, "===== CI1302 Screen Ready =====\r\n");
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      主循环中调用，从 FIFO 取字节送入协议解析状态机
//-------------------------------------------------------------------------------------------------------------------
void screen_poll(void)
{
    uint32 count = fifo_used(&rx_fifo);
    if(count == 0)
    {
        return;
    }

    fifo_read_buffer(&rx_fifo, rx_tmp, &count, FIFO_READ_AND_CLEAN);

    uint32 i;
    for(i = 0; i < count; i++)
    {
        parse_byte(rx_tmp[i]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      串口接收中断处理（在 isr.c 的 uart0_rx_isr 中调用）
//-------------------------------------------------------------------------------------------------------------------
void screen_uart_rx_handler(void)
{
    uint8 dat;
    uart_query_byte(DEBUG_UART_INDEX, &dat);
    fifo_write_buffer(&rx_fifo, &dat, 1);
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      获取最近收到的命令 ID
//-------------------------------------------------------------------------------------------------------------------
uint8 screen_get_last_cmd(void)
{
    return last_cmd_id;
}
