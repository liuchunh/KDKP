/*
 * angle_control.c
 *
 * Description: 转向轮角度控制模块
 */

#include "zf_common_headfile.h"
#include "angle_control.h"

AngleControl_TypeDef angle_ctrl;

#define ANGLE_DEGREE_PER_PULSE  (360.0f / (ANGLE_PPR * ANGLE_GEAR_RATIO))

// 用于将int16编码器值累积为int32
int32 accumulated_encoder_count = 0;
static int16 prev_raw_count = 0;
static uint8 encoder_first_read = 1;

/**
 * @brief 读取正交编码器累计计数值，处理 int16 溢出
 * @param 无
 * @note 首次调用时自动初始化基准值；后续调用通过差值检测处理编码器环绕溢出
 * @retval 累计编码器计数值（int32）
 */
static int32 encoder_get_accumulated_count(void)
{
    int16 raw = encoder_get_count(ANGLE_ENCODER);

    // ------------ 首次读取初始化基准值 ------------
    if (encoder_first_read) {
        prev_raw_count = raw;
        accumulated_encoder_count = (int32)raw;
        encoder_first_read = 0;
    } else {
        // ------------ 计算差值并处理编码器溢出 ------------
        int32 diff = (int32)raw - (int32)prev_raw_count;
        // encoder_get_count quad模式下已乘4，范围约8192，差值阈值应相应调整
        if (diff > 8192) diff -= 16384;
        else if (diff < -8192) diff += 16384;
        accumulated_encoder_count += diff;
        prev_raw_count = raw;
    }
    return accumulated_encoder_count;
}

/**
 * @brief 将编码器累计计数值转换为角度值（度）
 * @param count 编码器累计计数值
 * @note 角度值以零点计数值为基准偏移计算，零点需通过 init 或 reset 设定
 * @retval 当前角度值（float，单位：度）
 */
static float angle_control_get_angle_from_count(int32 count)
{
    return (float)(count - angle_ctrl.encoder_zero_count) * ANGLE_DEGREE_PER_PULSE;
}

/**
 * @brief 初始化角度控制模块，配置 PWM、编码器、PID 并标定零点
 * @param 无
 * @note 调用后编码器从当前值开始累计，零点设为 0；需在系统启动时调用一次
 */
void angle_control_init(void) {
    // ------------ 初始化 PWM 输出通道 ------------
    pwm_init(ANGLE_PWM_IN1, 10000, 0);
    pwm_init(ANGLE_PWM_IN2, 10000, 0);

    // ------------ 初始化正交编码器 ------------
    encoder_quad_init(ANGLE_ENCODER, ANGLE_ENCODER_A_PIN, ANGLE_ENCODER_B_PIN);

    // ------------ 初始化 PID 参数与状态变量 ------------
    PID_Init(&angle_ctrl.pid, ANGLE_DEFAULT_KP, ANGLE_DEFAULT_KI, ANGLE_DEFAULT_KD, ANGLE_OUTPUT_MAX);
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;
    angle_ctrl.encoder_zero_count = 0;
    angle_ctrl.control_count = 0;

    // ------------ 初始化编码器累积状态 ------------
    encoder_first_read = 1;
    accumulated_encoder_count = 0;
    prev_raw_count = 0;

    // ------------ 零点设为0，使accumulated_encoder_count重新计数 ------------
    angle_ctrl.encoder_zero_count = 0;
}

/**
 * @brief 根据 PWM 值控制角度电机方向和占空比
 * @param pwm_value PWM 输出值，正值为正转，负值为反转，零为停止
 * @note 内部自动将 pwm_value 限幅到 [-ANGLE_OUTPUT_MAX, ANGLE_OUTPUT_MAX] 范围
 */
