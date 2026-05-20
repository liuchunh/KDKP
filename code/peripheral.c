/*
 * peripheral.c
 *
 *  Created on: 2025年11月20日
 *      Author: 18905
 */

#include "zf_common_headfile.h"
#include "rear_motor/rear_motor.h"

uint8 TIM_FLAG1 = 0;
uint8 TIM_FLAG2 = 0;
uint8 TIM_FLAG3 = 0;
Encoder_t Speed_ecd;
Encoder_t guandao_ecd;
Encoder_t Steer_ecd;
uint8 rack_test_stage = 0;
int16 rack_test_speed_target = 0;
int32 rack_test_steer_target = 0;
float rack_straight_target_yaw = 0.0f;
float rack_straight_yaw_error = 0.0f;
float rack_straight_steer_target = 0.0f;
static float rack_straight_last_yaw_error = 0.0f;

#define RACK_STRAIGHT_KP        (2.0f)
#define RACK_STRAIGHT_KD        (0.45f)
#define RACK_STRAIGHT_LIMIT_DEG (22.0f)

/**
 * @brief 初始化所有外设：显示、按键、蜂鸣器、舵机、编码器、电机、IMU、GPS 等
 * @note 一次性初始化函数，在 main 中调用；初始化顺序不能随意更改（如 IMU 需在初始化管道状态前完成零偏标定）
 */
void Init_All(void)
{
    // ------------ 初始化各硬件外设 ------------
    Display_Init();
    Key_Init();
    Buzzer_Init();
    Steer_init();
    Encoder_Init();
    Motor_init();
    IMU_init();
    GPS_Init();

    // ------------ 编码器计数结构体清零 ------------
    Encoder_count_init(&Speed_ecd);
    Encoder_count_init(&guandao_ecd);
    Encoder_count_init(&Steer_ecd);

    // ------------ 管道状态结构体初始化和链表构建 ------------
    guandao_state_init(&INS);
    guandao_state_init(&passage);
    guandao_state_init(&portion_2);
    guandao_state_init(&portion_3);
    guandao_chain_init();
//    gps_work_init();

    // ------------ 卡尔曼滤波器初始化 ------------
    KWC_Init(&klm_lat,2,1,0.01,0.0);
    KWC_Init(&klm_lon,2,1,0.01,0.0 );

    // ------------ 转向电机初始化 ------------
    Steer_Moter_Init();
}

/**
 * @brief 初始化 4 个按键和 2 个拨码开关为 GPIO 输入模式
 * @note KEY1/2/3/4 配置为上拉输入（默认高电平）；SWITCH1/2 配置为浮空输入（默认高电平）
 */
void Key_Init(void)
{
    // ------------ 初始化 4 个按键 GPIO（上拉输入） ------------
    gpio_init(KEY1, GPI, GPIO_LOW, GPI_PULL_UP);           // 初始化 KEY1 引脚 默认高电平 上拉输入
    gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY2 引脚 默认高电平 上拉输入
    gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY3 引脚 默认高电平 上拉输入
    gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY4 引脚 默认高电平 上拉输入

    // ------------ 初始化 2 个拨码开关 GPIO（浮空输入） ------------
    gpio_init(SWITCH1, GPI, GPIO_HIGH, GPI_FLOATING_IN);    // 初始化 SWITCH1 引脚 默认高电平 浮空输入
    gpio_init(SWITCH2, GPI, GPIO_HIGH, GPI_FLOATING_IN);    // 初始化 SWITCH2 引脚 默认高电平 浮空输入

}


// **************************** 按键相关 ****************************
uint8 key1_state = 1;                                                               // 按键当前状态
uint8 key2_state = 1;                                                               // 按键当前状态
uint8 key3_state = 1;                                                               // 按键当前状态
uint8 key4_state = 1;                                                               // 按键当前状态

uint8 switch1_state = 0;                                                            // 拨码开关读取状态
uint8 switch2_state = 0;                                                            // 拨码开关读取状态

