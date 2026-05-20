/*
 * IMU.h
 *
 *  Created on: 2025年1月21日
 *      Author: ORRN
 */

#ifndef CODE_IMU_H_
#define CODE_IMU_H_

//结构体定义
typedef struct{
    float Xdata;   //陀螺漂移X
    float Ydata;   //陀螺漂移Y
    float Zdata;   //陀螺漂移Z
}gyro_param_t ;

typedef struct{
    float acc_x;   //x轴加速度
    float acc_y;   //y轴加速度
    float acc_z;   //z轴加速度

    float gyro_x;  //x轴角速度
    float gyro_y;  //y轴角速度
    float gyro_z;  //z轴角速度
}IMU_param_t ;

extern int IMU_1_Open_flag;
extern float Yaw_1;
extern IMU_param_t  IMU_Data;

/**
 * @brief 初始化 IMU963RA 传感器并标定陀螺仪零偏
 */
void IMU_init(void);

/**
 * @brief 采集 1000 个陀螺样本计算零偏平均值，用于后续数据校准
 */
void IMU_gyro_Offset_Init(void);

/**
 * @brief 将原始陀螺数据转换为实际角速度值，减去零偏并加入死区滤波
 */
void IMU_GetValues(void);

/**
 * @brief 对角速度进行积分累加得到偏航角，并将角度限制在 ±180 范围内
 */
void IMU_Handle_180(void);

/**
 * @brief 读取陀螺仪原始数据，若 IMU 已开启则进行数据处理
 */
void IMU_data_get(void);

void IMU_text(void);

extern float Yaw_1;



#endif /* CODE_IMU_H_ */
