#ifndef _BUZZER_H_
#define _BUZZER_H_

#include "zf_common_headfile.h"

#define BUZZER_PWM_CHANNEL    (ATOM2_CH0_P33_10)

void buzzer_init(void);
void buzzer_set_freq_duty(uint32 freq, uint32 duty);
void buzzer_stop(void);

#endif