uint8 key1_state_last = 0;                                                          // 上一次按键读取状态
uint8 key2_state_last = 0;                                                          // 上一次按键读取状态
uint8 key3_state_last = 0;                                                          // 上一次按键读取状态
uint8 key4_state_last = 0;                                                          // 上一次按键读取状态

uint8 switch1_state_last = 0;                                                       // 上一次拨码开关读取状态
uint8 switch2_state_last = 0;                                                       // 上一次拨码开关读取状态

uint8 key1_flag =0 ;
uint8 key2_flag =0 ;
uint8 key3_flag =0 ;
uint8 key4_flag= 0 ;

uint8 key_val;
uint8 key_value;

/**
 * @brief 扫描按键状态，检测边沿变化并设置按键标志位
 * @note 使用上升沿检测（松开按键时置位 flag）；该函数应在主循环中周期性调用；不会阻塞等待
 */
void Key_Scan(void)
{

    //使用此方法的优点在于：不需要使用while(1) 等待，避免处理资源浪费

    // ------------ 保存上一次按键状态 ------------
    key1_state_last = key1_state;
    key2_state_last = key2_state;
    key3_state_last = key3_state;
    key4_state_last = key4_state;

    // ------------ 读取当前按键状态 ------------
    key1_state = gpio_get_level(KEY1);
    key2_state = gpio_get_level(KEY2);
    key3_state = gpio_get_level(KEY3);
    key4_state = gpio_get_level(KEY4);

    // ------------ 检测上升沿（松开按键）并置位标志 ------------
    if(key1_state && !key1_state_last)   {key1_flag = 1;}
    if(key2_state && !key2_state_last)   {key2_flag = 1;}
    if(key3_state && !key3_state_last)   {key3_flag = 1;}
    if(key4_state && !key4_state_last)   {key4_flag = 1;}


}

/**
 * @brief 初始化蜂鸣器 GPIO 为推挽输出
 * @note 初始输出低电平（关闭状态）
 */
void Buzzer_Init(void)
{
    // ------------ 配置蜂鸣器引脚为推挽输出 ------------
    gpio_init(BUZZER_PIN, GPO, 0, GPO_PUSH_PULL);
}

/**
 * @brief 蜂鸣器鸣响指定毫秒数后关闭
 * @param time2 鸣响持续时间（毫秒）
 * @note 函数内部使用 system_delay_ms 阻塞等待；调用期间不响应其他事件
 */
void Buzzer_check(int time2)//蜂鸣器鸣响自检函数
{
    // ------------ 打开蜂鸣器，延时后关闭 ------------
    uint32 toggle_count = 0;
    uint32 i = 0;

    if(time2 <= 0)
    {
        gpio_set_level(BUZZER_PIN,0);
        return;
    }

    toggle_count = ((uint32)time2 * 1000u) / PASSIVE_BUZZER_HALF_PERIOD_US;
    for(i = 0; i < toggle_count; i++)
    {
        gpio_toggle_level(BUZZER_PIN);
        system_delay_us(PASSIVE_BUZZER_HALF_PERIOD_US);
    }
    gpio_set_level(BUZZER_PIN,0);
}

/**
 * @brief 初始化舵机 PWM 并设置中位
 * @note PWM 频率为 50Hz，占空比初始化为 SERVO_MOTOR_MID（中位 80）
 */
void Steer_init(void)//舵机初始化
{
    // ------------ 初始化舵机 PWM（50Hz，中位） ------------
    pwm_init(SERVO_MOTOR_PWM, SERVO_MOTOR_FREQ, (uint32)SERVO_MOTOR_DUTY(SERVO_MOTOR_MID));

}

/**
 * @brief 设置舵机角度，自动限制在左右极限范围内
 * @param angle 目标舵机角度值
 * @note 角度自动 clamp 到 [SERVO_MOTOR_LMAX, SERVO_MOTOR_RMAX] 即 [65, 95] 范围内
 */
void Steer_set(int angle)//舵机设置
{
    // ------------ 角度限幅 ------------
    if(angle<SERVO_MOTOR_LMAX){angle=SERVO_MOTOR_LMAX;}
    if(angle>SERVO_MOTOR_RMAX){angle=SERVO_MOTOR_RMAX;}

    // ------------ 设置 PWM 占空比 ------------
    pwm_set_duty(SERVO_MOTOR_PWM, (uint32)SERVO_MOTOR_DUTY(angle));

}

