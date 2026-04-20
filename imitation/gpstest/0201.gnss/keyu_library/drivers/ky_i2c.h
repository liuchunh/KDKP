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
 * 文件名：[ky_i2c.h]
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
#ifndef __KY_I2C_H__
#define __KY_I2C_H__

#include "ky_typedef.h"
#include "ky_gpio.h"
#include "I2c/Std/IfxI2c.h"
#include "I2c/I2c/IfxI2c_I2c.h"

// TC264 I2C 引脚映射定义
// I2C_1 (TC264 I2C0)
// MAP_00: SCL=P13.1, SDA=P13.2
// MAP_01: SCL=P15.4, SDA=P15.5

typedef enum {
    I2C_1 = 0, // 对应 TC264 I2C0
    I2C_2 = 1  // 对应 TC264 I2C1
} I2C_Type;
// I2C引脚映射选择
typedef enum {
    I2C_MAP_00 = 0, // 默认引脚 (I2C0: P13.1/P13.2)
    I2C_MAP_01 = 1, // 备用引脚 (I2C0: P15.4/P15.5)
    I2C_MAP_10 = 2, // 预留
    I2C_MAP_11 = 3  // 预留
} I2C_PinMap;
typedef struct {
    I2C_Type i2c_type;      
    I2C_PinMap pin_map;     // 引脚映射选择
    uint32_t speed;         // 总线速度，单位Hz (例如 100000, 400000)
} I2C_InitTypeDef;
// I2C状态
typedef enum {
    I2C_OK = 0,
    I2C_ERROR,
    I2C_BUSY,
    I2C_TIMEOUT,
    I2C_NACK,
} I2C_Status;
void i2_c_init(I2C_InitTypeDef *i2c_init);                                                          //I2C初始化
I2C_Status i2_c_start(I2C_Type i2c);                                                                //I2C启动信号
I2C_Status i2_c_stop(I2C_Type i2c);                                                                 //I2C停止信号
I2C_Status i2_c_write_byte(I2C_Type i2c, uint8_t dat);                                              //I2C写单字节
uint8_t i2_c_read_byte(I2C_Type i2c, uint8_t ack);                                                  //I2C读单字节
I2C_Status i2_c_master_transmit(I2C_Type i2c, uint8_t dev_addr, uint8_t *pData, uint16_t size);    //I2C主机发送
I2C_Status i2_c_master_receive(I2C_Type i2c, uint8_t dev_addr, uint8_t *pData, uint16_t size);     //I2C主机接收
#endif /* __KY_I2C_H__ */