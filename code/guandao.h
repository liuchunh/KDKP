/*
 * guandao.h
 *
 *  Created on: 2026年3月16日
 *      Author: 18905
 */

#ifndef CODE_GUANDAO_H_
#define CODE_GUANDAO_H_


//宏
#define ONE_TICK_DISTANCE                      0.000378f                                     //每每个编码器实际距离单位：cm
#define MAX_LENGTH_INDEX                      400                                             //存储上限
#define MAX_GPS_RECORD                          30
#define M_PI                                                  3.14159265358979323846f
//#define RECORD_THRESHOLD                     0.4f                                   // 记录间隔阈值 单位：m
//#define PURSUIT_THRESHOLD                     0.6f                                  //追踪缓冲阈值 单位：m
//#define PREVIEW_SPETS                               2                                    //前视步数
#define WHEEL_BASE                                     0.724f                               //轴距 单位：m
#define TRACK_WIDTH                                   0.594f                              //轮距 单位：m
//#define BASE_SPEED                                      10.0f                               // 基准速度控制
#define MIN_SPEED                                        10.0f                                //最小速度
#define MAX_STEERING_RAD                        90.0f                                //转向角限幅
#define SLIP_CHEAK_INDEX                            4.0f                                 //打滑角速度绝对差阈值
#define START_GPS_FLAG                              1                                      //启动GPS标志
#define PORTION_TWO_INDEX                      3
#define ANGLE_CORRECT_KP                         0.0f
#define CORRECT_ANGLE_1                           -90.0f
#define CORRECT_ANGLE_3                           180.0f
//全局变量


typedef struct {
        float x;
        float y;
        float theta;
}state_t;


typedef struct {
        double lat;
        double lon;
        float theta;
        int16 cheak_flag;
}GPS_state;

typedef struct guandao{
        state_t current_state;
        state_t record_map[MAX_LENGTH_INDEX];
        GPS_state record_gpsmap[MAX_GPS_RECORD];

        int16 length_index;
        int current_point_index;

        int16 gps_record_length;

        struct guandao * next;
}guandao_state;

typedef enum {
    NONE,
    Left_Slip,
    Right_Slip,
}SLIP_Cheak;

extern SLIP_Cheak slip_state;
extern guandao_state INS;
extern guandao_state passage;                    //1 = route_setting_choice
extern guandao_state portion_3;                     //2 = route_setting_choice
extern guandao_state portion_2;                    //3 = route_setting_choice
extern float out_v_l ;
extern float out_v_r ;
extern float out_servo ;
extern uint8 route_setting_choice;
extern int16 daoche_point_length ;
extern float daoche_speed ;
extern float base_speed ;
extern uint16 portion3_foint_flag ;
extern uint8 daoche_flash_cheack ;
extern float persuit_threshold ;  //PURSUIT_THRESHOLD
extern float record_threshold ;//RECORD_THRESHOLD
extern int16 preview_spets ;       //PREVIEW_SPETS
extern float final_dsts ;
//函数
/**
 * @brief 初始化管道状态结构体的各项参数为零
 */
void guandao_state_init(guandao_state * e);

/**
 * @brief 将所有管道状态节点链接成链表
 */
void guandao_chain_init(void);

/**
 * @brief 根据编码器数据实时更新管道当前位置（里程计推算）
 */
void update_state(guandao_state * state , Encoder_t * ecd);

/**
 * @brief 计算两个状态点之间的欧氏距离
 */
float get_distance(state_t p1, state_t p2);

/**
 * @brief 按距离阈值间隔自动记录航点
 */
void record_waypoint(guandao_state * state);

/**
 * @brief 纯追踪路径跟踪控制算法的核心实现
 */
void pursuit_control_mode(guandao_state * state,float * out_v_l,float * out_v_r,float *out_servo);

/**
 * @brief 在 IPS200 屏幕上显示记录的管道位置信息
 */
void guandao_show(guandao_state * p);

/**
 * @brief 在 IPS200 屏幕上显示追踪管道位置信息（当前点与目标点）
 */
void follow_points_show(guandao_state * p);

/**
 * @brief 手动构建测试用地图数据
 */
void build_map_text(guandao_state * state);

/**
 * @brief 根据编码器计算实际车速
 */
float speed_calculate(Encoder_t * ecd , float time_tick);

/**
 * @brief 轮子打滑检测，通过比较理论角速度与 IMU 实测角速度判断打滑方向
 */
void slip_cheak(Encoder_t * ecd,float steer_angle);

/**
 * @brief 计算预瞄点的角度和距离，用于纯追踪控制
 */
void pursuit_midhandle(guandao_state * state ,state_t * current_state , int index ,float * angle , float * distance);

/**
 * @brief 管道路径记录的核心函数，支持多路线选择和 Flash 存储
 */
void guandao_record(guandao_state * state);

/**
 * @brief 主路径跟踪函数，根据路线选择标志执行纯追踪控制及 GPS 轨迹跟踪
 */
void guandao_trace(guandao_state * state);

/**
 * @brief 第一段路径跟踪：从里程计出发，执行纯追踪控制到达终点
 */
void portion_1(void);

/**
 * @brief 重置第一段路径的状态（里程计清零、编码器清零、停止电机）
 */
void portion_1_reset(void);

/**
 * @brief 对第三段路径进行翻转和镜像处理，生成回程轨迹
 */
uint8 portion3_points_switch(void);

/**
 * @brief 在 IPS200 屏幕上绘制管道路线地图及进度条动画
 */
void Guandao_Points_Show(guandao_state * e);

/**
 * @brief 手动按键在当前管道位置记录一个点
 */
void Key_Record_Point(guandao_state * e);

/**
 * @brief 第二段路径点的记录，由按键触发后按指定长度记录航点
 */
void portion2_points_record(void);

/**
 * @brief 构建对称的第二段路径（8 字形重复路径拼接）
 */
uint8 portion2_points_build(void);

/**
 * @brief 语音/通道控制的第二段路径多段跟踪，使用状态机切换不同路径段
 */
void portion2_points_trace(uint8 channal1 , uint8 channal2 ,uint8 state );

/**
 * @brief 终点航向校正：在接近终点时根据目标航向角修正转向输出
 */
void azimuth_adjust(guandao_state * state , float start_d , float dist_to_final , float * target_steering , float target_yaw );
#endif /* CODE_GUANDAO_H_ */
