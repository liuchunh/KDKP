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
 * 文件名：[ky_icm45686.h]
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
#ifndef KEYU_DEVICE_ICM45686_H_
#define KEYU_DEVICE_ICM45686_H_ 

#include "ky_math.h"
#include "ky_typedef.h"
#include "ky_imu_base.h"

typedef enum {
    ICM45686_AFS_32G = 0,        // ±32g
    ICM45686_AFS_16G,            // ±16g
    ICM45686_AFS_8G,             // ±8g
    ICM45686_AFS_4G,             // ±4g
    ICM45686_AFS_2G              // ±2g
} ICM45686Afs_t;
typedef enum {
    ICM45686_GFS_4000DPS = 0,    // ±4000dps
    ICM45686_GFS_2000DPS,        // ±2000dps
    ICM45686_GFS_1000DPS,        // ±1000dps
    ICM45686_GFS_500DPS,         // ±500dps
    ICM45686_GFS_250DPS,         // ±250dps
    ICM45686_GFS_125DPS,         // ±125dps
    ICM45686_GFS_62_5DPS,        // ±62.5dps
    ICM45686_GFS_31_25DPS,       // ±31.25dps
    ICM45686_GFS_15_625DPS       // ±15.625dps
} ICM45686Gfs_t;
typedef enum {
    ICM45686_AODR_1_5625HZ = 0x0F,
    ICM45686_AODR_3_125HZ  = 0x0E,
    ICM45686_AODR_6_25HZ   = 0x0D,
    ICM45686_AODR_12_5HZ   = 0x0C,
    ICM45686_AODR_25HZ     = 0x0B,
    ICM45686_AODR_50HZ     = 0x0A,
    ICM45686_AODR_100HZ    = 0x09,
    ICM45686_AODR_200HZ    = 0x08,
    ICM45686_AODR_400HZ    = 0x07,
    ICM45686_AODR_800HZ    = 0x06,
    ICM45686_AODR_1600HZ   = 0x05,
    ICM45686_AODR_3200HZ   = 0x04,
    ICM45686_AODR_6400HZ   = 0x03
} ICM45686Aodr_t;
typedef enum {
    ICM45686_GODR_1_5625HZ = 0x0F,
    ICM45686_GODR_3_125HZ  = 0x0E,
    ICM45686_GODR_6_25HZ   = 0x0D,
    ICM45686_GODR_12_5HZ   = 0x0C,
    ICM45686_GODR_25HZ     = 0x0B,
    ICM45686_GODR_50HZ     = 0x0A,
    ICM45686_GODR_100HZ    = 0x09,
    ICM45686_GODR_200HZ    = 0x08,
    ICM45686_GODR_400HZ    = 0x07,
    ICM45686_GODR_800HZ    = 0x06,
    ICM45686_GODR_1600HZ   = 0x05,
    ICM45686_GODR_3200HZ   = 0x04,
    ICM45686_GODR_6400HZ   = 0x03
} ICM45686Godr_t;
// ICM45686寄存器地址定义
#define ICM45686_WHO_AM_I           0x72      // 器件ID寄存器
#define ICM45686_REG_MISC2          0x7F      // 配置寄存器
#define ICM45686_PWR_MGMT0          0x10      // 电源管理寄存器0
#define ICM45686_TEMP_DATA1_UI      0x0C      // 温度数据寄存器起始地址
#define ICM45686_ACCEL_DATA_X1_UI   0x00      // 加速度数据寄存器起始地址
#define ICM45686_GYRO_DATA_X1_UI    0x06      // 陀螺仪数据寄存器起始地址
#define ICM45686_ACCEL_CONFIG0      0x1B      // 加速度计配置寄存器
#define ICM45686_GYRO_CONFIG0       0x1C      // 陀螺仪配置寄存器

extern ImuData_t gICM45686Data;
void icm45686_init(void);       //ICM45686初始化
void icm45686_read_acc(void);   //ICM45686读取加速度
void icm45686_read_gyro(void);  //ICM45686读取陀螺仪
void icm45686_read_temp(void);  //ICM45686读取温度
#endif