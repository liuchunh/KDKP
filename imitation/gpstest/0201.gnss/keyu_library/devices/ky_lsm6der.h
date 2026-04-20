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
 * 文件名：[ky_lsm6der.h]
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
#ifndef KEYU_DEVICE_LSM6DSR_H
#define KEYU_DEVICE_LSM6DSR_H

#include "ky_typedef.h"
#include "ky_spi.h"
#include "ky_imu_base.h"

typedef enum {
    LSM6DSR_AFS_2G = 0, // ±2g
    LSM6DSR_AFS_4G,     // ±4g
    LSM6DSR_AFS_8G,     // ±8g
    LSM6DSR_AFS_16G     // ±16g
} Lsm6dsr_Afs_t;
typedef enum {
    LSM6DSR_GFS_125DPS = 0, // ±125 dps
    LSM6DSR_GFS_250DPS,     // ±250 dps
    LSM6DSR_GFS_500DPS,     // ±500 dps
    LSM6DSR_GFS_1000DPS,    // ±1000 dps
    LSM6DSR_GFS_2000DPS,    // ±2000 dps
    LSM6DSR_GFS_4000DPS     // ±4000 dps
} Lsm6dsr_Gfs_t;
typedef enum {
    LSM6DSR_AODR_POWER_DOWN = 0, LSM6DSR_AODR_12_5HZ,
    LSM6DSR_AODR_26HZ,           LSM6DSR_AODR_52HZ,
    LSM6DSR_AODR_104HZ,          LSM6DSR_AODR_208HZ,
    LSM6DSR_AODR_416HZ,          LSM6DSR_AODR_833HZ,
    LSM6DSR_AODR_1666HZ,         LSM6DSR_AODR_3332HZ,
    LSM6DSR_AODR_6664HZ
} Lsm6dsr_Aodr_t;
typedef enum {
    LSM6DSR_GODR_POWER_DOWN = 0, LSM6DSR_GODR_12_5HZ,
    LSM6DSR_GODR_26HZ,           LSM6DSR_GODR_52HZ,
    LSM6DSR_GODR_104HZ,          LSM6DSR_GODR_208HZ,
    LSM6DSR_GODR_416HZ,          LSM6DSR_GODR_833HZ,
    LSM6DSR_GODR_1666HZ,         LSM6DSR_GODR_3332HZ,
    LSM6DSR_GODR_6664HZ
} Lsm6dsr_Godr_t;
extern ImuData_t gLsm6dsrData;
void lsm6dsr_init(void);        //LSM6DSR初始化
void lsm6dsr_read_acc(void);    //LSM6DSR读取加速度
void lsm6dsr_read_gyro(void);   //LSM6DSR读取陀螺仪
// --- 寄存器地址宏定义 ---
#define LSM6DSR_WHO_AM_I        0x0F // 器件ID
#define LSM6DSR_CTRL1_XL        0x10 // 加速度计控制1
#define LSM6DSR_CTRL2_G         0x11 // 陀螺仪控制2
#define LSM6DSR_CTRL3_C         0x12 // 通用控制3
#define LSM6DSR_CTRL6_C         0x15 // 通用控制6
#define LSM6DSR_CTRL7_G         0x16 // 陀螺仪控制7
#define LSM6DSR_OUTX_L_G        0x22 // 陀螺仪数据输出寄存器起始地址
#define LSM6DSR_OUTX_L_A        0x28 // 加速度计数据输出寄存器起始地址

#endif