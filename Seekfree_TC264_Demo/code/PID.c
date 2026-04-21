/*
 * PID.c
 *
 *  Created on: 2026-04-07
 *      Author: ASUS1
 *      Description: PID control algorithm with enhanced features
 */

#include "zf_common_headfile.h"

/**
 * @brief Initialize PID controller with parameters
 * @param pid        PID structure pointer
 * @param kp         Proportional coefficient
 * @param ki         Integral coefficient
 * @param kd         Derivative coefficient
 * @param max_output Output limit
 */
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_output) {
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->MaxOutput = max_output;
    pid->IntegralMax = 2000;  // Integral saturation limit

    PID_Reset(pid);
}

/**
 * @brief Reset PID controller state
 * @param pid PID structure pointer
 */
void PID_Reset(PID_TypeDef *pid) {
    pid->Target = 0;
    pid->Current = 0;
    pid->Error = 0;
    pid->LastError = 0;
    pid->Integral = 0;
    pid->Output = 0;
}

/**
 * @brief Calculate PID output
 * @param pid     PID structure pointer
 * @param target  Target value
 * @param current Current measured value
 * @return float  PID output (supports bidirectional)
 */
float PID_Compute(PID_TypeDef *pid, float target, float current) {
    pid->Target = target;
    pid->Current = current;
    pid->Error = pid->Target - pid->Current;

    // 1. Integral term with anti-windup (integral saturation)
    pid->Integral += pid->Error;
    if (pid->Integral > pid->IntegralMax) pid->Integral = pid->IntegralMax;
    if (pid->Integral < -pid->IntegralMax) pid->Integral = -pid->IntegralMax;

    // 2. PID calculation (position form)
    pid->Output = (pid->Kp * pid->Error) +
                  (pid->Ki * pid->Integral) +
                  (pid->Kd * (pid->Error - pid->LastError));

    pid->LastError = pid->Error;

    // 3. Output saturation (supports bidirectional output)
    if (pid->Output > pid->MaxOutput)  pid->Output = pid->MaxOutput;
    if (pid->Output < -pid->MaxOutput) pid->Output = -pid->MaxOutput;

    return pid->Output;
}
