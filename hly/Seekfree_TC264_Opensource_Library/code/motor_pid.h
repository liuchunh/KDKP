/*
 * motor_pid.h
 */

#ifndef CODE_MOTOR_PID_H_
#define CODE_MOTOR_PID_H_

#include "zf_common_headfile.h"
#include "PID.h"

#define MOTOR_PID_SAMPLE_TIME  10
#define MOTOR_DEFAULT_KP       0.5f
#define MOTOR_DEFAULT_KI       0.2f
#define MOTOR_DEFAULT_KD       0.1f
#define MOTOR_MAX_SPEED        500
#define MOTOR_OUTPUT_MAX       10000

typedef struct {
    PID_TypeDef pid;
    int16 target_speed;
    int16 current_speed;
    int16 last_count;
    uint32 control_count;
} MotorPID_TypeDef;

void motor_pid_init(float kp, float ki, float kd);
void motor_pid_update(void);
void motor_pid_set_target(int16 target_speed);
int16 motor_pid_get_current_speed(void);
void motor_pid_reset(void);

extern MotorPID_TypeDef motor_ctrl;

#endif
