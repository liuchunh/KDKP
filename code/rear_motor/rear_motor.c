/*
 * rear_motor.c
 *
 * 后轮独立驱动模块实现
 * 架构: 目标 m/s -> 100ms脉冲目标 -> 前馈 + PID修正 -> PWM -> DIR+PWM驱动
 * 编码器: 固定周期 10ms 读取累计差值, 累加至 100ms 供 PID 使用
 */

#include "zf_common_headfile.h"
#include "rear_motor/rear_motor.h"

/* ---- 模块内部状态 ---- */
static float  target_mps      = 0.0f;
static float  actual_mps      = 0.0f;
static int16  current_pwm     = 0;
static int16  encoder_10ms    = 0;
static int32  encoder_100ms   = 0;
static int32  encoder_100ms_last = 0;
static volatile uint32 encoder_sample_count = 0;
static uint32 last_encoder_sample_count = 0;
static uint8  encoder_div = 0;
static int16  last_encoder_count = 0;
static uint8  encoder_first_read = 1;

/* PID 状态 */
static float  integral    = 0.0f;
static float  last_error  = 0.0f;
static int    last_pwm    = 0;

/**
 * @brief 设置后轮电机PWM占空比，包含速率限制、硬限幅和方向控制
 * @param pwm 目标PWM值，正值为正转，负值为反转
 * @note 内部自动进行PWM变化率限制（REAR_PWM_RATE_LIMIT）和绝对值硬限幅（REAR_PWM_HARD_LIMIT）；
 *       PWM>=0时GPIO置高（正转），PWM<0时GPIO置低（反转）；
 *       实际输出的PWM由静态变量last_pwm和current_pwm记录
 */
static void rear_motor_set_pwm(int16 pwm)
{
    // ------------ PWM变化率限制，防止突变 ------------
    int diff = pwm - last_pwm;
    if(diff > REAR_PWM_RATE_LIMIT)  diff = REAR_PWM_RATE_LIMIT;
    if(diff < -REAR_PWM_RATE_LIMIT) diff = -REAR_PWM_RATE_LIMIT;
    last_pwm += diff;

    // ------------ PWM硬限幅 ------------
    if(last_pwm > REAR_PWM_HARD_LIMIT)  last_pwm = REAR_PWM_HARD_LIMIT;
    if(last_pwm < -REAR_PWM_HARD_LIMIT) last_pwm = -REAR_PWM_HARD_LIMIT;

    current_pwm = last_pwm;

    // ------------ 根据PWM符号设置方向和占空比 ------------
    if(current_pwm >= 0)
    {
        pwm_set_duty(PWM_L,  current_pwm);
        gpio_set_level(MOTOR_GPIO_L, 1);
        pwm_set_duty(PWM_R,  current_pwm);
        gpio_set_level(MOTOR_GPIO_R, 1);
    }
    else
    {
        pwm_set_duty(PWM_L,  -current_pwm);
        gpio_set_level(MOTOR_GPIO_L, 0);
        pwm_set_duty(PWM_R,  -current_pwm);
        gpio_set_level(MOTOR_GPIO_R, 0);
    }
}

/**
 * @brief 初始化后轮电机模块，配置PWM频率和GPIO方向，清零所有状态变量
 * @note 调用前确保PWM_L/PWM_R引脚和MOTOR_GPIO_L/MOTOR_GPIO_R宏已正确定义；
 *       PWM频率设为17000Hz；首次读取编码器计数作为基准值
 */
void rear_motor_init(void)
{
    // ------------ 初始化PWM和GPIO硬件 ------------
    pwm_init(PWM_L, 17000, 0);
    pwm_init(PWM_R, 17000, 0);
    gpio_init(MOTOR_GPIO_L, GPO, 1, GPO_PUSH_PULL);
    gpio_init(MOTOR_GPIO_R, GPO, 1, GPO_PUSH_PULL);

    // ------------ 清零速度、PWM和编码器状态 ------------
    target_mps  = 0.0f;
    actual_mps  = 0.0f;
    current_pwm = 0;
    encoder_10ms  = 0;
    encoder_100ms = 0;
    encoder_100ms_last = 0;
    encoder_sample_count = 0;
    last_encoder_sample_count = 0;
    encoder_div = 0;
    last_encoder_count = encoder_get_count(TIM2_ENCODER);
    encoder_first_read = 0;

    // ------------ 清零PID状态 ------------
    integral    = 0.0f;
    last_error  = 0.0f;
    last_pwm    = 0;
}

/**
 * @brief 停止后轮电机，复位目标速度为零，清除PID状态和编码器累加值，PWM输出置零
 * @note 同时设置GPIO为高电平（使能端无效），确保电机完全停止；
 *       encoder_first_read清零，下次启动时重新初始化编码器基准值
 */
void rear_motor_stop(void)
{
    // ------------ 清零目标速度和PID状态 ------------
    target_mps  = 0.0f;
    integral    = 0.0f;
    last_error  = 0.0f;
    last_pwm    = 0;

    // ------------ 清零编码器累加值 ------------
    encoder_100ms = 0;
    encoder_100ms_last = 0;
    encoder_div = 0;
    encoder_10ms = 0;
    last_encoder_count = encoder_get_count(TIM2_ENCODER);
    encoder_first_read = 0;

    // ------------ 关闭PWM输出 ------------
    pwm_set_duty(PWM_L, 0);
    pwm_set_duty(PWM_R, 0);
    gpio_set_level(MOTOR_GPIO_L, 1);
    gpio_set_level(MOTOR_GPIO_R, 1);
    current_pwm = 0;
}

