#include "motor.h"

/* ---- 电机1 ---- */
void motor1_init(void)
{
    gpio_init(MOTOR1_DIR_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);
    pwm_init(MOTOR1_PWM_CH, MOTOR_PWM_FREQ, 0);
}

void motor1_set_duty(int16 duty)
{
    if (duty >= 0) {
        gpio_set_level(MOTOR1_DIR_PIN, GPIO_HIGH);
        pwm_set_duty(MOTOR1_PWM_CH, (uint16)duty);
    } else {
        gpio_set_level(MOTOR1_DIR_PIN, GPIO_LOW);
        pwm_set_duty(MOTOR1_PWM_CH, (uint16)(-duty));
    }
}

void motor1_stop(void)
{
    pwm_set_duty(MOTOR1_PWM_CH, 0);
}

/* ---- 电机2 ---- */
void motor2_init(void)
{
    gpio_init(MOTOR2_DIR_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);
    pwm_init(MOTOR2_PWM_CH, MOTOR_PWM_FREQ, 0);
}

void motor2_set_duty(int16 duty)
{
    if (duty >= 0) {
        gpio_set_level(MOTOR2_DIR_PIN, GPIO_HIGH);
        pwm_set_duty(MOTOR2_PWM_CH, (uint16)duty);
    } else {
        gpio_set_level(MOTOR2_DIR_PIN, GPIO_LOW);
        pwm_set_duty(MOTOR2_PWM_CH, (uint16)(-duty));
    }
}

void motor2_stop(void)
{
    pwm_set_duty(MOTOR2_PWM_CH, 0);
}
