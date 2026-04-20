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
 * 文件名：[ky_ips.c]
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
#include "ky_ips.h"
#include "ky_font.h"
#include "ky_delay.h"
#include "ky_gpio.h"
#include "ky_utils.h"
#include "ky_spi.h"
#include <stdio.h>

static IPS_Type_t gIPSCurrentType = IPS_TYPE_114;
static Color_t gIPSPenColor = BLACK;
static Color_t gIPSBackgroundColor = WHITE;
static IPS_Direction_t gIPSDisplayDirection = IPS_DIR_LANDSCAPE_180;
static uint16_t gIPSMaxWidth;
static uint16_t gIPSMaxHeight;
static uint16_t gIPSNativeWidth;
static uint16_t gIPSNativeHeight;

/***************************************************************
 *  @brief     通过SPI发送一个8位数据
 *  @param     dat     要发送的字节
 *  @Sample usage:     ips_write_data_8bit(0x55);
 **************************************************************/
static void ips_write_data_8bit(uint8_t dat)
{
    IPS_SET_CS(0);
    spi_write_byte(IPS_SPI_CHANNEL, dat);
    IPS_SET_CS(1);
}

/***************************************************************
 *  @brief     向屏幕发送一个命令
 *  @param     command     要发送的命令字节
 *  @Sample usage:     ips_write_command(0x2A);
 **************************************************************/
static void ips_write_command(uint8_t command)
{
    IPS_SET_DC(0);
    ips_write_data_8bit(command);
    IPS_SET_DC(1);
}

/***************************************************************
 *  @brief     通过SPI发送一个16位数据
 *  @param     dat     要发送的16位数据 (高字节在前)
 *  @Sample usage:     ips_write_data_16bit(0xFFFF);
 **************************************************************/
static void ips_write_data_16bit(uint16_t dat)
{
    IPS_SET_CS(0);
    spi_write_byte(IPS_SPI_CHANNEL, (uint8_t)(dat >> 8));
    spi_write_byte(IPS_SPI_CHANNEL, (uint8_t)dat);
    IPS_SET_CS(1);
}

/***************************************************************
 *  @brief     设置屏幕上将要进行绘图操作的窗口区域
 *  @param     x1      窗口左上角X坐标
 *  @param     y1      窗口左上角Y坐标
 *  @param     x2      窗口右下角X坐标
 *  @param     y2      窗口右下角Y坐标
 *  @Sample usage:     ips_set_address(0, 0, 100, 100);
 **************************************************************/
void ips_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    int offsetX = 0, offsetY = 0;

    if (x1 >= gIPSMaxWidth) x1 = gIPSMaxWidth - 1;
    if (x2 >= gIPSMaxWidth) x2 = gIPSMaxWidth - 1;
    if (y1 >= gIPSMaxHeight) y1 = gIPSMaxHeight - 1;
    if (y2 >= gIPSMaxHeight) y2 = gIPSMaxHeight - 1;

    if (x1 > x2)
    {
        uint16_t t = x1;
        x1 = x2;
        x2 = t;
    }
    if (y1 > y2)
    {
        uint16_t t = y1;
        y1 = y2;
        y2 = t;
    }

    switch (gIPSDisplayDirection)
    {
    case IPS_DIR_PORTRAIT:
        if (gIPSCurrentType == IPS_TYPE_114)
        {
            offsetX = 52;
            offsetY = 40;
        }
        break;
    case IPS_DIR_PORTRAIT_180:
        if (gIPSCurrentType == IPS_TYPE_114)
        {
            offsetX = 53;
            offsetY = 40;
        }
        break;
    case IPS_DIR_LANDSCAPE:
        if (gIPSCurrentType == IPS_TYPE_114)
        {
            offsetX = 40;
            offsetY = 53;
        }
        break;
    case IPS_DIR_LANDSCAPE_180:
        if (gIPSCurrentType == IPS_TYPE_114)
        {
            offsetX = 40;
            offsetY = 52;
        }
        break;
    }

    ips_write_command(0x2A);
    ips_write_data_16bit(x1 + offsetX);
    ips_write_data_16bit(x2 + offsetX);
    ips_write_command(0x2B);
    ips_write_data_16bit(y1 + offsetY);
    ips_write_data_16bit(y2 + offsetY);
    ips_write_command(0x2C);
}

