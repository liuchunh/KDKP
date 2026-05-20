/*
 * IMU_2.c
 *
 *  Created on: 2025年2月18日
 *      Author: ORRN
 */

#include "zf_common_headfile.h"

float param_kp = 2.12;          // 加速度计修正量的比例系数,用于调节加速度计数据的修正速度
float i_error_x, i_error_y, i_error_z;// 积分误差
float param_ki = 0.0028;        // 加速度计修正量的积分系数,用于调节积分误差数据的积分收敛效果


Imu_gyro_param_t gyro_offset;
Imu_data_param_t imu_data;
euler_param_t    euler_angle;          // 姿态解算欧拉角结构体实例
quater_param_t   q_info = {1, 0, 0};  // 姿态解算四元数结构体实例

//void IMU_init(void)//IMU初始化
//{
//    imu660ra_init();   //IMU660陀螺仪初始化
//    Init_Gyro_Offset();// 陀螺仪零漂初始化
//}

/**
 * @brief 快速平方根倒数近似算法（Quake III 经典实现）
 * @param x 输入浮点数
 * @retval 1/sqrt(x) 的近似值
 * @note 使用魔术数字 0x5f3759df 进行一次牛顿迭代；精度足够用于归一化操作
 */
float Sqrt_Fast(float x)
{
    // ------------ 位操作快速近似 sqrt 倒数 ------------
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long *) &y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *) &i;

    // ------------ 一次牛顿迭代提高精度 ------------
    y = y * (1.5f - (halfx * y * y));
    return y;
}

/**
 * @brief 校准 IMU660RA 陀螺仪的零偏，采集 100 个有效样本取平均
 * @note 仅当三轴陀螺原始值均在 [-10, 10] 范围内时才计入有效样本，避免运动中的异常数据；采样间隔 1ms
 */
void Init_Gyro_Offset(void)
{
    // ------------ 清零累加器 ------------
      unsigned int i;

    gyro_offset.x_data = 0;
    gyro_offset.y_data = 0;
    gyro_offset.z_data = 0;

    // ------------ 采集 100 个有效样本（排除异常值） ------------
    for (i = 0; i < 100;)
    {

        imu660ra_get_gyro();// 读取陀螺仪GYRO数据

        // ------------ 硬件滤波：只取稳定范围内的样本 ------------
      if(imu660ra_gyro_x > -10 && imu660ra_gyro_x < 10&& imu660ra_gyro_y > -10 && imu660ra_gyro_y < 10&& imu660ra_gyro_z > -10 && imu660ra_gyro_z < 10)
                {
                     i++;
                   gyro_offset.x_data += imu660ra_gyro_x;
                   gyro_offset.y_data += imu660ra_gyro_y;
                   gyro_offset.z_data += imu660ra_gyro_z;
                }

        system_delay_ms(1) ;
    }

    // ------------ 计算平均值作为零偏 ------------
    gyro_offset.x_data /= 100;
    gyro_offset.y_data /= 100;
    gyro_offset.z_data /= 100;
}

/**
 * @brief 对加速度数据进行低通滤波，并将陀螺数据转换为弧度/秒
 * @note 静态函数，仅在 IMU_2.c 内部使用；一阶低通滤波 alpha=0.3；陀螺量程 16.4 LSB/(deg/s)
 */
static void Get_Values_ICM(void)
{
    // ------------ 一阶低通滤波加速度数据 ------------
    static double last_acc[3] = { 0,0,0 };

    // 一阶低通滤波
    imu_data.acc_x = (((float) imu660ra_acc_x) * alpha)  + last_acc[0] * (1 - alpha);
    imu_data.acc_y = (((float) imu660ra_acc_y) * alpha)  + last_acc[1] * (1 - alpha);
    imu_data.acc_z = (((float) imu660ra_acc_z) * alpha)  + last_acc[2] * (1 - alpha);

        // 保存本次acc数据
    last_acc[0] = imu_data.acc_x;
    last_acc[1] = imu_data.acc_y;
    last_acc[2] = imu_data.acc_z;

    // ------------ 陀螺原始值减去零偏后转换为弧度/秒 ------------
    imu_data.gyro_x = ((float) imu660ra_gyro_x - gyro_offset.x_data) * PI / 180 / 16.4f;
    imu_data.gyro_y = ((float) imu660ra_gyro_y - gyro_offset.y_data) * PI / 180 / 16.4f;
    imu_data.gyro_z = ((float) imu660ra_gyro_z - gyro_offset.z_data) * PI / 180 / 16.4f;
}

