/**
 * nav_controller.c - 科目1 航点导航控制器实现
 *
 * 坐标系: NED (北-东-地), 与 INS 融合输出一致
 * 航向: yaw (rad), 0=北, π/2=东
 */

#include "nav_controller.h"
#include <math.h>

Waypoint g_waypoints[MAX_WAYPOINTS];
uint8    g_waypoint_count = 0;
uint8    g_current_target = 0;
NavMode  g_nav_mode = NAV_IDLE;

/* 默认到达半径 (m) */
#define DEFAULT_ARRIVAL_RADIUS   0.8f     /* 普通锥桶: 80cm */
#define GARAGE_ARRIVAL_RADIUS    0.3f     /* 车库停靠点: 30cm */
#define SLOW_DOWN_DISTANCE       2.0f     /* 接近目标多少米开始减速 */

/*
 * 角度归一化到 [-π, π]
 */
static float wrap_pi(float angle) {
    while (angle >  (float)M_PI) angle -= 2.0f * (float)M_PI;
    while (angle < -(float)M_PI) angle += 2.0f * (float)M_PI;
    return angle;
}

/*
 * 记录一个航点: 以当前 INS 融合位置作为航点坐标
 */
uint8 nav_record_waypoint(uint8 type) {
    if (g_waypoint_count >= MAX_WAYPOINTS) {
        return 0;
    }

    Waypoint *wp = &g_waypoints[g_waypoint_count];
    wp->x   = g_nav.x;
    wp->y   = g_nav.y;
    wp->target_yaw = g_nav.yaw;           /* 记录当前朝向作为期望朝向 */
    wp->type = type;

    switch (type) {
        case 0:  wp->arrival_radius = DEFAULT_ARRIVAL_RADIUS;  break;
        case 1:  wp->arrival_radius = 0.5f;                    break;  /* 车库入口 */
        case 2:  wp->arrival_radius = GARAGE_ARRIVAL_RADIUS;   break;  /* 车库停靠点 */
        default: wp->arrival_radius = DEFAULT_ARRIVAL_RADIUS;  break;
    }

    g_waypoint_count++;
    return g_waypoint_count;
}

/*
 * 计算到当前目标航点的 距离 和 航向误差
 * 调用者用这两个值驱动 PID 控制器:
 *   - 距离 → 速度 PID (越近越慢)
 *   - 航向误差 → 转向 PID (误差越大转向越猛)
 */
void nav_compute_errors(float *distance, float *yaw_error) {
    if (g_current_target >= g_waypoint_count) {
        *distance  = 0.0f;
        *yaw_error = 0.0f;
        return;
    }

    const Waypoint *wp = &g_waypoints[g_current_target];

    /* 水平距离 */
    float dx = wp->x - g_nav.x;
    float dy = wp->y - g_nav.y;
    *distance = sqrtf(dx * dx + dy * dy);

    /* 期望航向 (从车指向目标的反方向... 不对, 是车头要朝向目标) */
    float desired_yaw = atan2f(dy, dx);

    /* 如果是车库入口/停靠点, 使用记录的 target_yaw 而不是指向目标的方向 */
    if (wp->type == 1 || wp->type == 2) {
        desired_yaw = wp->target_yaw;
    }

    /* 航向误差 = 期望航向 - 当前航向, 归一化到 [-π, π] */
    *yaw_error = wrap_pi(desired_yaw - g_nav.yaw);
}

/*
 * 检查是否到达当前航点, 到达则自动切换到下一个
 * 返回 1 表示正在接近减速区, 返回 2 表示完全到达并已切换
 */
uint8 nav_check_arrival(void) {
    if (g_current_target >= g_waypoint_count) {
        g_nav_mode = NAV_FINISHED;
        return 2;
    }

    const Waypoint *wp = &g_waypoints[g_current_target];

    float dx = wp->x - g_nav.x;
    float dy = wp->y - g_nav.y;
    float dist = sqrtf(dx * dx + dy * dy);

    /* 到达判定 */
    if (dist < wp->arrival_radius) {
        g_current_target++;
        if (g_current_target >= g_waypoint_count) {
            g_nav_mode = NAV_FINISHED;
        }
        return 2;
    }

    /* 接近减速区 */
    if (dist < SLOW_DOWN_DISTANCE) {
        return 1;
    }

    return 0;
}

void nav_clear_waypoints(void) {
    memset(g_waypoints, 0, sizeof(g_waypoints));
    g_waypoint_count = 0;
    g_current_target = 0;
    g_nav_mode = NAV_IDLE;
}

const Waypoint* nav_get_current_target(void) {
    if (g_current_target < g_waypoint_count) {
        return &g_waypoints[g_current_target];
    }
    return NULL;
}

void nav_debug_print(void) {
    uart_write_string(DEBUG_UART_INDEX, "=== Waypoints ===\n");
    for (uint8 i = 0; i < g_waypoint_count; i++) {
        uart_write_string(DEBUG_UART_INDEX, "  #");
        uart_write_integer(DEBUG_UART_INDEX, i);
        uart_write_string(DEBUG_UART_INDEX, ": (");
        uart_write_float(DEBUG_UART_INDEX, g_waypoints[i].x);
        uart_write_string(DEBUG_UART_INDEX, ", ");
        uart_write_float(DEBUG_UART_INDEX, g_waypoints[i].y);
        uart_write_string(DEBUG_UART_INDEX, ") yaw=");
        uart_write_float(DEBUG_UART_INDEX, g_waypoints[i].target_yaw * 180.0f / (float)M_PI);
        uart_write_string(DEBUG_UART_INDEX, "deg\n");
    }
}
