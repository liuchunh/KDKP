#ifndef _encoder_h_
#define _encoder_h_

#include "zf_common_headfile.h"

/* ---- 编码器1 (电机1) ---- */
#define ENCODER1_TIM        (TIM2_ENCODER)
#define ENCODER1_A_PIN      (TIM2_ENCODER_CH1_P33_7)
#define ENCODER1_B_PIN      (TIM2_ENCODER_CH2_P33_6)

/* ---- 编码器2 (电机2) ---- */
#define ENCODER2_TIM        (TIM5_ENCODER)
#define ENCODER2_A_PIN      (TIM5_ENCODER_CH1_P10_3)
#define ENCODER2_B_PIN      (TIM5_ENCODER_CH2_P10_1)

/* 编码器参数 */
#define ENCODER_PPR         (1024)
#define ENCODER_SAMPLE_MS   (100)       /* 采样周期 ms */

extern volatile uint32 g_encoder_sample_count;

/* 编码器1 */
void    encoder1_init           (void);
void    encoder1_update         (void);
int16   encoder1_get_speed      (void);
float   encoder1_get_rpm        (void);

/* 编码器2 */
void    encoder2_init           (void);
void    encoder2_update         (void);
int16   encoder2_get_speed      (void);
float   encoder2_get_rpm        (void);

#endif
