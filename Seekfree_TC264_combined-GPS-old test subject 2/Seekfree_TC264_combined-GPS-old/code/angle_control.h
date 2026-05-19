/*
 * angle_control.h
 *
 * Angle motor position PID control with encoder feedback
 * Adapted to use pid_controller.h API (PidParams + PidState)
 */

#ifndef CODE_ANGLE_CONTROL_H_
#define CODE_ANGLE_CONTROL_H_

#include "zf_common_headfile.h"
#include "pid_controller.h"

/* Angle motor PWM pin definitions */
#define ANGLE_PWM_IN1       ATOM0_CH7_P02_7        /* Forward PWM */
#define ANGLE_PWM_IN2       ATOM0_CH6_P02_6        /* Reverse PWM */

/* Angle encoder definitions */
#define ANGLE_ENCODER       TIM4_ENCODER
#define ANGLE_ENCODER_A_PIN TIM4_ENCODER_CH1_P02_8 /* Encoder A pin */
#define ANGLE_ENCODER_B_PIN TIM4_ENCODER_CH2_P00_9 /* Encoder B pin */

/* Encoder mechanical parameters */
#define ANGLE_PPR           1024                   /* Pulses per revolution (quad mode: HW1024/4) */
#define ANGLE_GEAR_RATIO    300                    /* Gear reduction ratio */
#define ANGLE_MAX_DEGREE    60                     /* Max target angle (degrees) */
#define ANGLE_MIN_DEGREE    -60                    /* Min target angle (degrees) */
#define ANGLE_DEFAULT_KP    320.0f
#define ANGLE_DEFAULT_KI    500.0f
#define ANGLE_DEFAULT_KD    1.0f
#define ANGLE_OUTPUT_MAX    10000
#define ANGLE_OUTPUT_MIN    3000
#define ANGLE_MOTOR_DIR     (-1)
#define ANGLE_ENCODER_DIR   (-1)
#define ANGLE_DEAD_BAND     0.1f                   /* Dead band (degrees) */
#define ANGLE_PID_DT        0.01f                  /* Control period 10ms */

typedef struct {
    PidParams params;           /* PID parameters */
    PidState  state;            /* PID state */
    float target_angle;         /* Target angle (degrees) */
    float current_angle;        /* Current angle (degrees) */
    int32 encoder_zero_count;   /* Encoder count at zero position */
    uint32 control_count;       /* Control loop execution count */
} AngleControl_TypeDef;

extern AngleControl_TypeDef angle_ctrl;
extern int32 accumulated_encoder_count;

void angle_control_init(void);
void angle_control_update(void);
void angle_control_set_target(int32 target_angle);
void angle_control_rotate_relative(int32 delta_angle);
void angle_motor_set_pwm(int32 pwm_value);
int32 angle_control_get_current_angle(void);
int32 angle_control_get_target_angle(void);
void angle_control_reset(void);

#endif /* CODE_ANGLE_CONTROL_H_ */
