/*********************************************************************************************************************
* TC264 Opensourec Library 锟斤拷锟斤拷TC264 锟斤拷源锟解）锟斤拷一锟斤拷锟斤拷锟节官凤拷 SDK 锟接口的碉拷锟斤拷锟斤拷锟斤拷源锟斤拷
* Copyright (c) 2022 SEEKFREE 锟斤拷煽萍锟�
*
* 锟斤拷锟侥硷拷锟斤拷 TC264 锟斤拷源锟斤拷锟揭伙拷锟斤拷锟�
*
* TC264 锟斤拷源锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷锟�
* 锟斤拷锟斤拷锟皆革拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷岱拷锟斤拷锟� GPL锟斤拷GNU General Public License锟斤拷锟斤拷 GNU通锟矫癸拷锟斤拷锟斤拷锟斤拷证锟斤拷锟斤拷锟斤拷锟斤拷
* 锟斤拷 GPL 锟侥碉拷3锟芥（锟斤拷 GPL3.0锟斤拷锟斤拷锟斤拷选锟斤拷模锟斤拷魏魏锟斤拷锟斤拷陌姹撅拷锟斤拷锟斤拷路锟斤拷锟斤拷锟�/锟斤拷锟睫革拷锟斤拷
*
* 锟斤拷锟斤拷源锟斤拷姆锟斤拷锟斤拷锟较ｏ拷锟斤拷锟斤拷芊锟斤拷锟斤拷锟斤拷茫锟斤拷锟斤拷锟轿达拷锟斤拷锟斤拷锟斤拷魏蔚谋锟街�
* 锟斤拷锟斤拷没锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟皆伙拷锟绞猴拷锟截讹拷锟斤拷途锟侥憋拷证
* 锟斤拷锟斤拷细锟斤拷锟斤拷渭锟� GPL
*
* 锟斤拷应锟斤拷锟斤拷锟秸碉拷锟斤拷锟斤拷源锟斤拷锟酵憋拷盏锟揭伙拷锟� GPL 锟侥革拷锟斤拷
* 锟斤拷锟矫伙拷校锟斤拷锟斤拷锟斤拷<https://www.gnu.org/licenses/>
*
* 锟斤拷锟斤拷注锟斤拷锟斤拷
* 锟斤拷锟斤拷源锟斤拷使锟斤拷 GPL3.0 锟斤拷源锟斤拷锟斤拷证协锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷为锟斤拷锟侥版本
* 锟斤拷锟斤拷锟斤拷锟斤拷英锟侥帮拷锟斤拷 libraries/doc 锟侥硷拷锟斤拷锟铰碉拷 GPL3_permission_statement.txt 锟侥硷拷锟斤拷
* 锟斤拷锟斤拷证锟斤拷锟斤拷锟斤拷 libraries 锟侥硷拷锟斤拷锟斤拷 锟斤拷锟斤拷锟侥硷拷锟斤拷锟铰碉拷 LICENSE 锟侥硷拷
* 锟斤拷迎锟斤拷位使锟矫诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟睫革拷锟斤拷锟斤拷时锟斤拷锟诫保锟斤拷锟斤拷煽萍锟斤拷陌锟饺拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
*
* 锟侥硷拷锟斤拷锟斤拷          zf_common_debug
* 锟斤拷司锟斤拷锟斤拷          锟缴讹拷锟斤拷煽萍锟斤拷锟斤拷薰锟剿�
* 锟芥本锟斤拷息          锟介看 libraries/doc 锟侥硷拷锟斤拷锟斤拷 version 锟侥硷拷 锟芥本说锟斤拷
* 锟斤拷锟斤拷锟斤拷锟斤拷          ADS v1.10.2
* 锟斤拷锟斤拷平台          TC264D
* 锟斤拷锟斤拷锟斤拷锟斤拷          https://seekfree.taobao.com/
*
* 锟睫改硷拷录
* 锟斤拷锟斤拷              锟斤拷锟斤拷                锟斤拷注
* 2022-09-15       pudding            first version
* 2022-05-26       pudding            锟斤拷锟斤拷锟斤拷锟皆达拷锟节凤拷锟酵猴拷锟斤拷
********************************************************************************************************************/

#include "zf_common_interrupt.h"
#include "zf_common_fifo.h"
#include "zf_driver_uart.h"
#include "zf_common_debug.h"

static debug_output_struct  debug_output_info;
static volatile uint8       zf_debug_init_flag = 0;
static volatile uint8       zf_debug_assert_enable = 1;