/**
 * @brief 手动按键调试舵机角度的测试函数
 * @note KEY1/2 每次增减 10 个单位，KEY3/4 每次增减 1 个单位；在 IPS200 上显示当前角度值
 */
void Steer_text(void)//舵机测试
{
    // ------------ 按键调节舵机角度 ------------
   static int32 angle=SERVO_MOTOR_MID;


      if(key1_flag)
         {
             key1_flag=0;
             angle+=10;
         }
      if(key2_flag)
         {
             key2_flag=0;
             angle-=10;
         }
      if(key3_flag)
         {
             key3_flag=0;
             angle+=1;
//             angle=55;//左打满
         }
      if(key4_flag)
         {
             key4_flag=0;
            angle-=1;
//             angle=85;//右打满
         }

      // ------------ 屏幕显示和舵机设置 ------------
      ips200_show_int(100,  16*3,angle, 5);
      Steer_set(angle);

}

int16 encoder_l = 0;
int16 encoder_r = 0;
int motor_pwm_l = 0;
int motor_pwm_r = 0;


/**
 * @brief 复位编码器计数结构体的所有字段
 * @param count 指向待复位的编码器结构体的指针
 * @note 将左右计数器、增量、以及上一次计数值全部清零
 */
void Encoder_count_init(Encoder_t *count)
{
    // ------------ 所有编码器字段清零 ------------
    count->left_counter =0;
    count->right_counter = 0;
    count->delta_l = 0;
    count->delta_r = 0;
    count->last_ecdcount_l = 0;
    count ->last_ecdcount_r =0;

}

/**
 * @brief 初始化正交编码器硬件
 * @note 使用 TIM2 编码器模式，A/B 相分别接 P33_7 和 P33_6
 */
void Encoder_Init(void)
{
    // ------------ 初始化正交解码编码器 ------------
    encoder_quad_init(ENCODER_QUADDEC, ENCODER_QUADDEC_A, ENCODER_QUADDEC_B);

}

/**
 * @brief 读取编码器计数，计算增量变化并进行滑动平均滤波
 * @param count 指向编码器结构体的指针，读取的计数和计算的增量将写入此处
 * @note 使用 4 点滑动平均 ((prev*3 + raw)/4) 平滑增量；当前左右轮共用同一个编码器（右轮等于左轮）
 */
void Encoder_Get(Encoder_t *count)
{
    // ------------ 读取左轮编码器原始计数 ------------
    count->left_counter = l_ecdcounter();                  // 读取左轮编码器计数值

    // ------------ 计算增量变化（4点滑动平均） ------------
    int32 raw_delta = calculate_delta(count->left_counter,count ->last_ecdcount_l);
    count->delta_l = (count->delta_l * 3 + raw_delta) / 4;

    // ------------ 右轮增量等于左轮（当前只有一个编码器） ------------
    count->right_counter  = count->left_counter;           // 当前只使用左轮编码器为右轮提供速度参考
    count->delta_r = count->delta_l;
//    ips200_show_int(X(1),  Y(8),count->delta_l ,5);
//    ips200_show_int(X(10),  Y(8),count->delta_r ,5);

    // ------------ 保存本次计数值供下次计算增量 ------------
    count ->last_ecdcount_l = count->left_counter;
    count-> last_ecdcount_r = count->right_counter ;
//    encoder_clear_count(ENCODER_QUADDEC);                                       // 清零编码器计数值
    if (DebugStatus) uart_write_printf(DEBUG_UART_INDEX, "Encoder Data: %d\n", count->left_counter);
}

/**
 * @brief 初始化电机 PWM 和方向控制 GPIO
 * @note PWM 频率 17KHz，初始占空比 0；方向引脚默认高电平（正转方向）
 */
