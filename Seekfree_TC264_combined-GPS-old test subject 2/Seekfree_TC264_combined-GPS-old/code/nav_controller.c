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

/* 倒车入库参数 */
#define PARKING_STOP_MS          1000    /* 到达停靠点后等待时间 (ms) */
#define PARKING_REVERSE_MS       5000    /* 倒车持续时间 (ms), 0.4m/s×5s=2.0m */
#define PARKING_REVERSE_SPEED    (-0.4f) /* 倒车速度 (m/s), 负值=后退 */
#define PARKING_REVERSE_KP       30.0f   /* 倒车航向修正增益 (deg/rad) */

/* 倒车入库子状态 */
typedef enum {
    PARK_STOP = 0,              /* 停车等待 */
    PARK_REVERSE,               /* 倒车中 */
    PARK_DONE,                  /* 倒车完成 */
} ParkingState;

static ParkingState g_park_state = PARK_STOP;
static uint32 g_park_timer = 0;         /* 倒车计时器 (ms) */

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
 *  横向偏差计算 (Stanley 控制器用)
 * ================================================================ */

void nav_compute_crosstrack(float *cross_track_err, float *path_yaw) {
    if (g_current_target >= g_waypoint_count) {
        *cross_track_err = 0.0f;
        *path_yaw = 0.0f;
        return;
    }

    const Waypoint *wp = &g_waypoints[g_current_target];

    /* 路径方向: 前一个航点 → 当前航点 */
    float prev_x, prev_y;
    if (g_current_target > 0) {
        prev_x = g_waypoints[g_current_target - 1].x;
        prev_y = g_waypoints[g_current_target - 1].y;
    } else {
        /* 第一个航点: 用起点 (0,0) */
        prev_x = 0.0f;
        prev_y = 0.0f;
    }

    float path_dx = wp->x - prev_x;
    float path_dy = wp->y - prev_y;
    float path_len = sqrtf(path_dx * path_dx + path_dy * path_dy);

    if (path_len < 0.01f) {
        /* 路径太短, 退化为距离 */
        float dx = wp->x - g_nav.x;
        float dy = wp->y - g_nav.y;
        *cross_track_err = sqrtf(dx * dx + dy * dy);
        *path_yaw = 0.0f;
        return;
    }

    *path_yaw = atan2f(path_dy, path_dx);

    /* 车到路径起点的向量 */
    float dx = g_nav.x - prev_x;
    float dy = g_nav.y - prev_y;

    /* 叉积: path × car, 正值=车在路径右侧, 负值=左侧 */
    float cross = path_dx * dy - path_dy * dx;
    *cross_track_err = cross / path_len;
}

/* ================================================================
 *  转弯角度计算 (前瞻减速用)
 * ================================================================ */

float nav_get_turn_angle(void) {
    if (g_current_target >= g_waypoint_count) {
        return 0.0f;
    }

    /* 没有下一个航点, 无法计算转弯 */
    if (g_current_target + 1 >= g_waypoint_count) {
        return 0.0f;
    }

    const Waypoint *cur  = &g_waypoints[g_current_target];
    const Waypoint *next = &g_waypoints[g_current_target + 1];

    /* 路径1: 车当前位置 → 当前航点 */
    float dx1 = cur->x - g_nav.x;
    float dy1 = cur->y - g_nav.y;
    float len1 = sqrtf(dx1 * dx1 + dy1 * dy1);

    /* 路径2: 当前航点 → 下一个航点 */
    float dx2 = next->x - cur->x;
    float dy2 = next->y - cur->y;
    float len2 = sqrtf(dx2 * dx2 + dy2 * dy2);

    if (len1 < 0.1f || len2 < 0.1f) {
        return 0.0f;
    }

    /* 余弦定理: cos(θ) = (a·b) / (|a|×|b|) */
    float dot = dx1 * dx2 + dy1 * dy2;
    float cos_angle = dot / (len1 * len2);
    if (cos_angle >  1.0f) cos_angle =  1.0f;
    if (cos_angle < -1.0f) cos_angle = -1.0f;

    return acosf(cos_angle) * 180.0f / (float)M_PI;
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

/* ================================================================
 *  倒车入库控制
 * ================================================================ */

void nav_parking_update(float *motor_speed, float *steer_angle) {
    /* 每次调用假设间隔约10ms (与 IMU tick 同步) */
    g_park_timer += 10;

    switch (g_park_state) {
        case PARK_STOP:
            /* 阶段1: 停车等待 */
            *motor_speed = 0.0f;
            *steer_angle = 0.0f;
            if (g_park_timer >= PARKING_STOP_MS) {
                g_park_state = PARK_REVERSE;
                g_park_timer = 0;
                uart_write_string(DEBUG_UART_INDEX,
                    "[PARK] >>> Start reversing...\r\n");
            }
            break;

        case PARK_REVERSE:
            /* 阶段2: 倒车, 保持记录时的航向 */
            *motor_speed = PARKING_REVERSE_SPEED;   /* 负值 = 后退 */
            if (g_waypoint_count > 0) {
                /* 用最后一个航点的 target_yaw 做航向修正 */
                float target_yaw = g_waypoints[g_waypoint_count - 1].target_yaw;
                float yaw_err = wrap_pi(target_yaw - g_nav.yaw);
                float steer = PARKING_REVERSE_KP * yaw_err;   /* deg */
                if (steer >  25.0f) steer =  25.0f;
                if (steer < -25.0f) steer = -25.0f;
                *steer_angle = steer;
            } else {
                *steer_angle = 0.0f;
            }
            if (g_park_timer >= PARKING_REVERSE_MS) {
                g_park_state = PARK_DONE;
                g_park_timer = 0;
            }
            break;

        case PARK_DONE:
            /* 阶段3: 停车完成 */
            *motor_speed = 0.0f;
            *steer_angle = 0.0f;
            g_nav_mode = NAV_FINISHED;
            uart_write_string(DEBUG_UART_INDEX,
                "[PARK] === Parking complete! Subject 1 done. ===\r\n");
            break;
    }
}

uint8 nav_get_park_state(void) {
    return (uint8)g_park_state;
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
