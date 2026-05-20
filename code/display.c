/*
 * display.c
 *
 *  Created on: 2025年11月20日
 *      Author: 18905
 */
#include "zf_common_headfile.h"
uint8 key_mode1 = 1;
uint8 key_mode2 = 2;
uint8 CarGo_Flag = 0;
float *p;
int16 *p1;
Mode_Choice main_mode = Mode_IDLE;

/**
 * @brief 初始化 IPS200 LCD 显示屏
 * @param 无
 * @note 设置竖屏方向和白底黑字配色；需在系统启动时调用一次
 */
void Display_Init(void)
{
    // ------------ 配置屏幕方向、颜色并初始化 ------------
    ips200_set_dir(IPS200_PORTAIT);
    ips200_set_color(RGB565_WHITE , RGB565_BLACK);
    ips200_init(IPS200_TYPE);
}


/*                                                                          菜单控制层                                                                                  */
/**
 * @brief 主菜单循环，基于状态机管理各子菜单页面
 * @param 无
 * @note 该函数为死循环，通过 key_mode2 切换不同子页面；CarGo_Flag 置位时退出循环
 */
void Menu_control(void)
{
    while(1)
    {

        // ------------ 按键采集 ------------
        key_value = Key_Get();

        // ------------ 根据状态机跳转子页面 ------------
        if(key_mode2 == 1)          Menu_Main();              //菜单选择首页
        else if(key_mode2 == 2)  Menu_1();
        else if(key_mode2 == 3)  Menu_Parameter();
        else if(key_mode2 == 4)  Menu_Mode_Choice();
        else if(key_mode2 == 5)  Menu_Record_Points();
        else if(key_mode2 == 6)  Menu_Show_Route();
        else if(key_mode2 == 7)  Show_Route();
        else if(key_mode2 == 8) Menu_PID_P();
        else if(key_mode2 == 9) Menu_Control_P();

        // ------------ 发车指令：清屏并退出循环 ------------
        if(CarGo_Flag == 1){ips200_clear();break;}

    }

}

/**
 * @brief 主菜单页面显示与导航
 * @param 无
 * @note 按键1/2上下移动光标，按键3确认进入子页面，按键4返回；光标循环滚动
 */
void Menu_Main(void)
{
    // ------------ 显示主菜单选项 ------------
    ips200_show_string( X(10) ,Y(0) ,"Menu_Main");
    ips200_show_string( X(3) ,Y(2) ,"Car_Go");
    ips200_show_string( X(3) ,Y(3) ,"Parameter");
    ips200_show_string( X(3) ,Y(4) ,"Mode_Choice");
    ips200_show_string( X(3) ,Y(5) ,"Show_Route");

    // ------------ 光标导航逻辑 ------------
    prompt();                                                                                  //光标标识
    if(key_value == 1)key_mode1 ++;                                          //按键处理+限幅
    else if(key_value == 2)key_mode1 --;
    key_mode1 =(key_mode1 > 5) ? 2  : key_mode1;
    key_mode1 =(key_mode1 < 2) ? 5  : key_mode1;

    // ------------ 确认键：进入对应子页面 ------------
    if(key_value == 3)                                                                  //确认后执行
    {
        ips200_clear();
        if(key_mode1 == 5 ){ key_mode2 = 6; }
        else{  key_mode2 = key_mode1;}

    }
}

/**
 * @brief 路径显示选择菜单
 * @param 无
 * @note 提供 INS、passage、portion_3 三种路径查看选项；按键3确认，按键4返回
 */
void Menu_Show_Route(void)
{
    // ------------ 显示路径选项 ------------
    ips200_show_string( X(10) ,Y(0) ,"Show_Route");
    ips200_show_string( X(3) ,Y(2) ,"INS");
    ips200_show_string( X(3) ,Y(3) ,"passage");
    ips200_show_string( X(3) ,Y(4) ,"portion_3");

    // ------------ 光标导航 ------------
    prompt();                                                                                  //光标标识
    if(key_value == 1)key_mode1 ++;                                          //按键处理+限幅
    else if(key_value == 2)key_mode1 --;
    key_mode1 =(key_mode1 > 4) ? 2  : key_mode1;
    key_mode1 =(key_mode1 < 2) ? 4  : key_mode1;

    // ------------ 确认选择对应路径并跳转显示 ------------
    if(key_value == 3){
        switch(key_mode1)
        {
            case 2:
                route_setting_choice = 0;
                break;
            case 3:
                route_setting_choice = 1;
                break;
            case 4:
                route_setting_choice = 2;
                break;
                break;
            default :break;
        }
        key_mode2 = 7; ips200_clear();
    }
    // ------------ 返回键 ------------
    if(key_value == 4){key_mode2 =1; ips200_clear();}


}