/**
 * @brief Mahony AHRS 姿态解算算法：使用 PI 校正将加速度计数据融合到陀螺积分中
 * @param gx 陀螺 x 轴角速度（输入/输出，会被 PI 校正修改）
 * @param gy 陀螺 y 轴角速度（输入/输出，会被 PI 校正修改）
 * @param gz 陀螺 z 轴角速度（输入/输出，会被 PI 校正修改）
 * @param ax 加速度计 x 轴（归一化后的输入）
 * @param ay 加速度计 y 轴（归一化后的输入）
 * @param az 加速度计 z 轴（归一化后的输入）
 * @note 静态函数，内部使用；四元数 q_info 为全局状态；PI 参数 param_kp/param_ki 需预先设置
 */
static void Update_AHRS_ICM(float gx, float gy, float gz, float ax, float ay, float az)
{
    // ------------ 计算积分半步长，用于四元数微分方程 ------------
    float half_t = 0.5 * DELTA_T;

    // ------------ 从四元数结构体中获取当前四元数分量 ------------
    float vx, vy, vz;       // 当前机体坐标系上的重力分量,通过四元数旋转得到
    float ex, ey, ez;       // 四元数估算值与加速度计测量值的叉乘误差
    float q0 = q_info.q0;
    float q1 = q_info.q1;
    float q2 = q_info.q2;
    float q3 = q_info.q3;

    // ------------ 预计算四元数平方项和叉乘项 ------------
    float q0q0 = q0 * q0;
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q1q1 = q1 * q1;
//    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;

    // ------------ 归一化加速度计数据 ------------
    float norm = Sqrt_Fast(ax * ax + ay * ay + az * az);
    ax = ax * norm;
    ay = ay * norm;
    az = az * norm;

    // ------------ 根据当前四元数姿态估计理论重力方向 ------------
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    // ------------ 叉乘计算估计重力与实际重力的误差 ------------
    ex = ay * vz - az * vy;
    ey = az * vx - ax * vz;
    ez = ax * vy - ay * vx;

    // ------------ PI 校正：用加速度计误差修正陀螺数据 ------------
    i_error_x += half_t * ex;
    i_error_y += half_t * ey;
    i_error_z += half_t * ez;
    gx = gx + param_kp * ex + param_ki * i_error_x;
    gy = gy + param_kp * ey + param_ki * i_error_y;
    gz = gz + param_kp * ez + param_ki * i_error_z;

    // ------------ 一阶龙格库塔法更新四元数 ------------
    q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * half_t;
    q1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * half_t;
    q2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * half_t;
    q3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * half_t;

    // ------------ 归一化四元数 ------------
    norm = Sqrt_Fast(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q_info.q0 = q0 * norm;
    q_info.q1 = q1 * norm;
    q_info.q2 = q2 * norm;
    q_info.q3 = q3 * norm;
}

/**
 * @brief 主 IMU 更新函数：读取传感器数据、运行 AHRS 算法、输出欧拉角
 * @note 调用流程：读加速度 -> 读陀螺 -> 低通滤波 -> AHRS 解算 -> 四元数转欧拉角 -> 角度动态调整
 */
void Get_Angles_ICM(void)
{
    // ------------ 读取传感器原始数据 ------------
    float q0,q1,q2,q3;  // 存储四元数的四个分量

    imu660ra_get_acc(); // 读取imu660ra陀螺仪加速度数据

    imu660ra_get_gyro();// 读取imu660ra陀螺仪角加速度数据

    // ------------ 数据预处理（低通滤波 + 单位转换） ------------
    Get_Values_ICM();   // 对传感器数据滤波并转换为实际物理值

    // ------------ Mahony AHRS 姿态解算 ------------
    Update_AHRS_ICM(imu_data.gyro_x, imu_data.gyro_y, imu_data.gyro_z, imu_data.acc_x, imu_data.acc_y, imu_data.acc_z);

    // ------------ 从 AHRS 更新后的姿态信息中获取四元数 ------------
    q0 = q_info.q0;
    q1 = q_info.q1;
    q2 = q_info.q2;
    q3 = q_info.q3;

    // ------------ 四元数转欧拉角 ------------
    euler_angle.pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 180 / PI - 180;
    euler_angle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 / PI;
    euler_angle.yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 / PI;

    // ------------ 动态调整欧拉角范围（pitch/roll 转换到 0~360，yaw 转换到 0~360） ------------
    if (euler_angle.pitch > 0)
    {
        euler_angle.pitch = 180 - euler_angle.pitch;
    }
    else if (euler_angle.pitch < 0)
    {
        euler_angle.pitch = -(180 + euler_angle.pitch);
    }
    if (euler_angle.roll > 0)
    {
        euler_angle.roll = 180 - euler_angle.roll;
    }
    else if (euler_angle.roll < 0)
    {
        euler_angle.roll = -(180 + euler_angle.roll);
    }
    if (euler_angle.yaw > 360)
    {
        euler_angle.yaw -= 360;
    }
    else if (euler_angle.yaw < 0)
    {
        euler_angle.yaw += 360;
    }
}
