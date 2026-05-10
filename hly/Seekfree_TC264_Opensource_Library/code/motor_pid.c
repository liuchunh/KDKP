/*
 * motor_pid.c
 *
 *  Created on: 2026-04-16
 *      Author: ASUS1
 */


#include "zf_common_headfile.h"
#include "motor_pid.h"
#include "motor.h"

MotorPID_TypeDef motor_ctrl;

void motor_pid_init(float kp, float ki, float kd) {
    motor_init();
    PID_Init(&motor_ctrl.pid, kp, ki, kd, MOTOR_OUTPUT_MAX);
    motor_ctrl.target_speed = 0;
    motor_ctrl.current_speed = 0;
    motor_ctrl.last_count = 0;
    motor_ctrl.control_count = 0;
    motor_speed_reset();
}

void motor_pid_update(void) {
    int16 current_count = get_motor_speed();
    motor_ctrl.current_speed = current_count - motor_ctrl.last_count;
    motor_ctrl.last_count = current_count;
    
    float pid_output = PID_Compute(&motor_ctrl.pid, 
                                    (float)motor_ctrl.target_speed, 
                                    (float)motor_ctrl.current_speed);
    
    set_motor_speed((int32)pid_output);
    motor_ctrl.control_count++;
}

void motor_pid_set_target(int16 target_speed) {
    if (target_speed > MOTOR_MAX_SPEED) {
        motor_ctrl.target_speed = MOTOR_MAX_SPEED;
    }
    else if (target_speed < -MOTOR_MAX_SPEED) {
        motor_ctrl.target_speed = -MOTOR_MAX_SPEED;
    }
    else {
        motor_ctrl.target_speed = target_speed;
    }
}

int16 motor_pid_get_current_speed(void) {
    return motor_ctrl.current_speed;
}

void motor_pid_reset(void) {
    PID_Reset(&motor_ctrl.pid);
    motor_ctrl.target_speed = 0;
    motor_ctrl.current_speed = 0;
    motor_ctrl.last_count = 0;
    set_motor_speed(0);
    motor_speed_reset();
}
