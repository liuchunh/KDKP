/*
 * control.c
 *
 *  Created on: 2025年11月21日
 *      Author: 18905
 */
#include "zf_common_headfile.h"
#include "angle_control.h"

_pid MoterPID_L ={ .Kp =0.5 , .Ki = 1.0 , .Kd = 0 ,  .out_max = 3000 , .out_min = -3000};
_pid MoterPID_R ={ .Kp =0.5 , .Ki = 1.0 , .Kd = 0 ,  .out_max = 3000 , .out_min = -3000};
_pid SteerPID ={ .Kp =1.0 , .Ki = 0 , .Kd = 0.5 ,  .out_max = 13.0f , .out_min = -13.0};
_pid SteerUpPID = {.Kp =0.03 , .Ki = 0.08 , .Kd = 0 ,  .out_max = 13.0f , .out_min = -13.0 ,.target_val =0.0f};
_pid Steer_S_Loop = {.Kp =500.0f , .Ki = 0 , .Kd = 0.2 ,  .out_max = 9000.0f , .out_min = -9000.0f ,.target_val =0.0f};
_pid Steer_D_Loop = {.Kp =1000.0f , .Ki = 0.0f , .Kd = 0.1f ,  .out_max = 9000.0f , .out_min = -9000.0f ,.target_val =0.0f , .limit = 100.0f};

MOTER_control_mode control_mode = IDLE;
int num = 0;
int num1 =500;
float C;
uint8 Main_Key_Flag = 0;

int16 Steer_Mid_Value = 0;
int angle = 0;
int angle_speed = 0;
int angle_test = 0;

/**
 * @brief 初始化转向电机，委托角度控制模块完成 PWM、编码器和 PID 配置
 * @param 无
 * @note 调用后将目标角度设为零点；应在系统启动阶段调用一次
 */
void Steer_Moter_Init(void)
{
    // ------------ 初始化角度控制模块并归零 ------------
    angle_control_init();
    angle_control_set_target(0);
}


/**
 * @brief 左右轮速度闭环 PID 控制
 * @param tar_l 左轮目标速度
 * @param tar_r 右轮目标速度
 * @note 目标速度自动限幅到 [-200, 200]；内部使用增量式 PID 累加输出并限幅后驱动电机
 */
void Speed_Control(float tar_l, float tar_r)
{
    static int all_l = 0;
    static int all_r = 0;
    static float Kp =0;

    // ------------ 设置目标速度并限幅 ------------
    MoterPID_L.target_val =tar_l;
    MoterPID_R.target_val =tar_r;
    Value_Limit_float(&MoterPID_L.target_val , -200 , 200);
    Value_Limit_float(&MoterPID_R.target_val , -200 , 200);

    // ------------ 读取编码器速度 ------------
    Encoder_Get(&Speed_ecd);
    angle_speed += Speed_ecd.delta_r;

    // ------------ 左右轮增量式 PID 计算 ------------
    PID_Up(&MoterPID_L , (float)Speed_ecd.delta_l);
    PID_Up(&MoterPID_R , (float)Speed_ecd.delta_r);

    // ------------ 累积输出并限幅 ------------
    all_l += MoterPID_L.out;
    all_r += MoterPID_R.out;
    Value_Limit_int(&all_l , -MOTER_MAX , MOTER_MAX);
    Value_Limit_int(&all_r , -MOTER_MAX , MOTER_MAX);

    // ------------ 输出驱动电机 ------------
    Moter_Set(all_l , all_r );

}


/**
 * @brief 舵机角度伺服控制，限幅后调用角度 PID 更新
 * @param servo_out 舵机目标角度
 * @note 输入角度自动限幅到 [ANGLE_MIN_DEGREE, ANGLE_MAX_DEGREE] 范围
 */
void Steer_Moter_control(float servo_out)
{
    // ------------ 限幅并设置目标角度 ------------
    Value_Limit_float(&servo_out, ANGLE_MIN_DEGREE, ANGLE_MAX_DEGREE);
    angle_control_set_target((int32)servo_out);

    // ------------ 执行角度 PID 控制 ------------
    angle_control_update();

    // ------------ 更新当前角度和速度供外部使用 ------------
    angle = angle_control_get_current_angle();
    angle_speed = (int)angle_ctrl.pid.Output;


}

/**
 * @brief 舵机增量式 PID 控制
 * @param p PID 参数结构体指针
 * @param error 角度偏差（目标值 - 实测值）
 * @note 输出限幅到 [-13.0, 13.0]，以 SERVO_MOTOR_MID 为基准偏移输出
 */
void Steer_UpPID(_pid*p ,float error)
{
    static float final_out = 0;

    // ------------ 增量式 PID 计算 ------------
    PID_Up(p, -error);

    // ------------ 累加输出并限幅 ------------
    final_out += p->out;
    Value_Limit_float(&final_out , -13.0 ,13.0);

    // ------------ 输出至舵机 ------------
    Steer_set(SERVO_MOTOR_MID-final_out);
}

/**
 * @brief 舵机直接角度控制（非线性映射）
 * @param tar 目标舵机位置（500 为基准中位）
 * @note 使用三次方映射函数将目标值转换为舵机 PWM 值，实现非线性转角控制
 */
void Steer_Control(int tar)
{
    static int num = 0;

    // ------------ 非线性映射计算并输出 ------------
    num = (tar - 500)*(tar - 500)*(tar - 500)/2000000+80;
    Steer_set(num);

}

/**
 * @brief 舵机位置式 PID 控制
 * @param p PID 参数结构体指针
 * @param error 角度偏差值
 * @note 输出以 SERVO_MOTOR_MID 为基准偏移，由位置式 PID 计算绝对输出值
 */
void Steer_PID(_pid*p ,float error)
{
    static int num = 0;

    // ------------ 位置式 PID 计算 ------------
    PID_Place(p,error);

    // ------------ 输出至舵机 ------------
    num = (int)p->out;
    Steer_set(SERVO_MOTOR_MID-num);

}
