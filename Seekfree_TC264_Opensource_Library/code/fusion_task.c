/**
 * fusion_task.c - GPS+IMU 混合融合任务实现
 *
 * 混合策略:
 *   GPS 可用时: 位置直接用 GPS (无滞后), 速度 = 70%GPS差分 + 30%IMU积分
 *   GPS 丢失时: 切换到 ESKF 死算 (IMU 推算), pos_std 随时间增长
 *   姿态/航向: 始终由 ESKF + 磁力计互补滤波提供
 *
 * 保留 ESKF 的两个核心优势:
 *   1. GPS 丢失时 IMU 死算, 不会突然丢失定位
 *   2. 协方差估计 pos_std, 可据此减速/停车
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

/* IMU 预测计数 (用于降频磁力计更新) */
static uint32 g_imu_count = 0;

/* GPS NMEA 帧接收计数 (用于调试) */
static uint32 g_gps_frame_count = 0;

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

/* GPS 直接位置 (不经过 ESKF, GPS 可用时直接输出) */
static float g_gps_x = 0.0f;
static float g_gps_y = 0.0f;
static float g_gps_z = 0.0f;
static uint8 g_gps_pos_valid = 0;

/* 速度估计 (GPS 差分 + IMU 积分混合) */
static float g_vel_x = 0.0f;
static float g_vel_y = 0.0f;
static double g_last_gps_time = -1.0;
static float g_last_gps_x = 0.0f;
static float g_last_gps_y = 0.0f;

/* GPS 丢失计时 (超过阈值则切换到 ESKF 死算) */
static uint32 g_gps_lost_count = 0;
#define GPS_LOST_THRESHOLD  50      /* 50 次 IMU 更新无 GPS = 0.5秒 */

/* 磁偏角 (北京约 -6° = -0.105 rad, 需根据实际地点调整) */
#define MAG_DECLINATION  (-0.105f)

/* 磁力计更新计数 (每10次IMU更新做一次磁力计修正 = 10Hz) */
static uint32 g_mag_count = 0;
#define MAG_UPDATE_INTERVAL  10

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
    g_gps_frame_count = 0;
    g_gps_pos_valid = 0;
    g_gps_lost_count = 0;
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
 *
 *  策略:
 *    - ESKF 始终运行 (航向估计 + GPS 丢失时的死算备份)
 *    - 磁力计互补滤波 10Hz
 *    - GPS 可用时: 输出 GPS 直接位置, 速度用混合估计
 *    - GPS 丢失时: 输出 ESKF 位置/速度 (死算)
 *    - pos_std 始终来自 ESKF 协方差
 * ================================================================ */

void fusion_imu_predict(void) {
    if (!g_imu.fresh) return;
    g_imu.fresh = 0;

    g_imu_count++;
    g_mag_count++;

    /* ---- 1. ESKF 预测 (始终运行, 航向+死算) ---- */
    ins_predict(&g_ins,
                g_imu.acc_x, g_imu.acc_y, g_imu.acc_z,
                g_imu.gyro_x, g_imu.gyro_y, g_imu.gyro_z,
                IMU_DT);

    /* ---- 2. 磁力计航向更新 (10Hz) ---- */
    if (g_mag_count >= MAG_UPDATE_INTERVAL) {
        g_mag_count = 0;

        /* 计算磁力计航向 (用于误差统计) */
        float yaw_mag_raw = atan2f(-g_imu.mag_y, g_imu.mag_x) + MAG_DECLINATION;
        g_nav.yaw_mag = yaw_mag_raw;

        ins_update_mag(&g_ins,
                       g_imu.mag_x, g_imu.mag_y, g_imu.mag_z,
                       MAG_DECLINATION);
    }

    /* ---- 3. 航向始终来自 ESKF (磁力计互补滤波) ---- */
    ins_get_attitude(&g_ins, &g_nav.pitch, &g_nav.roll, &g_nav.yaw);

    /* 航向漂移统计 (ESKF - 磁力计) */
    if (g_mag_count == 0 && g_nav.yaw_mag != 0.0f) {
        float drift = g_nav.yaw - g_nav.yaw_mag;
        while (drift >  M_PI) drift -= 2.0f * M_PI;
        while (drift < -M_PI) drift += 2.0f * M_PI;
        g_nav.yaw_drift = drift;
    }

    /* ---- 4. 位置和速度: 根据 GPS 状态选择来源 ---- */
    if (g_gps_pos_valid && g_gps_lost_count < GPS_LOST_THRESHOLD) {
        /* GPS 可用: 位置直接用 GPS, 非 GPS 更新周期用速度积分插值 */
        float yaw = g_nav.yaw;
        float c = cosf(yaw), s = sinf(yaw);
        /* 加速度转 NED (简化: 只用 yaw 旋转, 假设车身水平) */
        float ax_ned = c * g_imu.acc_x - s * g_imu.acc_y;
        float ay_ned = s * g_imu.acc_x + c * g_imu.acc_y;
        /* 速度积分 (IMU 部分, GPS 更新时会混合) */
        g_vel_x += ax_ned * IMU_DT;
        g_vel_y += ay_ned * IMU_DT;
        /* 位置积分 */
        g_gps_x += g_vel_x * IMU_DT;
        g_gps_y += g_vel_y * IMU_DT;

        g_nav.x = g_gps_x;
        g_nav.y = g_gps_y;
        g_nav.z = g_gps_z;
        g_nav.vx = g_vel_x;
        g_nav.vy = g_vel_y;
        g_nav.vz = 0.0f;
    } else {
        /* GPS 丢失: 切换到 ESKF 死算 */
        g_gps_lost_count++;
        ins_get_position(&g_ins, &g_nav.x, &g_nav.y, &g_nav.z);
        ins_get_velocity(&g_ins, &g_nav.vx, &g_nav.vy, &g_nav.vz);
    }

    /* ---- 5. pos_std 始终来自 ESKF 协方差 ---- */
    float sum_p = g_ins.P[0][0] + g_ins.P[1][1] + g_ins.P[2][2];
    g_nav.pos_std = (sum_p > 0.0f) ? sqrtf(sum_p) : 0.0f;

    /* ---- 6. ESKF 零偏估计 (用于调试) ---- */
    g_nav.acc_bias[0] = g_ins.state[10];  /* S_BAX */
    g_nav.acc_bias[1] = g_ins.state[11];  /* S_BAY */
    g_nav.acc_bias[2] = g_ins.state[12];  /* S_BAZ */
    g_nav.gyro_bias[0] = g_ins.state[13]; /* S_BWX */
    g_nav.gyro_bias[1] = g_ins.state[14]; /* S_BWY */
    g_nav.gyro_bias[2] = g_ins.state[15]; /* S_BWZ */

    /* ---- 7. GPS 与 ESKF 位置偏差 (用于调试) ---- */
    float eskf_x, eskf_y, eskf_z;
    ins_get_position(&g_ins, &eskf_x, &eskf_y, &eskf_z);
    g_nav.gps_dx = g_gps_x - eskf_x;
    g_nav.gps_dy = g_gps_y - eskf_y;
}