void Motor_init(void)//电机初始化
{
    // ------------ 初始化左右电机 PWM（17KHz，占空比0） ------------
       pwm_init(PWM_L, 17000, 0);                                                 // PWM 通道 L1 初始化 频率 17KHz 占空比初始为 0
       pwm_init(PWM_R, 17000, 0);                                                // PWM 通道 L2 初始化 频率 17KHz 占空比初始为 0

    // ------------ 初始化方向控制 GPIO（推挽输出，默认高电平正转） ------------
       gpio_init(MOTOR_GPIO_L, GPO, 1, GPO_PUSH_PULL);       //左电机  1正转
       gpio_init(MOTOR_GPIO_R, GPO, 1, GPO_PUSH_PULL);       //右电机  1正转
}

/**
 * @brief 设置转向电机 PWM（委托给 angle_motor_set_pwm）
 * @param moter 目标 PWM 值，自动限制在 [-S_MOTER_MAX, S_MOTER_MAX]
 * @note 该函数是对 angle_motor_set_pwm 的封装，增加了限幅保护
 */
void VeerMoter_Set(int moter )
{
    // ------------ 限幅后设置转向电机 PWM ------------
    moter=LimitMax(moter,S_MOTER_MAX);
    angle_motor_set_pwm(moter);

}

/**
 * @brief 设置左右电机 PWM 占空比及方向，自动处理正反转
 * @param moter_l 左电机 PWM 值（正=正转，负=反转）
 * @param moter_r 右电机 PWM 值（正=正转，负=反转）
 * @note 输入值自动限幅到 [-MOTER_MAX, MOTER_MAX]；正数设置方向引脚高电平，负数设置低电平并取反占空比
 */
void Moter_Set(int moter_l , int moter_r)
{
    // ------------ 左右电机 PWM 限幅 ------------
    moter_l =LimitMax(moter_l,MOTER_MAX);
    moter_r = LimitMax(moter_r,MOTER_MAX);
    motor_pwm_l = moter_l;
    motor_pwm_r = moter_r;

    // ------------ 左电机方向和占空比设置 ------------
    if(moter_l>=0)
    {
        pwm_set_duty(PWM_L, moter_l);
        gpio_set_level(MOTOR_GPIO_L, 1);
    }
    else if(moter_l<0)
    {
        pwm_set_duty(PWM_L, -moter_l);
        gpio_set_level(MOTOR_GPIO_L ,0);
    }

    // ------------ 右电机方向和占空比设置 ------------
    if(moter_r>=0)
    {
        pwm_set_duty(PWM_R, moter_r);
        gpio_set_level(MOTOR_GPIO_R ,1);
    }
    else if(moter_r<0)
    {
        pwm_set_duty(PWM_R,-moter_r );
        gpio_set_level(MOTOR_GPIO_R ,0);
    }




}

/**
 * @brief 将整数限制在 [-max, max] 范围内
 * @param input 输入整数值
 * @param max 限幅上限（绝对值）
 * @retval 限幅后的整数值
 * @note 对称限幅，超出范围则 clamp 到边界值
 */
int LimitMax(int input, int max)
    {
        if (input > max)
        {
            input = max;
        }
        else if (input < -max)
        {
            input = -max;
        }
        return input;
    }

/**
 * @brief 手动按键控制电机和舵机的测试函数
 * @note KEY1/2 控制电机增减 30，KEY3/4 控制舵机增减 100；在 IPS200 上显示编码器和设定值
 */
void Control(void)
{
    // ------------ 屏幕显示编码器和设定值 ------------
    ips200_show_int(0,  16*3,encoder_l , 5);
    ips200_show_int(50,  16*3,encoder_r, 5);
    ips200_show_int(100,  16*3,num, 5);
    ips200_show_int(150,  16*3,num1, 5);

   // ------------ 舵机控制 ------------
   Steer_Control(num1);

   // ------------ 按键调节电机和舵机参数 ------------
   if(key1_flag ==1)
   {
       key1_flag = 0;
       num+=30;

   }
   if(key2_flag ==1)
   {
       key2_flag = 0;
       num-=30;

   }
   if(key3_flag ==1)
   {
       key3_flag = 0;
       num1+=100;

   }
   if(key4_flag ==1)
   {
       key4_flag = 0;
       num1-=100;

   }



}

