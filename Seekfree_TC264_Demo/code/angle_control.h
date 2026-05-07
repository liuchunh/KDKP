/*
 * angle_control.h
 *
 *  Created on: 2026-04-19
 *      Author: ASUS1
 *      Description: �������������Ķ���Ƕȿ���
 */

#ifndef CODE_ANGLE_CONTROL_H_
#define CODE_ANGLE_CONTROL_H_
#include "zf_common_headfile.h"
#include "PID.h"

// �Ƕȵ�� PWM ���Ŷ���
#define ANGLE_PWM_IN1       ATOM0_CH7_P02_7        // ��ת PWM
#define ANGLE_PWM_IN2       ATOM0_CH6_P02_6        // ��ת PWM

// ����������
#define ANGLE_ENCODER       TIM4_ENCODER
#define ANGLE_ENCODER_A_PIN TIM4_ENCODER_CH1_P02_8 // �ű����� A ��
#define ANGLE_ENCODER_B_PIN TIM4_ENCODER_CH2_P00_9 // �ű����� B ��

// ���������������
#define ANGLE_PPR           1024                    // ������ÿȦ����
#define ANGLE_GEAR_RATIO    600             // ���ٱ�
#define ANGLE_MAX_DEGREE    360                    // ���Ŀ��Ƕ�
#define ANGLE_MIN_DEGREE    -360                   // ��СĿ��Ƕ�
#define ANGLE_DEFAULT_KP    30.0f
#define ANGLE_DEFAULT_KI    0.0f
#define ANGLE_DEFAULT_KD    0.0f
#define ANGLE_OUTPUT_MAX    10000
#define ANGLE_DEAD_BAND     5.0f

typedef struct {
    PID_TypeDef pid;          // �ǶȻ� PID ������״̬
    float target_angle;       // Ŀ��Ƕ�
    float current_angle;      // ��ǰ�Ƕ�
    int32 encoder_zero_count; // ��λ��Ӧ�ı���������
    uint32 control_count;     // ����ѭ��ִ�д���
} AngleControl_TypeDef;

extern AngleControl_TypeDef angle_ctrl;

// ��ʼ���Ƕȿ���ģ�飬���� PWM���������� PID ����
void angle_control_init(void);

// ��ȡ��ǰ�ǶȲ�ִ��һ��λ��ʽ PID ����
void angle_control_update(void);

// ����Ŀ��Ƕȣ���������������Χ��
void angle_control_set_target(int32 target_angle);

// �ڵ�ǰ�ǶȻ���������һ�����ת��
void angle_control_rotate_relative(int32 delta_angle);

// ���ݿ��������õ������ת PWM ռ�ձ�
void angle_motor_set_pwm(int32 pwm_value);

// ��ȡ��ǰ�Ƕ�
int32 angle_control_get_current_angle(void);

// ��� PID ״̬�����¼�¼��λ���رյ�����
void angle_control_reset(void);

#endif /* CODE_ANGLE_CONTROL_H_ */
