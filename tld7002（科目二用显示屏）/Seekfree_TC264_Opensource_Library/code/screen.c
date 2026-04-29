#include "screen.h"

#pragma section all "cpu0_dsram"

// 串口 FIFO 相关变量
static uint8  rx_buf[64];
static uint8  rx_tmp[64];
static fifo_struct rx_fifo;

static uint8 current_mode = SCREEN_CMD_OFF;

// 命令码 -> 图案映射（cmd 1~6 对应索引 0~5）
static const dot_matrix_pattern_t cmd_to_pattern[] = {
    DOT_MATRIX_PATTERN_DOUBLE_FLASH,    // cmd 1
    DOT_MATRIX_PATTERN_TURN_LEFT,       // cmd 2
    DOT_MATRIX_PATTERN_TURN_RIGHT,      // cmd 3
    DOT_MATRIX_PATTERN_LOW_BEAM,        // cmd 4
    DOT_MATRIX_PATTERN_HIGH_BEAM,       // cmd 5
    DOT_MATRIX_PATTERN_FOG_LIGHT,       // cmd 6
};

static const char *cmd_names[] = {
    "OFF",
    "Double Flash",
    "Turn Left",
    "Turn Right",
    "Low Beam",
    "High Beam",
    "Fog Light",
};

#pragma section all restore

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      初始化屏幕模块（FIFO + 点阵屏 + 自检）
//  注意          需在 debug_init() 之后调用，UART 已由 debug_init 初始化
//-------------------------------------------------------------------------------------------------------------------
void screen_init(void)
{
    fifo_init(&rx_fifo, FIFO_DATA_8BIT, rx_buf, 64);

    gpio_init(P11_2, GPO, 1, GPO_PUSH_PULL);

    system_delay_ms(100);
    dot_matrix_screen_init();
    current_mode = SCREEN_CMD_OFF;

    // 开机自检：双闪3秒
    uart_write_string(DEBUG_UART_INDEX, "\r\n[DIAG] Self-test: DoubleFlash ON\r\n");
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_show_led_pattern(DOT_MATRIX_PATTERN_DOUBLE_FLASH);
    system_delay_ms(3000);
    dot_matrix_screen_set_brightness(0);
    dot_matrix_screen_clear_pattern();
    uart_write_string(DEBUG_UART_INDEX, "[DIAG] Self-test: OFF\r\n");

    uart_write_string(DEBUG_UART_INDEX,
        "===== Screen Controller Ready =====\r\n"
        "CMD: 0=OFF 1=DoubleFlash 2=Left 3=Right 4=LowBeam 5=HighBeam 6=Fog\r\n");
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      主循环中调用，轮询 FIFO 并处理串口命令控制屏幕
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
        uint8 ch = rx_tmp[i];
        uint8 cmd;

        // 文本模式：'0'~'6'
        if(ch >= '0' && ch <= '6')
        {
            cmd = ch - '0';
        }
        // HEX 模式：0x00~0x06
        else if(ch <= SCREEN_CMD_MAX)
        {
            cmd = ch;
        }
        else
        {
            if(ch != '\r' && ch != '\n')
            {
                uart_write_string(DEBUG_UART_INDEX, "[ERR] Invalid cmd\r\n");
            }
            continue;
        }

        current_mode = cmd;

        // 控制屏幕显示
        if(cmd == SCREEN_CMD_OFF)
        {
            dot_matrix_screen_set_brightness(0);
            dot_matrix_screen_clear_pattern();
        }
        else
        {
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern(cmd_to_pattern[cmd - 1]);
        }

        // 回显
        uart_write_string(DEBUG_UART_INDEX, "[SET] Mode ");
        uart_write_byte(DEBUG_UART_INDEX, '0' + cmd);
        uart_write_string(DEBUG_UART_INDEX, " -> ");
        uart_write_string(DEBUG_UART_INDEX, cmd_names[cmd]);
        uart_write_string(DEBUG_UART_INDEX, "\r\n");
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
//  函数简介      获取当前灯光模式命令码
//-------------------------------------------------------------------------------------------------------------------
uint8 screen_get_mode(void)
{
    return current_mode;
}
