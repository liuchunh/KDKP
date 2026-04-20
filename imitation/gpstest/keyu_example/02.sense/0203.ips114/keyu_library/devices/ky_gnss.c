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
 * 文件名：[ky_gnss.c]
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
#include "ky_gnss.h"
#include "ky_ringbuffer.h"
#include "ky_uart.h"
#include "ky_delay.h"
#include "ky_math.h"
#include "ky_dma.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define ANGLE_TO_RAD(x) ((x) * KEYU_MATH_PI / 180.0)
#define RAD_TO_ANGLE(x) ((x) * 180.0 / KEYU_MATH_PI)

// 缓冲区大小 (必须是2的幂次)
#define gnss_buffer_size    256

// 全局GNSS数据
Gnss_Data_t GnssData;

// DMA接收缓冲区
static uint8_t GnssRxBuffer[1];

// RingBuffer相关
static ringbuffer gnss_fifo;                            // 环形缓冲区
static uint8_t gnss_fifo_buffer[gnss_buffer_size];      // 底层缓冲区

// RMC和GGA语句缓冲区
static uint8_t gnss_rmc_buffer[128];
static uint8_t gnss_gga_buffer[128];

// 语句状态标志
static volatile uint8_t gnss_rmc_ready = 0;     // RMC语句就绪
static volatile uint8_t gnss_gga_ready = 0;     // GGA语句就绪

// 状态机相关
static uint8_t gnss_state = 0;      // 0:等待$, 1:接收头部, 2:接收RMC, 3:接收GGA
static uint8_t gnss_header[6];
static uint8_t gnss_header_idx = 0;

/***************************************************************
  *  @brief     (内部) 字符转数值辅助函数
 **************************************************************/
/* Unused function

  *  @brief     HEXCHARTO整数
  *  @param     c     [c description]
  *  @Sample usage:     hex_char_to_int(c);

static uint8_t hex_char_to_int(uint8_t c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    
    return 0;
}
*/

/***************************************************************
  *  @brief     NMEA校验和验证
  *  @param     sentence    NMEA语句 (以$开头)
  *  @return    1: 校验通过, 0: 校验失败
 **************************************************************/
static uint8_t gnss_verify_checksum(const char* sentence)
{
    uint8_t checksum = 0;
    const char* p = sentence;
    char* asterisk;
    uint8_t received_checksum;
    char c1, c2;
    uint8_t h1, h2;
    
    // 跳过$符号
    if (*p == '$') p++;
    
    // 计算校验和 (XOR所有字符直到*)
    while (*p && *p != '*')
    {
        checksum ^= *p++;
    }
    
    // 查找*号并解析校验和
    asterisk = strstr(sentence, "*");
    if (asterisk == NULL) return 0;
    
    // 手动解析两位十六进制校验和
    c1 = asterisk[1];
    c2 = asterisk[2];
    
    if (c1 >= '0' && c1 <= '9') h1 = c1 - '0';
    else if (c1 >= 'A' && c1 <= 'F') h1 = c1 - 'A' + 10;
    else if (c1 >= 'a' && c1 <= 'f') h1 = c1 - 'a' + 10;
    else return 0;
    
    if (c2 >= '0' && c2 <= '9') h2 = c2 - '0';
    else if (c2 >= 'A' && c2 <= 'F') h2 = c2 - 'A' + 10;
    else if (c2 >= 'a' && c2 <= 'f') h2 = c2 - 'a' + 10;
    else return 0;
    
    received_checksum = (h1 << 4) | h2;
    
    return (checksum == received_checksum) ? 1 : 0;
}

/***************************************************************
  *  @brief     获取NMEA语句中的指定字段
  *  @param     sentence    NMEA语句
  *  @param     fieldIndex  字段索引 (从0开始)
  *  @param     buffer      输出缓冲区
  *  @param     bufSize     缓冲区大小
  *  @return    指向buffer的指针，失败返回NULL
 **************************************************************/
