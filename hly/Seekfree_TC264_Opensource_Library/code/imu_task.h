/**
 * imu_task.h - IMU963RA 数据采集任务
 *
 * 在 PIT 定时器中断中以 100Hz 频率读取 IMU 数据
 * 包含零偏初始标定功能 (静止1秒取均值)
 */

#ifndef IMU_TASK_H_
#define IMU_TASK_H_

#include "zf_common_headfile.h"
#include "zf_device_imu963ra.h"

/* IMU 原始数据结构 (已转为物理单位) */
typedef struct {
    float acc_x, acc_y, acc_z;       /* 加速度 m/s^2 (体坐标系) */
    float gyro_x, gyro_y, gyro_z;    /* 角速度 rad/s (体坐标系) */
    float mag_x, mag_y, mag_z;       /* 磁场强度 Gauss (可选) */
    uint8 fresh;                     /* 1=本轮有新数据 */
} ImuData;

/* 全局 IMU 数据 */
extern ImuData g_imu;

/* 初始化 IMU963RA (SPI模式) */
uint8 imu_task_init(void);

/* 从 IMU963RA 读取一次数据并转换为物理单位, 由 PIT ISR 调用 */
void imu_task_read(void);

/* IMU 零偏标定: 采集 n 个样本, 计算加速度计和陀螺仪的静止零偏 */
void imu_task_calibrate(uint16 num_samples);

#endif /* IMU_TASK_H_ */
