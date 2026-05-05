/*
 * servo.h
 *
 *  Created on: 2026年5月5日
 *      Author: Spark44
 */

#ifndef CODE_SERVO_H_
#define CODE_SERVO_H_

#include "zf_common_headfile.h"

// **************************** 舵机配置 ****************************

#define SERVO_MOTOR_PWM             (ATOM2_CH4_P33_8)                           // 定义主板上舵机对应引脚
#define SERVO_MOTOR_FREQ            (50 )                                       // 定义主板上舵机频率  请务必注意范围 50-300

#define SERVO_MOTOR_L_MAX           (0  )                                       // 定义主板上舵机活动范围 角度
#define SERVO_MOTOR_R_MAX           (180)                                       // 定义主板上舵机活动范围 角度

#if (SERVO_MOTOR_FREQ<50 || SERVO_MOTOR_FREQ>300)
    #error "SERVO_MOTOR_FREQ ERROE!"
#endif

// **************************** 函数声明 ****************************

void    servo_init      (void);                                                 // 舵机初始化
void    servo_set_angle (float angle);                                          // 设置舵机角度
float   servo_get_angle (void);                                                 // 获取舵机当前角度
void    servo_sweep     (void);                                                 // 舵机来回摆动（需循环调用）

#endif /* CODE_SERVO_H_ */