/***************************************************************
  *  @brief     在指定坐标绘制一个像素点
  *  @param     x     点的X坐标
  *  @param     y     点的Y坐标
  *  @param     color 点的16位RGB565颜色
  *  @Sample usage:    ips_draw_point(10, 20, RED);
 **************************************************************/
void ips_draw_point(uint16_t x, uint16_t y, Color_t color)
{
    if (x >= gIPSMaxWidth || y >= gIPSMaxHeight) return;
    ips_set_address(x, y, x, y);
    ips_write_data_16bit(color);
}

/***************************************************************
  *  @brief     使用Bresenham算法在两点之间绘制一条直线
  *  @param     x1    起始点的X坐标
  *  @param     y1    起始点的Y坐标
  *  @param     x2    结束点的X坐标
  *  @param     y2    结束点的Y坐标
  *  @param     color 线的16位RGB565颜色
  *  @Sample usage:    ips_draw_line(0, 0, 50, 50, BLACK);
 **************************************************************/
void ips_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color_t color)
{
    int16_t dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int16_t dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int8_t sx = (x1 < x2) ? 1 : -1;
    int8_t sy = (y1 < y2) ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) / 2;
    int16_t e2;

    for (;;)
    {
        ips_draw_point(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy)  { err += dx; y1 += sy; }
    }
}

/***************************************************************
 *  @brief     填充矩形区域
 *  @param     x1      左上角X坐标
 *  @param     y1      左上角Y坐标
 *  @param     x2      右下角X坐标
 *  @param     y2      右下角Y坐标
 *  @param     color   填充颜色
 *  @Sample usage:    ips_fill_rectangle(0, 0, 50, 50, RED);
 **************************************************************/
