/*
 * angle_control.h
 *
 *  Created on: 2026-04-19
 *      Author: ASUS1
 *      Description: Angle control for steering motor with encoder feedback
 */

#ifndef CODE_ANGLE_CONTROL_H_
#define CODE_ANGLE_CONTROL_H_
#include "zf_common_headfile.h"
#include "PID.h"

// Angle motor PWM pin definitions
#define ANGLE_PWM_IN1      ATOM0_CH7_P02_7   // Forward PWM
#define ANGLE_PWM_IN2      ATOM0_CH6_P02_6   // Reverse PWM

// Encoder definitions
#define ANGLE_ENCODER      TIM4_ENCODER
#define ANGLE_ENCODER_COUNT_PIN TIM4_ENCODER_CH1_P02_8  // Pulse input pin for TIM4
#define ANGLE_ENCODER_DIR_PIN   TIM4_ENCODER_CH2_P00_9  // Direction input pin for TIM4

// Encoder parameters
#define ANGLE_PPR          20                // Pulses Per Revolution (pulse+direction encoder)
#define ANGLE_GEAR_RATIO   1                 // Gear reduction ratio
#define ANGLE_MAX_DEGREE   360               // Max angle in degrees
#define ANGLE_MIN_DEGREE   -360              // Min angle in degrees

typedef struct {
    PID_TypeDef pid;
    int32 target_angle;    // Target angle in degrees
    int32 current_angle;   // Current angle in degrees
    int32 last_count;      // Last encoder count
    uint32 control_count;  // Control loop count
} AngleControl_TypeDef;

extern AngleControl_TypeDef angle_ctrl;

// ============ Function declarations ============
void angle_control_init(void);
void angle_control_update(void);
void angle_control_set_target(int32 target_angle);
void angle_motor_set_pwm(int32 pwm_value);
int32 angle_control_get_current_angle(void);
void angle_control_reset(void);

#endif /* CODE_ANGLE_CONTROL_H_ */
