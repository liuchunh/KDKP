/*
 * IMU.c
 *
 *  Created on: 2025年1月21日
 *      Author: ORRN
 */
#include "zf_common_headfile.h"

gyro_param_t Gyro_Offset;
IMU_param_t  IMU_Data;

float Yaw_1 = 0;
float Roll_1 = 0;
float Picth_1 = 0;

int IMU_1_Open_flag = 0;//开关IMU标志位

/**
 * @brief 初始化 IMU963RA 传感器并标定陀螺仪零偏
 * @note 调用后 IMU 传感器开始工作，Yaw_1 初始值为零；需在系统启动时调用一次
 */
void IMU_init(void)//IMU初始化
{
    // ------------ 初始化 IMU963RA 传感器 ------------
    imu963ra_init();  //IMU963RA陀螺仪初始化

    // ------------ 标定陀螺仪零偏 ------------
    IMU_gyro_Offset_Init();// 陀螺仪零漂初始化
}

/**
 * @brief 采集 1000 个陀螺样本计算零偏平均值，用于后续数据校准
 * @note 采样频率约 200Hz（每 5ms 一次），总耗时约 5 秒；调用期间系统处于阻塞状态
 */
void IMU_gyro_Offset_Init(void)
{
    // ------------ 清零累加器 ------------
    Gyro_Offset.Zdata = 0;

    // ------------ 采集 1000 个样本并累加 ------------
    for (uint16_t i = 0; i < 1000; i++)
    {
        imu963ra_get_gyro();
        Gyro_Offset.Zdata += imu963ra_gyro_z;
        system_delay_ms(5);   // 频率 1Khz
    }

    // ------------ 计算平均值作为零偏 ------------
    Gyro_Offset.Zdata /= 1000.0;
}

/**
 * @brief 将原始陀螺数据转换为实际角速度值，减去零偏并加入死区滤波
 * @note 角速度死区为 [-0.025, 0.025] rad/s；死区内的微动被抑制，死区外调用 IMU_Handle_180 积分更新角度
 */
void IMU_GetValues(void)//将采集的原始值转换为实际角速度值, 并过滤掉陀螺仪去零漂的误差
{
    // ------------ 原始值减去零偏，转换为弧度/秒 ------------
    IMU_Data.gyro_z = ((float) imu963ra_gyro_z - Gyro_Offset.Zdata)* PI / 180.0f/ 16.384f;

    // ------------ 死区滤波：小于阈值的不处理 ------------
    if(IMU_Data.gyro_z<0.025&&IMU_Data.gyro_z>-0.025)//滤波
    {
        Yaw_1-=0;
    }
    else
    {
        IMU_Handle_180();
     }

}

/**
 * @brief 对角速度进行积分累加得到偏航角，并将角度限制在 ±180 范围内
 * @note 积分周期约 9.16ms（对应 IMU 采样周期）；顺时针为正（减操作），逆时针为负；角度自动 wrap 到 [-180, 180]
 */
void IMU_Handle_180(void)
{
    // ------------ 角速度积分累加（顺时针为正） ------------
    Yaw_1-=RAD_TO_ANGLE(IMU_Data.gyro_z*0.00916  );//(积分归一化)原来逆时针为正,此处改为顺时针为正

    // ------------ 角度 wrapping 到 [-180, 180] ------------
   if(Yaw_1>180 && Yaw_1<=360)
    {
        Yaw_1-=360;
    }
    else if(Yaw_1<(-180) && Yaw_1>=(-360))
    {
        Yaw_1+=360;
    }

}

/**
 * @brief 读取陀螺仪原始数据，若 IMU 已开启则进行数据处理
 * @note 该函数应在主循环中周期性调用；IMU_1_Open_flag 为 1 时才会处理数据
 */
void IMU_data_get(void)
{
    // ------------ 读取陀螺仪原始数据 ------------
    imu963ra_get_gyro();

    // ------------ 如果 IMU 已开启则处理数据 ------------
    if(IMU_1_Open_flag==1)
    {
        IMU_GetValues();
    }
}

//void IMU_text(void)
//{
//    IMU_1_Open_flag=1;
//    while(1)
//    {
//        ips_show_string(8*0, 16*0, "Yaw:");      ips_show_float(8*5,16*0,Yaw_1,3,6);
//    }
//}
