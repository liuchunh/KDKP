/**
 * nav_controller.c - 科目1 航点导航控制器实现
 *
 * 坐标系: NED (北-东-地), 与 INS 融合输出一致
 * 航向: yaw (rad), 0=北, π/2=东
 *
 * 适配 TC264 (TriCore) 平台
 */

#include "nav_controller.h"
#include "common.h"
#include <math.h>

Waypoint g_waypoints[MAX_WAYPOINTS];
uint8    g_waypoint_count = 0;
uint8    g_current_target = 0;
NavMode  g_nav_mode = NAV_IDLE;

/* 默认到达半径 (m) */
#define DEFAULT_ARRIVAL_RADIUS   0.8f
#define GARAGE_ARRIVAL_RADIUS    0.3f
#define SLOW_DOWN_DISTANCE       2.0f

/* 角度归一化到 [-π, π] */
static float wrap_pi(float angle) {
    while (angle >  (float)M_PI) angle -= 2.0f * (float)M_PI;
    while (angle < -(float)M_PI) angle += 2.0f * (float)M_PI;
    return angle;
}

/* ================================================================
 *  记录航点
 * ================================================================ */

uint8 nav_record_waypoint(uint8 type) {
    if (g_waypoint_count >= MAX_WAYPOINTS) {
        return 0;
    }

    Waypoint *wp = &g_waypoints[g_waypoint_count];
    wp->x   = g_nav.x;
    wp->y   = g_nav.y;
    wp->target_yaw = g_nav.yaw;
    wp->type = type;

    switch (type) {
        case 0:  wp->arrival_radius = DEFAULT_ARRIVAL_RADIUS;  break;
        case 1:  wp->arrival_radius = 0.5f;                    break;
        case 2:  wp->arrival_radius = GARAGE_ARRIVAL_RADIUS;   break;
        default: wp->arrival_radius = DEFAULT_ARRIVAL_RADIUS;  break;
    }

    g_waypoint_count++;
    return g_waypoint_count;
}

/* ================================================================
 *  计算距离和航向误差
 * ================================================================ */

void nav_compute_errors(float *distance, float *yaw_error) {
    if (g_current_target >= g_waypoint_count) {
        *distance  = 0.0f;
        *yaw_error = 0.0f;
        return;
    }

    const Waypoint *wp = &g_waypoints[g_current_target];

    float dx = wp->x - g_nav.x;
    float dy = wp->y - g_nav.y;
    *distance = sqrtf(dx * dx + dy * dy);

    /* 期望航向: 车头朝向目标 */
    float desired_yaw = atan2f(dy, dx);

    /* 车库航点: 使用记录的 target_yaw */
    if (wp->type == 1 || wp->type == 2) {
        desired_yaw = wp->target_yaw;
    }

    *yaw_error = wrap_pi(desired_yaw - g_nav.yaw);
}

/* ================================================================
 *  到达检测
 * ================================================================ */

uint8 nav_check_arrival(void) {
    if (g_current_target >= g_waypoint_count) {
        g_nav_mode = NAV_FINISHED;
        return 2;
    }

    const Waypoint *wp = &g_waypoints[g_current_target];

    float dx = wp->x - g_nav.x;
    float dy = wp->y - g_nav.y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist < wp->arrival_radius) {
        g_current_target++;
        if (g_current_target >= g_waypoint_count) {
            g_nav_mode = NAV_FINISHED;
        }
        return 2;
    }

    if (dist < SLOW_DOWN_DISTANCE) {
        return 1;
    }

    return 0;
}

/* ================================================================
 *  辅助函数
 * ================================================================ */

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
    uint8 i;
    for (i = 0; i < g_waypoint_count; i++) {
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