/**
 * @brief 设置后轮目标速度（m/s），超出限幅自动钳位，目标为零时清除PID积分
 * @param mps 目标速度，单位：米/秒
 * @note 输入值被限制在[REAR_SPEED_MIN_MPS, REAR_SPEED_MAX_MPS]范围内；
 *       当目标为零时自动清除积分和误差历史，防止重新启动时积分残留导致突跳
 */
void rear_motor_set_target_mps(float mps)
{
    // ------------ 速度限幅 ------------
    if(mps > REAR_SPEED_MAX_MPS)  mps = REAR_SPEED_MAX_MPS;
    if(mps < REAR_SPEED_MIN_MPS)  mps = REAR_SPEED_MIN_MPS;
    target_mps = mps;

    // ------------ 目标为零时清除PID积分历史 ------------
    if(mps == 0.0f)
    {
        integral   = 0.0f;
        last_error = 0.0f;
        last_pwm   = 0;
    }
}

/**
 * @brief 每10ms定时调用，读取TIM2编码器累计差值并记录采样计数
 * @note 首次调用时初始化基准值，不清零编码器硬件计数以避免与惯导共用TIM2时互相抢数据；
 *       每次调用递增encoder_sample_count，供100ms更新函数判断是否有新数据
 */
void rear_motor_encoder_update_10ms(void)
{
    // ------------ 读取当前编码器计数 ------------
    int16 current_count = encoder_get_count(TIM2_ENCODER);

    // ------------ 首次读取：初始化基准值 ------------
    if(encoder_first_read)
    {
        last_encoder_count = current_count;
        encoder_10ms = 0;
        encoder_first_read = 0;
    }
    // ------------ 非首次读取：计算差值 ------------
    else
    {
        encoder_10ms = (int16)calculate_delta(current_count, last_encoder_count);
        last_encoder_count = current_count;
    }

    encoder_sample_count++;
}

/**
 * @brief 主循环调用：累积10ms编码器样本，每100ms执行一次PID前馈+反馈控制
 * @note 依赖rear_motor_encoder_update_10ms()提供新样本；无新样本时直接返回；
 *       每10次10ms（100ms）触发一次PID计算：
 *       实际速度 -> 目标脉冲 -> 误差 -> 积分抗饱和 -> 微分 -> 前馈+PID合成 -> PWM输出；
 *       目标速度为零时自动调用rear_motor_stop()
 */
void rear_motor_pid_update_100ms(void)
{
    // ------------ 无新编码器样本则直接返回 ------------
    if(last_encoder_sample_count == encoder_sample_count)
    {
        return;
    }

    // ------------ 更新实际速度和编码器累加 ------------
    last_encoder_sample_count = encoder_sample_count;
    actual_mps = (float)((int32)encoder_10ms * 10) / REAR_EFFECTIVE_PPR * REAR_WHEEL_CIRCUM_M / 0.1f;
    encoder_100ms += (int32)encoder_10ms;
    encoder_div++;

    // ------------ 不满10次（100ms）则等待 ------------
    if(encoder_div < 10)
    {
        return;
    }

    // ------------ 满100ms，执行PID计算 ------------
    encoder_div = 0;
    encoder_100ms_last = encoder_100ms;

    // ------------ 目标速度为零时停止电机 ------------
    if(target_mps == 0.0f)
    {
        encoder_100ms = 0;
        rear_motor_stop();
        return;
    }

    // ------------ 计算目标脉冲数和误差 ------------
    float target_pulses = target_mps * REAR_EFFECTIVE_PPR / REAR_WHEEL_CIRCUM_M * 0.1f;
    float error = target_pulses - (float)encoder_100ms;

    // ------------ 积分抗饱和（死区阈值内才积分） ------------
    if(error < REAR_INTEGRAL_THRESHOLD && error > -REAR_INTEGRAL_THRESHOLD)
    {
        integral += error * 0.1f;
        if(integral > REAR_INTEGRAL_LIMIT)   integral = REAR_INTEGRAL_LIMIT;
        if(integral < -REAR_INTEGRAL_LIMIT)  integral = -REAR_INTEGRAL_LIMIT;
    }

    // ------------ 微分计算 ------------
    float derivative = (error - last_error) / 0.1f;
    last_error = error;

    // ------------ 前馈 + PID反馈合成PWM输出 ------------
    float ff     = target_pulses * REAR_FF_GAIN;
    float pid    = REAR_KP * error + REAR_KI * integral + REAR_KD * derivative;
    float pwm_f  = ff + pid;

    encoder_100ms = 0;
    rear_motor_set_pwm((int16)pwm_f);
}

/**
 * @brief 获取当前设置的目标速度（m/s）
 * @retval 目标速度值，单位：米/秒
 */
float  rear_motor_get_target_mps(void)      { return target_mps; }
/**
 * @brief 获取最近一次计算的实际速度（m/s）
 * @retval 实际速度值，单位：米/秒
 */
float  rear_motor_get_speed_mps(void)       { return actual_mps; }
/**
 * @brief 获取当前PWM输出值
 * @retval 当前PWM值，范围[-REAR_PWM_HARD_LIMIT, REAR_PWM_HARD_LIMIT]
 */
int16  rear_motor_get_pwm(void)             { return current_pwm; }
/**
 * @brief 获取最近一次10ms编码器差值
 * @retval 10ms编码器脉冲差值
 */
int16  rear_motor_get_encoder_10ms(void)    { return encoder_10ms; }
/**
 * @brief 获取上一次100ms周期内累计的编码器脉冲数
 * @retval 100ms累计脉冲数
 */
int32  rear_motor_get_encoder_100ms(void)   { return encoder_100ms_last; }
