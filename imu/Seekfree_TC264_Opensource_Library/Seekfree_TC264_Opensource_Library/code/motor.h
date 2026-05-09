#ifndef _motor_h_
#define _motor_h_

#include "zf_common_headfile.h"

/* ---- 电机1 硬件配置 (DRV8701E R2) ---- */
#define MOTOR1_DIR_PIN      (P21_2)                 /* 方向引脚 */
#define MOTOR1_PWM_CH       (ATOM0_CH1_P21_3)      /* PWM 输出引脚 */

/* ---- 电机2 硬件配置 (DRV8701E L2) ---- */
#define MOTOR2_DIR_PIN      (P21_4)                 /* 方向引脚 */
#define MOTOR2_PWM_CH       (ATOM0_CH3_P21_5)      /* PWM 输出引脚 */

/* ---- 公共参数 ---- */
#define MOTOR_PWM_FREQ      (17000)                 /* PWM 频率 17kHz */
#define MOTOR_MAX_DUTY      (PWM_DUTY_MAX)          /* 最大占空比 (10000) */

/* 电机1 */
void motor1_init     (void);
void motor1_set_duty (int16 duty);
void motor1_stop     (void);

/* 电机2 */
void motor2_init     (void);
void motor2_set_duty (int16 duty);
void motor2_stop     (void);

#endif
