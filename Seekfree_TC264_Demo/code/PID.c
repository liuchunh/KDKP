/*
 * PID.c
 *
 *  Created on: 2026-04-07
 *      Author: ASUS1
 *      Description: 位置式 PID 控制算法
 */

#include "zf_common_headfile.h"

// 初始化 PID 参数
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_output) {
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->MaxOutput = max_output;
    pid->IntegralMax = 2000;  // 积分限幅

    PID_Reset(pid);
}

// 清空 PID 历史状态
void PID_Reset(PID_TypeDef *pid) {
    pid->Target = 0;
    pid->Current = 0;
    pid->Error = 0;
    pid->LastError = 0;
    pid->Integral = 0;
    pid->Output = 0;
}

// 计算位置式 PID 输出
float PID_Compute(PID_TypeDef *pid, float target, float current) {
    pid->Target = target;
    pid->Current = current;
    pid->Error = pid->Target - pid->Current;

    // 积分项累加，并做抗积分饱和限幅。
    pid->Integral += pid->Error;
    if (pid->Integral > pid->IntegralMax) pid->Integral = pid->IntegralMax;
    if (pid->Integral < -pid->IntegralMax) pid->Integral = -pid->IntegralMax;

    // 位置式 PID：输出由比例、积分、微分三项直接叠加得到。
    pid->Output = (pid->Kp * pid->Error) +
                  (pid->Ki * pid->Integral) +
                  (pid->Kd * (pid->Error - pid->LastError));

    pid->LastError = pid->Error;

    // 输出双向限幅，兼容正反转控制。
    if (pid->Output > pid->MaxOutput)  pid->Output = pid->MaxOutput;
    if (pid->Output < -pid->MaxOutput) pid->Output = -pid->MaxOutput;

    return pid->Output;
}