static char* gnss_get_field(const char* sentence, uint8_t fieldIndex, char* buffer, uint8_t bufSize)
{
    uint8_t currentField = 0;
    const char* start = sentence;
    const char* end;
    uint8_t len;
    
    buffer[0] = '\0';
    
    // 跳过消息ID前的$符号
    if (*start == '$') start++;
    
    // 查找目标字段
    while (currentField < fieldIndex)
    {
        start = strstr(start, ",");
        if (start == NULL) return NULL;
        start++;  // 跳过逗号
        currentField++;
    }
    
    // 找到字段结束位置
    end = strstr(start, ",");
    if (end == NULL)
    {
        end = strstr(start, "*");
        if (end == NULL)
        {
            end = start + strlen(start);
        }
    }
    
    // 复制字段内容
    len = (uint8_t)(end - start);
    if (len >= bufSize) len = bufSize - 1;
    
    memcpy(buffer, start, len);
    buffer[len] = '\0';
    
    return buffer;
}

/***************************************************************
  *  @brief     解析坐标值 (度分格式 -> 十进制度)
  *  @param     str         坐标字符串 (如 "2840.852404")
  *  @param     direction   方向 (N/S/E/W)
  *  @return    十进制度数
 **************************************************************/
static double gnss_parse_coordinate(const char* str, char direction)
{
    double value;
    double degrees, minutes;
    int intDegrees;
    
    if (str == NULL || str[0] == '\0') return 0.0;
    
    value = atof(str);
    
    // 提取度数部分
    intDegrees = (int)(value / 100);
    
    degrees = (double)intDegrees;
    minutes = value - (degrees * 100);
    
    // 度分转十进制度
    value = degrees + (minutes / 60.0);
    
    // 南纬和西经为负
    if (direction == 'S' || direction == 'W')
    {
        value = -value;
    }
    
    return value;
}

/***************************************************************
  *  @brief     解析GNRMC语句
  *  @param     sentence    完整的RMC语句
  *  @return    1: 解析成功, 0: 失败
 **************************************************************/
static uint8_t gnss_parse_rmc(const char* sentence)
{
    char field[20];
    char latDir, lonDir;
    
    // 验证校验和
    if (!gnss_verify_checksum(sentence)) return 0;
    
    // 字段2: 定位状态 (A=有效, V=无效)
    if (gnss_get_field(sentence, 2, field, sizeof(field)) == NULL) return 0;
    GnssData.isValid = (field[0] == 'A') ? 1 : 0;
    
    if (!GnssData.isValid) return 1;  // 无效定位也算解析成功
    
    // 字段4: 纬度方向
    if (gnss_get_field(sentence, 4, field, sizeof(field)) == NULL) return 0;
    latDir = field[0];
    
    // 字段6: 经度方向
    if (gnss_get_field(sentence, 6, field, sizeof(field)) == NULL) return 0;
    lonDir = field[0];
    
    // 转换坐标
    GnssData.latitude = gnss_parse_coordinate(gnss_get_field(sentence, 3, field, sizeof(field)), latDir);
    GnssData.longitude = gnss_parse_coordinate(gnss_get_field(sentence, 5, field, sizeof(field)), lonDir);
    
    // 字段7: 速度 (节 -> km/h)
    if (gnss_get_field(sentence, 7, field, sizeof(field)))
    {
        GnssData.speed = (float)(atof(field) * 1.852);
    }
    
    // 字段8: 航向
    if (gnss_get_field(sentence, 8, field, sizeof(field)))
    {
        GnssData.course = (float)atof(field);
    }
    
    return 1;
}

/***************************************************************
  *  @brief     解析GNGGA语句
  *  @param     sentence    完整的GGA语句
  *  @return    1: 解析成功, 0: 失败
 **************************************************************/
static uint8_t gnss_parse_gga(const char* sentence)
{
    char field[20];
    
    // 验证校验和
    if (!gnss_verify_checksum(sentence)) return 0;
    
    // 字段6: 定位质量 (0=无效, 1=GPS, 2=DGPS)
    if (gnss_get_field(sentence, 6, field, sizeof(field)))
    {
        if (field[0] == '0')
        {
            GnssData.isValid = 0;
        }
    }
    
    // 字段7: 使用的卫星数量
    if (gnss_get_field(sentence, 7, field, sizeof(field)))
    {
        GnssData.satellites = (uint8_t)atoi(field);
    }
    
    // 字段8: HDOP
    if (gnss_get_field(sentence, 8, field, sizeof(field)))
    {
        GnssData.hdop = (float)atof(field);
    }
    
    // 字段9: 海拔高度
    if (gnss_get_field(sentence, 9, field, sizeof(field)))
    {
        GnssData.altitude = (float)atof(field);
    }
    
    return 1;
}