/**
 * @brief 齿条测试模式各阶段切换时重置目标值
 * @note 静态函数，仅在 peripheral.c 内部使用；Stage 3 时额外调用 Rack_Straight_Reset
 */
static void Rack_Test_Reset_Targets(void)
{
    // ------------ 清零测试目标值并停止后轮电机 ------------
    rack_test_speed_target = 0;
    rack_test_steer_target = 0;
    rear_motor_stop();

    // ------------ Stage 3 时重置直线保持状态 ------------
    if(rack_test_stage == 3)
    {
        Rack_Straight_Reset();
    }
}

/**
 * @brief 重置直线保持模式的目标偏航角和误差状态
 * @note 将当前 Yaw_1 设为期望偏航角的目标值，清零所有误差相关变量
 */
void Rack_Straight_Reset(void)
{
    // ------------ 记录当前偏航角作为目标，清零误差 ------------
    rack_straight_target_yaw = Yaw_1;
    rack_straight_yaw_error = 0.0f;
    rack_straight_last_yaw_error = 0.0f;
    rack_straight_steer_target = 0.0f;
}

/**
 * @brief 直线保持 PD 控制更新：根据偏航误差计算舵机修正量
 * @note 使用 PD 控制（Kp=2.0, Kd=0.45），输出限制在 [-22, 22] 度范围内
 */
void Rack_Straight_Update(void)
{
    // ------------ 计算偏航误差和误差微分 ------------
    float yaw_error = rack_straight_target_yaw - Yaw_1;
    float yaw_diff;

    angle_plan(&yaw_error);
    yaw_diff = yaw_error - rack_straight_last_yaw_error;
    angle_plan(&yaw_diff);

    // ------------ 保存本次误差，更新 PD 输出 ------------
    rack_straight_last_yaw_error = yaw_error;
    rack_straight_yaw_error = yaw_error;

    // ------------ PD 计算：转向 = -(Kp*error + Kd*diff) ------------
    rack_straight_steer_target = -(RACK_STRAIGHT_KP * yaw_error + RACK_STRAIGHT_KD * yaw_diff);

    // ------------ 限幅 ------------
    Value_Limit_float(&rack_straight_steer_target, -RACK_STRAIGHT_LIMIT_DEG, RACK_STRAIGHT_LIMIT_DEG);
}

/**
 * @brief 齿条测试模式状态机运行函数（Stage 0-3：空闲/转向测试/速度闭环/直线保持）
 * @note KEY1/2 切换阶段（前进/后退），KEY3/4 在当前阶段内调整参数；Stage 3 进入时自动记录当前 Yaw 作为直线基准
 */