/**
 * @brief 在屏幕上显示 INS 路径点并等待返回
 * @param 无
 * @note 进入后持续显示路径点，按下按键4返回上级菜单
 */
void Show_Route(void)
{
    // ------------ 显示 INS 路径点 ------------
    Guandao_Points_Show(&INS);

    // ------------ 等待返回按键 ------------
    while(1){
        key_value = Key_Get();   //按键采集
        if(key_value == 4){key_mode2 =6;ips200_clear();break;}
    }

}

/**
 * @brief 记录轨迹点菜单
 * @param 无
 * @note 提供 INS、passage、portion_3 三种记录模式；确认后进入遥控模式并开始记录
 */
void Menu_Record_Points(void)
{
    // ------------ 显示记录选项 ------------
    ips200_show_string( X(7) ,Y(0) ,"Record_Points");
    ips200_show_string( X(3) ,Y(2) ,"INS");
    ips200_show_string( X(3) ,Y(3) ,"passage");
    ips200_show_string( X(3) ,Y(4) ,"portion_3");

    // ------------ 光标导航 ------------
    prompt();                                                                                  //光标标识
    if(key_value == 1)key_mode1 ++;                                          //按键处理+限幅
    else if(key_value == 2)key_mode1 --;
    key_mode1 =(key_mode1 > 4) ? 2  : key_mode1;
    key_mode1 =(key_mode1 < 2) ? 4  : key_mode1;

    // ------------ 确认：进入记录模式并设置参数 ------------
    if(key_value == 3)
    {
        main_mode = Guandao_Record_Mode;
        control_mode = YAOKONG;
        switch(key_mode1)
        {
            case 2:
                route_setting_choice = 0;
                break;
            case 3:
                route_setting_choice = 1;
                break;
            case 4:
                route_setting_choice = 2;
                break;
            default :break;
        }
         Buzzer_check(50);
    }
    // ------------ 返回键 ------------
    if(key_value == 4){key_mode2 =4; ips200_clear();}

}

/**
 * @brief Car_Go 子菜单页面，显示启动选项
 * @param 无
 * @note 光标固定在第2项；按下按键3确认后置位 CarGo_Flag 触发发车
 */
void Menu_1(void)
{
    // ------------ 显示菜单标题和选项 ------------
    ips200_show_string( X(10) ,Y(0) ,"Car_Go");
    ips200_show_string( X(3) ,Y(2) ,"Start");
//    if(key_value == 1)key_mode1 ++;
//    else if(key_value == 2)key_mode1 --;
//    key_mode1 =(key_mode1 > 4) ? 2  : key_mode1;
//    key_mode1 =(key_mode1 < 2) ? 4  : key_mode1;

    key_mode1 =2;

    // ------------ 光标与确认处理 ------------
    prompt();
    if(key_value == 3&& key_mode1 ==2){CarGo_Flag =1;}

    // ------------ 返回键 ------------
    if(key_value == 4){key_mode2 =1;ips200_clear();}

}

/**
 * @brief 运行模式选择菜单
 * @param 无
 * @note 提供六种运行模式选项；按键1/2切换，按键3确认，按键4返回
 */
void Menu_Mode_Choice(void)
{
    // ------------ 显示模式选项 ------------
    ips200_show_string( X(10) ,Y(0) ,"Mode");

    ips200_show_string( X(3) ,Y(2) ,"NULL_Mode_IDLE");
    ips200_show_string( X(3) ,Y(3) ,"Guandao_Record_Mode");
    ips200_show_string( X(3) ,Y(4) ,"Guandao_portion_1");
    ips200_show_string( X(3) ,Y(5) ,"Voice_Mode");
    ips200_show_string( X(3) ,Y(6) ,"Guandao_portion_3");
    ips200_show_string( X(3) ,Y(7) ,"Rack_Test");

    // ------------ 光标导航 ------------
    prompt();

    if(key_value == 1)key_mode1 ++;
    else if(key_value == 2)key_mode1 --;
    key_mode1 =(key_mode1 > 7) ? 2  : key_mode1;
    key_mode1 =(key_mode1 < 2) ? 7  : key_mode1;

    // ------------ 确认：根据选择设置模式并执行相应初始化 ------------
    if(key_value == 3)
    {
        if(key_mode1 == 3){key_mode2 =5 ; ips200_clear(); }
        else if (key_mode1 ==4){main_mode = Guandao_portion_1 ;  route_setting_choice = 0; daoche_speed = (float)control[1]; portion_1_reset(); control_mode = GUANDAO ; Buzzer_check(50);}
        else if( key_mode1==5){main_mode = Guandao_Voice ; route_setting_choice = 3; control_mode = GUANDAO ;Buzzer_check(50);}
        else if( key_mode1==6){main_mode = Guandao_portion_3 ; route_setting_choice = 2; control_mode = GUANDAO ;Buzzer_check(50);}
        else if( key_mode1==7){main_mode = Rack_Test_Mode ; control_mode = RACK_TEST ; rack_test_stage = 0; rack_test_speed_target = 0; rack_test_steer_target = 0; Rack_Straight_Reset(); MoterPID_L.Kp = 0.5f; MoterPID_R.Kp = 0.5f; MoterPID_L.Ki = 1.0f; MoterPID_R.Ki = 1.0f; MoterPID_L.Kd = 0.0f; MoterPID_R.Kd = 0.0f; CarGo_Flag = 1; ips200_clear(); Buzzer_check(50);}
//        main_mode  = key_mode1 - 2;
//        Buzzer_check(50);
    }
    // ------------ 返回键 ------------
    if(key_value == 4){key_mode2 =1;ips200_clear();}
}