/***************************************************************
  *  @brief     计算两点间距离 (Haversine公式)
 **************************************************************/
double gnss_get_distance(double lat1, double lon1, double lat2, double lon2)
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
  *  @brief     计算两点间方位角
 **************************************************************/
double gnss_get_azimuth(double lat1, double lon1, double lat2, double lon2)
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
  *  @brief     DMA接收回调函数 (使用RingBuffer)
  *  @param     idx    DMA通道索引 (未使用，因为固定是GNSS串口)
 **************************************************************/
void gnss_uart_callback(uint8_t dat)
{
    uint32_t len;
    static uint32_t byte_count = 0;
    static uint8_t raw_buffer[100];
    static uint8_t raw_idx = 0;
        
    byte_count++;
    
    // 收集原始字节用于调试打印
    raw_buffer[raw_idx++] = dat;
    
    // 每1000字节打印一次统计(已完全注释避免阻塞)
    // if(byte_count % 1000 == 0) {
    //     printf("[GNSS] Received %u bytes total\n", (unsigned int)byte_count);
    //     raw_idx = 0;
    // }
    if(raw_idx >= 100) raw_idx = 0;  // 防止溢出
    
    // 检测到$符号，开始新帧
    if (dat == '$')
    {
        // 帧检测(已注释以减少输出)
        // static uint32_t frame_count = 0;
        // frame_count++;
        gnss_state = 1;
        gnss_header_idx = 0;
        gnss_header[gnss_header_idx++] = dat;
        ringbuffer_clear(&gnss_fifo);
        ringbuffer_putc(&gnss_fifo, dat);
    }
    // 状态1: 接收头部 (共6字节: $GNRMC 或 $GNGGA)
    else if (gnss_state == 1)
    {
        gnss_header[gnss_header_idx++] = dat;
        ringbuffer_putc(&gnss_fifo, dat);
        
        if (gnss_header_idx >= 6)
        {
            // 判断语句类型
            if (gnss_header[3] == 'R' && gnss_header[4] == 'M' && gnss_header[5] == 'C')
            {
                // printf("[GNSS] RMC sentence detected\n");  // 注释以减少输出
                gnss_state = 2;  // RMC语句
            }
            else if (gnss_header[3] == 'G' && gnss_header[4] == 'G' && gnss_header[5] == 'A')
            {
                // printf("[GNSS] GGA sentence detected\n");  // 注释以减少输出
                gnss_state = 3;  // GGA语句
            }
            else
            {
                // 打印未识别的语句头 (已注释 - 中断中禁止printf!)
                // printf("[GNSS] Unknown sentence: %c%c%c%c%c%c\n", 
                //        gnss_header[0], gnss_header[1], gnss_header[2],
                //        gnss_header[3], gnss_header[4], gnss_header[5]);
                gnss_state = 0;  // 其他语句，丢弃
                ringbuffer_clear(&gnss_fifo);
            }
        }
    }
    // 状态2: 接收RMC语句
    else if (gnss_state == 2)
    {
        ringbuffer_putc(&gnss_fifo, dat);
        
        // 检测到换行符，语句接收完成
        if (dat == '\n')
        {
            len = ringbuffer_len(&gnss_fifo);
            // printf("[GNSS] RMC complete, length=%u\n", (unsigned int)len);  // 注释以减少输出
            if (len < 128)
            {
                ringbuffer_read(&gnss_fifo, gnss_rmc_buffer, len);
                gnss_rmc_buffer[len] = '\0';
                // printf("[GNSS] RMC raw: %s", (char*)gnss_rmc_buffer);  // 注释避免阻塞
                gnss_rmc_ready = 1;
            }
            else
            {
                // printf("[GNSS] RMC too long, discarded\n");  // 注释 - 中断中禁止printf!
            }
            gnss_state = 0;
            ringbuffer_clear(&gnss_fifo);
        }
    }
    // 状态3: 接收GGA语句
    else if (gnss_state == 3)
    {
        ringbuffer_putc(&gnss_fifo, dat);
        
        if (dat == '\n')
        {
            len = ringbuffer_len(&gnss_fifo);
            // printf("[GNSS] GGA complete, length=%u\n", (unsigned int)len);  // 注释以减少输出
            if (len < 128)
            {
                ringbuffer_read(&gnss_fifo, gnss_gga_buffer, len);
                gnss_gga_buffer[len] = '\0';
                // printf("[GNSS] GGA raw: %s", (char*)gnss_gga_buffer);  // 注释避免阻塞
                gnss_gga_ready = 1;
            }
            else
            {
                // printf("[GNSS] GGA too long, discarded\n");  // 注释 - 中断中禁止printf!
            }
            gnss_state = 0;
            ringbuffer_clear(&gnss_fifo);
        }
    }
}

