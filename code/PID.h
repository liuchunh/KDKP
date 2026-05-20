/*
 * PID.h
 *
 * Description: 位置式 PID 控制算法
 */

#ifndef CODE_PID_H_
#define CODE_PID_H_

typedef struct {
    float Target;      // 目标值
    float Current;     // 当前反馈值
    float Error;       // 当前误差
    float LastError;   // 上一次误差
    float Integral;    // 积分累计值
    float Kp, Ki, Kd;  // PID 系数
    float Output;      // 当前输出值
    float MaxOutput;   // 输出限幅，默认 10000
    float IntegralMax; // 积分限幅
} PID_TypeDef;

/** @brief 位置式PID计算，根据目标值和当前测量值计算PID输出 */
float PID_Compute(PID_TypeDef *pid, float target, float current);
/** @brief 初始化PID控制器参数（Kp/Ki/Kd）和输出限幅，并复位所有状态 */
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_output);
/** @brief 复位PID所有状态变量为零，包括误差、积分累加和历史值 */
void PID_Reset(PID_TypeDef *pid);

#endif /* CODE_PID_H_ */
