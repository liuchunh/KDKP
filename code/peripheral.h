/*
 * peripheral.h
 *
 *  Created on: 2025年11月20日
 *      Author: 18905
 */

#ifndef CODE_PERIPHERAL_H_
#define CODE_PERIPHERAL_H_


//外部变量
extern uint8 TIM_FLAG1;
extern uint8 TIM_FLAG2;
extern uint8 TIM_FLAG3 ;
extern uint8 key1_flag ;
extern uint8 key2_flag ;
extern uint8 key3_flag ;
extern uint8 key4_flag ;
extern uint8 key_val;
extern uint8 key_value;

extern int16 encoder_l ;
extern int16 encoder_r ;
extern int motor_pwm_l;
extern int motor_pwm_r;
extern uint8 rack_test_stage;
extern int16 rack_test_speed_target;
extern int32 rack_test_steer_target;
extern float rack_straight_target_yaw;
extern float rack_straight_yaw_error;
extern float rack_straight_steer_target;

typedef struct{
        int16 left_counter;
        int16 right_counter;
        int32 delta_l;
        int32 delta_r;
        int16 last_ecdcount_l;
        int16 last_ecdcount_r;
}Encoder_t;

extern Encoder_t Speed_ecd;
extern Encoder_t guandao_ecd;
extern Encoder_t Steer_ecd;

//宏定义
#define r_ecdcounter()    encoder_get_count(ENCODER_QUADDEC)
#define l_ecdcounter()    encoder_get_count(ENCODER_QUADDEC)
#define BUZZER_PIN  (P33_10)      //蜂鸣器

#define KEY1                    (P20_6)    //按键引脚定义   //P20_6  //P11_3
#define KEY2                    (P20_7)                               //P20_7 //P11_2
#define KEY3                    (P11_2)                                //P11_3 //P20_7
#define KEY4                    (P11_3)                               //P11_2 //P20_6
#define SWITCH2                 (P33_12)
#define SWITCH1                 (P33_11)

#define SERVO_MOTOR_PWM             (ATOM1_CH1_P33_9)
#define SERVO_MOTOR_FREQ            (50)
#define SERVO_MOTOR_DUTY(x)         ((float)PWM_DUTY_MAX/(1000.0/(float)SERVO_MOTOR_FREQ)*(0.27+(float)(x)/90.0))
#define SERVO_MOTOR_MID             (80)                                        //中值80
#define SERVO_MOTOR_LMAX            (65)                                        //左打满60
#define SERVO_MOTOR_RMAX            (95)                                       //右打满100

#define ENCODER_QUADDEC                 (TIM2_ENCODER)
#define ENCODER_QUADDEC_A               (TIM2_ENCODER_CH1_P33_7)
#define ENCODER_QUADDEC_B               (TIM2_ENCODER_CH2_P33_6)

#define PWM_L              (ATOM0_CH3_P21_5)
#define PWM_R              (ATOM0_CH1_P21_3)
#define MOTOR_GPIO_L              (P21_4)
#define MOTOR_GPIO_R              (P21_2)
#define MOTER_MAX       (7000)
#define MOTER_MIN       (-7000)
#define S_MOTER_MAX       (5000)
#define S_MOTER_MIN       (-5000)
//函数

/**
 * @brief 初始化所有外设：显示、按键、蜂鸣器、舵机、编码器、电机、IMU、GPS 等
 */
void Init_All(void);

/**
 * @brief 初始化 4 个按键和 2 个拨码开关为 GPIO 输入模式
 */
void Key_Init(void);

/**
 * @brief 扫描按键状态，检测边沿变化并设置按键标志位
 */
void Key_Scan(void);

/**
 * @brief 初始化蜂鸣器 GPIO 为推挽输出
 */
void Buzzer_Init(void);

/**
 * @brief 蜂鸣器鸣响指定毫秒数后关闭
 */
void Buzzer_check(int time2);

/**
 * @brief 初始化舵机 PWM 并设置中位
 */
void Steer_init(void);

/**
 * @brief 设置舵机角度，自动限制在左右极限范围内
 */
void Steer_set(int angle);

/**
 * @brief 手动按键调试舵机角度的测试函数
 */
void Steer_text(void);

/**
 * @brief 初始化正交编码器硬件
 */
void Encoder_Init(void);

/**
 * @brief 读取编码器计数，计算增量变化并进行滑动平均滤波
 */
void Encoder_Get(Encoder_t *count);

/**
 * @brief 初始化电机 PWM 和方向控制 GPIO
 */
void Motor_init(void);

/**
 * @brief 设置左右电机 PWM 占空比及方向，自动处理正反转
 */
void Moter_Set(int moter_l , int moter_r);

/**
 * @brief 将整数限制在 [-max, max] 范围内
 */
int LimitMax(int input, int max);

/**
 * @brief 手动按键控制电机和舵机的测试函数
 */
void Control(void);

/**
 * @brief 齿条测试模式状态机运行函数（Stage 0-3：空闲/转向测试/速度闭环/直线保持）
 */
void Rack_Test_Run(void);

/**
 * @brief 重置直线保持模式的目标偏航角和误差状态
 */
void Rack_Straight_Reset(void);

/**
 * @brief 直线保持 PD 控制更新：根据偏航误差计算舵机修正量
 */
void Rack_Straight_Update(void);

/**
 * @brief 初始化 GNSS 模块
 */
void GPS_Init(void);

/**
 * @brief 复位编码器计数结构体的所有字段
 */
void Encoder_count_init(Encoder_t *count);

/**
 * @brief 设置转向电机 PWM（委托给 angle_motor_set_pwm）
 */
void VeerMoter_Set(int moter );
#endif /* CODE_PERIPHERAL_H_ */
