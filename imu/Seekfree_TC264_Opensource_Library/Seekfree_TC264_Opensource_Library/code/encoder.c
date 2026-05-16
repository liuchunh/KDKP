#include "encoder.h"

static volatile int16 g_encoder1_speed = 0;
static volatile int16 g_encoder2_speed = 0;

/* 编码器更新 (由PIT中断调用, 10ms周期) */
void encoder_irq_update(void)
{
    g_encoder1_speed = encoder_get_count(ENCODER1_TIM);
    encoder_clear_count(ENCODER1_TIM);
    g_encoder2_speed = encoder_get_count(ENCODER2_TIM);
    encoder_clear_count(ENCODER2_TIM);
}

/* ---- 编码器1 ---- */
void encoder1_init(void)
{
    encoder_quad_init(ENCODER1_TIM, ENCODER1_A_PIN, ENCODER1_B_PIN);
}

void encoder1_update(void)
{
    /* 兼容旧代码, 不再需要 */
}

int16 encoder1_get_speed(void)
{
    return g_encoder1_speed;
}

float encoder1_get_rpm(void)
{
    return ((float)g_encoder1_speed / (float)ENCODER_PPR)
         * (60000.0f / (float)ENCODER_SAMPLE_MS);
}

/* ---- 编码器2 ---- */
void encoder2_init(void)
{
    encoder_quad_init(ENCODER2_TIM, ENCODER2_A_PIN, ENCODER2_B_PIN);
}

void encoder2_update(void)
{
    /* 兼容旧代码, 不再需要 */
}

int16 encoder2_get_speed(void)
{
    return g_encoder2_speed;
}

float encoder2_get_rpm(void)
{
    return ((float)g_encoder2_speed / (float)ENCODER_PPR)
         * (60000.0f / (float)ENCODER_SAMPLE_MS);
}
