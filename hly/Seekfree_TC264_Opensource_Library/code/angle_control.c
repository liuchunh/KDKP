/*
 * angle_control.c
 *
 *  Created on: 2026-04-19
 *      Author: ASUS1
 */

#include "zf_common_headfile.h"
#include "angle_control.h"

AngleControl_TypeDef angle_ctrl;

/* 每脉冲对应角度 (度), 初始值由 PPR 和减速比计算, 可通过校准修正 */
static float g_degree_per_pulse = -(360.0f / (ANGLE_PPR * ANGLE_GEAR_RATIO));

// 溢出处理：把int16累积成int32
static int32 accumulated_encoder_count = 0;
static int16 prev_raw_count = 0;
static uint8 encoder_first_read = 1;

/**
 * @brief 读取编码器累积计数值，处理 int16 溢出
 *
 * 通过记录上一次读数并计算差值，将 int16 的编码器值累积为 int32，
 * 避免长时间运行时因溢出导致计数错误。
 *
 * @return 当前累积的编码器计数值（int32）
 *
 * @example
 * @code
 * int32 count = encoder_get_accumulated_count();
 * float angle = (float)(count - zero_count) * ANGLE_DEGREE_PER_PULSE;
 * @endcode
 *
 * @note 首次调用时会以当前读数初始化累积值，之后通过差值累加。
 *       调用前需确保编码器已通过 encoder_quad_init 初始化。
 */
static int32 encoder_get_accumulated_count(void)
{
    int16 raw = encoder_get_count(ANGLE_ENCODER);

    if (encoder_first_read) {
        prev_raw_count = raw;
        accumulated_encoder_count = (int32)raw;
        encoder_first_read = 0;
    } else {
        int32 diff = (int32)raw - (int32)prev_raw_count;
        // 处理int16溢出：修正阈值为32767
        if (diff > 32767) diff -= 65536;
        if (diff < -32768) diff += 65536;
        accumulated_encoder_count += diff;
        prev_raw_count = raw;
    }
    return accumulated_encoder_count;
}

/**
 * @brief 根据编码器计数值计算当前角度（度）
 *
 * 利用零位偏移和每脉冲对应的角度换算系数，将编码器绝对计数值转换为角度值。
 *
 * @param  count  编码器累积计数值（int32）
 * @return 当前角度，单位：度（float），正负表示方向
 *
 * @example
 * @code
 * int32 count = encoder_get_accumulated_count();
 * float angle = angle_control_get_angle_from_count(count);
 * @endcode
 *
 * @note 返回值基于 encoder_zero_count 零位基准，ANGLE_DEGREE_PER_PULSE 为负值
 *       以匹配实际旋转方向。
 */
static float angle_control_get_angle_from_count(int32 count)
{
    return (float)(count - angle_ctrl.encoder_zero_count) * g_degree_per_pulse;
}

void angle_control_init(void) {
    pwm_init(ANGLE_PWM_IN1, 10000, 0);
    pwm_init(ANGLE_PWM_IN2, 10000, 0);

    encoder_quad_init(ANGLE_ENCODER, ANGLE_ENCODER_A_PIN, ANGLE_ENCODER_B_PIN);

    PID_Init(&angle_ctrl.pid, ANGLE_DEFAULT_KP, ANGLE_DEFAULT_KI, ANGLE_DEFAULT_KD, ANGLE_OUTPUT_MAX);
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;
    angle_ctrl.encoder_zero_count = 0;
    angle_ctrl.control_count = 0;

    // 初始化溢出处理状态
    encoder_first_read = 1;
    accumulated_encoder_count = 0;
    prev_raw_count = 0;

    // 零位设为0，与accumulated_encoder_count保持一致
    angle_ctrl.encoder_zero_count = 0;
}

/**
 * @brief 根据控制量设置角度电机的正反转 PWM 占空比 (H桥驱动)
 *
 * 通过 IN1/IN2 两个 PWM 通道实现电机正反转：正值正转、负值反转、零值停止。
 *
 * @param  pwm_value  PWM 控制量，正=正转，负=反转，0=停止
 */
void angle_motor_set_pwm(int32 pwm_value) {
    if (pwm_value > ANGLE_OUTPUT_MAX) {
        pwm_value = ANGLE_OUTPUT_MAX;
    } else if (pwm_value < -ANGLE_OUTPUT_MAX) {
        pwm_value = -ANGLE_OUTPUT_MAX;
    }

    if (pwm_value > 0) {
        /* 右转: IN1=PWM, IN2=0 */
        pwm_set_duty(ANGLE_PWM_IN1, pwm_value);
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    } else if (pwm_value < 0) {
        /* 左转: IN1=0, IN2=PWM */
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, -pwm_value);
    } else {
        /* 停止 */
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    }
}

/**
 * @brief 读取当前角度并执行一次位置式 PID 控制
 *
 * 读取编码器计数 → 计算当前角度 → PID 计算输出 → 死区判断 → 驱动电机。
 * 死区内（误差 < ANGLE_DEAD_BAND）输出归零并清除积分项。
 *
 * @example
 * @code
 * // 在定时中断或主循环中周期调用
 * while (1) {
 *     angle_control_update();
 *     system_delay_ms(10);
 * }
 * @endcode
 *
 * @note 死区触发时会清零 PID 积分项 Integral，防止积分累积。
 *       建议以固定周期调用以保证 PID 控制效果稳定。
 */
