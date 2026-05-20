/*
 * RemteControl.c
 *
 *  Created on: 2025年2月6日
 *      Author: FELMLY
 */

#include "zf_common_headfile.h"     //声明一下所有的头文件

////各通道引脚
int16 x6f_pin_map[6] = {X6F_CH1, X6F_CH2, X6F_CH3, X6F_CH4, X6F_CH5, X6F_CH6};
////各通道高电平计数变量
int16 x6f_count[6];
////各通道高电平计数输出
int16 x6f_out[6];
//
int   HotRC_GO_FLAG=0;//遥控发车标志位
////===================================================逐飞遥控===================================================
//

 float hot_rc_speed = 0;
 float hot_rc_steer = 0;
 float hot_rc_delta = 0;

/**
 * @brief 初始化遥控器接收机引脚为带上拉的输入模式，并启动10us定时器中断用于PWM脉冲扫描
 * @note 初始化CH1~CH4四个通道引脚（CH5/CH6被注释，如需要可取消注释）；
 *       pit_us_init设置为10us周期中断，在中断服务函数中调用x6f_scan进行脉宽测量
 */
void hotRc_Control_init(void)//遥控器引脚初始化
{
    // ------------ 初始化接收机引脚为带上拉输入 ------------
//    pit_ms_init(CCU60_CH1, 1);
   gpio_init(X6F_CH1, GPI, GPIO_LOW, GPI_PULL_UP);
   gpio_init(X6F_CH2, GPI, GPIO_LOW, GPI_PULL_UP);
   gpio_init(X6F_CH3, GPI, GPIO_LOW, GPI_PULL_UP);
   gpio_init(X6F_CH4, GPI, GPIO_LOW, GPI_PULL_UP);
//   gpio_init(X6F_CH5, GPI, GPIO_LOW, GPI_PULL_UP);
//   gpio_init(X6F_CH6, GPI, GPIO_LOW, GPI_PULL_UP);

   // ------------ 启动10us定时器用于PWM脉宽测量 ------------
   pit_us_init(CCU60_CH1, 10);


}

/**
 * @brief 在IPS200液晶屏上显示5个遥控器通道的实时测量值
 * @note 显示格式为整数，各通道水平排列（每4字符间距）；CH6显示被注释，调试需要时可取消注释
 */
void hotRc_Show(void)
{
    // ------------ 在IPS200上显示各通道值 ------------
    ips200_show_int(10*0,16*18,x6f_out[0],4);
    ips200_show_int(10*4,16*18,x6f_out[1],3);
    ips200_show_int(10*8,16*18,x6f_out[2],4);
    ips200_show_int(10*12,16*18,x6f_out[3],3);
    ips200_show_int(10*16,16*18,x6f_out[4],4);
//    ips200_show_int(10*20,16*18,x6f_out[5],4);
//    ips200_show_string(10*0,16*8,"Actual;");               ips200_show_float(10*12, 16*8, Actual, 4, 6);
//    ips200_show_string(10*0,16*9,"Out;");                  ips200_show_float(10*12, 16*9, Out, 4, 6);
//    ips200_show_string(10*0,16*10,"Target;");               ips200_show_int(10*15,16*10,Target,3);
//    ips200_show_string(10*0,16*11,"Z_360;");                ips200_show_float(10*12, 16*11, Z_360, 4, 6);
//    ips200_show_string(10*0,16*12,"SE_Out;");               ips200_show_float(10*12, 16*12, SE_Out, 4, 6);
//    ips200_show_string(10*0,16*13,"SE_Target;");            ips200_show_int(10*15,16*13,SE_Target,3);
//    ips200_show_int(10*0,16*14,MOTOR_CHOOSE,3);

}

/**
 * @brief 扫描6个遥控器通道的PWM脉宽，通过10us定时器中断周期性调用
 * @note 每个通道独立计数高电平持续时间；检测到下降沿（高电平->低电平）时锁存计数值并清零计数器；
 *       调用频率决定分辨率，10us周期可分辨10us精度的PWM脉宽
 */
void  x6f_scan(void)//遥控器通道扫描,需要放到10us的中断里
{
    // ------------ 遍历6个通道进行PWM脉宽检测 ------------
    for(int i = 0; i < 6; i ++)
    {
        // ------------ 高电平期间累加计数 ------------
        if(gpio_get_level(x6f_pin_map[i]))
        {
            x6f_count[i]++;

        }
        // ------------ 下降沿锁存计数值并清零 ------------
        else if(x6f_count[i] > 0)
        {
            x6f_out[i] = x6f_count[i];
            x6f_count[i] = 0;
        }
    }

}

