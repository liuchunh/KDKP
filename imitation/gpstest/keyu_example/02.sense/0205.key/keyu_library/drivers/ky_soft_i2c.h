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
 * 文件名：[ky_soft_i2c.h]
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
#ifndef __KY_SOFT_I2C_H__
#define __KY_SOFT_I2C_H__

#include "ky_typedef.h"
#include "ky_gpio.h"

// 软件I2C信息结构体
typedef struct {
    GPIO_Pin_t  scl_pin;    // SCL引脚
    GPIO_Pin_t  sda_pin;    // SDA引脚
    uint8       addr;       // 器件地址 (7位地址模式)
    uint32      delay;      // 软延时时长
} SoftI2c_Info_t;
void    soft_i2c_start(SoftI2c_Info_t *i2c);                                                //软I2C启动信号
void    soft_i2c_stop(SoftI2c_Info_t *i2c);                                                 //软I2C停止信号
uint8   soft_i2c_wait_ack(SoftI2c_Info_t *i2c);                                             //软I2C等待应答
uint8   soft_i2c_send_byte(SoftI2c_Info_t *i2c, const uint8 data);                          //软I2C发送字节
uint8   soft_i2c_read_byte(SoftI2c_Info_t *i2c, uint8 ack);                                 //软I2C读取字节
void    soft_i2c_write_reg8(SoftI2c_Info_t *i2c, const uint8 reg, const uint8 data);        //软I2C写存8位寄存器
uint8   soft_i2c_read_reg8(SoftI2c_Info_t *i2c, const uint8 reg);                           //软I2C读取8位寄存器
void    soft_i2c_write_reg16(SoftI2c_Info_t *i2c, const uint8 reg, const uint16 data);      //软I2C写入16位寄存器
uint16  soft_i2c_read_reg16(SoftI2c_Info_t *i2c, const uint8 reg);                          //软I2C读取16位寄存器
void    soft_i2c_init(SoftI2c_Info_t *i2c, uint8 addr, uint32 delay, GPIO_Pin_t scl_pin, GPIO_Pin_t sda_pin);  //软I2C初始化
#endif /* __KY_SOFT_I2C_H__ */
