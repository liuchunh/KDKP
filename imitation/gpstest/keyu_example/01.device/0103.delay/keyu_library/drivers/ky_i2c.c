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
 * 文件名：[ky_i2c.c]
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
#include "ky_i2c.h"

static IfxI2c_I2c g_i2cHandles[2]; // 支持 I2C0, I2C1

/********************************************************
 *  @brief     I2C初始化 (基于iLLD)
 *  @param     i2c_init    I2C初始化结构体
 *  @Sample usage:     i2_c_init(&i2c_init);
 **************************************************************/
void i2_c_init(I2C_InitTypeDef *i2c_init)
{
    IfxI2c_I2c_Config i2cConfig;
    Ifx_I2C *i2cModule;
    
    if (i2c_init->i2c_type == I2C_1)
    {
        i2cModule = &MODULE_I2C0;
    }
    else
    {
        #if defined(MODULE_I2C1)
            i2cModule = &MODULE_I2C1;
        #else
            return;
        #endif
    }

    IfxI2c_I2c_initConfig(&i2cConfig, i2cModule);

    if (i2c_init->i2c_type == I2C_1)
    {
        if (i2c_init->pin_map == I2C_MAP_00)
        {
            const IfxI2c_Pins pins = {
                &IfxI2c0_SCL_P13_1_INOUT,
                &IfxI2c0_SDA_P13_2_INOUT,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
            };
            i2cConfig.pins = &pins;
        }
        else if (i2c_init->pin_map == I2C_MAP_01)
        {
            #if defined(IfxI2c0_SCL_P15_4_INOUT)
            const IfxI2c_Pins pins = {
                &IfxI2c0_SCL_P15_4_INOUT,
                &IfxI2c0_SDA_P15_5_INOUT,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
            };
            i2cConfig.pins = &pins;
            #endif
        }
    }

    i2cConfig.baudrate = (float32)i2c_init->speed;

    IfxI2c_I2c_initModule(&g_i2cHandles[i2c_init->i2c_type], &i2cConfig);
}

/********************************************************
 *  @brief     I2C起始信号 (硬件I2C自动处理，此处为空)
 *  @param     i2c     I2C类型
 *  @Sample usage:     i2_c_start(I2C_1);
 **************************************************************/
I2C_Status i2_c_start(I2C_Type i2c)
{
    return I2C_OK;
}

/********************************************************
 *  @brief     I2C停止信号 (硬件I2C自动处理，此处为空)
 *  @param     i2c     I2C类型
 *  @Sample usage:     i2_c_stop(I2C_1);
 **************************************************************/
I2C_Status i2_c_stop(I2C_Type i2c)
{
    return I2C_OK;
}

/********************************************************
 *  @brief     I2C写入一个字节 (不推荐单字节调用)
 *  @param     i2c     I2C类型
 *  @param     dat     数据
 *  @Sample usage:     i2_c_write_byte(I2C_1, 0x01);
 **************************************************************/
I2C_Status i2_c_write_byte(I2C_Type i2c, uint8_t dat)
{
    return I2C_ERROR; 
}

/********************************************************
 *  @brief     I2C读取一个字节 (不推荐单字节调用)
 *  @param     i2c     I2C类型
 *  @param     ack     是否发送ACK
 *  @Sample usage:     i2_c_read_byte(I2C_1, 0x01);
 **************************************************************/
uint8_t i2_c_read_byte(I2C_Type i2c, uint8_t ack)
{
    return 0xFF;
}

/********************************************************
 *  @brief     I2C主模式发送数据 (使用 iLLD)
 *  @param     i2c     I2C类型
 *  @param     dev_addr    设备地址 (7位地址)
 *  @param     pData       数据指针
 *  @param     size        数据长度
 *  @Sample usage:     i2_c_master_transmit(I2C_1, 0x50, &data, 1);
 **************************************************************/
I2C_Status i2_c_master_transmit(I2C_Type i2c, uint8_t dev_addr, uint8_t *pData, uint16_t size)
{
    IfxI2c_I2c_Device i2cDevice;
    IfxI2c_I2c_deviceConfig i2cDeviceConfig;
    
    IfxI2c_I2c_initDeviceConfig(&i2cDeviceConfig, &g_i2cHandles[i2c]);
    i2cDeviceConfig.deviceAddress = (dev_addr << 1);
    IfxI2c_I2c_initDevice(&i2cDevice, &i2cDeviceConfig);

    IfxI2c_I2c_Status status = IfxI2c_I2c_write(&i2cDevice, pData, size);

    if (status == IfxI2c_I2c_Status_ok)
    {
        return I2C_OK;
    }
    else if (status == IfxI2c_I2c_Status_nak)
    {
        return I2C_NACK;
    }
    else
    {
        return I2C_ERROR;
    }
}

/********************************************************
 *  @brief     I2C主模式接收数据 (使用 iLLD)
 *  @param     i2c     I2C类型
 *  @param     dev_addr    设备地址 (7位地址)
 *  @param     pData       数据指针
 *  @param     size        数据长度
 *  @Sample usage:     i2_c_master_receive(I2C_1, 0x50, &data, 1);
 **************************************************************/
I2C_Status i2_c_master_receive(I2C_Type i2c, uint8_t dev_addr, uint8_t *pData, uint16_t size)
{
    IfxI2c_I2c_Device i2cDevice;
    IfxI2c_I2c_deviceConfig i2cDeviceConfig;
    
    IfxI2c_I2c_initDeviceConfig(&i2cDeviceConfig, &g_i2cHandles[i2c]);
    i2cDeviceConfig.deviceAddress = (dev_addr << 1) | 0x01;
    IfxI2c_I2c_initDevice(&i2cDevice, &i2cDeviceConfig);

    IfxI2c_I2c_Status status = IfxI2c_I2c_read(&i2cDevice, pData, size);

    if (status == IfxI2c_I2c_Status_ok)
    {
        return I2C_OK;
    }
    else if (status == IfxI2c_I2c_Status_nak)
    {
        return I2C_NACK;
    }
    else
    {
        return I2C_ERROR;
    }
}

