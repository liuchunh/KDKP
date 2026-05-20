/*
 * formula.c
 *
 *  Created on: 2025年11月21日
 *      Author: 18905
 */

#include "zf_common_headfile.h"

KalmanWithComp klm_lat;
KalmanWithComp klm_lon;

/**
 * @brief 初始化带速度补偿的卡尔曼滤波器
 * @param kf 卡尔曼滤波器结构体指针
 * @param Q 过程噪声协方差
 * @param R 观测噪声协方差
 * @param dt 采样周期（秒）
 * @param comp_ratio 滞后补偿系数（推荐 0.3~0.5）
 * @note 初始协方差 P 设为 1.0，初始位置和速度均为 0
 */
void KWC_Init(KalmanWithComp *kf, double Q, double R, double dt, double comp_ratio) {
    // ------------ 初始化滤波器状态与参数 ------------
    kf->x = 0.0f;
    kf->P = 1.0;      // 初始不确定度从 1e-9f
    kf->Q = Q;
    kf->R = R;

    // ------------ 初始化速度补偿相关变量 ------------
    kf->last_x = 0.0f;
    kf->velocity = 0.0f;
    kf->dt = dt;
    kf->comp_ratio = comp_ratio;  // 推荐 0.3~0.5
}

/**
 * @brief 卡尔曼滤波器更新，含速度估计与滞后补偿
 * @param kf 卡尔曼滤波器结构体指针
 * @param z 当前观测值
 * @note 先对观测值做卡尔曼滤波，再估计速度，最后叠加滞后补偿输出
 * @retval 滤波并补偿后的输出值（double）
 */
double KWC_UpdateFast(KalmanWithComp *kf, double z) {
    // ------------ 卡尔曼滤波预测与更新 ------------
        float P_pred = kf->P + kf->Q;
        float K = P_pred / (P_pred + kf->R);
        float filtered = kf->x + K * (z - kf->x);
        kf->P = (1.0f - K) * P_pred;

        // ------------ 速度估计（指数加权平滑） ------------
        float raw_vel = (filtered - kf->last_x) / kf->dt;
        kf->velocity = 0.7f * kf->velocity + 0.3f * raw_vel;

        // ------------ 滞后补偿 ------------
        float output = filtered + kf->velocity * kf->dt * kf->comp_ratio;

        // ------------ 更新状态 ------------
        kf->last_x = filtered;
        kf->x = filtered;

        return output;

}

/**
 * @brief 计算 int16 差值并处理溢出环绕
 * @param current 当前 int16 值
 * @param last 上一次 int16 值
 * @note int16 范围 [-32768, 32767]，当差值超过半范围时认为发生了溢出环绕
 * @retval 处理溢出后的差值（int32）
 */
 int32_t calculate_delta(int16_t current, int16_t last)
{
    // ------------ 计算差值 ------------
    int32_t delta = (int32_t)current - (int32_t)last;

    // ------------ 检测并修正溢出环绕 ------------
    if (delta > 32767)
    {
        delta -= 65536;
    }
    else if (delta < -32768)
    {
        delta += 65536;
    }

    return delta;
}

/**
 * @brief 将 float 值限幅到 [min, max] 范围
 * @param value 指向待限幅值的指针
 * @param min 下限值
 * @param max 上限值
 * @note 通过指针直接修改原变量的值；min 必须小于 max
 */
void Value_Limit_float(float* value , float min ,float max)
{
    *value =((*value)<(min)?(min):((*value)>(max)?(max):(*value)));
}

/**
 * @brief 将 int 值限幅到 [min, max] 范围
 * @param value 指向待限幅值的指针
 * @param min 下限值
 * @param max 上限值
 * @note 通过指针直接修改原变量的值；min 必须小于 max
 */
void Value_Limit_int(int * value , int min ,int max)
{
    *value =((*value)<(min)?(min):((*value)>(max)?(max):(*value)));
}

/**
 * @brief 位置式 PID 算法
 * @param p PID 参数结构体指针
 * @param now 当前实际值（反馈值）
 * @note 积分项受 limit 限幅；输出受 out_min/out_max 限幅；Ki=0 时积分项清零
 */
void PID_Place(_pid*p ,float now)
{
    // ------------ 计算偏差 ------------
    p->err = now;

    p->err_last = p->err;

    // ------------ 积分计算与限幅 ------------
    if(p->Ki == 0)p->i_result =0;
    else if (p->Ki != 0)p->i_result +=p->Ki*p->err;
    if(p->i_result >p->limit)p->i_result = p->limit;
    if(p->i_result < -p->limit)p->i_result = p->limit;

    // ------------ 比例计算 ------------
    p->p_result =p->Kp * p->err;

    // ------------ 微分计算 ------------
    p->d_result =p->Kd * (p->err - p->err_last) ;

    // ------------ 合成输出并限幅 ------------
    p->out = p->p_result + p->i_result + p->d_result;


   Value_Limit_float(&p->out,p->out_min,p->out_max);



}

/**
 * @brief 增量式 PID 算法
 * @param p PID 参数结构体指针
 * @param now 当前实际值（反馈值）
 * @note 偏差 = target_val - now；输出增量 = Kp*(e(k)-e(k-1)) + Ki*e(k) + Kd*(e(k)+e(k-2)-2e(k-1))
 */
void PID_Up(_pid*p  ,float now)
{
    // ------------ 计算当前偏差 ------------
    p->err = p->target_val - now;

    // ------------ 比例项（增量） ------------
    p->p_result = p->Kp * (p->err - p->err_last);

    // ------------ 积分项 ------------
    p->i_result = p->Ki * p->err;

    // ------------ 微分项 ------------
    p->d_result = p->Kd * (p->err + p->err_previous - 2.0 * p->err_last);

    // ------------ 合成输出并限幅 ------------
    p->out = p->p_result + p->i_result + p->d_result;

    if(p->out >p->out_max)p->out = p->out_max;
    if(p->out <p->out_min)p->out = p->out_min;

    // ------------ 更新历史偏差 ------------
    p->err_last = p->err;
    p->err_previous  = p->err_last  ;


}
