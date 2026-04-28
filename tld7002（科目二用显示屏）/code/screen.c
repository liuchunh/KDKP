#include "screen.h"
#include "isr_config.h"

#pragma section all "cpu0_dsram"

// 串口配置
#define SCREEN_UART_INDEX       (DEBUG_UART_INDEX   )
#define SCREEN_UART_BAUDRATE    (DEBUG_UART_BAUDRATE)
#define SCREEN_UART_TX_PIN      (DEBUG_UART_TX_PIN  )
#define SCREEN_UART_RX_PIN      (DEBUG_UART_RX_PIN  )

// 串口 FIFO 相关变量
static uint8 uart_get_data[64];
static uint8 fifo_get_data[64];
static uint8 get_data = 0;
static uint32 fifo_data_count = 0;
static fifo_struct uart_data_fifo;

static uint8 current_mode = SCREEN_CMD_OFF;

#pragma section all restore

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      初始化屏幕模块（含串口、FIFO、点阵屏）
//-------------------------------------------------------------------------------------------------------------------
void screen_init(void)
{
    fifo_init(&uart_data_fifo, FIFO_DATA_8BIT, uart_get_data, 64);
    uart_init(SCREEN_UART_INDEX, SCREEN_UART_BAUDRATE, SCREEN_UART_TX_PIN, SCREEN_UART_RX_PIN);
    uart_rx_interrupt(SCREEN_UART_INDEX, 1);

    system_delay_ms(100);
    dot_matrix_screen_init();
    current_mode = SCREEN_CMD_OFF;

    uart_write_string(SCREEN_UART_INDEX, "Screen Ready.\r\n");
    uart_write_string(SCREEN_UART_INDEX, "Send 0-6 to switch mode: 0=OFF 1=DoubleFlash 2=Left 3=Right 4=Low 5=High 6=Fog\r\n");
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      主循环中调用，轮询 FIFO 并处理串口命令控制屏幕
//-------------------------------------------------------------------------------------------------------------------
void screen_poll(void)
{
    fifo_data_count = fifo_used(&uart_data_fifo);
    if(fifo_data_count == 0)
    {
        return;
    }

    fifo_read_buffer(&uart_data_fifo, fifo_get_data, &fifo_data_count, FIFO_READ_AND_CLEAN);

    uint32 i;
    for(i = 0; i < fifo_data_count; i++)
    {
        uint8 ch = fifo_get_data[i];
        uint8 cmd;

        // 支持文本模式：字符 '0'-'6' 转为命令码 0x00-0x06
        if(ch >= '0' && ch <= '6')
        {
            cmd = ch - '0';
        }
        // 也支持HEX模式：直接发送 0x00-0x06
        else if(ch <= SCREEN_CMD_FOG_LIGHT)
        {
            cmd = ch;
        }
        else
        {
            // 忽略回车换行等无效字符
            if(ch != '\r' && ch != '\n')
            {
                uart_write_string(SCREEN_UART_INDEX, "Invalid cmd.\r\n");
            }
            continue;
        }

        current_mode = cmd;

        // 控制屏幕显示
        if(cmd == 0)
        {
            dot_matrix_screen_set_brightness(0);
            dot_matrix_screen_show_string("   ");
        }
        else
        {
            dot_matrix_screen_set_brightness(5000);
            dot_matrix_screen_show_led_pattern((dot_matrix_pattern_t)(cmd - 1));
        }

        // 回显当前模式
        uart_write_string(SCREEN_UART_INDEX, "Mode set: ");
        uart_write_byte(SCREEN_UART_INDEX, '0' + cmd);
        uart_write_string(SCREEN_UART_INDEX, "\r\n");
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      串口接收中断处理（在 isr.c 的 uart0_rx_isr 中调用）
//-------------------------------------------------------------------------------------------------------------------
void uart_rx_interrupt_handler(void)
{
    uart_query_byte(SCREEN_UART_INDEX, &get_data);
    fifo_write_buffer(&uart_data_fifo, &get_data, 1);
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      获取当前灯光模式命令码
//-------------------------------------------------------------------------------------------------------------------
uint8 screen_get_mode(void)
{
    return current_mode;
}