/**
 * @brief 参数设置菜单页面
 * @param 无
 * @note 提供 PID 参数和控制参数两个子菜单入口；按键3确认进入
 */
void Menu_Parameter(void)
{
    // ------------ 显示参数选项 ------------
    ips200_show_string( X(10) ,Y(0) ,"Parameter");
    ips200_show_string( X(3) ,Y(2) ,"PID");
    ips200_show_string( X(3) ,Y(3) ,"Control");


    // ------------ 光标导航 ------------
    prompt();

    if(key_value == 1)key_mode1 ++;
    else if(key_value == 2)key_mode1 --;
    key_mode1 =(key_mode1 > 3) ? 2  : key_mode1;
    key_mode1 =(key_mode1 < 2) ? 3  : key_mode1;

    // ------------ 确认进入子菜单 ------------
    if(key_value==3)
    {
        if(key_mode1 ==2){key_mode2 = 8;ips200_clear();}
        else if(key_mode1 ==3){key_mode2 = 9;ips200_clear();}

    }
    // ------------ 返回键 ------------
    if(key_value == 4){key_mode2 =1;ips200_clear();}

}

/**
 * @brief PID 参数调节菜单
 * @param 无
 * @note 可调节6个速度 PID 参数；拨码开关 SWITCH1 按下时进入编辑模式
 */
void Menu_PID_P(void)
{
    // ------------ 显示 PID 参数选项 ------------
    ips200_show_string( X(10) ,Y(0) ,"PID_P");
    ips200_show_string( X(3) ,Y(2) ,"Speed_kp");
    ips200_show_string( X(3) ,Y(3) ,"Speed_ki");
    ips200_show_string( X(3) ,Y(4) ,"Speed_kd");
    ips200_show_string( X(3) ,Y(5) ,"Record_The");
    ips200_show_string( X(3) ,Y(6) ,"Persuit_The");
    ips200_show_string( X(3) ,Y(7) ,"End_Dec_D");

    // ------------ 显示当前值并绘制光标 ------------
    Menu_2Value();
    prompt();

    // ------------ 光标导航 ------------
    if(key_value == 1)key_mode1 ++;
    else if(key_value == 2)key_mode1 --;
    key_mode1 =(key_mode1 > 7) ? 2  : key_mode1;
    key_mode1 =(key_mode1 < 2) ? 7  : key_mode1;

    // ------------ 拨码开关按下时进入实时编辑 ------------
    while (gpio_get_level(SWITCH1))
    {

        Menu_2Value();
        Key_Scan();

        // ------------ 根据光标位置选择对应参数进行按键编辑 ------------
        for(uint8 i = 0 ; i < 7; i++ )
        {
            if(i == key_mode1-2)
            {
                p =&speed_pid[i];
                Menu_key_Operation_float(p);
            }
        }
    }

    // ------------ 返回键 ------------
    if(key_value == 4){key_mode2 =3 ;ips200_clear();}
}

/**
 * @brief 控制参数调节菜单
 * @param 无
 * @note 可调节基础速度、倒车速度、预瞄步数等控制参数；按键3进入编辑，按键4返回
 */
