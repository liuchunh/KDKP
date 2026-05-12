/**
 * fusion_task.c - GPS+IMU ESKF 融合任务实现
 *
 * 协调 IMU 预测 (100Hz) 和 GPS 更新 (10Hz)
 * 包含 GPS→NED 坐标转换和 5 层 NaN 防护
 *
 * 适配 TC264 (TriCore) 平台
 */

#include "fusion_task.h"
#include "zf_device_gnss.h"
#include "zf_driver_uart.h"
#include "common.h"
#include <math.h>

/* 单例 ESKF 滤波器 */
static InsSolver g_ins;

/* 导航状态 */
NavState g_nav;

/* IMU 预测计数 (用于降频 GPS 更新) */
static uint32 g_imu_count = 0;

/* ========== 默认噪声参数 ========== */
static float g_Qc_diag[15] = {
    0.001f, 0.001f, 0.001f,    /* 位置过程噪声 */
    0.001f, 0.001f, 0.001f,    /* 速度过程噪声 */
    0.01f,  0.01f,  0.01f,     /* 姿态过程噪声 */
    0.0001f, 0.0001f, 0.0001f, /* 加计零偏驱动噪声 */
    0.001f,  0.001f,  0.001f   /* 陀螺零偏驱动噪声 */
};

static float g_Rc_diag[3] = {
    1.0f, 1.0f, 2.0f           /* GPS 测量噪声 (z轴更大) */
};

/* GPS 基准点 (首次有效定位) */
static double g_lat0 = 0.0;
static double g_lon0 = 0.0;
static float  g_h0   = 0.0f;
static uint8  g_origin_set = 0;

/* GPS 位置直接使用 (不经过 ESKF) */
static float g_gps_x = 0.0f;
static float g_gps_y = 0.0f;
static float g_gps_z = 0.0f;
static uint8 g_gps_pos_valid = 0;

/* 速度估计 (GPS 差分) */
static float g_vel_x = 0.0f;
static float g_vel_y = 0.0f;
static double g_last_gps_time = -1.0;
static float g_last_gps_x = 0.0f;
static float g_last_gps_y = 0.0f;

/* ================================================================
 *  初始化
 * ================================================================ */

