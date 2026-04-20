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
 * 文件名：[ky_vi5300.h]
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
#ifndef __KEYU_DEVICE_VI5300_H__
#define __KEYU_DEVICE_VI5300_H__

#include "ky_typedef.h"
#include "ky_gpio.h"


#define vi5300_scl_pin  GPIO_P21_3
#define vi5300_sda_pin  GPIO_P21_2
#define vi5300_xsh_pin  GPIO_P21_4

typedef enum 
{
    vi5300_ok       = 0x00,
    vi5300_ranging  = 0x01,
    vi5300_busy     = 0x02,
    vi5300_bus_busy = 0x03,
    vi5300_sleep    = 0x04,
    vi5300_booting  = 0x05,
    vi5300_error    = 0x06
} vi5300_status_t;

typedef struct {
    uint16_t distance_mm;
    uint8_t  confidence;
    uint8_t  is_valid;
} vi5300_data_t;

vi5300_status_t vi5300_init(void);                      //VI5300初始化
vi5300_status_t vi5300_get_distance(vi5300_data_t *data);  //VI5300获取距离

#endif
