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
 * === 文件信息 ===
 * 文件名：[ky_imu_base.c]
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
 * 2026.01.11 |   毛毛   | V3.1 (Renamed from ky_icm.c)
 *********************************************************************************/
#include "ky_imu_base.h"
#include "ky_gpio.h"
#include "ky_delay.h"

/***************************************************************
  *  @brief     向IMU传感器写入单个寄存器
  *  @param     reg     目标寄存器地址
  *  @param     dat     要写入的8位数据
  *  @Sample usage:     icm_write_reg(0x20, 0x47);
 **************************************************************/
void icm_write_reg(uint8_t reg, uint8_t dat)
{
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = dat;

    ICM_SET_SPI_CS(0);
    delay_us(10);
    spi_write_byte_array(ICM_SPI, buffer, 2);
    ICM_SET_SPI_CS(1);
}

/***************************************************************
  *  @brief     从IMU传感器连续读取多个寄存器
  *  @param     reg     起始寄存器地址
  *  @param     buffer  用于存储读取数据的缓冲区
  *  @param     length  要读取的字节数
  *  @Sample usage:     uint8_t data[6];
  *                     icm_read_regs(0x28, data, 6);
 **************************************************************/
void icm_read_regs(uint8_t reg, uint8_t *buffer, uint16_t length)
{
    ICM_SET_SPI_CS(0);
    delay_us(10);
    
    spi_write_byte(ICM_SPI, reg | 0x80);

    spi_read_byte_array(ICM_SPI, buffer, length);

    ICM_SET_SPI_CS(1);    
}

/***************************************************************
  *  @brief     通用IMU传感器初始化
  *  @param     None
  *  @Sample usage:     icm_init();
 **************************************************************/
void icm_init(void)
{
    // Fix: correct order is MOSI, MISO, SCLK
    spi_init(ICM_SPI, ICM_SPI_MODE, ICM_SPI_BAUD,
            ICM_SPI_SDA, ICM_SPI_SDO, ICM_SPI_SCL);
    
    delay_ms(1);
    
    gpio_init(ICM_SPI_CS_PIN, GPIO_MODE_OUT_PP, GPIO_HIGH);
    gpio_set_speed(ICM_SPI_CS_PIN, GPIO_SPEED_FAST);

    delay_ms(10);
}
