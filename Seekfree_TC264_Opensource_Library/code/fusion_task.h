/**
 * fusion_task.h - GPS+IMU ESKF 融合任务
 *
 * 协调 IMU 预测 (100Hz) 和 GPS 更新 (10Hz)
 * 在主循环中调用, 或由调度器按周期触发
 */

#ifndef FUSION_TASK_H_
#define FUSION_TASK_H_

#include "zf_common_headfile.h"
#include "ins_solver.h"
#include "imu_task.h"

/* 系统状态 */
typedef struct {
    float x, y, z;           /* 当前位置 (m, NED) */
    float vx, vy, vz;        /* 当前速度 (m/s) */
    float pitch, roll, yaw;  /* 当前姿态 (rad) */
    float pos_std;           /* 位置标准差 (从协方差对角提取) */
    uint8 gps_valid;         /* 上次GPS更新是否有效 */
} NavState;

/* 全局导航状态 */
extern NavState g_nav;

/* 初始化融合系统: IMU + GNSS + ESKF */
void fusion_init(void);

/* IMU 预测步骤 (100Hz, 在 PIT 定时器中断或主循环中调用) */
void fusion_imu_predict(void);

/* GPS 更新步骤 (有新的 GPS 帧时调用) */
void fusion_gps_update(void);

/* 获取最新导航结果 */
void fusion_get_nav_state(NavState *nav);

/* 调试输出: 通过串口打印当前导航状态 */
void fusion_debug_print(void);

#endif /* FUSION_TASK_H_ */
