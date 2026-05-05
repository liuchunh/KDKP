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
    0.0f,   0.0f,   0.0f,      /* 速度过程噪声 */
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

    /* ESKF 预测 */
    ins_predict(&g_ins,
                g_imu.acc_x, g_imu.acc_y, g_imu.acc_z,
                g_imu.gyro_x, g_imu.gyro_y, g_imu.gyro_z,
                IMU_DT);

    /* 磁力计航向修正 (降频到 10Hz, 避免与陀螺积分冲突)
     * 北京磁偏角约 -6° = -0.105 rad */
    if (g_imu_count % 10 == 0) {
        ins_update_mag(&g_ins,
                       g_imu.mag_x, g_imu.mag_y, g_imu.mag_z,
                       -0.105f);
    }

    /* 更新导航状态 */
    ins_get_position(&g_ins, &g_nav.x, &g_nav.y, &g_nav.z);
    ins_get_velocity(&g_ins, &g_nav.vx, &g_nav.vy, &g_nav.vz);
    ins_get_attitude(&g_ins, &g_nav.pitch, &g_nav.roll, &g_nav.yaw);

    /* 位置标准差 (P 对角元素 sqrt) */
    float sum_p = g_ins.P[0][0] + g_ins.P[1][1] + g_ins.P[2][2];
    g_nav.pos_std = (sum_p > 0.0f) ? sqrtf(sum_p) : 0.0f;
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

    /* ESKF GPS 更新 */
    ins_update_gps(&g_ins, x_n, y_e, z_d);

    /* 刷新导航输出 */
    ins_get_position(&g_ins, &g_nav.x, &g_nav.y, &g_nav.z);
    ins_get_velocity(&g_ins, &g_nav.vx, &g_nav.vy, &g_nav.vz);
    ins_get_attitude(&g_ins, &g_nav.pitch, &g_nav.roll, &g_nav.yaw);
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
