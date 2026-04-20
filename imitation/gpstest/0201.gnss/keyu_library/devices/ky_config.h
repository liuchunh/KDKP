/*********************************************************************************
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 版权所有：[2025] [北京科宇通博科技有限公司]
 *
 * 许可协议：采用 GNU GPL v3.0 开源许可
 *
 * === 文件信息 ===
 * 文件名：[ky_config.h]
 * 开发单位：北京科宇通博科技有限公司
 * 描述：    设备层统一配置文件 (MT9V034配置表, VI5300固件, GNSS指令等)
 *
 * === 修订记录 ===
 * 日期       |  开发者  | 变更说明
 * -----------|----------|----------------------
 * 2026.01.07 |   毛毛   | V3.0
 *********************************************************************************/
#ifndef __KY_CONFIG_H__
#define __KY_CONFIG_H__

#include "ky_all.h"

extern const uint16_t mt9v034_reg_config_table[][2];
extern const uint16_t mt9v034_reg_config_table_size;

/*********************************************************************************
 *  MT9V034 配置结构体
 *  用户应先修改此结构体的参数，然后调用 mt9v034_apply_config() 统一应用
 *********************************************************************************/
typedef struct {
  uint8_t fps;            /* 帧率 (1-200) */
  uint8_t auto_exposure;  /* 自动曝光/增益使能 (0=关闭, 非0=开启) */
  uint16_t exposure_time; /* 曝光时间 (1-32767) */
  uint8_t gain;           /* 增益 (16-64) */
  uint8_t brightness;     /* 目标亮度 (1-64, 仅自动曝光模式有效) */
} mt9v034_config_t;

/* 全局配置实例 - 用户可修改此结构体的值 */
extern mt9v034_config_t mt9v034_config;

extern const uint8_t vi5300_firmware_ranging[];
extern const uint16_t vi5300_firmware_ranging__size;

extern const uint8_t CMD_RATE_10HZ[];
extern const uint8_t CMD_RMC_ON[];
extern const uint8_t CMD_GGA_ON[];
extern const uint8_t CMD_GLL_OFF[];
extern const uint8_t CMD_GSA_OFF[];
extern const uint8_t CMD_GSV_OFF[];
extern const uint8_t CMD_VTG_OFF[];
extern const uint8_t CMD_ZDA_OFF[];
extern const uint8_t CMD_GST_OFF[];
extern const uint8_t CMD_GNTXT_OFF[];


// --- TAU1202 Sizes ---
extern const uint16_t CMD_RATE_10HZ_size;
extern const uint16_t CMD_RMC_ON_size;
extern const uint16_t CMD_GGA_ON_size;
extern const uint16_t CMD_GLL_OFF_size;
extern const uint16_t CMD_GSA_OFF_size;
extern const uint16_t CMD_GSV_OFF_size;
extern const uint16_t CMD_VTG_OFF_size;
extern const uint16_t CMD_ZDA_OFF_size;
extern const uint16_t CMD_GST_OFF_size;
extern const uint16_t CMD_GNTXT_OFF_size;

#endif /* __KY_CONFIG_H__ */