void ips_fill_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color_t color)
{
    uint32_t i, totalPoints;

    if (x1 > x2)
    {
        uint16_t t = x1;
        x1 = x2;
        x2 = t;
    }
    if (y1 > y2)
    {
        uint16_t t = y1;
        y1 = y2;
        y2 = t;
    }

    if (x1 >= gIPSMaxWidth || y1 >= gIPSMaxHeight) return;
    if (x2 >= gIPSMaxWidth) x2 = gIPSMaxWidth - 1;
    if (y2 >= gIPSMaxHeight) y2 = gIPSMaxHeight - 1;

    ips_set_address(x1, y1, x2, y2);
    totalPoints = (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    for (i = 0; i < totalPoints; i++)
    {
        ips_write_data_16bit(color);
    }
}

/***************************************************************
 *  @brief     显示一个字符
 *  @param     x           X坐标
 *  @param     y           Y坐标
 *  @param     character   要显示的字符
 *  @Sample usage:    ips_show_char(10, 10, 'A');
 **************************************************************/
void ips_show_char(uint16_t x, uint16_t y, const char character)
{
    uint8_t i, j;
    uint8_t temp;

    if (x + 7 >= gIPSMaxWidth || y + 15 >= gIPSMaxHeight) return;
    if (character < ' ' || character > '~') return;

    ips_set_address(x, y, x + 7, y + 15);
    for (i = 0; i < 16; i++)
    {
        temp = ascii_font_8x16[character - ' '][i];
        for (j = 0; j < 8; j++)
        {
            if (temp & 0x01)
                ips_write_data_16bit(gIPSPenColor);
            else
                ips_write_data_16bit(gIPSBackgroundColor);
            temp >>= 1;
        }
    }
}
/***************************************************************
  *  @brief     在指定位置显示一个字符串
  *  @param     x     字符串左上角的X坐标
  *  @param     y     字符串左上角的Y坐标
  *  @param     pStr  要显示的以'\0'结尾的字符串
  *  @Sample usage:    ips200_show_string(10, 30, "Hello!");
 **************************************************************/
void ips_show_string(uint16_t x, uint16_t y, const char *pStr)
{
    while (*pStr != '\0')
    {
        if (x > gIPSMaxWidth - 8)
        {
            x = 0;
            y += 16;
        }
        if (y > gIPSMaxHeight - 16) break;
        ips_show_char(x, y, *pStr);
        x += 8;
        pStr++;
    }
}

/***************************************************************
  *  @brief     显示一个带符号整数
  *  @param     x        显示位置的X坐标
  *  @param     y        显示位置的Y坐标
  *  @param     num      要显示的整数
  *  @param     min_len  显示的最小位数（不足补空格）
  *  @Sample usage:    ips_show_int(10, 50, -123, 5);
 **************************************************************/
void ips_show_int(uint16_t x, uint16_t y, int32_t num, uint8_t min_len)
{
    char buf[12];
    uint8_t len;

    len = utils_int2_str(num, buf);
    while (len < min_len)
    {
        ips_show_char(x, y, ' ');
        x += 8;
        min_len--;
    }
    ips_show_string(x, y, buf);
}

/***************************************************************
  *  @brief     显示一个无符号长整数
  *  @param     x        显示位置的X坐标
  *  @param     y        显示位置的Y坐标
  *  @param     num      要显示的无符号整数
  *  @param     min_len  显示的最小位数（不足补空格）
  *  @Sample usage:    ips_show_u_int(10, 90, 4294967295, 10);
 **************************************************************/
void ips_show_u_int(uint16_t x, uint16_t y, uint32_t num, uint8_t min_len)
{
    char buf[12];
    uint8_t len;

    len = utils_u_int2_str(num, buf);
    while (len < min_len)
    {
        ips_show_char(x, y, ' ');
        x += 8;
        min_len--;
    }
    ips_show_string(x, y, buf);
}

/***************************************************************
  *  @brief     显示一个浮点数
  *  @param     x        显示位置的X坐标
  *  @param     y        显示位置的Y坐标
  *  @param     num      要显示的浮点数
  *  @param     min_len  整数部分最小位数（不足补空格）
  *  @param     frac     小数点后显示的位数 (0-6)
  *  @Sample usage:    ips_show_float(10, 70, 3.14, 2, 2);
 **************************************************************/
void ips_show_float(uint16_t x, uint16_t y, float num, uint8_t min_len, uint8_t frac)
{
    char buf[16];
    uint8_t len;

    len = utils_float2_str(num, buf, frac);
    while (len < min_len)
    {
        ips_show_char(x, y, ' ');
        x += 8;
        min_len--;
    }
    ips_show_string(x, y, buf);
}


/***************************************************************
 *  @brief     显示图片
 *  @param     x       X坐标
 *  @param     y       Y坐标
 *  @param     dat     图片数据指针
 *  @param     width   图片宽度
 *  @param     height  图片高度
 *  @Sample usage:    ips_show_image(0, 0, imgData, 100, 100);
 **************************************************************/
void ips_show_image(uint16_t x, uint16_t y, const uint8_t *dat, uint16_t width, uint16_t height)
{
    uint32_t totalPoints, i;
    if (x + width > gIPSMaxWidth || y + height > gIPSMaxHeight) return;

    ips_set_address(x, y, x + width - 1, y + height - 1);
    totalPoints = (uint32_t)width * height;

    IPS_SET_CS(0);
    for( i = 0; i < (uint32_t)width * height; i++)
    {
        // 手动交换字节发送
        spi_write_byte(IPS_SPI_CHANNEL, dat[i*2 + 1]); // 先发低字节
        spi_write_byte(IPS_SPI_CHANNEL, dat[i*2]);     // 后发高字节
    }
    IPS_SET_CS(1);
}

/***************************************************************
  *  @brief     显示灰度图像
  *  @param     x          显示位置的X坐标
  *  @param     y          显示位置的Y坐标
  *  @param     w          图像宽度
  *  @param     h          图像高度
  *  @param     pGrayData  灰度图像数据数组
  *  @Sample usage:    ips_show_gray_image(0, 0, 100, 100, imageData);
 **************************************************************/
void ips_show_gray_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *pGrayData)
{
    uint32_t totalPixels = (uint32_t)w * h;
    uint32_t processed = 0;
    uint16_t chunkPixels;
    uint16_t i;
    uint8_t gray;
    uint16_t rgb;
    uint8_t buffer[512];
    uint16_t bufferIdx = 0;

    if (x + w > gIPSMaxWidth || y + h > gIPSMaxHeight) return;

    ips_set_address(x, y, x + w - 1, y + h - 1);
    IPS_SET_CS(0);

    while (processed < totalPixels)
    {
        if ((totalPixels - processed) > 256)
        {
            chunkPixels = 256;
        }
        else
        {
            chunkPixels = (uint16_t)(totalPixels - processed);
        }

        bufferIdx = 0;
        for (i = 0; i < chunkPixels; i++)
        {
            gray = pGrayData[processed + i];
            rgb = ((uint16_t)(gray >> 3) << 11) | ((uint16_t)(gray >> 2) << 5) | (uint16_t)(gray >> 3);
            buffer[bufferIdx++] = (uint8_t)(rgb >> 8);
            buffer[bufferIdx++] = (uint8_t)(rgb & 0xFF);
        }

        spi_write_byte_array(IPS_SPI_CHANNEL, buffer, chunkPixels * 2);
        processed += chunkPixels;
    }

    IPS_SET_CS(1);
}

