/**
 * imu_task.c - IMU963RA 数据采集实现
 *
 * 通过 SPI 读取 IMU963RA 的加速度计和陀螺仪数据
 * 包含零偏标定功能 (静止状态取均值)
 *
 * 使用 Seekfree 库提供的 imu963ra_acc_transition / imu963ra_gyro_transition
 * 宏进行原始值→物理单位的转换, 确保与库内部量程配置一致
 *
 * 适配 TC264 (TriCore) 平台, Seekfree 逐飞库
 */

#include "imu_task.h"
#include <math.h>

/*
 * Seekfree 库默认量程 (zf_device_imu963ra.h):
 *   加速度计: ±8G  → transition_factor[0] = 4098  (LSB→g)
 *   陀螺仪:   ±2000dps → transition_factor[1] = 14.3 (LSB→°/s)
 *
 * imu963ra_acc_transition(x)  = x / 4098   → 单位: g
 * imu963ra_gyro_transition(x) = x / 14.3   → 单位: °/s
 *
 * ESKF 需要:
 *   加速度: m/s²   → 1g = 9.80665 m/s²
 *   角速度: rad/s   → 1°/s = π/180 rad/s
 */

#define G_TO_MS2    9.80665f                /* g → m/s² */
#define DPS_TO_RADS (3.14159265f / 180.0f)  /* °/s → rad/s */

/* 全局 IMU 数据 */
ImuData g_imu;

/* 零偏 (标定后自动扣除, 单位: m/s² 和 rad/s) */
static float g_acc_bias[3]  = {0.0f, 0.0f, 0.0f};
static float g_gyro_bias[3] = {0.0f, 0.0f, 0.0f};

/* ================================================================
 *  初始化
 * ================================================================ */

uint8 imu_task_init(void) {
    memset(&g_imu, 0, sizeof(ImuData));

    /* 初始化 IMU963RA (SPI 模式) */
    /* Seekfree 库函数: imu963ra_init() 返回 0=成功 */
    uint8 ret = imu963ra_init();

    if (ret != 0) {
        /* 初始化失败 */
        return 1;
    }

    return 0;
}

/* ================================================================
 *  数据读取
 *
 *  使用库宏 imu963ra_acc_transition / imu963ra_gyro_transition
 *  自动匹配库内部量程配置, 再乘以单位转换系数
 * ================================================================ */

void imu_task_read(void) {
    /* 读取原始数据 (Seekfree 库函数, 更新全局 imu963ra_acc_x 等) */
    imu963ra_get_acc();
    imu963ra_get_gyro();

    /* 转换为物理单位并扣除零偏
     * imu963ra_acc_transition  → g
     * imu963ra_gyro_transition → °/s */
    g_imu.acc_x  = imu963ra_acc_transition(imu963ra_acc_x)  * G_TO_MS2  - g_acc_bias[0];
    g_imu.acc_y  = imu963ra_acc_transition(imu963ra_acc_y)  * G_TO_MS2  - g_acc_bias[1];
    g_imu.acc_z  = imu963ra_acc_transition(imu963ra_acc_z)  * G_TO_MS2  - g_acc_bias[2];

    g_imu.gyro_x = imu963ra_gyro_transition(imu963ra_gyro_x) * DPS_TO_RADS - g_gyro_bias[0];
    g_imu.gyro_y = imu963ra_gyro_transition(imu963ra_gyro_y) * DPS_TO_RADS - g_gyro_bias[1];
    g_imu.gyro_z = imu963ra_gyro_transition(imu963ra_gyro_z) * DPS_TO_RADS - g_gyro_bias[2];

    /* 磁力计 (可选, IMU963RA 内置, 暂不使用) */
    g_imu.mag_x = 0.0f;
    g_imu.mag_y = 0.0f;
    g_imu.mag_z = 0.0f;

    g_imu.fresh = 1;  /* 标记有新数据 */
}

/* ================================================================
 *  零偏标定
 *
 *  静止状态采集 num_samples 帧, 计算均值作为零偏
 *  加速度计: 静止时应读到 [0, 0, g], 所以 z 轴偏置要减去 1g
 *  陀螺仪:   静止时应读到 [0, 0, 0]
 * ================================================================ */

void imu_task_calibrate(uint16 num_samples) {
    if (num_samples == 0) num_samples = 200;

    double sum_acc[3]  = {0.0, 0.0, 0.0};
    double sum_gyro[3] = {0.0, 0.0, 0.0};

    uint16 i;
    for (i = 0; i < num_samples; i++) {
        /* 读取原始数据 (不扣零偏) */
        imu963ra_get_acc();
        imu963ra_get_gyro();

        /* 转为物理单位后累加 */
        sum_acc[0]  += (double)imu963ra_acc_transition(imu963ra_acc_x)  * G_TO_MS2;
        sum_acc[1]  += (double)imu963ra_acc_transition(imu963ra_acc_y)  * G_TO_MS2;
        sum_acc[2]  += (double)imu963ra_acc_transition(imu963ra_acc_z)  * G_TO_MS2;

        sum_gyro[0] += (double)imu963ra_gyro_transition(imu963ra_gyro_x) * DPS_TO_RADS;
        sum_gyro[1] += (double)imu963ra_gyro_transition(imu963ra_gyro_y) * DPS_TO_RADS;
        sum_gyro[2] += (double)imu963ra_gyro_transition(imu963ra_gyro_z) * DPS_TO_RADS;

        system_delay_ms(10);  /* 等待 10ms (100Hz) */
    }

    /* 计算均值作为零偏 */
    float inv = 1.0f / (float)num_samples;

    /* 加速度计零偏: 静止时应读到 [0, 0, g], 减去重力 */
    g_acc_bias[0] = (float)(sum_acc[0] * inv);
    g_acc_bias[1] = (float)(sum_acc[1] * inv);
    g_acc_bias[2] = (float)(sum_acc[2] * inv) - G_TO_MS2;  /* 减去重力 */

    /* 陀螺仪零偏: 静止时应读到 [0, 0, 0] */
    g_gyro_bias[0] = (float)(sum_gyro[0] * inv);
    g_gyro_bias[1] = (float)(sum_gyro[1] * inv);
    g_gyro_bias[2] = (float)(sum_gyro[2] * inv);
}