void angle_control_update(void) {
    int32 current_count = encoder_get_accumulated_count();
    float pid_output = 0;

    angle_ctrl.current_angle = angle_control_get_angle_from_count(current_count);

    pid_output = PID_Compute(&angle_ctrl.pid, angle_ctrl.target_angle, angle_ctrl.current_angle);

    if ((angle_ctrl.target_angle - angle_ctrl.current_angle < ANGLE_DEAD_BAND) &&
        (angle_ctrl.target_angle - angle_ctrl.current_angle > -ANGLE_DEAD_BAND)) {
        pid_output = 0;
        angle_ctrl.pid.Integral = 0;
    }

    angle_motor_set_pwm((int32)pid_output);
    angle_ctrl.control_count++;
}

/**
 * @brief 设置目标角度，自动限幅在有效范围内
 *
 * 将目标角度限制在 [ANGLE_MIN_DEGREE, ANGLE_MAX_DEGREE] 范围内。
 *
 * @param  target_angle  目标角度（整数，单位：度）
 *
 * @example
 * @code
 * angle_control_set_target(90);   // 目标转到 90°
 * angle_control_set_target(-45);  // 目标转到 -45°
 * @endcode
 *
 * @note 超出 ±360° 的值会被自动截断到边界值。
 */
void angle_control_set_target(int32 target_angle) {
    if (target_angle > ANGLE_MAX_DEGREE) {
        angle_ctrl.target_angle = ANGLE_MAX_DEGREE;
    } else if (target_angle < ANGLE_MIN_DEGREE) {
        angle_ctrl.target_angle = ANGLE_MIN_DEGREE;
    } else {
        angle_ctrl.target_angle = (float)target_angle;
    }
}

/**
 * @brief 在当前角度基础上叠加一个增量旋转
 *
 * 新目标角度 = 当前角度 + delta_angle，内部自动限幅。
 *
 * @param  delta_angle  增量角度（正数顺时针/正转，负数逆时针/反转，单位：度）
 *
 * @example
 * @code
 * angle_control_rotate_relative(30);   // 从当前位置再正转 30°
 * angle_control_rotate_relative(-60);  // 从当前位置反转 60°
 * @endcode
 *
 * @note 当前角度为 float 强转 int32 后再加 delta_angle，存在截断误差。
 *       叠加后的目标角度受 ANGLE_MAX_DEGREE / ANGLE_MIN_DEGREE 限幅。
 */
void angle_control_rotate_relative(int32 delta_angle) {
    angle_control_set_target((int32)(angle_ctrl.current_angle + (float)delta_angle));
}

/**
 * @brief 获取当前角度（整数，单位：度）
 *
 * @return 当前角度值，float 强转 int32 后返回
 *
 * @example
 * @code
 * int32 angle = angle_control_get_current_angle();
 * printf("Current angle: %d\r\n", angle);
 * @endcode
 *
 * @note 返回值为 float 强转 int32，小数部分会被截断而非四舍五入，
 *       如需高精度请直接读取 angle_ctrl.current_angle（float）。
 */
int32 angle_control_get_current_angle(void) {
    return (int32)angle_ctrl.current_angle;
}

/**
 * @brief 重置角度控制器状态，关闭电机输出
 *
 * 清零 PID 状态（积分项、微分项）、目标角度、当前角度、编码器零位及控制计数，
 * 并将两个 PWM 通道输出置零。
 *
 * @example
 * @code
 * angle_control_reset();           // 完全重置，电机停止
 * angle_control_set_target(0);     // 可重新设定目标
 * @endcode
 *
 * @note 调用后零位被重置为 0，若需以当前位置为新零位，
 *       应在 reset 后重新读取编码器值赋给 encoder_zero_count。
 *       运行中调用会立即停止电机并丢失当前位置信息。
 */
void angle_control_reset(void) {
    PID_Reset(&angle_ctrl.pid);
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;
    // 重置溢出处理状态
    encoder_first_read = 1;
    accumulated_encoder_count = 0;
    prev_raw_count = 0;
    // 零位直接设为0，与accumulated_encoder_count保持一致
    angle_ctrl.encoder_zero_count = 0;
    angle_ctrl.control_count = 0;
    pwm_set_duty(ANGLE_PWM_IN1, 0);
    pwm_set_duty(ANGLE_PWM_IN2, 0);
}

/**
 * @brief 校准角度换算系数
 *
 * 用户手动将轮子转到一个已知角度, 然后调用此函数传入实际角度,
 * 程序根据当前编码器读数自动计算正确的每脉冲角度系数。
 *
 * @param actual_angle_deg  轮子当前的实际角度 (度), 正=右转, 负=左转
 *
 * 使用步骤:
 *   1. 先发送 "0" 让轮子回零 (记录零位)
 *   2. 手动把轮子转到一个已知角度 (如转到右转 10°)
 *   3. 发送 "c10" 告诉程序当前实际角度是 10°
 *   4. 程序自动修正换算系数, 之后角度读数就准确了
 */
void angle_control_calibrate(float actual_angle_deg) {
    int32 current_count = encoder_get_accumulated_count();
    int32 delta_count = current_count - angle_ctrl.encoder_zero_count;

    if (delta_count == 0 || actual_angle_deg == 0.0f) {
        return;
    }

    /* 取反: 电机转向与编码器计数方向相反 */
    g_degree_per_pulse = -(actual_angle_deg / (float)delta_count);
}
