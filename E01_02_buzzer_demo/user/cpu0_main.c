/*********************************************************************************************************************
* TC264 Opensourec Library
* Copyright (c) 2022 SEEKFREE
*
* File: cpu0_main
* Platform: TC264D
********************************************************************************************************************/
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"

// **************************** Hardware ****************************
#define BUZZER_PWM              (ATOM2_CH0_P33_10)       // passive buzzer PWM channel

const uint32 music_freq[] = {500, 1000};

int core0_main(void)
{
    clock_init();
    debug_init();

    uint8 freq_index = 0;

    pwm_init(BUZZER_PWM, music_freq[freq_index], 5000);

    cpu_wait_event_ready();
    while (TRUE)
    {
        freq_index++;
        if(freq_index >= sizeof(music_freq) / sizeof(music_freq[0]))
            freq_index = 0;

        pwm_init(BUZZER_PWM, music_freq[freq_index], 5000);

        system_delay_ms(500);
    }
}

#pragma section all restore
