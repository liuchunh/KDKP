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
 * 文件名：[ky_icm45686.c]
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
#include "ky_icm45686.h"
#include "ky_delay.h"
#include "ky_ips.h"
#include <stdio.h>

#define ICM45686_AUTO_CALIBRATE_ON_STARTUP

ImuData_t gICM45686Data;

static float gAccScale = 1.0f; 
static float gGyroScale = 1.0f;

static int16_t gAccBiasX = 0, gAccBiasY = 0, gAccBiasZ = 0;
static int16_t gGyroBiasX = 0, gGyroBiasY = 0, gGyroBiasZ = 0;

/***************************************************************
  *  @brief     检查ICM45686的设备ID
  *  @param     None
  *  @Sample usage:     icm45686_check_id();
 **************************************************************/
static void icm45686_check_id(void)
{
    uint8_t modelId = 0xFF;
    uint8_t timeoutCount = 0;

    while (1)
    {
        icm_read_regs(ICM45686_WHO_AM_I, &modelId, 1);
        if (modelId == 0xE9 || modelId == 0xFD)
        {
            // printf("ICM45686 FOUND! ID: 0x%02X\r\n", modelId);
            ips_show_string(10, 10, "ICM45686 OK");
            break; 
        }
        else
        {
            delay_ms(10); 
            if (timeoutCount++ > 10)
            {
                ips_clear(WHITE);
                ips_set_color(RED, WHITE);
                ips_show_string(10, 10, "ICM45686 Error!");
                ips_show_string(10, 30, "ID:");
                ips_show_u_int(40, 30, modelId, 3);
                while (1);
            }
        }
    }
}

/***************************************************************
  *  @brief     设置加速度计的量程和数据输出速率
  *  @param     afs     量程 (Full-Scale)
  *  @param     aodr    数据输出速率
  *  @Sample usage:     icm45686_set_afs_aodr(ICM45686_AFS_16G, ICM45686_AODR_800HZ);
 **************************************************************/
static void icm45686_set_afs_aodr(ICM45686Afs_t afs, ICM45686Aodr_t aodr)
{
    uint8_t config = ((afs & 0x07) << 4) | (aodr & 0x0F);
    icm_write_reg(ICM45686_ACCEL_CONFIG0, config);
    
    switch (afs)
    {
    case ICM45686_AFS_32G:  gAccScale = 1.0f / 1024.0f; break; 
    case ICM45686_AFS_16G:  gAccScale = 1.0f / 2048.0f; break; 
    case ICM45686_AFS_8G:   gAccScale = 1.0f / 4096.0f; break; 
    case ICM45686_AFS_4G:   gAccScale = 1.0f / 8192.0f; break; 
    case ICM45686_AFS_2G:   gAccScale = 1.0f / 16384.0f; break;
    default:                gAccScale = 1.0f; break;
    }
}

/***************************************************************
  *  @brief     设置陀螺仪的量程和数据输出速率
  *  @param     gfs     量程 (Full-Scale)
  *  @param     godr    数据输出速率
  *  @Sample usage:     icm45686_set_gfs_godr(ICM45686_GFS_2000DPS, ICM45686_GODR_800HZ);
 **************************************************************/
static void icm45686_set_gfs_godr(ICM45686Gfs_t gfs, ICM45686Godr_t godr)
{
    uint8_t config = ((gfs & 0x0F) << 4) | (godr & 0x0F);
    icm_write_reg(ICM45686_GYRO_CONFIG0, config);
    
    switch (gfs)
    {
    case ICM45686_GFS_4000DPS:  gGyroScale = 1.0f / 8.2f; break; 
    case ICM45686_GFS_2000DPS:  gGyroScale = 1.0f / 16.4f; break;
    case ICM45686_GFS_1000DPS:  gGyroScale = 1.0f / 32.8f; break;
    case ICM45686_GFS_500DPS:   gGyroScale = 1.0f / 65.5f; break;
    case ICM45686_GFS_250DPS:   gGyroScale = 1.0f / 131.0f; break;
    case ICM45686_GFS_125DPS:   gGyroScale = 1.0f / 262.0f; break;
    case ICM45686_GFS_62_5DPS:  gGyroScale = 1.0f / 524.3f; break;
    case ICM45686_GFS_31_25DPS: gGyroScale = 1.0f / 1048.6f; break;
    case ICM45686_GFS_15_625DPS:gGyroScale = 1.0f / 2097.2f; break;
    default:                    gGyroScale = 1.0f; break;
    }
}

/***************************************************************
  *  @brief     对IMU进行零点偏置校准
  *  @param     None
  *  @Sample usage:     icm45686_calibrate_bias();
 **************************************************************/