/***************************************************************
 *  @brief     清屏
 *  @param     backgroundColor 清屏使用的背景色
 *  @Sample usage:    ips_clear(WHITE);
 **************************************************************/
void ips_clear(Color_t backgroundColor)
{
    ips_fill_rectangle(0, 0, gIPSMaxWidth - 1, gIPSMaxHeight - 1, backgroundColor);
}

/***************************************************************
 *  @brief     设置显示方向
 *  @param     direction   显示方向枚举值
 *  @Sample usage:    ips_set_direction(IPS_DIR_LANDSCAPE);
 **************************************************************/
void ips_set_direction(IPS_Direction_t direction)
{
    gIPSDisplayDirection = direction;
    if (direction == IPS_DIR_LANDSCAPE || direction == IPS_DIR_LANDSCAPE_180)
    {
        gIPSMaxWidth = gIPSNativeHeight;
        gIPSMaxHeight = gIPSNativeWidth;
    }
    else
    {
        gIPSMaxWidth = gIPSNativeWidth;
        gIPSMaxHeight = gIPSNativeHeight;
    }
}

/***************************************************************
 *  @brief     显示摄像头采集的 RGB565 图像 (SPI 搬运)
 *  @param     x, y    起始坐标
 *  @param     width   图像宽度
 *  @param     height  图像高度
 *  @param     imgDat  图像数据指针 (RGB565格式)
 **************************************************************/
void ips_show_camera_image(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *imgDat)
{
    uint32_t i;
    uint32_t totalBytes = (uint32_t)width * height * 2; 

    ips_set_address(x, y, x + width - 1, y + height - 1);

    IPS_SET_CS(0);
    
    for(i = 0; i < totalBytes; i+=2)
    {
        spi_write_byte(IPS_SPI_CHANNEL, imgDat[i]);
        spi_write_byte(IPS_SPI_CHANNEL, imgDat[i+1]);
    }

    IPS_SET_CS(1);
}

/***************************************************************
 *  @brief     设置画笔和背景颜色
 *  @param     penColor        画笔颜色
 *  @param     backgroundColor 背景颜色
 *  @Sample usage:    ips_set_color(RED, WHITE);
 **************************************************************/
void ips_set_color(Color_t penColor, Color_t backgroundColor)
{
    gIPSPenColor = penColor;
    gIPSBackgroundColor = backgroundColor;
}

/***************************************************************
 *  @brief     初始化IPS屏幕
 *  @param     type    屏幕类型
 *  @Sample usage:    ips_init(IPS_TYPE_114);
 **************************************************************/
