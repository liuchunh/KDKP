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
 * 文件名：[ky_tau1202.c]
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
#include "ky_tau1202.h"
#include "ky_config.h"
#include "ky_frame_queue.h"
#include "ky_uart.h"
#include "ky_delay.h"
#include "ky_math.h"
#include "ky_dma.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define ANGLE_TO_RAD(x) ((x) * KEYU_MATH_PI / 180.0)
#define RAD_TO_ANGLE(x) ((x) * 180.0 / KEYU_MATH_PI)

#define TAU1202_QUEUE_DEPTH  3  // 队列缓冲深度


TAU1202_Data_t TAU1202Data;

static uint8_t gDmaRxBuffer[1];

static uint8_t gGpsRmcBuffer[TAU1202_QUEUE_DEPTH][TAU1202_RX_BUF_SIZE];
static uint8_t gGpsGgaBuffer[TAU1202_QUEUE_DEPTH][TAU1202_RX_BUF_SIZE];

static FrameQueue_t gGpsRmcQueue;
static FrameQueue_t gGpsGgaQueue;   



 
 

 
 
 
 
 
 
 


/***************************************************************
  *  @brief     (内部) 字符转数值辅助函数
 **************************************************************/
static uint8_t hex_char_to_int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

/***************************************************************
  *  @brief     (内部) 异或校验函数
  *  @param     line 待校验的字符串 (含 $ 和 *)
  *  @return    1:校验通过, 0:失败
 **************************************************************/
static uint8_t tau1202_check_sum(char *line)
{
    char *pStar;
    uint8_t xorResult = 0;
    uint8_t checkSumVal = 0;
    uint8_t i;

    // 寻找 * 号
    pStar = strchr(line, '*');
    if (pStar == NULL) return 0;

    // 计算 $ 与 * 之间的异或和
    // line[0] 是 '$'，不参与计算
    for (i = 1; &line[i] < pStar; i++)
    {
        xorResult ^= line[i];
    }

    // 提取 * 后面的校验码
    if (*(pStar + 1) == '\0' || *(pStar + 2) == '\0') return 0;
    
    checkSumVal = (hex_char_to_int(*(pStar + 1)) << 4) | hex_char_to_int(*(pStar + 2));

    return (xorResult == checkSumVal) ? 1 : 0;
}

/***************************************************************
  *  @brief     (内部) NMEA 格式坐标转度格式
  *  @param     raw NMEA格式 (ddmm.mmmm)
  *  @return    度格式 (dd.ddddd)
 **************************************************************/
static double nmea_to_degree(double raw)
{
    int dd = (int)(raw / 100);
    double mm = raw - (dd * 100);
    return (double)dd + (mm / 60.0);
}

/***************************************************************
  *  @brief     (内部) 获取逗号分隔的字段指针
  *  @param     str 输入字符串
  *  @param     num 第几个逗号后的字段 (从1开始)
  *  @return    字段起始指针，若未找到则返回NULL
 **************************************************************/
static char* get_field_ptr(char *str, uint8_t num)
{
    uint8_t commaCount = 0;
    char *p = str;
    
    while (*p)
    {
        if (*p == ',')
        {
            commaCount++;
            if (commaCount == num)
            {
                return p + 1;
            }
        }
        p++;
    }
    return NULL;
}

/***************************************************************
  *  @brief     (内部) 解析 GNRMC 语句
 **************************************************************/
