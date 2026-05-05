#include "buzzer.h"

void buzzer_init(void)
{
    pwm_init(BUZZER_PWM_CHANNEL, 1000, 0);
}

void buzzer_set_freq_duty(uint32 freq, uint32 duty)
{
    pwm_init(BUZZER_PWM_CHANNEL, freq, duty);
}

void buzzer_stop(void)
{
    pwm_set_duty(BUZZER_PWM_CHANNEL, 0);
}