/* ================================================================
 *  GPS 更新步骤 (10Hz)
 *
 *  5 层 NaN 防护 + GPS 直接定位 + 速度混合
 * ================================================================ */

void fusion_gps_update(void) {
    if (!gnss_flag) return;
    gnss_flag = 0;
    g_gps_frame_count++;

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
    g_gps_lost_count = 0;   /* GPS 恢复, 清除丢失计数 */

    /* ---- 速度估计: 70% GPS 差分 + 30% IMU 积分 ---- */
    double now = (double)g_imu_count * IMU_DT;
    if (g_last_gps_time > 0) {
        float dt = (float)(now - g_last_gps_time);
        if (dt > 0.001f) {
            float vx_gps = (x_n - g_last_gps_x) / dt;
            float vy_gps = (y_e - g_last_gps_y) / dt;
            g_vel_x = 0.7f * vx_gps + 0.3f * g_vel_x;
            g_vel_y = 0.7f * vy_gps + 0.3f * g_vel_y;
        }
    }
    g_last_gps_x = x_n;
    g_last_gps_y = y_e;
    g_last_gps_time = now;

    /* ---- GPS 位置直接使用 ---- */
    g_gps_x = x_n;
    g_gps_y = y_e;
    g_gps_z = z_d;
    g_gps_pos_valid = 1;

    /* ---- 同步 ESKF 位置 (保持 ESKF 与 GPS 一致, 用于 GPS 丢失时无缝切换) ---- */
    ins_set_initial_pose(&g_ins, x_n, y_e, z_d,
                         g_nav.yaw * 180.0f / (float)M_PI);
    /* 重置协方差矩阵 (防止 P 在纯 IMU 推算期间累积增长) */
    mat15_diag(g_ins.P, 1.0f);

    /* ---- 刷新导航输出 ---- */
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
    uart_write_string(DEBUG_UART_INDEX, "m gps=");
    uart_write_integer(DEBUG_UART_INDEX, g_nav.gps_valid);
    uart_write_string(DEBUG_UART_INDEX, " origin=");
    uart_write_integer(DEBUG_UART_INDEX, g_origin_set);
    uart_write_string(DEBUG_UART_INDEX, "\n");
}

/* GPS 调试: 打印 GPS 模块原始状态 */
void fusion_debug_gps(void) {
    uart_write_string(DEBUG_UART_INDEX, "[GPS] state=");
    uart_write_integer(DEBUG_UART_INDEX, gnss.state);
    uart_write_string(DEBUG_UART_INDEX, " lat=");
    uart_write_float(DEBUG_UART_INDEX, (float)gnss.latitude);
    uart_write_string(DEBUG_UART_INDEX, " lon=");
    uart_write_float(DEBUG_UART_INDEX, (float)gnss.longitude);
    uart_write_string(DEBUG_UART_INDEX, " h=");
    uart_write_float(DEBUG_UART_INDEX, (float)gnss.height);
    uart_write_string(DEBUG_UART_INDEX, " sats=");
    uart_write_integer(DEBUG_UART_INDEX, gnss.satellite_used);
    uart_write_string(DEBUG_UART_INDEX, " frames=");
    uart_write_integer(DEBUG_UART_INDEX, g_gps_frame_count);
    uart_write_string(DEBUG_UART_INDEX, "\n");
}

/* 磁力计调试: 打印磁力计原始数据和计算航向 */
void fusion_debug_mag(void) {
    float yaw_mag = atan2f(-g_imu.mag_y, g_imu.mag_x) + MAG_DECLINATION;
    uart_write_string(DEBUG_UART_INDEX, "[MAG] x=");
    uart_write_float(DEBUG_UART_INDEX, g_imu.mag_x);
    uart_write_string(DEBUG_UART_INDEX, " y=");
    uart_write_float(DEBUG_UART_INDEX, g_imu.mag_y);
    uart_write_string(DEBUG_UART_INDEX, " z=");
    uart_write_float(DEBUG_UART_INDEX, g_imu.mag_z);
    uart_write_string(DEBUG_UART_INDEX, " yaw_mag=");
    uart_write_float(DEBUG_UART_INDEX, yaw_mag * 180.0f / (float)M_PI);
    uart_write_string(DEBUG_UART_INDEX, "deg\n");
}
