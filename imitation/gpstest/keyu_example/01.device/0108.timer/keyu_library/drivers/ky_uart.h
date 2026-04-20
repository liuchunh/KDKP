/*********************************************************************************
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 版权所有：[2025] [北京科宇通博科技有限公司]
 *
 * 许可协议：采用 GNU GPL v3.0 开源许可
 * 您可依据协议进行二次开发、传播，但须保留原始版权信息
 * 协议详情参见：https://www.gnu.org/licenses/gpl-3.0.html
 *
 * 免责声明：本组件库仅提供技术参考，使用方需自行验证适用性
 *
 * 协议文件：GPL v3.0 完整文本位于根目录下
 *
 * === 文件信息 ===
 * 文件名：[ky_uart.h]
 * 开发单位：北京科宇通博科技有限公司
 * 适用环境：[AURIX TC264DA]
 * 官方渠道：
 *   - 代码仓库：[https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library]
 *   - 淘宝店铺：https://kyznc.taobao.com/
 *   - 技术支持：QQ群 974530818
 *
 * === 修订记录 ===
 * 日期       |  开发者  | 变更说明
 * -----------|----------|----------------------
 * 2026.01.07 |   毛毛   | V3.0
 *********************************************************************************/
#ifndef KEYU_LIB_INCLUDE_KEYU_DRIVERS_KEYU_DRIVER_UART_H_
#define KEYU_LIB_INCLUDE_KEYU_DRIVERS_KEYU_DRIVER_UART_H_
#include "Cpu/Std/Ifx_Types.h"
#include "Asclin/Asc/IfxAsclin_Asc.h"

#define UART_RX_BUFFER_SIZE         128     //接收缓冲区大小
#define UART_TX_BUFFER_SIZE         512    //发送缓冲区大小

//调试串口配置（用于printf重定向
#define DEBUG_UART_INDEX            UART_0  
#define DEBUG_UART_BAUDRATE         115200  
#define DEBUG_UART_RX_PIN           UART0_RXA_P14_1
#define DEBUG_UART_TX_PIN           UART0_TX_P14_0

//UART外设索引（对应ASCLIN模块）
typedef enum {
    UART_0 = 0,  
    UART_1 = 1,  
    UART_2 = 2,  
    UART_3 = 3   
} UART_Index_t;
//UART RX引脚选择
typedef enum {
    // ASCLIN0 RX
    UART0_RXA_P14_1 = 0 ,UART0_RXB_P15_3,   
    // ASCLIN1 RX
    UART1_RXA_P15_1     ,UART1_RXB_P15_5,   UART1_RXC_P20_9,   UART1_RXD_P14_8,   
    UART1_RXE_P11_10    ,UART1_RXF_P33_13,  UART1_RXG_P02_3,   
    // ASCLIN2 RX
    UART2_RXA_P14_3     ,UART2_RXB_P02_1,   UART2_RXD_P10_6,   UART2_RXE_P33_8,   
    UART2_RXG_P02_0,   
    // ASCLIN3 RX
    UART3_RXA_P15_7     ,UART3_RXC_P20_3,   UART3_RXD_P32_2,   UART3_RXE_P00_1,   
    UART3_RXF_P21_6    
} UART_Rx_Pin_t;
//UART TX引脚选择
typedef enum {
    // ASCLIN0 TX 
    UART0_TX_P14_0 = 0, UART0_TX_P14_1  , UART0_TX_P15_2  , UART0_TX_P15_3,     
    // ASCLIN1 TX
    UART1_TX_P02_2    , UART1_TX_P11_12 , UART1_TX_P14_10 , UART1_TX_P15_0,     
    UART1_TX_P15_1    , UART1_TX_P15_4  , UART1_TX_P15_5  , UART1_TX_P20_10,    
    UART1_TX_P33_12   , UART1_TX_P33_13 ,    
    // ASCLIN2 TX
    UART2_TX_P02_0    , UART2_TX_P10_5  , UART2_TX_P14_2  , UART2_TX_P14_3, 
    UART2_TX_P33_8    , UART2_TX_P33_9  , 
    // ASCLIN3 TX
    UART3_TX_P00_0    , UART3_TX_P00_1  , UART3_TX_P15_6  , UART3_TX_P15_7, 
    UART3_TX_P20_0    , UART3_TX_P20_3  , UART3_TX_P21_7  , UART3_TX_P32_2, 
    UART3_TX_P32_3      
} UART_Tx_Pin_t;
//UART回调函数类型
typedef void (*UART_Callback_t)(uint8 dat);
//UART句柄结构体
typedef struct {
    IfxAsclin_Asc asc;                         // ASCLIN ASC句柄
    uint8 txBuffer[UART_TX_BUFFER_SIZE];       // 发送缓冲区
    uint8 rxBuffer[UART_RX_BUFFER_SIZE];       // 接收缓冲区
    uint16 rxReadIndex;                        // 接收读指针
    uint16 rxWriteIndex;                       // 接收写指针
    UART_Callback_t rxCallback;                // 接收回调函数
} UartHandle_t;
extern UartHandle_t g_uartHandle[4];        //UART句柄数组（4个ASCLIN通道）

void uart_init(UART_Index_t uartIndex, uint32 baudRate, UART_Rx_Pin_t rxPin, UART_Tx_Pin_t txPin);  //UART初始化
void uart_send_string(UART_Index_t uartIndex, const char *str);                                     //UART发送字符串
void uart_send_byte(UART_Index_t uartIndex, uint8 dat);                                             //UART发送字节
void uart_send_buffer(UART_Index_t uartIndex, const uint8 *buffer, uint16 length);                  //UART发送缓冲区
boolean uart_read_byte(UART_Index_t uartIndex, uint8 *data);                                        //UART读取字节
uint16 uart_available(UART_Index_t uartIndex);                                                      //UART可用字节数
void uart_flush_rx(UART_Index_t uartIndex);                                                         //UART清空接收缓冲
void uart_set_callback(UART_Index_t uartIndex, UART_Callback_t cb);                                 //UART设置回调
void uart_process_isr(UART_Index_t uartIndex);                                                      //UART中断处理
int _putchar(int c);                                                                                //printf字符输出
void uart_debug_init(void);                                                                         //调试串口初始化
void uart_send_byte_bare(uint8 byte);                                                               //UART裸发送字节
void uart_send_string_bare(const char *str);                                                        //UART裸发送字符串
#endif /* KEYU_LIB_INCLUDE_KEYU_DRIVERS_KEYU_DRIVER_UART_H_ */