static void icm45686_calibrate_bias(void)
{
    const int numSamples = 200; 
    uint8_t dataBuffer[6];
    int16_t rawX, rawY, rawZ;
    int32_t accSumX = 0, accSumY = 0, accSumZ = 0;
    int32_t gyroSumX = 0, gyroSumY = 0, gyroSumZ = 0;
    int16_t accZGravityRaw;
    int i;

    for (i = 0; i < numSamples; i++)
    {
        icm_read_regs(ICM45686_ACCEL_DATA_X1_UI, dataBuffer, 6);
        rawX = (int16_t)((dataBuffer[1] << 8) | dataBuffer[0]);
        rawY = (int16_t)((dataBuffer[3] << 8) | dataBuffer[2]);
        rawZ = (int16_t)((dataBuffer[5] << 8) | dataBuffer[4]);
        accSumX += rawX;
        accSumY += rawY;
        accSumZ += rawZ;

        icm_read_regs(ICM45686_GYRO_DATA_X1_UI, dataBuffer, 6);
        rawX = (int16_t)((dataBuffer[1] << 8) | dataBuffer[0]);
        rawY = (int16_t)((dataBuffer[3] << 8) | dataBuffer[2]);
        rawZ = (int16_t)((dataBuffer[5] << 8) | dataBuffer[4]);
        gyroSumX += rawX;
        gyroSumY += rawY;
        gyroSumZ += rawZ;

        delay_ms(5);
    }

    gGyroBiasX = (int16_t)(gyroSumX / numSamples);
    gGyroBiasY = (int16_t)(gyroSumY / numSamples);
    gGyroBiasZ = (int16_t)(gyroSumZ / numSamples);

    gAccBiasX = (int16_t)(accSumX / numSamples);
    gAccBiasY = (int16_t)(accSumY / numSamples);
    
    if(gAccScale == (1.0f / 2048.0f)) { 
        accZGravityRaw = 2048;
    } else {
        accZGravityRaw = 0; 
    }
    gAccBiasZ = (int16_t)(accSumZ / numSamples) - accZGravityRaw;
}

/***************************************************************
  *  @brief     初始化ICM45686传感器
  *  @param     None
  *  @Sample usage:     icm45686_init();
 **************************************************************/
void icm45686_init(void)
{ 
    icm_init();
    
    icm45686_check_id();

    icm_write_reg(ICM45686_REG_MISC2, 0x02);
    delay_ms(10);

    icm_write_reg(ICM45686_PWR_MGMT0, 0x0F);

    icm45686_set_afs_aodr(ICM45686_AFS_16G, ICM45686_AODR_800HZ);
    icm45686_set_gfs_godr(ICM45686_GFS_2000DPS, ICM45686_GODR_800HZ);
#ifdef ICM45686_AUTO_CALIBRATE_ON_STARTUP
    delay_ms(50);
    
    icm45686_calibrate_bias();
#endif
    delay_ms(10);
}

/***************************************************************
  *  @brief     从传感器读取最新的加速度计数据
  *  @param     None
  *  @Sample usage:     icm45686_read_acc();
 **************************************************************/
void icm45686_read_acc(void)
{
    uint8_t dataBuffer[6];
    int16_t rawX, rawY, rawZ;
    icm_read_regs(ICM45686_ACCEL_DATA_X1_UI, dataBuffer, 6);

     rawX = (int16_t)((dataBuffer[1] << 8) | dataBuffer[0]);
     rawY = (int16_t)((dataBuffer[3] << 8) | dataBuffer[2]);
     rawZ = (int16_t)((dataBuffer[5] << 8) | dataBuffer[4]);
    rawX -= gAccBiasX;
    rawY -= gAccBiasY;
    rawZ -= gAccBiasZ;
    gICM45686Data.acc.x = (float)rawX * gAccScale;
    gICM45686Data.acc.y = (float)rawY * gAccScale;
    gICM45686Data.acc.z = (float)rawZ * gAccScale;
}

/***************************************************************
  *  @brief     从传感器读取最新的陀螺仪数据
  *  @param     None
  *  @Sample usage:     icm45686_read_gyro();
 **************************************************************/
void icm45686_read_gyro(void)
{
    uint8_t dataBuffer[6];
    int16_t rawX, rawY, rawZ;
    icm_read_regs(ICM45686_GYRO_DATA_X1_UI, dataBuffer, 6);

     rawX = (int16_t)((dataBuffer[1] << 8) | dataBuffer[0]);
     rawY = (int16_t)((dataBuffer[3] << 8) | dataBuffer[2]);
     rawZ = (int16_t)((dataBuffer[5] << 8) | dataBuffer[4]);
    rawX -= gGyroBiasX;
    rawY -= gGyroBiasY;
    rawZ -= gGyroBiasZ;
    gICM45686Data.gyro.x = (float)rawX * gGyroScale;
    gICM45686Data.gyro.y = (float)rawY * gGyroScale;
    gICM45686Data.gyro.z = (float)rawZ * gGyroScale;
}

/***************************************************************
  *  @brief     从传感器读取温度数据
  *  @param     None
  *  @Sample usage:     icm45686_read_temp();
 **************************************************************/
void icm45686_read_temp(void)
{
    uint8_t dataBuffer[2];
    int16_t rawTemp;
    icm_read_regs(ICM45686_TEMP_DATA1_UI, dataBuffer, 2);

     rawTemp = (int16_t)((dataBuffer[1] << 8) | dataBuffer[0]);
    
    gICM45686Data.temperature = (float)rawTemp / 132.48f + 25.0f;
}
