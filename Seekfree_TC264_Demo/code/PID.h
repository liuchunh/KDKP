/*
 * PID.h
 *
 *  Created on: 2026-04-07
 *      Author: ASUS1
 *      Description: PID control algorithm
 */

#ifndef CODE_PID_H_
#define CODE_PID_H_

typedef struct {
    float Target;      // Target value
    float Current;     // Current measured value
    float Error;       // Current error
    float LastError;   // Last error
    float Integral;    // Integral term
    float Kp, Ki, Kd;  // PID coefficients
    float Output;      // Output value
    float MaxOutput;   // Output limit (default 10000)
    float IntegralMax; // Integral limit
} PID_TypeDef;

// ============ Function declarations ============
float PID_Compute(PID_TypeDef *pid, float target, float current);
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_output);
void PID_Reset(PID_TypeDef *pid);

#endif /* CODE_PID_H_ */