/***************************************************************
  *  @brief     Gnss解析处理函数
  *  @return    uint8_t 1: 成功解析新数据, 0: 无新数据
 **************************************************************/
uint8_t gnss_process(void)
{
    uint8_t result = 0;
    static uint32_t call_count = 0;
    
    call_count++;
    // 降低打印频率,避免阻塞 (已完全注释)
    // if(call_count % 10000 == 0) {
    //     printf("[GNSS] Process called %u times\n", (unsigned int)call_count);
    // }
    
    // 解析RMC语句
    if (gnss_rmc_ready)
    {
        gnss_rmc_ready = 0;
        // printf("[GNSS] Parsing RMC: %s", (char*)gnss_rmc_buffer);  // 原始数据太长,注释避免阻塞
        if (gnss_parse_rmc((char*)gnss_rmc_buffer))
        {
            // printf("[GNSS] RMC OK\n");  // 注释避免阻塞
            result = 1;
        }
    }
    
    // 解析GGA语句
    if (gnss_gga_ready)
    {
        gnss_gga_ready = 0;
        // printf("[GNSS] Parsing GGA: %s", (char*)gnss_gga_buffer);  // 原始数据太长,注释避免阻塞
        if (gnss_parse_gga((char*)gnss_gga_buffer))
        {
            // printf("[GNSS] GGA OK\n");  // 注释避免阻塞
            result = 1;
        }
    }
    
    return result;
}

/***************************************************************
  *  @brief     测试函数 - 直接打印原始帧数据
  *  @return    uint8_t 1: 有数据打印, 0: 无数据
  *  @note      用于调试，不解析数据，直接用printf打印
 **************************************************************/
uint8_t gnss_test_print(void)
{
    uint8_t result = 0;
    
    // 检查RMC语句
    if (gnss_rmc_ready)
    {
        gnss_rmc_ready = 0;
        printf("[RMC] %s", (char*)gnss_rmc_buffer);
        result = 1;
    }
    
    // 检查GGA语句
    if (gnss_gga_ready)
    {
        gnss_gga_ready = 0;
        printf("[GGA] %s", (char*)gnss_gga_buffer);
        result = 1;
    }
    
    return result;
}

/***************************************************************
  *  @brief     初始化 Gnss 模块
 **************************************************************/
void gnss_init(void)
{
    printf("[GNSS] Initializing GNSS module...\n");
    
    // 初始化GNSS数据结构
    memset(&GnssData, 0, sizeof(Gnss_Data_t));
    
    // 初始化状态机
    gnss_state = 0;
    gnss_header_idx = 0;
    gnss_rmc_ready = 0;
    gnss_gga_ready = 0;
    
    printf("[GNSS] Initializing ring buffer...\n");
    // 初始化环形缓冲区
    ringbuffer_init(&gnss_fifo, gnss_fifo_buffer, gnss_buffer_size);
    printf("[GNSS] Ring buffer initialized!\n");
    
    printf("[GNSS] Initializing UART%d at %d baud, RX=P15.7, TX=P15.6\n", 
           gnss_uart_index, gnss_uart_baud);
    // 初始化串口
    uart_init(gnss_uart_index, gnss_uart_baud, gnss_rx_pin, gnss_tx_pin);
    
    delay_ms(100);
    
    printf("[GNSS] Setting UART RX callback (using interrupt instead of DMA)...\n");
    // 使用 UART 中断回调代替 DMA
    uart_set_callback(gnss_uart_index, gnss_uart_callback);
    
    printf("[GNSS] Initialization complete!\n");
}