void Rack_Test_Run(void)
{
    // ------------ KEY1 前进一个阶段 ------------
    if(key1_flag == 1)
    {
        key1_flag = 0;
        rack_test_stage++;
        if(rack_test_stage > 3) rack_test_stage = 0;
        Rack_Test_Reset_Targets();
    }

    // ------------ KEY2 后退一个阶段 ------------
    if(key2_flag == 1)
    {
        key2_flag = 0;
        if(rack_test_stage == 0) rack_test_stage = 3;
        else rack_test_stage--;
        Rack_Test_Reset_Targets();
    }

    // ------------ KEY3 增加当前阶段参数 ------------
    if(key3_flag == 1)
    {
        key3_flag = 0;
        if(rack_test_stage == 1)
            rack_test_steer_target += 10;
        else if(rack_test_stage == 2 || rack_test_stage == 3)
            rear_motor_set_target_mps(rear_motor_get_target_mps() + 0.5f);
    }

    // ------------ KEY4 减少当前阶段参数 ------------
    if(key4_flag == 1)
    {
        key4_flag = 0;
        if(rack_test_stage == 1)
            rack_test_steer_target -= 10;
        else if(rack_test_stage == 2 || rack_test_stage == 3)
            rear_motor_set_target_mps(rear_motor_get_target_mps() - 0.5f);
    }

    // ------------ 转向角度限幅 ------------
    if(rack_test_steer_target > 60)  rack_test_steer_target = 60;
    if(rack_test_steer_target < -60) rack_test_steer_target = -60;

    /* 通用显示 */
    ips200_show_string(X(8), Y(0), "Rack_Test");
    ips200_show_string(X(1), Y(2), "Stage");       ips200_show_int(X(10), Y(2), rack_test_stage, 3);

    // ------------ Stage 0/1：空闲监测和转向测试 ------------
    if(rack_test_stage <= 1)
    {
        /* Stage 0: 传感器数据监测, Stage 1: 前轮转向调试 */
        ips200_show_string(X(1), Y(3), "Yaw");         ips200_show_float(X(10), Y(3), Yaw_1, 4, 2);
        ips200_show_string(X(1), Y(4), "EncL");        ips200_show_int(X(10), Y(4), Speed_ecd.delta_l, 5);
        ips200_show_string(X(1), Y(5), "EncR");        ips200_show_int(X(10), Y(5), Speed_ecd.delta_r, 5);
        ips200_show_string(X(1), Y(6), "SteerT");      ips200_show_int(X(10), Y(6), rack_test_steer_target, 5);
        ips200_show_string(X(1), Y(7), "SteerA");      ips200_show_int(X(10), Y(7), angle, 5);
        ips200_show_string(X(1), Y(8), "SteerO");      ips200_show_int(X(10), Y(8), angle_speed, 5);
    }
    // ------------ Stage 2：后轮速度闭环测试 ------------
    else if(rack_test_stage == 2)
    {
        /* Stage 2: 后轮速度闭环控制 (m/s) */
        rear_motor_pid_update_100ms();
        ips200_show_string(X(1), Y(3), "TgtMps");     ips200_show_float(X(10), Y(3), rear_motor_get_target_mps(), 3, 2);
        ips200_show_string(X(1), Y(4), "ActMps");     ips200_show_float(X(10), Y(4), rear_motor_get_speed_mps(), 3, 2);
        ips200_show_string(X(1), Y(5), "PWM");        ips200_show_int(X(10), Y(5), rear_motor_get_pwm(), 5);
        ips200_show_string(X(1), Y(6), "Enc10");      ips200_show_int(X(10), Y(6), rear_motor_get_encoder_10ms(), 5);
        ips200_show_string(X(1), Y(7), "Enc100");     ips200_show_int(X(10), Y(7), rear_motor_get_encoder_100ms(), 5);
        ips200_show_string(X(1), Y(8), "Yaw");        ips200_show_float(X(10), Y(8), Yaw_1, 4, 2);
    }
    // ------------ Stage 3：直线保持测试 ------------
    else
    {
        /* Stage 3: 直线保持测试, 进入本阶段时锁定当前Yaw */
        rear_motor_pid_update_100ms();
        ips200_show_string(X(1), Y(3), "TgtMps");     ips200_show_float(X(10), Y(3), rear_motor_get_target_mps(), 3, 2);
        ips200_show_string(X(1), Y(4), "ActMps");     ips200_show_float(X(10), Y(4), rear_motor_get_speed_mps(), 3, 2);
        ips200_show_string(X(1), Y(5), "TgtYaw");     ips200_show_float(X(10), Y(5), rack_straight_target_yaw, 4, 2);
        ips200_show_string(X(1), Y(6), "YawErr");     ips200_show_float(X(10), Y(6), rack_straight_yaw_error, 3, 2);
        ips200_show_string(X(1), Y(7), "Steer");      ips200_show_float(X(10), Y(7), rack_straight_steer_target, 3, 2);
        ips200_show_string(X(1), Y(8), "PWM");        ips200_show_int(X(10), Y(8), rear_motor_get_pwm(), 5);
    }
}

/**
 * @brief 初始化 GNSS 模块
 * @note 使用 TAU1201 型号的 GNSS 模块；上电后模块会自动开始搜星定位
 */
void GPS_Init(void)
{
    // ------------ 初始化 GNSS 模块（TAU1201） ------------
    gnss_init(TAU1201);               // GN42A 为GPS模块 GN43RFA 为RTK模块

}
