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
 * 文件名：[ky_ips.h]
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


/* IPS200 Pin Mapping (Board: xxx)
 * Signal   Macro          MCU Pin   Direction   Note
 * CS       IPS_GPIO_CS    P15_2     OUT         SPI chip select
 * DC       IPS_GPIO_DC    P15_0     OUT         0=cmd,1=data
 * RST      IPS_GPIO_RST   P15_1     OUT         active low reset
 * BLK      IPS_GPIO_BLK   P15_4     OUT         backlight enable
 * MOSI     IPS_SPI_MOSI   P15_5     AF/SPI      SPI data out
 * MISO     IPS_SPI_MISO   Pxx_x     AF/SPI      optional
 * SCLK     IPS_SPI_SCLK   P15_3     AF/SPI      SPI clock
 */





#ifndef __KY_IPS_H__
#define __KY_IPS_H__

#include "ky_font.h"
#include "ky_typedef.h"
#include "ky_dma.h"
#include "ky_gpio.h"
#include "ky_spi.h"
#include "ky_utils.h"

#define IPS_SPI_CHANNEL     SPI_2                             // SPI通道
#define IPS_SPI_SPEED       50000000                          // SPI速度 (65M太高会导致噪点/闪屏，回退到50M)
#define IPS_SPI_MODE        SPI_MODE0                         // SPI模式
#define IPS_SPI_MOSI        SPI2_MOSI_P15_5                   // SPI MOSI引脚
#define IPS_SPI_MISO        SPI_MISO_NONE                     // SPI MISO引脚 (未使用，避免与GPS UART冲突)
#define IPS_SPI_SCLK        SPI2_SCLK_P15_3                   // SPI SCLK引脚

#define IPS_GPIO_RST        GPIO_P15_1                        // RST引脚
#define IPS_GPIO_DC         GPIO_P15_0                        // DC引脚
#define IPS_GPIO_CS         GPIO_P15_2                        // CS引脚
#define IPS_GPIO_BLK        GPIO_P15_4                        // BLK引脚

#define IPS_SET_DC(x)       gpio_set_level(IPS_GPIO_DC, x)    // 设置DC引脚
#define IPS_SET_CS(x)       gpio_set_level(IPS_GPIO_CS, x)    // 设置CS引脚

typedef enum {
    IPS_DIR_PORTRAIT,      // 竖屏 (默认)
    IPS_DIR_PORTRAIT_180,  // 竖屏旋转180度
    IPS_DIR_LANDSCAPE,     // 横屏
    IPS_DIR_LANDSCAPE_180, // 横屏旋转180度
} IPS_Direction_t;
typedef enum {
    IPS_TYPE_114, // 1.14英寸屏幕
    IPS_TYPE_200, // 2.0英寸屏幕
} IPS_Type_t;
void ips_set_color(Color_t penColor, Color_t backgroundColor);                                      //设置颜色
void ips_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);                            //设置地址窗口
void ips_init(IPS_Type_t type);                                                                      //IPS屏幕初始化
void ips_clear(Color_t backgroundColor);                                                             //清屏
void ips_show_gray_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *pGrayData); //显示灰度图像
void ips_show_camera_image(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *imgDat);  //显示摄像头图像
void ips_draw_point(uint16_t x, uint16_t y, Color_t color);                                         //画点
void ips_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color_t color);              //画线
void ips_fill_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color_t color);         //填充矩形
void ips_show_char(uint16_t x, uint16_t y, const char character);                                   //显示字符
void ips_show_image(uint16_t x, uint16_t y, const uint8_t *dat, uint16_t width, uint16_t height);   //显示图像
void ips_show_string(uint16_t x, uint16_t y, const char *pStr);                                     //显示字符串
void ips_show_int(uint16_t x, uint16_t y, int32_t num, uint8_t min_len);                            //显示整数
void ips_show_u_int(uint16_t x, uint16_t y, uint32_t num, uint8_t min_len);                         //显示无符号整数
void ips_show_float(uint16_t x, uint16_t y, float num, uint8_t min_len, uint8_t frac);              //显示浮点数
void ips_set_direction(IPS_Direction_t direction);                                                  //设置屏幕方向
#endif /* __KY_IPS_H__ */
