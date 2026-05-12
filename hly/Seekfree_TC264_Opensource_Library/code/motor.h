/*
 * motor.h
 *
 *  Created on: 2026-04-16
 *      Author: ASUS1
 *      Description: Motor driver with encoder feedback
 */

#ifndef CODE_MOTOR_H_
#define CODE_MOTOR_H_
#include "zf_common_headfile.h"

// Motor PWM pin definitions
#define pwm_L_IN1  ATOM0_CH4_P02_4   // Left motor forward
#define pwm_L_IN2  ATOM0_CH5_P02_5   // Left motor backward

// Encoder definitions (configurable per hardware)
#define MOTOR_ENCODER  TIM5_ENCODER   // Using TIM5 as motor encoder
#define ENCODER_CH1    TIM5_ENCODER_CH1_P21_7
#define ENCODER_CH2    TIM5_ENCODER_CH2_P21_6

// Encoder parameters
#define ENCODER_PPR    20             // Pulses Per Revolution
#define GEAR_RATIO     1              // Gear reduction ratio

// ============ Function declarations ============
void motor_init(void);
void set_motor_speed(int32 speed);
int16 get_motor_speed(void);        // Get motor speed (pulse count)
void motor_speed_reset(void);       // Reset encoder counter

#endif /* CODE_MOTOR_H_ */
