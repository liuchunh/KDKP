/*
 * control.h
 *
 *  Created on: 2025年11月21日
 *      Author: 18905
 */

#ifndef CODE_CONTROL_H_
#define CODE_CONTROL_H_

//外部变量 extern
#define VEER_MOTOR_MID         180
#define VEER_MOTOR_MAX         280
#define VEER_MOTOR_MIN         92

typedef  enum{
    IDLE,
    YAOKONG,
    GUANDAO,
    GPS,
    DAOCHE,
    RACK_TEST
}MOTER_control_mode;

extern MOTER_control_mode control_mode;

extern int angle ;
extern int angle_speed ;
extern int angle_test ;
extern  _pid MoterPID_L ;
extern  _pid MoterPID_R ;
extern  _pid SteerPID ;
extern _pid SteerUpPID ;
extern _pid Steer_S_Loop;
extern _pid Steer_D_Loop;
extern int num;
extern int num1;
extern float C;
extern uint8 Main_Key_Flag;

extern int16 Steer_Mid_Value;
//宏定义

//函数

/**
 * @brief 左右轮速度闭环 PID 控制
 */
void Speed_Control( float tar_l, float tar_r);

/**
 * @brief 舵机直接角度控制（非线性映射）
 */
void Steer_Control(int tar);

/**
 * @brief 舵机位置式 PID 控制
 */
void Steer_PID(_pid*p ,float error);

/**
 * @brief 舵机增量式 PID 控制
 */
void Steer_UpPID(_pid*p ,float error);

/**
 * @brief 舵机角度伺服控制，包含限幅与角度 PID 更新
 */
void Steer_Moter_control(float servo_out);

/**
 * @brief 检测舵机中位
 */
uint8 Steer_Mid_Cheak(void);

/**
 * @brief 初始化转向电机（委托角度控制模块）
 */
void Steer_Moter_Init(void);

#endif /* CODE_CONTROL_H_ */
