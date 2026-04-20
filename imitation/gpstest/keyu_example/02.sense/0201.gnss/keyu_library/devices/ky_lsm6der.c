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
 * 文件名：[ky_lsm6der.c]
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
#include "ky_lsm6der.h"
#include "ky_delay.h"
#include "ky_ips.h" 
#include <stdio.h>

ImuData_t gLsm6dsrData;

static float gAccScale = 1.0f;  
static float gGyroScale = 1.0f; 

/***************************************************************
  *  @brief     检查LSM6DSR的设备ID是否正确
  *  @param     None
  *  @Sample usage:     lsm6dsr_check_id();
 **************************************************************/
static void lsm6dsr_check_id(void)
{
    uint8_t modelId = 0xFF;
    uint8_t timeoutCount = 0;

    while (1)
    {
        icm_read_regs(LSM6DSR_WHO_AM_I, &modelId, 1);
        if (modelId == 0x7f || modelId == 0x6b) 
        {
            // printf("LSM6DSR FOUND!\r\n");
            ips_show_string(10, 10, "LSM6DSR OK");
            break; 
        }
        else
        {
            delay_ms(10); 
            if (timeoutCount++ > 10)
            {
                // printf("LSM6DSR Init Error! ID: 0x%02X\r\n", modelId);
                ips_clear(WHITE);
                ips_set_color(RED, WHITE);
                ips_show_string(10, 10, "LSM6DSR Error!"); 
                ips_show_string(10, 30, "ID:");
                ips_show_u_int(40, 30, modelId, 2);
                while (1);
            }
        }
    }
}

/***************************************************************
  *  @brief     设置加速度计的量程和数据输出速率
  *  @param     afs     量程 (Full-Scale)
  *  @param     aodr    数据输出速率
  *  @Sample usage:     lsm6dsr_set_afs_aodr(LSM6DSR_AFS_16G, LSM6DSR_AODR_6664HZ);
 **************************************************************/
static void lsm6dsr_set_afs_aodr(Lsm6dsr_Afs_t afs, Lsm6dsr_Aodr_t aodr)
{
    icm_write_reg(LSM6DSR_CTRL1_XL, (aodr << 4) | (afs << 2));
    
    switch (afs)
    {
    case LSM6DSR_AFS_2G:   gAccScale = 0.061f / 1000.0f; break; 
    case LSM6DSR_AFS_4G:   gAccScale = 0.122f / 1000.0f; break;
    case LSM6DSR_AFS_8G:   gAccScale = 0.244f / 1000.0f; break;
    case LSM6DSR_AFS_16G:  gAccScale = 0.488f / 1000.0f; break;
    default:               gAccScale = 1.0f; break;
    }
}

/***************************************************************
  *  @brief     设置陀螺仪的量程和数据输出速率
  *  @param     gfs     量程 (Full-Scale)
  *  @param     godr    数据输出速率
  *  @Sample usage:     lsm6dsr_set_gfs_godr(LSM6DSR_GFS_2000DPS, LSM6DSR_GODR_6664HZ);
 **************************************************************/
static void lsm6dsr_set_gfs_godr(Lsm6dsr_Gfs_t gfs, Lsm6dsr_Godr_t godr)
{
    icm_write_reg(LSM6DSR_CTRL2_G, (godr << 4) | (gfs << 1));

    switch (gfs)
    {
    case LSM6DSR_GFS_125DPS:  gGyroScale = 4.375f / 1000.0f; break; 
    case LSM6DSR_GFS_250DPS:  gGyroScale = 8.75f  / 1000.0f; break;
    case LSM6DSR_GFS_500DPS:  gGyroScale = 17.50f / 1000.0f; break;
    case LSM6DSR_GFS_1000DPS: gGyroScale = 35.0f  / 1000.0f; break;
    case LSM6DSR_GFS_2000DPS: gGyroScale = 70.0f  / 1000.0f; break;
    case LSM6DSR_GFS_4000DPS: gGyroScale = 140.0f / 1000.0f; break;
    default:                  gGyroScale = 1.0f; break;
    }
}

/***************************************************************
  *  @brief     初始化LSM6DSR传感器
  *  @param     None
  *  @Sample usage:     lsm6dsr_init();
 **************************************************************/
void lsm6dsr_init(void)
{
    icm_init();
    
    lsm6dsr_check_id();

    icm_write_reg(LSM6DSR_CTRL3_C, 0x01); 
    delay_ms(10);

    lsm6dsr_set_afs_aodr(LSM6DSR_AFS_16G, LSM6DSR_AODR_6664HZ);
    lsm6dsr_set_gfs_godr(LSM6DSR_GFS_2000DPS, LSM6DSR_GODR_6664HZ);

    icm_write_reg(LSM6DSR_CTRL6_C, 0x00);
    icm_write_reg(LSM6DSR_CTRL7_G, 0x00);

    delay_ms(10);
}

/***************************************************************
  *  @brief     从传感器读取最新的加速度计数据
  *  @param     None
  *  @Sample usage:     lsm6dsr_read_acc();
 **************************************************************/
void lsm6dsr_read_acc(void)
{
    uint8_t dataBuffer[6];
    icm_read_regs(LSM6DSR_OUTX_L_A, dataBuffer, 6);

    gLsm6dsrData.acc.x = gAccScale * (int16_t)(((uint16_t)dataBuffer[1] << 8) | dataBuffer[0]);
    gLsm6dsrData.acc.y = gAccScale * (int16_t)(((uint16_t)dataBuffer[3] << 8) | dataBuffer[2]);
    gLsm6dsrData.acc.z = gAccScale * (int16_t)(((uint16_t)dataBuffer[5] << 8) | dataBuffer[4]);
}

/***************************************************************
  *  @brief     从传感器读取最新的陀螺仪数据
  *  @param     None
  *  @Sample usage:     lsm6dsr_read_gyro();
 **************************************************************/
void lsm6dsr_read_gyro(void)
{
    uint8_t dataBuffer[6];
    icm_read_regs(LSM6DSR_OUTX_L_G, dataBuffer, 6);

    gLsm6dsrData.gyro.x = gGyroScale * (int16_t)(((uint16_t)dataBuffer[1] << 8) | dataBuffer[0]);
    gLsm6dsrData.gyro.y = gGyroScale * (int16_t)(((uint16_t)dataBuffer[3] << 8) | dataBuffer[2]);
    gLsm6dsrData.gyro.z = gGyroScale * (int16_t)(((uint16_t)dataBuffer[5] << 8) | dataBuffer[4]);
}
