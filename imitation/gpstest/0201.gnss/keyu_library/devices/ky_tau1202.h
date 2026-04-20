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
 * 文件名：[ky_tau1202.h]
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
#ifndef __KY_TAU1202_H__
#define __KY_TAU1202_H__

#include "ky_typedef.h"
#include "ky_uart.h" // For UART_3 definition

#define TAU1202_UART_INDEX      UART_3
#define TAU1202_UART_BAUD       115200
#define TAU1202_RX_PIN          UART3_RXA_P15_7
#define TAU1202_TX_PIN          UART3_TX_P15_6

// 接收缓冲区大小
#define TAU1202_RX_BUF_SIZE     64

typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} TAU1202_Time_t;
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
    
} TAU1202_Data_t;
extern TAU1202_Data_t TAU1202Data;
void tau1202_init(void);                                                                    //TAU1202初始化
uint8_t tau1202_process(void);                                                              //TAU1202数据处理
void tau1202_uart_callback(uint8_t idx);                                                    //TAU1202串口回调
double tau1202_get_distance(double lat1, double lon1, double lat2, double lon2);           //计算两点间距离
double tau1202_get_azimuth(double lat1, double lon1, double lat2, double lon2);            //计算两点间方位角
#endif /* __KY_TAU1202_H__ */