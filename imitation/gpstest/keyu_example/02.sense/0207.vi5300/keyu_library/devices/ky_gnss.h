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
 * 文件名：[ky_gnss.h]
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
#ifndef _KEYU_DEVICE_GNSS_H_
#define _KEYU_DEVICE_GNSS_H_

#include "ky_typedef.h"
#include "ky_uart.h"

#define gnss_uart_index         UART_3
#define gnss_uart_baud          115200
#define gnss_rx_pin             UART3_RXA_P15_7
#define gnss_tx_pin             UART3_TX_P15_6

// 接收缓冲区大小 
#define gnss_rx_buf_size        128
// 队列深度
#define gnss_queue_depth        4

typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} Gnss_Time_t;
typedef struct {
    // 基础状态
    uint8_t         isValid;        // 定位是否有效 (1:有效, 0:无效)
    uint8_t         satellites;     // 卫星数量
    
    // 位置信息 (WGS84)
    double          latitude;       // 纬度 (单位: 度, 如 30.123456)
    double          longitude;      // 经度 (单位: 度, 如 120.123456)
    float           altitude;       // 海拔高度 (单位: 米)
    
    // 运动信息
    float           speed;          // 地面速率 (单位: km/h)
    float           course;         // 地面航向 (0.0~359.9度, 真北参考)
    
    // 定位精度
    float           hdop;           // 水平精度因子 (值越小精度越高, <1优秀, 1-2良好, 2-5中等, >5差)
    
    // 时间信息 (已转换为北京时间 UTC+8)
    // Gnss_Time_t   time;
} Gnss_Data_t;
// 全局GNSS数据 (定义在.c文件中)
extern Gnss_Data_t GnssData;
void gnss_init(void);                                                                       //GNSS初始化
uint8_t gnss_process(void);                                                                 //GNSS数据处理
void gnss_uart_callback(uint8_t idx);                                                       //GNSS串口回调
double gnss_get_distance(double lat1, double lon1, double lat2, double lon2);              //计算两点间距离
double gnss_get_azimuth(double lat1, double lon1, double lat2, double lon2);               //计算两点间方位角
uint8_t gnss_test_print(void);                                                              //GNSS测试打印
//=============================================================================
// NMEA 消息输出控制指令
//=============================================================================
// RMC - 推荐最小定位信息
#define gnss_cmd_rmc_on             "$qxcfgmsg,1,0,1"
#define gnss_cmd_rmc_off            "$qxcfgmsg,1,0,0"

// GGA - GPS定位信息
#define gnss_cmd_gga_on             "$qxcfgmsg,1,2,1"
#define gnss_cmd_gga_off            "$qxcfgmsg,1,2,0"

// GSA - GPS DOP和活动卫星
#define gnss_cmd_gsa_on             "$qxcfgmsg,1,3,1"
#define gnss_cmd_gsa_off            "$qxcfgmsg,1,3,0"

// GSV - 可见卫星信息
#define gnss_cmd_gsv_on             "$qxcfgmsg,1,4,1"
#define gnss_cmd_gsv_off            "$qxcfgmsg,1,4,0"

// GLL - 地理位置信息
#define gnss_cmd_gll_on             "$qxcfgmsg,1,5,1"
#define gnss_cmd_gll_off            "$qxcfgmsg,1,5,0"

// VTG - 地面速度信息
#define gnss_cmd_vtg_on             "$qxcfgmsg,1,1,1"
#define gnss_cmd_vtg_off            "$qxcfgmsg,1,1,0"

// GST - GPS伪距误差统计
#define gnss_cmd_gst_on             "$qxcfgmsg,1,7,1"
#define gnss_cmd_gst_off            "$qxcfgmsg,1,7,0"

// ATT - 姿态信息 (惯导产品支持)
#define gnss_cmd_att_on             "$qxcfgmsg,0,4,1"
#define gnss_cmd_att_off            "$qxcfgmsg,0,4,0"

// DRS - 航位推算 (惯导产品支持)
#define gnss_cmd_drs_on             "$qxcfgmsg,0,5,1"
#define gnss_cmd_drs_off            "$qxcfgmsg,0,5,0"

//=============================================================================
// 串口波特率设置指令
//=============================================================================
#define gnss_cmd_baud_9600          "$QXCFGPRT,0,0,,9600,h00000005,h00000003"
#define gnss_cmd_baud_115200        "$QXCFGPRT,0,0,,115200,h00000005,h00000003"

//=============================================================================
// 卫星系统选择指令
//=============================================================================
#define gnss_cmd_sat_beidou         "$qxcfggnss,hF,h1B,h01030909,h00000000"   // 单北斗
#define gnss_cmd_sat_all            "$qxcfggnss,h2,h1B,h00000900,h00000000"   // 全系统

//=============================================================================
// 定位频率设置指令
//=============================================================================
#define gnss_cmd_rate_1_hz           "$qxcfgrate,1000,1"
#define gnss_cmd_rate_2_hz           "$qxcfgrate,500,1"
#define gnss_cmd_rate_5_hz           "$qxcfgrate,200,1"
#define gnss_cmd_rate_10_hz          "$qxcfgrate,100,1"

//=============================================================================
// 启动指令
//=============================================================================
#define gnss_cmd_cold_start         "$QXCFGRST,1,2"  // 冷启动
#define gnss_cmd_hot_start          "$QXCFGRST,1,0"  // 热启动

//=============================================================================
// 配置保存与恢复
//=============================================================================
#define gnss_cmd_save_config        "$qxcfgsave"     // 保存配置
#define gnss_cmd_factory_reset      "$qxcfgclear"    // 恢复出厂设置

//=============================================================================

#endif
