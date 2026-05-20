/*
 * formula.h
 *
 *  Created on: 2025年11月21日
 *      Author: 18905
 */

#ifndef CODE_FORMULA_H_
#define CODE_FORMULA_H_

//外部变量 extern


//宏定义
typedef struct
{
    float target_val;               //目标值
    float actual_val;               //实际值
//    float integral;                 //积分累积值

    float err;                      //当前偏差值
    float err_last;                 //  e(k-1)
    float err_previous;             //  e(k-2)

    float Kp;               //比例、积分、微分系数
    float Ki;               //比例、积分、微分系数
    float Kd;               //比例、积分、微分系数
    float limit;            //积分限幅
    float p_result;         //比例、积分、微分计算结果
    float i_result;         //比例、积分、微分计算结果
    float d_result;         //比例、积分、微分计算结果
//    float inte_exce;        //积分过冲

    float out;                 //输出
    float out_max;                 //输出限幅
    float out_min;                 //输出限幅
} _pid;

typedef struct {
    // 卡尔曼滤波参数
    double x;      // 滤波后位置
    double P;      // 协方差

    // 参数
    double Q;      // 过程噪声
    double R;      // 观测噪声

    // 滞后补偿
    double last_x;     // 上一次滤波值
    double velocity;   // 估计速度（m/s）
    double dt;         // 采样周期（秒）

    // 系数
    double comp_ratio; // 补偿系数（0~1）
} KalmanWithComp;

extern KalmanWithComp klm_lat;
extern KalmanWithComp klm_lon;

//函数

/**
 * @brief 位置式 PID 算法
 */
void PID_Place(_pid*p,float now);

/**
 * @brief 增量式 PID 算法
 */
void PID_Up(_pid*p,float now);

/**
 * @brief 将 float 值限幅到 [min, max] 范围
 */
void Value_Limit_float(float* value , float min ,float max);

/**
 * @brief 将 int 值限幅到 [min, max] 范围
 */
void Value_Limit_int(int * value , int min ,int max);

/**
 * @brief 计算 int16 差值并处理溢出环绕
 */
int32_t calculate_delta(int16_t current, int16_t last);

/**
 * @brief 初始化带速度补偿的卡尔曼滤波器
 */
void KWC_Init(KalmanWithComp *kf, double Q, double R, double dt, double comp_ratio);

/**
 * @brief 卡尔曼滤波器更新，含速度估计与滞后补偿
 */
double KWC_UpdateFast(KalmanWithComp *kf, double z);

#endif /* CODE_FORMULA_H_ */
