/*
 * RemteControl.h
 *
 *  Created on: 2025年2月6日
 *      Author: FELMLY
 */

#ifndef CODE_REMTECONTROL_H_
#define CODE_REMTECONTROL_H_

extern int16 x6f_count[6];
extern int16 x6f_out[6];
extern int HotRC_GO_FLAG;   //遥控器工作标志位

extern float hot_rc_speed ;
extern float hot_rc_steer ;
extern float hot_rc_delta ;
////逐飞
//#define X6F_CH1                     (P02_6)      //方向舵通道
//#define X6F_CH2                     (P02_7)      //油门通道
//#define X6F_CH3                     (P20_0)      //指示按钮通道，按下信号回100和200来回切换
//#define X6F_CH4                     (P20_3)      //拨动挡位通道，100,150,200三个档位
//#define X6F_CH5                     (P02_8)      //旋钮通道1（CH5）按下信号回100和200来回切换
//#define X6F_CH6                     (P00_9)      //旋钮通道2（CH6）按下信号回100和200来回切换

//#if ICE_OR_BIG
//    //编码器2,初始化,快乐无限组
//#endif
//#if !ICE_OR_BIG
//    //编码器2,初始化
//#endif


//快乐
#define X6F_CH1                     (P33_4)      //方向舵通道            ##注意这个引脚和逐飞方案GPS还有下载串口复用了
#define X6F_CH2                     (P33_5)      //油门通道
#define X6F_CH3                     (P20_10)      //指示按钮通道，按下信号回100和200来回切换
#define X6F_CH4                     (P33_6)      //拨动挡位通道，100,150,200三个档位
#define X6F_CH5                     (P33_7)      //旋钮通道1（CH5）按下信号回100和150来回切换
#define X6F_CH6                     (P14_5)      //旋钮通道2（CH6）按下信号回150和200来回切换（接下来会将这个引脚换掉，它和下载串口冲突了）

//大组长
//#define X6F_CH1                     (P21_5)      //方向舵通道
//#define X6F_CH2                     (P21_2)      //油门通道
//#define X6F_CH3                     (P21_4)      //指示按钮通道，按下信号回100和200来回切换
//#define X6F_CH4                     (P21_3)      //拨动挡位通道，100,150,200三个档位
//#define X6F_CH5                     (P02_3)      //旋钮通道1（CH5）按下信号回100和200来回切换
//#define X6F_CH6                     (P00_9)      //旋钮通道2（CH6）按下信号回100和200来回切换


/** @brief 初始化遥控器接收机引脚和10us定时器，用于PWM脉冲周期扫描 */
void hotRc_Control_init(void);
/** @brief 遥控器6通道PWM脉冲扫描，读取各通道高电平持续时间，置于10us定时器中断中调用 */
void x6f_scan(void);
/** @brief 检测遥控器工作标志位，根据CH3通道值判断遥控/自动模式 */
void HotRC_GO_Flag(void);
/** @brief 遥控器控制逻辑处理，将RC通道值转换为速度/转向/角速度命令 */
void hotRC_control(void);
/** @brief 在IPS200屏幕上显示5个RC通道的实时值 */
void hotRc_Show(void);
/** @brief 遥控器滚轮映射到PWM输出 */
uint16_t map_x6f_to_pwm(int16_t x6f_out_0);
/** @brief 电机端遥控器滚轮映射到PWM输出 */
int16_t motor_map_x6f_to_pwm(int16_t x6f_out_0);
//void Control_TEXT();

#endif /* CODE_REMTECONTROL_H_ */
