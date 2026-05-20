/*
 * PID.c
 *
 * Description: 位置式 PID 控制算法
 */

#include "zf_common_headfile.h"
#include "pid.h"

/**
 * @brief 初始化PID控制器参数，设置Kp/Ki/Kd系数和输出限幅值，并复位所有状态变量
 * @param pid PID结构体指针
 * @param kp 比例系数，控制响应速度和超调量
 * @param ki 积分系数，消除稳态误差
 * @param kd 微分系数，抑制振荡和超调
 * @param max_output 输出最大限幅值，防止输出过大
 * @note 内部自动将积分限幅IntegralMax设置为2000；调用前确保pid指针非空
 */
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_output) {
    // ------------ 设置PID系数和输出/积分限幅 ------------
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->MaxOutput = max_output;
    pid->IntegralMax = 2000;  // 积分限幅

    // ------------ 复位所有PID状态 ------------
    PID_Reset(pid);
}

/**
 * @brief 复位PID所有状态变量为零，清除历史误差、积分和输出缓存
 * @param pid PID结构体指针
 * @note 在模式切换、停机或控制目标跳变时必须调用，防止历史积分值污染新控制周期
 */
void PID_Reset(PID_TypeDef *pid) {
    // ------------ 清零所有PID状态变量 ------------
    pid->Target = 0;
    pid->Current = 0;
    pid->Error = 0;
    pid->LastError = 0;
    pid->Integral = 0;
    pid->Output = 0;
}

/**
 * @brief 位置式PID计算，根据目标值和当前测量值计算控制输出
 * @param pid PID结构体指针，保存PID系数和状态
 * @param target 目标设定值
 * @param current 当前测量反馈值
 * @retval 经过积分抗饱和和输出双向限幅处理后的PID输出值
 * @note 内部包含积分分离式抗饱和（IntegralMax限幅）和输出对称限幅（MaxOutput）；
 *       每个控制周期调用一次，LastError在调用结束后自动更新
 */
float PID_Compute(PID_TypeDef *pid, float target, float current) {
    // ------------ 记录目标值与当前值，计算偏差 ------------
    pid->Target = target;
    pid->Current = current;
    pid->Error = pid->Target - pid->Current;

    // ------------ 积分累加与抗饱和限幅 ------------
    pid->Integral += pid->Error;
    if (pid->Integral > pid->IntegralMax) pid->Integral = pid->IntegralMax;
    if (pid->Integral < -pid->IntegralMax) pid->Integral = -pid->IntegralMax;

    // ------------ 位置式PID输出计算（P + I + D） ------------
    pid->Output = (pid->Kp * pid->Error) +
                  (pid->Ki * pid->Integral) +
                  (pid->Kd * (pid->Error - pid->LastError));

    pid->LastError = pid->Error;

    // ------------ 输出双向限幅，防止超调过大 ------------
    if (pid->Output > pid->MaxOutput)  pid->Output = pid->MaxOutput;
    if (pid->Output < -pid->MaxOutput) pid->Output = -pid->MaxOutput;

    return pid->Output;
}