void angle_motor_set_pwm(int32 pwm_value) {
    // ------------ 输出值限幅 ------------
    if (pwm_value > ANGLE_OUTPUT_MAX) {
        pwm_value = ANGLE_OUTPUT_MAX;
    } else if (pwm_value < -ANGLE_OUTPUT_MAX) {
        pwm_value = -ANGLE_OUTPUT_MAX;
    }

    // ------------ 根据方向设置 PWM 占空比 ------------
    if (pwm_value > 0) {
        pwm_set_duty(ANGLE_PWM_IN1, pwm_value);
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    } else if (pwm_value < 0) {
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, -pwm_value);
    } else {
        pwm_set_duty(ANGLE_PWM_IN1, 0);
        pwm_set_duty(ANGLE_PWM_IN2, 0);
    }
}

/**
 * @brief 执行一次角度 PID 控制迭代，读取编码器、计算 PID 输出并驱动电机
 * @param 无
 * @note 当角度误差在死区范围内时清除积分项并停止输出，防止抖动；需周期性调用
 */
void angle_control_update(void) {
    // ------------ 读取编码器并计算当前角度 ------------
    int32 current_count = encoder_get_accumulated_count();
    float pid_output = 0;

    angle_ctrl.current_angle = angle_control_get_angle_from_count(current_count);

    // ------------ 执行 PID 计算 ------------
    pid_output = PID_Compute(&angle_ctrl.pid, angle_ctrl.target_angle, angle_ctrl.current_angle);

    // ------------ 死区判断：误差在死区内则清零积分和输出 ------------
    if ((angle_ctrl.target_angle - angle_ctrl.current_angle < ANGLE_DEAD_BAND) &&
        (angle_ctrl.target_angle - angle_ctrl.current_angle > -ANGLE_DEAD_BAND)) {
        pid_output = 0;
        angle_ctrl.pid.Integral = 0;
    }

    // ------------ 输出 PWM 驱动电机并递增控制计数 ------------
    angle_motor_set_pwm((int32)pid_output);
    angle_ctrl.control_count++;
}

/**
 * @brief 设置目标角度，自动限幅到允许范围
 * @param target_angle 目标角度值（int32，单位：度）
 * @note 超出 [ANGLE_MIN_DEGREE, ANGLE_MAX_DEGREE] 范围时自动钳位到边界值
 */
void angle_control_set_target(int32 target_angle) {
    // ------------ 限幅并设置目标角度 ------------
    if (target_angle > ANGLE_MAX_DEGREE) {
        angle_ctrl.target_angle = ANGLE_MAX_DEGREE;
    } else if (target_angle < ANGLE_MIN_DEGREE) {
        angle_ctrl.target_angle = ANGLE_MIN_DEGREE;
    } else {
        angle_ctrl.target_angle = (float)target_angle;
    }
}

/**
 * @brief 在当前角度基础上相对旋转指定角度
 * @param delta_angle 相对角度变化量（int32，单位：度）
 * @note 内部调用 angle_control_set_target，同样受角度限幅约束
 */
void angle_control_rotate_relative(int32 delta_angle) {
    angle_control_set_target((int32)(angle_ctrl.current_angle + (float)delta_angle));
}

/**
 * @brief 获取当前实际角度值
 * @param 无
 * @note 返回值为最近一次 update 计算的角度，非实时编码器值
 * @retval 当前角度（int32，单位：度）
 */
int32 angle_control_get_current_angle(void) {
    return (int32)angle_ctrl.current_angle;
}

/**
 * @brief 复位 PID 状态、清零编码器累积并关闭电机输出
 * @param 无
 * @note 调用后编码器重新从当前值累积，零点重置；适合急停或模式切换场景
 */
void angle_control_reset(void) {
    // ------------ 复位 PID 和状态变量 ------------
    PID_Reset(&angle_ctrl.pid);
    angle_ctrl.target_angle = 0;
    angle_ctrl.current_angle = 0;

    // ------------ 重置编码器累积状态 ------------
    encoder_first_read = 1;
    accumulated_encoder_count = 0;
    prev_raw_count = 0;

    // ------------ 零点直接设为0，使accumulated_encoder_count重新计数 ------------
    angle_ctrl.encoder_zero_count = 0;
    angle_ctrl.control_count = 0;

    // ------------ 关闭 PWM 输出 ------------
    pwm_set_duty(ANGLE_PWM_IN1, 0);
    pwm_set_duty(ANGLE_PWM_IN2, 0);
}
