/*
 * servo.c
 *
 *  Created on: 2026年5月5日
 *      Author: Spark44
 */

#include "servo.h"

// ------------------ 舵机占空比计算方式 ------------------
//
// 舵机对应的 0-180 活动角度对应 控制脉冲的 0.5ms-2.5ms 高电平
//
// 那么不同频率下的占空比计算方式就是
// PWM_DUTY_MAX/(1000/freq)*(1+Angle/180) 在 50hz 时就是 PWM_DUTY_MAX/(1000/50)*(1+Angle/180)
//
// ------------------ 舵机占空比计算方式 ------------------
#define SERVO_MOTOR_DUTY(x)         ((float)PWM_DUTY_MAX/(1000.0/(float)SERVO_MOTOR_FREQ)*(0.5+(float)(x)/90.0))

static float servo_motor_duty = 90.0;                                           // 舵机当前角度
static float servo_motor_dir = 1;                                               // 舵机摆动方向

//-------------------------------------------------------------------------------------------------------------------
// 函数名     servo_init
// 说明       初始化舵机PWM
// 参数       void
// 返回值     void
//-------------------------------------------------------------------------------------------------------------------
void servo_init (void)
{
    pwm_init(SERVO_MOTOR_PWM, SERVO_MOTOR_FREQ, (unsigned long)SERVO_MOTOR_DUTY(90.0));
    servo_motor_duty = 90.0;
    servo_motor_dir = 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名     servo_set_angle
// 说明       设置舵机到指定角度
// 参数       angle           目标角度（0-180）
// 返回值     void
//-------------------------------------------------------------------------------------------------------------------
void servo_set_angle (float angle)
{
    if(angle < SERVO_MOTOR_L_MAX) angle = SERVO_MOTOR_L_MAX;
    if(angle > SERVO_MOTOR_R_MAX) angle = SERVO_MOTOR_R_MAX;
    servo_motor_duty = angle;
    pwm_set_duty(SERVO_MOTOR_PWM, (unsigned long)SERVO_MOTOR_DUTY(angle));
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名     servo_get_angle
// 说明       获取舵机当前角度
// 参数       void
// 返回值     float           当前角度
//-------------------------------------------------------------------------------------------------------------------
float servo_get_angle (void)
{
    return servo_motor_duty;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名     servo_sweep
// 说明       舵机在L_MAX和R_MAX之间来回摆动，需在主循环中持续调用
// 参数       void
// 返回值     void
//-------------------------------------------------------------------------------------------------------------------
void servo_sweep (void)
{
    pwm_set_duty(SERVO_MOTOR_PWM, (unsigned long)SERVO_MOTOR_DUTY(servo_motor_duty));

    if(servo_motor_dir)
    {
        servo_motor_duty += 5;
        if(servo_motor_duty >= SERVO_MOTOR_R_MAX)
        {
            servo_motor_dir = 0x00;
        }
    }
    else
    {
        servo_motor_duty -= 5;
        if(servo_motor_duty <= SERVO_MOTOR_L_MAX)
        {
            servo_motor_dir = 0x01;
        }
    }
}
