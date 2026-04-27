/**
 * imu_task.c - IMU963RA 数据采集实现
 */

#include "imu_task.h"
#include <math.h>

ImuData g_imu;

#define M_PIf 3.1415926535898f

/* 全局零偏 (静止标定得到) */
static float g_acc_bias[3]  = {0.0f, 0.0f, 0.0f};
static float g_gyro_bias[3] = {0.0f, 0.0f, 0.0f};

uint8 imu_task_init(void) {
    memset(&g_imu, 0, sizeof(ImuData));

    uint8 ret = imu963ra_init();
    if (ret != 0) {
        /* IMU963RA 初始化失败, 可通过调试串口输出日志 */
        return 1;
    }

    /* 初始化后等待传感器稳定 */
    system_delay_ms(100);

    /* 可选: 自动标定零偏 */
    /* imu_task_calibrate(100); */

    return 0;
}

void imu_task_read(void) {
    /* 读取加速度计原始值 */
    imu963ra_get_acc();
    /* 读取陀螺仪原始值 */
    imu963ra_get_gyro();
    /* 读取磁力计 (可选, 用于航向修正) */
    imu963ra_get_mag();

    /* 转换为物理单位并扣除零偏 */
    g_imu.acc_x  = imu963ra_acc_transition(imu963ra_acc_x) * 9.8f - g_acc_bias[0];
    g_imu.acc_y  = imu963ra_acc_transition(imu963ra_acc_y) * 9.8f - g_acc_bias[1];
    g_imu.acc_z  = imu963ra_acc_transition(imu963ra_acc_z) * 9.8f - g_acc_bias[2];

    g_imu.gyro_x = imu963ra_gyro_transition(imu963ra_gyro_x) * (M_PIf / 180.0f) - g_gyro_bias[0];
    g_imu.gyro_y = imu963ra_gyro_transition(imu963ra_gyro_y) * (M_PIf / 180.0f) - g_gyro_bias[1];
    g_imu.gyro_z = imu963ra_gyro_transition(imu963ra_gyro_z) * (M_PIf / 180.0f) - g_gyro_bias[2];

    g_imu.mag_x  = imu963ra_mag_transition(imu963ra_mag_x);
    g_imu.mag_y  = imu963ra_mag_transition(imu963ra_mag_y);
    g_imu.mag_z  = imu963ra_mag_transition(imu963ra_mag_z);

    g_imu.fresh = 1;
}

void imu_task_calibrate(uint16 num_samples) {
    float acc_sum[3]  = {0.0f, 0.0f, 0.0f};
    float gyro_sum[3] = {0.0f, 0.0f, 0.0f};

    for (uint16 i = 0; i < num_samples; i++) {
        imu963ra_get_acc();
        imu963ra_get_gyro();

        acc_sum[0]  += imu963ra_acc_transition(imu963ra_acc_x) * 9.8f;
        acc_sum[1]  += imu963ra_acc_transition(imu963ra_acc_y) * 9.8f;
        acc_sum[2]  += imu963ra_acc_transition(imu963ra_acc_z) * 9.8f;
        gyro_sum[0] += imu963ra_gyro_transition(imu963ra_gyro_x) * (M_PIf / 180.0f);
        gyro_sum[1] += imu963ra_gyro_transition(imu963ra_gyro_y) * (M_PIf / 180.0f);
        gyro_sum[2] += imu963ra_gyro_transition(imu963ra_gyro_z) * (M_PIf / 180.0f);

        system_delay_ms(10);  /* 100Hz采样 */
    }

    /* 陀螺仪零偏 = 静止时角速度均值 (理想为0) */
    g_gyro_bias[0] = gyro_sum[0] / num_samples;
    g_gyro_bias[1] = gyro_sum[1] / num_samples;
    g_gyro_bias[2] = gyro_sum[2] / num_samples;

    /* 加速度计零偏: 静止时只有重力, x/y 均值应为0, z 均值应为 -9.8 */
    g_acc_bias[0] = acc_sum[0] / num_samples;
    g_acc_bias[1] = acc_sum[1] / num_samples;
    g_acc_bias[2] = acc_sum[2] / num_samples + 9.8f;   /* 扣掉重力分量 */
}
