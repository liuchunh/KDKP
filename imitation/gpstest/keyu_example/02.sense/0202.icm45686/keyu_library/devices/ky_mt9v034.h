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
 * 文件名：[ky_mt9v034.h]
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
#ifndef __KY_MT9V034_H__
#define __KY_MT9V034_H__

#include "ky_typedef.h"
#include "ky_gpio.h"

#define mt9_v034_iic_scl                 GPIO_P02_3                          // SCL引脚
#define mt9_v034_iic_sda                 GPIO_P02_2                          // SDA引脚
#define mt9_v034_err_none                0                                   // 正常
#define mt9_v034_err_id                  1                                   // ID不匹配
#define mt9_v034_iic_delay               (2000)                              // I2C延时 (大幅增加延时以排除时序问题)
#define mt9_v034_iic_addr                (0x5C)                              // mt9_v034 I2C地址 (7位地址, 写地址0xB8>>1)
#define mt9_v034_data_pin                GPIO_P00                            // 数据引脚 D0-D7 (P00_0 - P00_7)
#define mt9_v034_pclk_pin                GPIO_P02_1                          // 像素时钟引脚
#define mt9_v034_vsync_pin               GPIO_P02_0                          // 场同步引脚
#define mt9_v034_data_add                ((uint8 *)(&IfxPort_getAddress((IfxPort_Index)(mt9_v034_data_pin/32))->IN + (mt9_v034_data_pin%32)/8))
#define mt9_v034_height                  (120)                               // 图像高度
#define mt9_v034_width                   (188)                               // 图像宽度
#define mt9_v034_image_size              (mt9_v034_height * mt9_v034_width)    // 图像数据缓冲区大小
#define mt9_v034_dma_channel             (IfxDma_ChannelId_5)                // DMA通道号

// MT9V034 默认配置宏定义 (可在编译前通过 #define 覆盖)
#ifndef mt9_v034_cfg_auto_exp_def
#define mt9_v034_cfg_auto_exp_def        (0)                                 // 自动曝光/增益默认值 (0=关闭, 1=开启)
#endif

#ifndef mt9_v034_cfg_fps_def
#define mt9_v034_cfg_fps_def             (80)                                // 默认帧率
#endif

#ifndef mt9_v034_cfg_exp_time
#define mt9_v034_cfg_exp_time            (480)                               // 默认曝光时间 (行数), 范围 1 ~ 480
#endif

#ifndef mt9_v034_cfg_gain
#define mt9_v034_cfg_gain                (32)                                // 默认模拟增益 (16=1x, 32=2x, 64=4x), 范围 16 ~ 64
#endif

#ifndef mt9_v034_cfg_target_brightness
#define mt9_v034_cfg_target_brightness   (60)                                // 自动曝光的目标亮度 (范围 1~64)
#endif

#include "ky_config.h"

extern uint8 mt9_v034__image[mt9_v034_height][mt9_v034_width];                  // 图像缓冲区

uint8 mt9_v034_init(void);                          //MT9V034初始化
void mt9_v034_set_exposure(uint16 exposure);        //设置曝光时间
void mt9_v034_set_gain(uint16 gain);                //设置增益
uint8 mt9_v034_get_image(void);                     //获取图像
uint16 mt9v034_read_reg(uint8 reg);                 //读取寄存器
void mt9v034_write_reg(uint8 reg, uint16 data);     //写入寄存器
void mt9_v034_vsync_handler(void);                  //场同步中断处理
void mt9_v034_dma_handler(void);                    //DMA中断处理
#endif /* __KY_MT9V034_H__ */
