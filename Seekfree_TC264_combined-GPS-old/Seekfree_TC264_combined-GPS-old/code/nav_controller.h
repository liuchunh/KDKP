/**
 * nav_controller.h - 科目1 自动驾驶 航点导航控制器
 *
 * 使用 INS 融合后的 NED 坐标和航向角进行航点追踪
 *
 * 工作流程:
 *   教学阶段: 按按键在锥桶/车库入口/车库停靠点记录航点
 *   自动阶段: 依次导航到每个航点, 到达后自动切换下一个
 */

#ifndef NAV_CONTROLLER_H_
#define NAV_CONTROLLER_H_

#include "zf_common_headfile.h"
#include "fusion_task.h"

#define MAX_WAYPOINTS   20          /* 最大航点数 */

/* 单个航点 */
typedef struct {
    float x, y;                     /* NED 位置 (m, 相对起点的东北坐标) */
    float target_yaw;               /* 期望航向 (rad, 0=北), 用于车库对准 */
    float arrival_radius;           /* 到达判定半径 (m), 进入此范围即认为到达 */
    uint8 type;                     /* 0=普通锥桶, 1=车库入口, 2=车库停靠点 */
} Waypoint;

/* 导航状态 */
typedef enum {
    NAV_IDLE = 0,                   /* 空闲 */
    NAV_TEACHING,                   /* 教学记录模式 */
    NAV_RUNNING,                    /* 自动导航运行中 */
    NAV_PARKING,                    /* 倒车入库中 */
    NAV_FINISHED,                   /* 所有航点完成 */
} NavMode;

/* 全局 */
extern Waypoint g_waypoints[MAX_WAYPOINTS];
extern uint8    g_waypoint_count;
extern uint8    g_current_target;
extern NavMode  g_nav_mode;

/* 教学: 在当前位置记录一个航点 */
uint8 nav_record_waypoint(uint8 type);

/* 自动: 计算到当前目标航点的距离(m)和航向误差(rad) */
void  nav_compute_errors(float *distance, float *yaw_error);

/* 自动: 计算横向偏差(m, 正值=车在路径右侧)和路径航向(rad) */
void  nav_compute_crosstrack(float *cross_track_err, float *path_yaw);

/* 自动: 计算当前航点处的转弯角度 (度), 用于前瞻减速 */
float nav_get_turn_angle(void);

/* 自动: 检查是否到达当前航点, 是则自动切换到下一个 */
uint8 nav_check_arrival(void);

/* 清空所有航点 */
void  nav_clear_waypoints(void);

/* 获取当前目标航点指针 */
const Waypoint* nav_get_current_target(void);

/* 倒车入库控制 (在 NAV_PARKING 状态下调用) */
void  nav_parking_update(float *motor_speed, float *steer_angle);

/* 获取倒车入库子状态 (0=STOP, 1=REVERSE, 2=DONE) */
uint8 nav_get_park_state(void);

/* 调试: 打印所有航点 */
void  nav_debug_print(void);

#endif /* NAV_CONTROLLER_H_ */