void Menu_Control_P(void)
{
    static uint8 edit_flag = 0;
    int16 *target;

    // ------------ 显示控制参数选项 ------------
    ips200_show_string( X(10) ,Y(0) ,"Control_P");
    ips200_show_string( X(3) ,Y(2) ,"Base_Speed");
    ips200_show_string( X(3) ,Y(3) ,"Daoche_Speed");
    ips200_show_string( X(3) ,Y(4) ,"Preview_Spets");
    ips200_show_string( X(3) ,Y(5) ,"Run_Mps");

    key_mode1 =(key_mode1 > 4) ? 2  : key_mode1;
    key_mode1 =(key_mode1 < 2) ? 4  : key_mode1;

    // ------------ 显示当前值与光标 ------------
    Menu_Control_Value();
    prompt();
    if(edit_flag) ips200_show_string(X(1), Y(7), "EDIT");

    // ------------ 非编辑模式：导航与进入编辑 ------------
    if(edit_flag == 0)
    {
        if(key_value == 1)key_mode1 ++;
        else if(key_value == 2)key_mode1 --;
        key_mode1 =(key_mode1 > 4) ? 2  : key_mode1;
        key_mode1 =(key_mode1 < 2) ? 4  : key_mode1;

        if(key_value == 3){edit_flag = 1; ips200_clear();}
        if(key_value == 4){key_mode2 = 3;ips200_clear();}
    }
    // ------------ 编辑模式：按键调整参数值 ------------
    else
    {
        target = &control[key_mode1 - 2];

        if(key_value == 1)*target += 1;
        else if(key_value == 2)*target -= 1;
        else if(key_value == 3)*target += 10;
        else if(key_value == 4){edit_flag = 0; ips200_clear();}

        // ------------ 参数范围检查 ------------
        if(key_mode1 == 2)
        {
            if(control[0] < 0) control[0] = 0;
            if(control[0] > 50) control[0] = 50;
        }
        else if(key_mode1 == 3)
        {
            if(control[1] > 0) control[1] = 0;
            if(control[1] < -50) control[1] = -50;
        }
        else if(key_mode1 == 4)
        {
            if(control[2] < 1) control[2] = 1;
            if(control[2] > 20) control[2] = 20;
        }
    }
}

//
//if(key_value == 4){key_mode2 =1;ips200_clear();}
/**
 * @brief 显示 6 个 PID 速度参数值到屏幕上
 * @param 无
 * @note 依次显示 speed_pid[0]~speed_pid[5]，分别对应 Kp、Ki、Kd、记录阈值、追踪阈值、终点减速距离
 */
void Menu_2Value(void)
{
    // ------------ 分行显示6个PID参数 ------------
    ips200_show_float(X(17),Y(2),speed_pid[0],3,1);
    ips200_show_float(X(17),Y(3),speed_pid[1],3,1);
    ips200_show_float(X(17),Y(4),speed_pid[2],3,1);
    ips200_show_float(X(17),Y(5),speed_pid[3],3,1);
    ips200_show_float(X(17),Y(6),speed_pid[4],3,1);
    ips200_show_float(X(17),Y(7),speed_pid[5],3,1);
}

/**
 * @brief 显示控制参数值到屏幕上
 * @param 无
 * @note 依次显示基础速度、倒车速度、预瞄步数、运行速度换算值
 */
void Menu_Control_Value(void)
{
    // ------------ 显示4个控制参数 ------------
    ips200_show_int(X(16),Y(2),control[0],3);
    ips200_show_int(X(16),Y(3),control[1],3 );
    ips200_show_int(X(16),Y(4),control[2],3 );
    ips200_show_float(X(16),Y(5),(float)control[0] * 0.1f,2,1);
}
/*                                                                                         按键处理                                                                                                                       */
/**
 * @brief 按键扫描并返回键值
 * @param 无
 * @note 每次调用会消费按键标志（清零），返回值为0表示无按键；键值1/2上下，3确认，4返回
 * @retval 当前按键值（uint8），1=上 2=下 3=确认 4=返回 0=无按键
 */
uint8 Key_Get(void)
{
    uint8 value = 0 ;

    // ------------ 扫描按键并消费标志 ------------
    Key_Scan();
    if(key4_flag == 1)
    {
        key4_flag = 0;
        value = 2;
    }
    if(key3_flag == 1)
    {
        key3_flag = 0 ;
       value = 1;
    }
    if(key2_flag == 1)
    {
        key2_flag = 0;
        value = 3;
    }
    if(key1_flag == 1)
    {
        key1_flag =0;
        value = 4;
    }
    return value;
}

/**
 * @brief 按键操作模式下的 int16 参数调整
 * @param param_t 指向待调整的 int16 变量的指针
 * @note 按键1/2加/减10，按键3/4加/减1；通过指针直接修改变量值
 * @retval 调整后的参数值（int16）
 */
