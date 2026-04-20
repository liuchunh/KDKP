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
 * 文件名：[ky_font.h]
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
#ifndef KEYU_DEVICE_FONT_H_
#define KEYU_DEVICE_FONT_H_

#include "ky_typedef.h"

typedef enum {
    WHITE   = 0xFFFF, // 白色
    BLACK   = 0x0000, // 黑色
    BLUE    = 0x001F, // 蓝色
    PURPLE  = 0xF81F, // 紫色
    PINK    = 0xFE19, // 粉色
    RED     = 0xF800, // 红色
    MAGENTA = 0xF81F, // 品红
    GREEN   = 0x07E0, // 绿色
    CYAN    = 0x07FF, // 青色
    YELLOW  = 0xFFE0, // 黄色
    BROWN   = 0xBC40, // 棕色
    GRAY    = 0x8430, // 灰色
} Color_t;
extern const uint8_t ascii_font_6x8[][6];
extern const uint8_t ascii_font_8x16[][16];
#endif /* KEYU_DEVICE_FONT_H_ */