#if DEBUG_UART_USE_INTERRUPT                                                    // 锟斤拷锟斤拷锟斤拷锟� debug uart 锟斤拷锟斤拷锟叫讹拷
uint8                       debug_uart_buffer[DEBUG_RING_BUFFER_LEN];           // 锟斤拷锟捷达拷锟斤拷锟斤拷锟�
uint8                       debug_uart_data;
fifo_struct                 debug_uart_fifo;
#endif

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟斤拷时锟斤拷锟斤拷 锟斤拷 200MHz 锟斤拷锟斤拷一锟斤拷锟斤拷时锟斤拷 锟斤拷锟斤拷片锟斤拷锟斤拷要锟斤拷锟捷革拷锟斤拷时锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     void
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_delay();
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷锟侥硷拷锟节诧拷锟斤拷锟斤拷 锟矫伙拷锟斤拷锟矫癸拷注 也锟斤拷锟斤拷锟睫革拷
//-------------------------------------------------------------------------------------------------------------------
static void debug_delay (void)
{
    vuint32 loop_1 = 0, loop_2 = 0;
    for(loop_1 = 0; loop_1 <= 0xFFF; loop_1 ++)
        for(loop_2 = 0; loop_2 <= 0x1FFF; loop_2 ++);
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟斤拷锟斤拷锟斤拷涌锟�
// 锟斤拷锟斤拷说锟斤拷     *str        锟斤拷要锟斤拷锟斤拷锟斤拷址锟斤拷锟�
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_uart_str_output("Log message");
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷锟侥硷拷锟节诧拷锟斤拷锟斤拷 锟矫伙拷锟斤拷锟矫癸拷注 也锟斤拷锟斤拷锟睫革拷
//-------------------------------------------------------------------------------------------------------------------
static void debug_uart_str_output (const char *str)
{
    uart_write_string(DEBUG_UART_INDEX, str);
}



//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟斤拷涌锟�
// 锟斤拷锟斤拷说锟斤拷     *type       log 锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     *file       锟侥硷拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     line        目锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     *str        锟斤拷息
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_output("Log message", file, line, str);
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷锟侥硷拷锟节诧拷锟斤拷锟斤拷 锟矫伙拷锟斤拷锟矫癸拷注 也锟斤拷锟斤拷锟睫革拷
//-------------------------------------------------------------------------------------------------------------------
static void debug_output (char *type, char *file, int line, char *str)
{
    char *file_str;
    vuint16 i = 0, j = 0;
    vint32 len_origin = 0;
    vint16 show_len = 0;
    vint16 show_line_index = 0;
    len_origin = strlen(file);

    char output_buffer[256];
    char file_path_buffer[64];

    if(debug_output_info.type_index)
    {
        debug_output_info.output_screen_clear();
    }

    if(zf_debug_init_flag)
    {
        if(debug_output_info.type_index)
        {
            // 锟斤拷要锟斤拷锟叫斤拷锟侥硷拷锟斤拷路锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
            // <锟斤拷锟斤拷锟斤拷锟斤拷锟铰凤拷锟� 只锟斤拷锟揭伙拷锟侥柯� 锟斤拷锟斤拷 src/main.c>
            // 锟斤拷锟� line : xxxx
            debug_output_info.output_screen(0, show_line_index ++, type);

            file_str = file;
            len_origin = strlen(file);
            show_len = (debug_output_info.display_x_max / debug_output_info.font_x_size);

            while(*file_str++ != '\0');

            // 只取一锟斤拷目录 锟斤拷锟斤拷募锟斤拷锟斤拷锟斤拷谭锟斤拷锟侥柯� 锟斤拷锟斤拷 MDK 锟侥癸拷锟教革拷目录 锟酵伙拷直锟斤拷锟斤拷锟斤拷锟角澳柯�
            for(j = 0; (j < 2) && (len_origin >= 0); len_origin --)             // 锟斤拷锟斤拷锟斤拷锟斤拷 '/'
            {
                file_str --;
                if((*file_str == '/') || (*file_str == 0x5C))
                {
                    j ++;
                }
            }

            // 锟侥硷拷路锟斤拷锟斤拷锟芥到锟斤拷锟斤拷锟斤拷
            if(len_origin >= 0)
            {
                file_str ++;
                sprintf(output_buffer, "file: %s", file_str);
            }
            else
            {
                if(0 == j)
                {
                    sprintf(output_buffer, "file: mdk/%s", file_str);
                }
                else
                {
                    sprintf(output_buffer, "file: %s", file_str);
                }
            }

            // 锟斤拷幕锟斤拷示路锟斤拷
            for(i = 0; i < ((strlen(output_buffer) / show_len) + 1); i ++)
            {
                for(j = 0; j < show_len; j ++)
                {
                    if(strlen(output_buffer) < (j + i * show_len))
                    {
                        break;
                    }
                    file_path_buffer[j] = output_buffer[j + i * show_len];
                }

                file_path_buffer[j] = '\0';                                     // 末尾锟斤拷锟斤拷\0

                debug_output_info.output_screen(0, debug_output_info.font_y_size * show_line_index ++, file_path_buffer);
            }

            // 锟斤拷幕锟斤拷示锟叫猴拷
            sprintf(output_buffer, "line: %d", line);
            debug_output_info.output_screen(0, debug_output_info.font_y_size * show_line_index ++, output_buffer);

            // 锟斤拷幕锟斤拷示 Log 锟斤拷锟斤拷械幕锟�
            if(NULL != str)
            {
                for(i = 0; i < ((strlen(str) / show_len) + 1); i ++)
                {
                    for(j = 0; j < show_len; j ++)
                    {
                        if(strlen(str) < (j + i * show_len))
                        {
                            break;
                        }
                        file_path_buffer[j] = str[j + i * show_len];
                    }

                    file_path_buffer[j] = '\0';                                 // 末尾锟斤拷锟斤拷\0

                    debug_output_info.output_screen(0, debug_output_info.font_y_size * show_line_index ++, file_path_buffer);
                }
            }
        }
        else
        {
            char output_buffer[256];
            memset(output_buffer, 0, 256);
            debug_output_info.output_uart(type);
            if(NULL != str)
            {
                sprintf(output_buffer, "\r\nfile %s line %d: %s.\r\n", file, line, str);
            }
            else
            {
                sprintf(output_buffer, "\r\nfile %s line %d.\r\n", file, line);
            }
            debug_output_info.output_uart(output_buffer);
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     锟斤拷锟皆达拷锟节凤拷锟酵伙拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     *buff       锟斤拷锟斤拷锟斤拷锟捷达拷诺锟斤拷锟斤拷锟街革拷锟�
// 锟斤拷锟斤拷说锟斤拷     len         锟斤拷要锟斤拷锟酵的筹拷锟斤拷
// 锟斤拷锟截诧拷锟斤拷     uint32      剩锟斤拷未锟斤拷锟酵的筹拷锟斤拷
// 使锟斤拷示锟斤拷
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷 DEBUG_UART_USE_INTERRUPT 锟疥定锟斤拷趴锟绞癸拷锟�
//-------------------------------------------------------------------------------------------------------------------
uint32 debug_send_buffer(const uint8 *buff, uint32 len)
{
    uart_write_buffer(DEBUG_UART_INDEX, buff, len);
    return 0;
}

#if DEBUG_UART_USE_INTERRUPT                                                    // 锟斤拷锟斤拷锟斤拷锟斤拷 只锟斤拷锟斤拷锟斤拷锟矫达拷锟斤拷锟叫断才憋拷锟斤拷

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     锟斤拷取 debug 锟斤拷锟轿伙拷锟斤拷锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     *buff       锟斤拷锟斤拷锟斤拷锟捷达拷诺锟斤拷锟斤拷锟街革拷锟�
// 锟斤拷锟斤拷说锟斤拷     len         锟斤拷要锟斤拷取锟侥筹拷锟斤拷
// 锟斤拷锟截诧拷锟斤拷     uint32      锟斤拷锟斤拷锟斤拷锟捷碉拷实锟绞筹拷锟斤拷
// 使锟斤拷示锟斤拷
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷 DEBUG_UART_USE_INTERRUPT 锟疥定锟斤拷趴锟绞癸拷锟�
//-------------------------------------------------------------------------------------------------------------------
uint32 debug_read_ring_buffer (uint8 *buff, uint32 len)
{
    fifo_read_buffer(&debug_uart_fifo, buff, &len, FIFO_READ_AND_CLEAN);
    return len;
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟斤拷锟叫断达拷锟斤拷锟斤拷锟斤拷 isr.c 锟叫讹拷应锟斤拷锟斤拷锟叫断凤拷锟斤拷锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     void
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_interrupr_handler();
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷 DEBUG_UART_USE_INTERRUPT 锟疥定锟斤拷趴锟绞癸拷锟�
//              锟斤拷锟揭憋拷锟斤拷锟斤拷默锟较凤拷锟斤拷锟斤拷 UART1 锟侥达拷锟节斤拷锟斤拷锟叫断达拷锟斤拷锟斤拷
//-------------------------------------------------------------------------------------------------------------------
void debug_interrupr_handler (void)
{
    if(zf_debug_init_flag)
    {
        uart_query_byte(DEBUG_UART_INDEX, &debug_uart_data);                    // 锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷
        fifo_write_buffer(&debug_uart_fifo, &debug_uart_data, 1);               // 锟斤拷锟斤拷 FIFO
    }
}

#endif


//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     锟斤拷锟矫讹拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     void
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_assert_enable();
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷默锟较匡拷锟斤拷 锟斤拷锟介开锟斤拷锟斤拷锟斤拷
//-------------------------------------------------------------------------------------------------------------------
void debug_assert_enable (void)
{
    zf_debug_assert_enable = 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     锟斤拷锟矫讹拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     void
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_assert_disable();
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷默锟较匡拷锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷枚锟斤拷锟�
//-------------------------------------------------------------------------------------------------------------------
void debug_assert_disable (void)
{
    zf_debug_assert_enable = 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟皆达拷锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     pass        锟叫讹拷锟角否触凤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     *file       锟侥硷拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     line        目锟斤拷锟斤拷锟斤拷
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     zf_assert(0);
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街憋拷拥锟斤拷玫锟� 锟剿诧拷锟街诧拷锟斤拷锟斤拷锟矫伙拷锟斤拷锟斤拷
//             使锟斤拷 zf_commmon_debug.h 锟叫碉拷 zf_assert(x) 锟接匡拷
//-------------------------------------------------------------------------------------------------------------------
void debug_assert_handler (uint8 pass, char *file, int line)
{
    if(pass || !zf_debug_assert_enable)
    {
        return;
    }

    static uint8 assert_nest_index = 0;

    if(0 != assert_nest_index)
    {
        while(TRUE);
    }
    assert_nest_index ++;

    assert_interrupt_config();

    while(TRUE)
    {
        // 锟斤拷锟斤拷锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷锟酵Ｗ★拷锟�
        // 一锟斤拷锟斤拷暮锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷莩锟斤拷锟斤拷锟�
        // 锟斤拷锟斤拷锟斤拷锟皆硷拷锟斤拷锟矫碉拷 zf_assert(x) 锟接口达拷锟斤拷锟斤拷锟斤拷

        // 锟斤拷锟斤拷锟斤拷锟斤拷锟� debug_init 锟斤拷始锟斤拷锟斤拷 log 锟斤拷锟�
        // 锟斤拷锟节讹拷应锟斤拷锟斤拷锟斤拷锟饺ワ拷榭达拷锟斤拷母锟斤拷募锟斤拷锟斤拷锟揭伙拷斜锟斤拷锟�

        // 锟斤拷锟矫伙拷谐锟绞硷拷锟� debug
        // 锟角就匡拷锟斤拷锟斤拷锟� file 锟斤拷锟街凤拷锟斤拷值锟斤拷 line 锟斤拷锟斤拷锟斤拷
        // 锟角达拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥硷拷路锟斤拷锟斤拷锟狡和讹拷应锟斤拷锟斤拷锟斤拷锟斤拷

        // 锟斤拷去锟斤拷锟皆匡拷锟斤拷锟斤拷为什么锟斤拷锟斤拷锟斤拷锟斤拷
        debug_output("Assert error", file, line, NULL);
        debug_delay();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟斤拷锟斤拷息锟斤拷锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     pass        锟叫讹拷锟角否触凤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     *str        锟斤拷锟斤拷锟斤拷锟较�
// 锟斤拷锟斤拷说锟斤拷     *file       锟侥硷拷锟斤拷
// 锟斤拷锟斤拷说锟斤拷     line        目锟斤拷锟斤拷锟斤拷
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     zf_log(0, "Log Message");
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街憋拷拥锟斤拷玫锟� 锟剿诧拷锟街诧拷锟斤拷锟斤拷锟矫伙拷锟斤拷锟斤拷
//             使锟斤拷 zf_commmon_debug.h 锟叫碉拷 zf_log(x, str) 锟接匡拷
//-------------------------------------------------------------------------------------------------------------------
void debug_log_handler (uint8 pass, char *str, char *file, int line)
{
    if(pass)
    {
        return;
    }
    if(zf_debug_init_flag)
    {
        debug_output("Log message", file, line, str);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟斤拷锟斤拷锟较拷锟绞硷拷锟�
// 锟斤拷锟斤拷说锟斤拷     *info       debug 锟斤拷锟斤拷锟斤拷锟较拷峁癸拷锟�
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_output_struct_init(info);
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟揭伙拷悴伙拷锟斤拷没锟斤拷锟斤拷锟�
//-------------------------------------------------------------------------------------------------------------------
void debug_output_struct_init (debug_output_struct *info)
{
    info->type_index            = 0;                        // 锟斤拷幕锟斤拷息锟斤拷锟斤拷锟街�

    info->display_x_max         = 0xFFFF;                   // 锟斤拷幕锟斤拷示X锟斤拷锟斤拷锟街�
    info->display_y_max         = 0xFFFF;                   // 锟斤拷幕锟斤拷示Y锟斤拷锟斤拷锟街�

    info->font_x_size           = 0xFF;                     // 锟斤拷锟斤拷X锟斤拷锟斤拷锟街�
    info->font_y_size           = 0xFF;                     // 锟斤拷锟斤拷Y锟斤拷锟斤拷锟街�

    info->output_uart           = NULL;                     // 锟斤拷锟斤拷锟斤拷锟斤拷锟较拷拇锟斤拷锟�
    info->output_screen         = NULL;                     // 锟斤拷锟斤拷锟斤拷锟斤拷锟较拷锟斤拷锟侥�
    info->output_screen_clear   = NULL;                     // 锟斤拷幕锟斤拷锟斤拷
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟斤拷蠖ǔ锟绞硷拷锟�
// 锟斤拷锟斤拷说锟斤拷     *info       debug 锟斤拷锟斤拷锟斤拷锟较拷峁癸拷锟�
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_output_init(info);
// 锟斤拷注锟斤拷息     锟斤拷锟斤拷锟斤拷锟揭伙拷悴伙拷锟斤拷没锟斤拷锟斤拷锟�
//-------------------------------------------------------------------------------------------------------------------
void debug_output_init (debug_output_struct *info)
{
    debug_output_info.type_index            = info->type_index;

    debug_output_info.display_x_max         = info->display_x_max;
    debug_output_info.display_y_max         = info->display_y_max;

    debug_output_info.font_x_size           = info->font_x_size;
    debug_output_info.font_y_size           = info->font_y_size;

    debug_output_info.output_uart           = info->output_uart;
    debug_output_info.output_screen         = info->output_screen;
    debug_output_info.output_screen_clear   = info->output_screen_clear;

    zf_debug_init_flag = 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟�     debug 锟斤拷锟节筹拷始锟斤拷
// 锟斤拷锟斤拷说锟斤拷     void
// 锟斤拷锟截诧拷锟斤拷     void
// 使锟斤拷示锟斤拷     debug_init();
// 锟斤拷注锟斤拷息     锟斤拷源锟斤拷示锟斤拷默锟较碉拷锟斤拷 锟斤拷默锟较斤拷锟斤拷锟叫断斤拷锟斤拷
//-------------------------------------------------------------------------------------------------------------------
void debug_init (void)
{
    debug_output_struct info;                   // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷息锟结构锟斤拷
    debug_output_struct_init(&info);            // 锟斤拷始锟斤拷锟斤拷锟皆碉拷锟斤拷锟斤拷息
    info.output_uart = debug_uart_str_output;   // 锟斤拷锟矫讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
    debug_output_init(&info);

    uart_init(DEBUG_UART_INDEX,                 // 锟斤拷 zf_common_debug.h 锟叫查看锟斤拷应值
              DEBUG_UART_BAUDRATE,              // 锟斤拷 zf_common_debug.h 锟叫查看锟斤拷应值
              DEBUG_UART_TX_PIN,                // 锟斤拷 zf_common_debug.h 锟叫查看锟斤拷应值
              DEBUG_UART_RX_PIN);               // 锟斤拷 zf_common_debug.h 锟叫查看锟斤拷应值
#if DEBUG_UART_USE_INTERRUPT                                                    // 锟斤拷锟斤拷锟斤拷锟斤拷 只锟斤拷锟斤拷锟斤拷锟矫达拷锟斤拷锟叫断才憋拷锟斤拷
    fifo_init(&debug_uart_fifo, FIFO_DATA_8BIT, debug_uart_buffer, DEBUG_RING_BUFFER_LEN);
    uart_rx_interrupt(DEBUG_UART_INDEX, 1);                                     // 使锟杰讹拷应锟斤拷锟节斤拷锟斤拷锟叫讹拷
#endif
}