//void HotRC_GO_Flag(void) //遥控标志位函数,选择时遥控或自动模式，放在主循环或者定时器中断里
//{
//    if(x6f_out[2]>=150)
//    {
//        HotRC_GO_FLAG = 1;
//    }
//    else
//    {
//        HotRC_GO_FLAG = 0;
//    }
//}
/*****************************************
函数功能；遥控滚轮控制舵机目标角度
函数名称；uint16_t map_x6f_to_SE_Target(int16_t x6f_out_0)
输入参数；
输出参数
备注；
*****************************************/
//void map_x6f_to_SE_Target(int16_t x6f_out_0)
//{
//    // 输入范围限制
//    if (x6f_out_0 < 100)
//    {
////        SE_Target = 150;
//    } else if (x6f_out_0 > 200)
//    {
//        x6f_out_0 = 150;
//    }
//    // 线性映射计算
////    SE_Target = 150 - x6f_out_0;
//}

/*****************************************
函数功能；遥控滚轮控制舵机PWM
函数名称；uint16_t map_x6f_to_pwm(int16_t x6f_out_0)
输入参数；
输出参数
备注；
*****************************************/
//uint16_t map_x6f_to_pwm(int16_t x6f_out_0)
//{
//    // 输入范围限制
//    if (x6f_out_0 < 100)
//    {
//        x6f_out_0 = 150;
//    } else if (x6f_out_0 > 200)
//    {
//        x6f_out_0 = 150;
//    }
//    // 线性映射计算
//    uint16_t pwm = (uint16_t)(3 * x6f_out_0 + 300);
//    return pwm;
//}
//
//int16_t motor_map_x6f_to_pwm(int16_t x6f_out_0)
//{
//    // 输入范围限制
//    if (x6f_out_0 < 100)
//    {
//        x6f_out_0 = 150;
//    } else if (x6f_out_0 > 200)
//    {
//        x6f_out_0 = 150;
//    }
//    // 线性映射计算
//    int16_t pwm = (uint16_t)(198 * (x6f_out_0 - 150));
//    return pwm;
//}

/**
 * @brief 遥控器控制逻辑：将RC通道值映射为速度、转向角度和横摆角速度命令
 * @note 根据CH4挡位开关（x6f_out[3]）判断前进/后退模式，SWITCH2硬件开关切换速度档位；
 *       CH1映射转向角度（线性映射，死区±2以内归零），CH2映射速度（1.5x低速档/4x高速档）；
 *       最终计算 hot_rc_delta = v * tan(steer * PI / 540) / WHEEL_BASE
 */
void hotRC_control(void)    //放入主循环，或者中断里
{
    // ------------ CH1映射转向角度，死区±2以内归零 ------------
    hot_rc_steer = -2.125*x6f_out[0] +318.75;
    if(hot_rc_steer<2 &&hot_rc_steer>-2)hot_rc_steer = 0;

    // ------------ CH2映射速度，根据CH4挡位判断前进/后退 ------------
    if(x6f_out[3] != 100)
    {
        // ------------ 前进模式，SWITCH2切换高低速档 ------------
        if(!gpio_get_level(SWITCH2))
        {
            // 低速档
            hot_rc_speed = 1.5*x6f_out[1]-217.5;
            if(hot_rc_speed<0)hot_rc_speed = 0;
            if(hot_rc_speed >15)hot_rc_speed =  15;
        }
        else
        {
            // 高速档
            hot_rc_speed = 4*x6f_out[1]-580;
            if(hot_rc_speed<0)hot_rc_speed = 0;
            if(hot_rc_speed >30)hot_rc_speed =  40;
        }

    }
    else
    {
        // ------------ 后退模式 ------------
        hot_rc_speed = -1.5*x6f_out[1]+217.5;
        if(hot_rc_speed>0)hot_rc_speed = 0;
        if(hot_rc_speed <-15)hot_rc_speed =  -15;
    }

    // ------------ 计算横摆角速度 delta = v * tan(steer) / wheelbase ------------
    hot_rc_delta = (hot_rc_speed * tanf(hot_rc_steer/3.0/180.0f*M_PI)) / WHEEL_BASE;

}
