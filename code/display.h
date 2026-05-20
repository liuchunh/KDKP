/*
 * display.h
 *
 *  Created on: 2025年11月20日
 *      Author: 18905
 */

#ifndef CODE_DISPLAY_H_
#define CODE_DISPLAY_H_

//外部变量
typedef enum{
    Mode_IDLE,
    Guandao_Record_Mode,
    Guandao_portion_1,
    Guandao_Voice,
    Guandao_portion_3,
    Rack_Test_Mode
}Mode_Choice;

extern Mode_Choice main_mode;

extern uint8 key_mode1;
extern uint8 key_mode2;
extern uint8 CarGo_Flag;
extern float *p;
//宏定义
#define IPS200_TYPE     (IPS200_TYPE_SPI)    //屏幕初始化宏定义
#define X(x)                    8*(x)
#define Y(y)                    16*(y)
//函数声明

/**
 * @brief 初始化 IPS200 LCD 显示屏
 */
void Display_Init(void);

/**
 * @brief 按键扫描并返回键值
 */
uint8 Key_Get(void);

/**
 * @brief 绘制菜单光标箭头指示符
 */
void prompt(void);

/**
 * @brief 主菜单循环，基于状态机管理各子菜单页面
 */
void Menu_control(void);

/**
 * @brief 按键操作模式下的 int16 参数调整
 */
int16 Menu_key_Operation_int16(int16 *param_t );

/**
 * @brief 按键操作模式下的 float 参数调整
 */
float Menu_key_Operation_float(float *param_t );

/**
 * @brief 主菜单页面显示与导航
 */
void Menu_Main(void);

/**
 * @brief Car_Go 子菜单页面
 */
void Menu_1(void);

/**
 * @brief 参数设置菜单页面
 */
void Menu_Parameter(void);

/**
 * @brief 显示 6 个 PID 速度参数的数值
 */
void Menu_2Value(void);

/**
 * @brief 显示控制参数的数值
 */
void Menu_Control_Value(void);

/**
 * @brief 运行模式选择菜单
 */
void Menu_Mode_Choice(void);

/**
 * @brief 记录轨迹点菜单
 */
void Menu_Record_Points(void);

/**
 * @brief GPS 菜单光标与序号标签绘制
 */
void GPS_prompt(void);

/**
 * @brief 路径显示选择菜单
 */
void Menu_Show_Route(void);

/**
 * @brief 在屏幕上显示路径点
 */
void Show_Route(void);

/**
 * @brief PID 参数调节菜单
 */
void Menu_PID_P(void);

/**
 * @brief 控制参数调节菜单
 */
void Menu_Control_P(void);

#endif /* CODE_DISPLAY_H_ */
