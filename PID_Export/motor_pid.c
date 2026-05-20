#include "motor_pid.h"

/* 限幅函数 */
static float clampf(float val, float min, float max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static int16 clampi16(int16 val, int16 min, int16 max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/* PWM变化率限幅 */
static int16 rate_limit_pwm(int16 new_duty, int16 prev_duty)
{
    int16 diff = new_duty - prev_duty;
    if (diff > PWM_RATE_LIMIT) diff = PWM_RATE_LIMIT;
    if (diff < -PWM_RATE_LIMIT) diff = -PWM_RATE_LIMIT;
    return prev_duty + diff;
}

void motor_pid_init(MotorPID *pid, float kp, float ki, float kd, float ff_gain)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->ff_gain = ff_gain;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->target = 0.0f;
    pid->output = 0;
    pid->prev_output = 0;
    pid->enabled = 0;
}

void motor_pid_set_target(MotorPID *pid, float target)
{
    pid->target = target;
    pid->enabled = 1;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

int16 motor_pid_update(MotorPID *pid, int32 feedback)
{
    if (!pid->enabled || pid->target <= 0) {
        return pid->output;
    }

    float error = pid->target - (float)feedback;

    /* 积分分离: 误差大时不积分 */
    if (error < INTEGRAL_THRESHOLD && error > -INTEGRAL_THRESHOLD) {
        pid->integral += error * 0.1f;
        pid->integral = clampf(pid->integral, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);
    }

    float derivative = (error - pid->prev_error) / 0.1f;
    pid->prev_error = error;

    /* 前馈: 基础PWM按目标速度线性映射 */
    float ff_pwm = pid->target * pid->ff_gain;

    /* PID: 只修正误差 */
    float pid_pwm = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;

    /* 总输出 = 前馈 + PID */
    float pwm = ff_pwm + pid_pwm;

    /* 硬限幅 */
    int16 new_duty = (int16)clampf(pwm, -PWM_HARD_LIMIT, PWM_HARD_LIMIT);

    /* 变化率限幅 */
    new_duty = rate_limit_pwm(new_duty, pid->prev_output);
    pid->prev_output = new_duty;
    pid->output = new_duty;

    return pid->output;
}

void motor_pid_stop(MotorPID *pid)
{
    pid->enabled = 0;
    pid->target = 0.0f;
    pid->output = 0;
    pid->prev_output = 0;
}

void motor_pid_reset(MotorPID *pid)
{
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->prev_output = 0;
}

float pulses_to_mps(int32 pulses)
{
    return ((float)pulses / (float)EFFECTIVE_PPR) * WHEEL_CIRCUM_M / SAMPLE_SEC;
}

float mps_to_kmh(float mps)
{
    return mps * 3.6f;
}