int16 Menu_key_Operation_int16(int16 *param_t )//菜单下按键操作函数//指针变量为形参，是函数内的形参也要为指针形式
{
    // ------------ 采集并消费按键标志 ------------
     if(key1_flag){key1_flag=0;key_val=1;}
     if(key2_flag){key2_flag=0;key_val=2;}
     if(key3_flag){key3_flag=0;key_val=3;}
     if(key4_flag){key4_flag=0;key_val=4;}

//                    switch(key_val)
//                    {
//
//                        case 1:*param_t+=10 ,key_val=0; break;
//                        case 2:*param_t-=10 ,key_val=0; break;
//                        case 3:*param_t+=1  ,key_val=0; break;
//                        case 4:*param_t-=1  ,key_val=0; break;
//                        default:break;
//                    }
     // ------------ 根据键值调整参数 ------------
     if(key_val ==1){*param_t+=10 ;key_val=0;}
     if(key_val ==2){*param_t-=10 ;key_val=0;}
     if(key_val ==3){*param_t+=1 ;key_val=0;}
     if(key_val ==4){*param_t-=1 ;key_val=0;}

   return  *param_t;
}

/**
 * @brief 按键操作模式下的 float 参数调整
 * @param param_t 指向待调整的 float 变量的指针
 * @note 按键1/2加/减0.5，按键3/4加/减0.1；通过指针直接修改变量值
 * @retval 调整后的参数值（float）
 */
float Menu_key_Operation_float(float *param_t )
{
    // ------------ 采集并消费按键标志 ------------
    if(key1_flag){key1_flag=0;key_val=1;}
    if(key2_flag){key2_flag=0;key_val=2;}
    if(key3_flag){key3_flag=0;key_val=3;}
    if(key4_flag){key4_flag=0;key_val=4;}

//                    switch(key_val)
//                    {
//
//                        case 1:*param_t+=10 ,key_val=0; break;
//                        case 2:*param_t-=10 ,key_val=0; break;
//                        case 3:*param_t+=1  ,key_val=0; break;
//                        case 4:*param_t-=1  ,key_val=0; break;
//                        default:break;
//                    }
    // ------------ 根据键值调整参数 ------------
    if(key_val ==1){*param_t+=0.5 ;key_val=0;}
    if(key_val ==2){*param_t-=0.5 ;key_val=0;}
    if(key_val ==3){*param_t+=0.1 ;key_val=0;}
    if(key_val ==4){*param_t-=0.1 ;key_val=0;}

  return  *param_t;

}

/**
 * @brief 绘制菜单光标箭头指示符
 * @param 无
 * @note 根据 key_mode1 的值在对应行绘制 "->" 箭头，其余行清除
 */
void prompt(void)
{
    static uint8 i ;
    // ------------ 遍历屏幕行，在选中行绘制箭头 ------------
    for( i = 0 ; i<10; i++)
    {
        if( i == key_mode1 )
        {
            ips200_show_string( 0 ,16*i ,"->");
        }
        else
        {
            ips200_show_string( 0 ,16*i ,"  ");
        }
    }
}

/**
 * @brief 绘制 GPS 菜单光标与序号标签
 * @param 无
 * @note 仅在行2~3绘制光标箭头，并同时显示 [1]~[15] 序号标签
 */
void GPS_prompt(void)
{
    static uint8 i ;
    // ------------ 在行2~3绘制光标箭头 ------------
    for( i = 2 ; i<4; i++)
    {
        if( i == key_mode1 )
        {
            ips200_show_string( 0 ,16*i ,"->");
        }
        else
        {
            ips200_show_string( 0 ,16*i ,"  ");
        }
    }
    // ------------ 绘制序号标签 [1]~[15] ------------
    ips200_show_string( X(0) ,Y(4) ,"[1]");
    ips200_show_string( X(0) ,Y(5) ,"[2]");
    ips200_show_string( X(0) ,Y(6) ,"[3]");
    ips200_show_string( X(0) ,Y(7) ,"[4]");
    ips200_show_string( X(0) ,Y(8) ,"[5]");
    ips200_show_string( X(0) ,Y(9) ,"[6]");
    ips200_show_string( X(0) ,Y(10) ,"[7]");
    ips200_show_string( X(0) ,Y(11) ,"[8]");
    ips200_show_string( X(0) ,Y(12) ,"[9]");
    ips200_show_string( X(0) ,Y(13) ,"[10]");
    ips200_show_string( X(0) ,Y(14) ,"[11]");
    ips200_show_string( X(0) ,Y(15) ,"[12]");
    ips200_show_string( X(0) ,Y(16) ,"[13]");
    ips200_show_string( X(0) ,Y(17) ,"[14]");
    ips200_show_string( X(0) ,Y(18) ,"[15]");
}
