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
 * 文件名：[ky_imu_base.h]
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
#ifndef KEYU_DEVICE_IMU_H
#define KEYU_DEVICE_IMU_H

#include "ky_typedef.h"
#include "ky_spi.h"
#include "ky_gpio.h"

#define ICM_SPI         SPI_0                 // 使用 SPI_2
#define ICM_SPI_SCL     SPI0_SCLK_P20_11       // SPI 时钟引脚 SCL/SCK
#define ICM_SPI_SDA     SPI0_MOSI_P20_14       // SPI 数据输出引脚 MOSI/SDA
#define ICM_SPI_SDO     SPI0_MISO_P20_12       // SPI 数据输入引脚 MISO/SDO
#define ICM_SPI_CS_PIN  GPIO_P20_13           // SPI 片选引脚 CS/NSS
#define ICM_SPI_MODE    SPI_MODE0             // SPI 模式 (CPOL=0, CPHA=0)
#define ICM_SPI_BAUD    1000000              // SPI 波特率 1MHz (Debug)
// #define ICM_SPI_BAUD    10000000              // SPI 波特率 10MHz (Original)

#define ICM_SET_SPI_CS(x) (gpio_set_level(ICM_SPI_CS_PIN, x))

typedef struct {
    float x;
    float y;
    float z;
} ImuAxis_t;
typedef struct {
    ImuAxis_t acc;                          // 加速度计数据 (单位: g, 重力加速度)
    ImuAxis_t gyro;                         // 陀螺仪数据 (单位: dps, 度/秒)
    ImuAxis_t mag;                          // 磁力计数据 (单位: uT, 微特斯拉)
    float temperature;                      // 温度数据
} ImuData_t;
void icm_write_reg(uint8_t reg, uint8_t dat);                       //ICM写寄存器
void icm_read_regs(uint8_t reg, uint8_t *buffer, uint16_t length);  //ICM读寄存器
void icm_init(void);                                                //ICM初始化
#endif // KEYU_DEVICE_IMU_H