void ips_init(IPS_Type_t type)
{
    gpio_init(IPS_GPIO_DC,  GPIO_MODE_OUT_PP, GPIO_HIGH);
    gpio_init(IPS_GPIO_CS,  GPIO_MODE_OUT_PP, GPIO_HIGH);
    gpio_set_speed(IPS_GPIO_CS, GPIO_SPEED_FAST);
    
    gpio_init(IPS_GPIO_BLK, GPIO_MODE_OUT_PP, IPS_BLK_INACTIVE_LEVEL);
    gpio_init(IPS_GPIO_RST, GPIO_MODE_OUT_PP, GPIO_HIGH); 

    spi_init(IPS_SPI_CHANNEL, IPS_SPI_MODE, IPS_SPI_SPEED, IPS_SPI_MOSI, IPS_SPI_MISO, IPS_SPI_SCLK);

    gpio_set_level(IPS_GPIO_RST, GPIO_LOW);
    delay_ms(100);
    gpio_set_level(IPS_GPIO_RST, GPIO_HIGH);
    delay_ms(100);

    gIPSCurrentType = type;

    switch (gIPSCurrentType)
    {
    case IPS_TYPE_114:
        gIPSNativeWidth = 135;
        gIPSNativeHeight = 240;
        break;
    case IPS_TYPE_200:
        gIPSNativeWidth = 240;
        gIPSNativeHeight = 320;
        break;
    }

    ips_set_direction(gIPSDisplayDirection);
    ips_set_color(gIPSPenColor, gIPSBackgroundColor);

    ips_write_command(0x11);
    delay_ms(120);

    ips_write_command(0x36);
    delay_ms(120);
    if (gIPSDisplayDirection == IPS_DIR_PORTRAIT)
        ips_write_data_8bit(0x00);
    else if (gIPSDisplayDirection == IPS_DIR_PORTRAIT_180)
        ips_write_data_8bit(0xC0);
    else if (gIPSDisplayDirection == IPS_DIR_LANDSCAPE)
        ips_write_data_8bit(0x70);
    else
        ips_write_data_8bit(0xA0);

    ips_write_command(0x3A);
    ips_write_data_8bit(0x05);
    ips_write_command(0xB2);
    ips_write_data_8bit(0x0C);
    ips_write_data_8bit(0x0C);
    ips_write_data_8bit(0x00);
    ips_write_data_8bit(0x33);
    ips_write_data_8bit(0x33);
    ips_write_command(0xB7);
    ips_write_data_8bit(0x35);
    ips_write_command(0xBB);
    ips_write_data_8bit(0x19);
    ips_write_command(0xC0);
    ips_write_data_8bit(0x2C);
    ips_write_command(0xC2);
    ips_write_data_8bit(0x01);
    ips_write_command(0xC3);
    ips_write_data_8bit(0x12);
    ips_write_command(0xC4);
    ips_write_data_8bit(0x20);
    ips_write_command(0xC6);
    ips_write_data_8bit(0x0F);
    ips_write_command(0xD0);
    ips_write_data_8bit(0xA4);
    ips_write_data_8bit(0xA1);
    ips_write_command(0xE0);
    ips_write_data_8bit(0xD0);
    ips_write_data_8bit(0x04);
    ips_write_data_8bit(0x0D);
    ips_write_data_8bit(0x11);
    ips_write_data_8bit(0x13);
    ips_write_data_8bit(0x2B);
    ips_write_data_8bit(0x3F);
    ips_write_data_8bit(0x54);
    ips_write_data_8bit(0x4C);
    ips_write_data_8bit(0x18);
    ips_write_data_8bit(0x0D);
    ips_write_data_8bit(0x0B);
    ips_write_data_8bit(0x1F);
    ips_write_data_8bit(0x23);
    ips_write_command(0xE1);
    ips_write_data_8bit(0xD0);
    ips_write_data_8bit(0x04);
    ips_write_data_8bit(0x0C);
    ips_write_data_8bit(0x11);
    ips_write_data_8bit(0x13);
    ips_write_data_8bit(0x2C);
    ips_write_data_8bit(0x3F);
    ips_write_data_8bit(0x44);
    ips_write_data_8bit(0x51);
    ips_write_data_8bit(0x2F);
    ips_write_data_8bit(0x1F);
    ips_write_data_8bit(0x1F);
    ips_write_data_8bit(0x20);
    ips_write_data_8bit(0x23);
    ips_write_command(0x21);
    delay_ms(120);
    ips_write_command(0x29);

    ips_clear(gIPSBackgroundColor);
    gpio_set_level(IPS_GPIO_BLK, IPS_BLK_ACTIVE_LEVEL);
}
