/*********************************************************************************************************************
 * 文件名称: cpu1_main.h
 * 功能描述: CPU1 辅助核心头文件 - 科目1 自动驾驶
 *
 * 本文件声明了 cpu1_main.c 中的功能开关宏和公共接口
 ********************************************************************************************************************/

#ifndef CPU1_MAIN_H_
#define CPU1_MAIN_H_

#include "zf_common_headfile.h"

/* =====================================================================
 *  功能开关 (在 cpu1_main.c 中实现)
 *  修改这些宏值来启用/禁用各项功能
 * ===================================================================== */

/* 是否启用 IPS200 屏幕显示 (0=禁用, 1=启用) */
#ifndef ENABLE_IPS200_DISPLAY
#define ENABLE_IPS200_DISPLAY       (0)
#endif

/* 是否启用无线模块数据转发 (0=禁用, 1=启用) */
#ifndef ENABLE_WIRELESS_FORWARD
#define ENABLE_WIRELESS_FORWARD     (0)
#endif

/* 屏幕刷新间隔 (ms) */
#define SCREEN_REFRESH_INTERVAL_MS  (200)

/* 无线发送间隔 (ms) */
#define WIRELESS_SEND_INTERVAL_MS   (500)

#endif /* CPU1_MAIN_H_ */