void fusion_init(void) {
    /* 1. 初始化 IMU963RA */
    uint8 imu_ok = imu_task_init();
    if (imu_ok) {
        /* IMU 初始化失败 */
    }

    /* 2. 初始化 ESKF */
    ins_solver_init(&g_ins, g_Qc_diag, g_Rc_diag);

    /* 3. 初始化导航状态 */
    memset(&g_nav, 0, sizeof(NavState));
    g_imu_count = 0;
    g_origin_set = 0;
    g_gps_pos_valid = 0;
    g_vel_x = 0.0f;
    g_vel_y = 0.0f;
    g_last_gps_time = -1.0;
    g_last_gps_x = 0.0f;
    g_last_gps_y = 0.0f;
    g_gps_x = 0.0f;
    g_gps_y = 0.0f;
    g_gps_z = 0.0f;

    /* 4. 设置初始航向 (北向) */
    ins_set_initial_pose(&g_ins, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* ================================================================
 *  IMU 预测步骤 (100Hz)
 * ================================================================ */

void fusion_imu_predict(void) {
    if (!g_imu.fresh) return;
    g_imu.fresh = 0;

    g_imu_count++;

    /* ESKF 预测 (仅用于航向估计) */
    ins_predict(&g_ins,
                g_imu.acc_x, g_imu.acc_y, g_imu.acc_z,
                g_imu.gyro_x, g_imu.gyro_y, g_imu.gyro_z,
                IMU_DT);

    /* 磁力计航向修正 (降频到 10Hz) */
    float yaw_mag_raw = 0.0f;
    if (g_imu_count % 10 == 0) {
        /* 计算磁力计航向 (用于误差统计) */
        yaw_mag_raw = atan2f(-g_imu.mag_y, g_imu.mag_x) + (-0.105f);
        g_nav.yaw_mag = yaw_mag_raw;

        ins_update_mag(&g_ins,
                       g_imu.mag_x, g_imu.mag_y, g_imu.mag_z,
                       -0.105f);
    }

    /* 航向来自 ESKF (磁力计互补滤波) */
    ins_get_attitude(&g_ins, &g_nav.pitch, &g_nav.roll, &g_nav.yaw);

    /* 航向漂移: ESKF 航向 - 磁力计航向 */
    if (g_imu_count % 10 == 0) {
        float drift = g_nav.yaw - g_nav.yaw_mag;
        while (drift >  M_PI) drift -= 2.0f * M_PI;
        while (drift < -M_PI) drift += 2.0f * M_PI;
        g_nav.yaw_drift = drift;
    }

    /* 位置: GPS 直接使用, 非 GPS 更新周期用速度积分插值 */
    if (g_gps_pos_valid) {
        float yaw = g_nav.yaw;
        float c = cosf(yaw), s = sinf(yaw);
        /* 加速度转 NED */
        float ax_ned = c * g_imu.acc_x - s * g_imu.acc_y;
        float ay_ned = s * g_imu.acc_x + c * g_imu.acc_y;
        /* 速度积分 */
        g_vel_x += ax_ned * IMU_DT;
        g_vel_y += ay_ned * IMU_DT;
        /* 位置积分 */
        g_gps_x += g_vel_x * IMU_DT;
        g_gps_y += g_vel_y * IMU_DT;
    }

    /* ESKF 位置估计 (用于与 GPS 对比) */
    float eskf_x, eskf_y, eskf_z;
    ins_get_position(&g_ins, &eskf_x, &eskf_y, &eskf_z);

    /* GPS 与 ESKF 位置偏差 */
    g_nav.gps_dx = g_gps_x - eskf_x;
    g_nav.gps_dy = g_gps_y - eskf_y;

    /* 获取 ESKF 零偏估计 */
    g_nav.acc_bias[0] = g_ins.state[10];  /* S_BAX */
    g_nav.acc_bias[1] = g_ins.state[11];  /* S_BAY */
    g_nav.acc_bias[2] = g_ins.state[12];  /* S_BAZ */
    g_nav.gyro_bias[0] = g_ins.state[13]; /* S_BWX */
    g_nav.gyro_bias[1] = g_ins.state[14]; /* S_BWY */
    g_nav.gyro_bias[2] = g_ins.state[15]; /* S_BWZ */

    g_nav.x = g_gps_x;
    g_nav.y = g_gps_y;
    g_nav.z = g_gps_z;
    g_nav.vx = g_vel_x;
    g_nav.vy = g_vel_y;
    g_nav.vz = 0.0f;
    g_nav.pos_std = 1.0f;  /* GPS 精度约 1m */
}

/* ================================================================
 *  GPS 更新步骤 (10Hz)
 * ================================================================ */

void fusion_gps_update(void) {
    if (!gnss_flag) return;
    gnss_flag = 0;

    /* L1: 解析 NMEA 帧, 校验 */
    uint8 parse_ok = gnss_data_parse();
    if (parse_ok != 0) {
        g_nav.gps_valid = 0;
        return;
    }

    /* L2: 定位有效性 */
    if (gnss.state == 0) {
        g_nav.gps_valid = 0;
        return;
    }

    /* L3: 经纬度范围检查 (中国境内) */
    if (gnss.latitude < 15.0 || gnss.latitude > 55.0 ||
        gnss.longitude < 70.0 || gnss.longitude > 140.0) {
        g_nav.gps_valid = 0;
        return;
    }

    /* L4: 设置基准点 (首次有效定位为原点) */
    if (!g_origin_set) {
        g_lat0 = gnss.latitude;
        g_lon0 = gnss.longitude;
        g_h0   = gnss.height;
        g_origin_set = 1;
    }

    /* GPS → NED 坐标 */
    double dlat = gnss.latitude  - g_lat0;
    double dlon = gnss.longitude - g_lon0;
    float x_n = (float)(dlat * 111320.0);
    float y_e = (float)(dlon * 111320.0 * cos(g_lat0 * (double)M_PI / 180.0));
    float z_d = -(gnss.height - g_h0);

    /* L5: NaN/Inf 检查 */
    if (isnan(x_n) || isnan(y_e) || isnan(z_d) ||
        isinf(x_n) || isinf(y_e) || isinf(z_d)) {
        g_nav.gps_valid = 0;
        return;
    }

    g_nav.gps_valid = 1;

    /* GPS 位置直接使用 (不经过 ESKF) */
    /* 速度: GPS 位置差分 */
    double now = (double)g_imu_count * IMU_DT;
    if (g_last_gps_time > 0) {
        float dt = (float)(now - g_last_gps_time);
        if (dt > 0.001f) {
            float vx_gps = (x_n - g_last_gps_x) / dt;
            float vy_gps = (y_e - g_last_gps_y) / dt;
            /* 混合: 70% GPS 差分速度 + 30% IMU 积分速度 */
            g_vel_x = 0.7f * vx_gps + 0.3f * g_vel_x;
            g_vel_y = 0.7f * vy_gps + 0.3f * g_vel_y;
        }
    }
    g_last_gps_x = x_n;
    g_last_gps_y = y_e;
    g_last_gps_time = now;

    /* 位置: 直接使用 GPS */
    g_gps_x = x_n;
    g_gps_y = y_e;
    g_gps_z = z_d;
    g_gps_pos_valid = 1;

    /* 同时更新 ESKF 的位置状态 (保持同步) */
    ins_set_initial_pose(&g_ins, x_n, y_e, z_d,
                         g_nav.yaw * 180.0f / (float)M_PI);

    /* 刷新导航输出 */
    g_nav.x = g_gps_x;
    g_nav.y = g_gps_y;
    g_nav.z = g_gps_z;
    g_nav.vx = g_vel_x;
    g_nav.vy = g_vel_y;
}

/* ================================================================
 *  辅助函数
 * ================================================================ */

void fusion_get_nav_state(NavState *nav) {
    if (nav) memcpy(nav, &g_nav, sizeof(NavState));
}

void fusion_debug_print(void) {
    uart_write_string(DEBUG_UART_INDEX, "NAV: pos=(");
    uart_write_float(DEBUG_UART_INDEX, (float)g_nav.x);
    uart_write_string(DEBUG_UART_INDEX, ", ");
    uart_write_float(DEBUG_UART_INDEX, (float)g_nav.y);
    uart_write_string(DEBUG_UART_INDEX, ", ");
    uart_write_float(DEBUG_UART_INDEX, (float)g_nav.z);
    uart_write_string(DEBUG_UART_INDEX, ") yaw=");
    uart_write_float(DEBUG_UART_INDEX, (float)(g_nav.yaw * 180.0f / M_PI));
    uart_write_string(DEBUG_UART_INDEX, "deg std=");
    uart_write_float(DEBUG_UART_INDEX, g_nav.pos_std);
    uart_write_string(DEBUG_UART_INDEX, "m\n");
}