static void parse_gnrmc(char *line)
{
    char *p;

    // 字段2: 定位状态 (A=有效, V=无效)
    p = get_field_ptr(line, 2);
    if (p && *p == 'A')
    {
        TAU1202Data.isValid = 1;

        // 字段3: 纬度
        p = get_field_ptr(line, 3);
        if(p) TAU1202Data.latitude = nmea_to_degree(atof(p));

        // 字段4: 南北半球
        p = get_field_ptr(line, 4);
        if(p && *p == 'S') TAU1202Data.latitude = -TAU1202Data.latitude;

        // 字段5: 经度
        p = get_field_ptr(line, 5);
        if(p) TAU1202Data.longitude = nmea_to_degree(atof(p));

        // 字段6: 东西半球
        p = get_field_ptr(line, 6);
        if(p && *p == 'W') TAU1202Data.longitude = -TAU1202Data.longitude;

        // 字段7: 地面速率 (节 -> km/h)
        p = get_field_ptr(line, 7);
        if(p) TAU1202Data.speed = (float)atof(p) * 1.852f;

        // 字段8: 地面航向
        p = get_field_ptr(line, 8);
        if(p) TAU1202Data.course = (float)atof(p);
    }
    else
    {
        TAU1202Data.isValid = 0;
    }
}

/***************************************************************
  *  @brief     (内部) 解析 GNGGA 语句
 **************************************************************/
static void parse_gngga(char *line)
{
    char *p;

    // 字段7: 卫星数量
    p = get_field_ptr(line, 7);
    if (p) TAU1202Data.satellites = (uint8_t)atoi(p);

    // 字段9: 海拔高度
    p = get_field_ptr(line, 9);
    if (p) TAU1202Data.altitude = (float)atof(p);
}


/***************************************************************
  *  @brief     DMA回调函数 
 **************************************************************/
void tau1202_uart_callback(uint8_t idx)
{
    static uint8_t s_state = 0;   // 0:等待头部, 1:接收头部, 2:转发数据
    static uint8_t s_header[6];   
    static uint8_t s_idx = 0;
    static FrameQueue_t* s_currentQ = NULL; // 当前指向的队列
    uint8_t i;
    
    // 从DMA buffer读取一个字节
    uint8_t byte = gDmaRxBuffer[0];
    
    if(idx != TAU1202_UART_INDEX) return;

    if (byte == '$') 
    {
        s_state = 1;
        s_idx = 0;
        s_header[s_idx++] = byte;
        s_currentQ = NULL;
        // 重新启动DMA接收
        dma_uart_start_rx(TAU1202_UART_INDEX, gDmaRxBuffer, 1);
        return;
    }

    // 1 
    if (s_state == 1)
    {
        s_header[s_idx++] = byte;
        
        // $GNRMC || $GNGGA
        if (s_idx >= 6)
        {
            if (strstr((char*)s_header, "RMC"))
            {
                s_currentQ = &gGpsRmcQueue;
            }
            else if (strstr((char*)s_header, "GGA"))
            {
                s_currentQ = &gGpsGgaQueue;
            }
            else
            {
                s_currentQ = NULL; // 丢弃
            }

            if (s_currentQ != NULL)
            {
                for (i = 0; i < 6; i++)
                {
                    // 传入 0 表示不检测结束符
                    frame_queue_push_byte(s_currentQ, s_header[i], 0);
                }
            }
            
            s_state = 2; 
        }
    }
    // 2 
    else if (s_state == 2)
    {
        if (s_currentQ != NULL)
        {
            frame_queue_push_byte(s_currentQ, byte, '\n');
        }
        
        if (byte == '\n')
        {
            s_state = 0;
            s_currentQ = NULL;
        }
    }
    
    // 重新启动DMA接收
    dma_uart_start_rx(TAU1202_UART_INDEX, gDmaRxBuffer, 1);
}


/***************************************************************
  *  @brief     TAU1202处理
  *  @param     None
  *  @Sample usage:     tau1202_process();
 **************************************************************/
uint8_t tau1202_process(void)
{
    uint8_t isUpdated = 0;
    char* pFrame;

    pFrame = frame_queue_peek_latest(&gGpsRmcQueue);
    if (pFrame != NULL)
    {
        if (tau1202_check_sum(pFrame)) 
        {
            //printf("RMC Raw: %s", pFrame); 
            parse_gnrmc(pFrame);      
            isUpdated = 1;           
        }
        frame_queue_pop(&gGpsRmcQueue); 
    }

    pFrame = frame_queue_peek_latest(&gGpsGgaQueue);
    if (pFrame != NULL)
    {
        if (tau1202_check_sum(pFrame)) 
        {
            
            //printf("GAA Raw: %s", pFrame); 
            parse_gngga(pFrame);      
        }
        frame_queue_pop(&gGpsGgaQueue); 
    }
    
    return isUpdated;
}

