/**
 * fusion_task.c - GPS+IMU ESKF 融合任务实现
 */

#include "fusion_task.h"
#include "zf_device_gnss.h"
#include "zf_driver_uart.h"
#include <math.h>

/* 单例 ESKF 滤波器 */
static InsSolver g_ins;

/* 导航状态 */
NavState g_nav;

/* IMU预测计数 (用于降频GPS更新) */
static uint32 g_imu_count = 0;

/* ========== 默认噪声参数 ========== */
/* 过程噪声对角: [pos(3), vel(3), att(3), ba(3), bw(3)] */
static float g_Qc_diag[15] = {
    0.001f, 0.001f, 0.001f,    /* 位置过程噪声 */
    0.0f,   0.0f,   0.0f,      /* 速度过程噪声 */
    0.01f,  0.01f,  0.01f,     /* 姿态过程噪声 */
    0.0001f, 0.0001f, 0.0001f, /* 加计零偏驱动噪声 */
    0.0001f, 0.0001f, 0.0001f  /* 陀螺零偏驱动噪声 */
};

/* GPS测量噪声对角: [x, y, z] */
static float g_Rc_diag[3] = {
    1.0f, 1.0f, 2.0f           /* z轴(高度)噪声通常比水平方向大 */
};

void fusion_init(void) {
    /* 1. 初始化 IMU963RA */
    uint8 imu_ok = imu_task_init();
    if (imu_ok) {
        /* IMU 初始化失败 — 实际项目中在此处做错误处理 */
    }

    /* 2. 初始化 GNSS (GPS) — 已在 cpu0_main 中调用 gnss_init(GN42A) */

    /* 3. 初始化 ESKF */
    ins_solver_init(&g_ins, g_Qc_diag, g_Rc_diag);

    /* 4. 等待首帧 GPS 用于初始位置对准 */
    g_nav.gps_valid = 0;
    g_imu_count = 0;

    /* 初始航向未知, 需要运动或磁力计确定 yaw
     * 暂时设为 0 (北向), 后续通过磁力计或双天线GNSS修正 */
    ins_set_initial_pose(&g_ins, 0.0f, 0.0f, 0.0f, 0.0f);
}

void fusion_imu_predict(void) {
    if (!g_imu.fresh) return;
    g_imu.fresh = 0;

    g_imu_count++;

    /* 执行一次 ESKF 预测 */
    ins_predict(&g_ins,
                g_imu.acc_x, g_imu.acc_y, g_imu.acc_z,
                g_imu.gyro_x, g_imu.gyro_y, g_imu.gyro_z,
                IMU_DT);

    /* 更新导航状态 */
    ins_get_position(&g_ins, &g_nav.x, &g_nav.y, &g_nav.z);
    ins_get_velocity(&g_ins, &g_nav.vx, &g_nav.vy, &g_nav.vz);
    ins_get_attitude(&g_ins, &g_nav.pitch, &g_nav.roll, &g_nav.yaw);

    /* 位置标准差 (P的对角元素 sqrt), 加 NaN 保护 */
    float sum_p = g_ins.P[0][0] + g_ins.P[1][1] + g_ins.P[2][2];
    g_nav.pos_std = (sum_p > 0.0f) ? sqrtf(sum_p) : 0.0f;
}

void fusion_gps_update(void) {
    if (!gnss_flag) return;

    gnss_flag = 0;

    /* 1. 解析 GPS NMEA 帧, 检查返回值 */
    uint8 parse_ok = gnss_data_parse();
    if (parse_ok != 0) {
        /* 校验失败, 丢弃此帧 */
        g_nav.gps_valid = 0;
        return;
    }

    /* 2. 检查定位有效性 */
    if (gnss.state == 0) {
        g_nav.gps_valid = 0;
        return;
    }

    /* 3. 检查经纬度是否在合理范围 (中国境内: 纬度 18~54, 经度 73~135)
     *    GPS 模块刚上电时可能输出 0.0 或异常值 */
    if (gnss.latitude < 15.0 || gnss.latitude > 55.0 ||
        gnss.longitude < 70.0 || gnss.longitude > 140.0) {
        g_nav.gps_valid = 0;
        return;
    }

    /* 4. GPS 坐标 -> 本地 NED
     *    用第一次有效定位作为基准点 (LAT0, LON0, H0)
     *    避免硬编码导致的百万米级偏移 */
    static double LAT0 = 0.0;
    static double LON0 = 0.0;
    static float  H0   = 0.0f;
    static uint8  origin_set = 0;

    if (!origin_set) {
        LAT0 = gnss.latitude;
        LON0 = gnss.longitude;
        H0   = gnss.height;
        origin_set = 1;
    }

    double dlat = gnss.latitude  - LAT0;
    double dlon = gnss.longitude - LON0;
    float x_n = (float)(dlat * 111320.0);
    float y_e = (float)(dlon * 111320.0 * cos(LAT0 * M_PI / 180.0));
    float z_d = -(gnss.height - H0);

    /* 5. NaN/Inf 保护: 任何异常值直接丢弃 */
    if (isnan(x_n) || isnan(y_e) || isnan(z_d) ||
        isinf(x_n) || isinf(y_e) || isinf(z_d)) {
        g_nav.gps_valid = 0;
        return;
    }

    g_nav.gps_valid = 1;

    /* 6. ESKF GPS 更新 */
    ins_update_gps(&g_ins, x_n, y_e, z_d);

    /* 刷新导航输出 */
    ins_get_position(&g_ins, &g_nav.x, &g_nav.y, &g_nav.z);
    ins_get_velocity(&g_ins, &g_nav.vx, &g_nav.vy, &g_nav.vz);
    ins_get_attitude(&g_ins, &g_nav.pitch, &g_nav.roll, &g_nav.yaw);
}

void fusion_get_nav_state(NavState *nav) {
    if (nav) memcpy(nav, &g_nav, sizeof(NavState));
}

void fusion_debug_print(void) {
    /* 通过调试串口输出当前导航状态 */
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