/***************************************************************
  *  @brief     计算两点距离 (米)
 **************************************************************/
double tau1202_get_distance(double lat1, double lon1, double lat2, double lon2)
{
    const double EARTH_RADIUS = 6378137.0;
    double radLat1 = ANGLE_TO_RAD(lat1);
    double radLat2 = ANGLE_TO_RAD(lat2);
    double a = radLat1 - radLat2;
    double b = ANGLE_TO_RAD(lon1) - ANGLE_TO_RAD(lon2);

    double dst = 2 * asin(sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)));
    return dst * EARTH_RADIUS;
}

/***************************************************************
  *  @brief     计算两点方位角 (度)
 **************************************************************/
double tau1202_get_azimuth(double lat1, double lon1, double lat2, double lon2)
{
    double y, x, angle;
    
    lat1 = ANGLE_TO_RAD(lat1);
    lat2 = ANGLE_TO_RAD(lat2);
    lon1 = ANGLE_TO_RAD(lon1);
    lon2 = ANGLE_TO_RAD(lon2);

    x = sin(lon2 - lon1) * cos(lat2);
    y = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2 - lon1);

    angle = RAD_TO_ANGLE(atan2(x, y));

    if (angle < 0) angle += 360.0;
    return angle;
}


/***************************************************************
  *  @brief     初始化 TAU1202
 **************************************************************/

static void send_tau_cmd(const uint8_t *cmd, uint16_t size)
{
    uint8_t tx_temp_buf[64];
    if(size > 64) size = 64;
    memcpy(tx_temp_buf, cmd, size);
    dma_uart_start_tx(TAU1202_UART_INDEX, tx_temp_buf, size);
    delay_ms(50);
}

/***************************************************************
  *  @brief     初始化TAU1202 GPS模块
  *  @param     None
  *  @Sample usage:     tau1202_init();
 **************************************************************/
void tau1202_init(void)
{


    uart_init(TAU1202_UART_INDEX, TAU1202_UART_BAUD, TAU1202_RX_PIN, TAU1202_TX_PIN);

    frame_queue_init(&gGpsRmcQueue, (void*)gGpsRmcBuffer, TAU1202_RX_BUF_SIZE, TAU1202_QUEUE_DEPTH);
    frame_queue_init(&gGpsGgaQueue, (void*)gGpsGgaBuffer, TAU1202_RX_BUF_SIZE, TAU1202_QUEUE_DEPTH); 

    delay_ms(500); 

    

    send_tau_cmd(CMD_GLL_OFF, CMD_GLL_OFF_size);
    send_tau_cmd(CMD_GSA_OFF, CMD_GSA_OFF_size);
    send_tau_cmd(CMD_GSV_OFF, CMD_GSV_OFF_size);
    send_tau_cmd(CMD_VTG_OFF, CMD_VTG_OFF_size);
    send_tau_cmd(CMD_ZDA_OFF, CMD_ZDA_OFF_size);
    send_tau_cmd(CMD_GST_OFF, CMD_GST_OFF_size);
    send_tau_cmd(CMD_GNTXT_OFF, CMD_GNTXT_OFF_size);

    send_tau_cmd(CMD_RMC_ON, CMD_RMC_ON_size); 
    send_tau_cmd(CMD_GGA_ON, CMD_GGA_ON_size); 


    send_tau_cmd(CMD_RATE_10HZ, CMD_RATE_10HZ_size);

    dma_set_callback(DMA_TYPE_UART_RX, TAU1202_UART_INDEX, tau1202_uart_callback);
    dma_uart_start_rx(TAU1202_UART_INDEX, gDmaRxBuffer, 1);
}
