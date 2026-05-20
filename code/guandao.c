/*
 * guandao.c
 *
 *  Created on: 2026年3月16日
 *      Author: 18905
 */

#include "zf_common_headfile.h"
#include "rear_motor/rear_motor.h"

guandao_state INS;                               //0 = route_setting_choice
guandao_state passage;                    //1 = route_setting_choice
guandao_state portion_3;                     //2 = route_setting_choice
guandao_state portion_2;                    //3 = route_setting_choice

SLIP_Cheak slip_state = NONE;         // 打滑检测状态初始为无打滑

uint8 route_setting_choice = 0;        // 路线选择标志，0-3对应不同路线

float base_speed = 10.0f;
float persuit_threshold = 0.4f;         // 追踪阈值，到达目标点的距离容差
float record_threshold = 0.4f;         // 路径记录阈值
int16 preview_spets = 2;                  // 预瞄步数
float daoche_speed = -10.0;           //倒车速度
float final_dsts = 3.0f;                     // 终点减速距离阈值

int16 daoche_point_length = 0;    // 倒车点长度
uint8 daoche_flag =0;                    // 倒车标志
uint8 daoche_flash_cheack =0;// 倒车Flash检测标志
static uint8 portion1_state_flag = 0;
static uint16 portion1_finally_length = 0;

/**
 * @brief 初始化管道状态结构体的各项参数为零
 * @param e 指向待初始化的管道状态结构体的指针
 * @note 调用前确保结构体内存已分配；此函数会将 current_point_index、current_state、length_index、gps_record_length 全部置零
 */
void guandao_state_init(guandao_state * e)
{
    // ------------ 将所有状态字段初始化为零 ------------
    e->current_point_index =0;
    e->current_state.theta =0.0f;
    e->current_state.x=0.0f;
    e->current_state.y=0.0f;
    e->length_index=0;

    e ->gps_record_length =0;

}

/**
 * @brief 将所有管道状态节点链接成链表
 * @note 链接顺序为 INS -> passage -> portion_3 -> portion_2 -> NULL；在 Init_All 中调用一次即可
 */
void guandao_chain_init(void)
{
    // ------------ 将四个管道状态节点串联成链表 ------------
    INS.next = &passage;
    passage.next = &portion_3;
    portion_3.next = &portion_2;
    portion_2.next = NULL;
}

/**
 * @brief 计算两个状态点之间的欧氏距离
 * @param p1 第一个状态点
 * @param p2 第二个状态点
 * @retval 两点之间的欧氏距离（float）
 * @note 使用 hypotf 函数避免浮点溢出，适用于里程计坐标系的二维距离计算
 */
float get_distance(state_t p1, state_t p2)
{
    // ------------ 计算 sqrt(dx^2 + dy^2) ------------
    return hypotf(p2.x - p1.x, p2.y - p1.y);
}

/**
 * @brief 根据编码器数据实时更新管道当前位置（里程计推算）
 * @param state 指向待更新的管道状态结构体的指针
 * @param ecd 指向编码器结构体的指针，包含左右轮编码增量
 * @note 该函数会根据 slip_state 调整左右轮数据的对称处理；倒车模式下速度和角度有特殊处理
 */
void update_state(guandao_state * state , Encoder_t * ecd)
{
    // ------------ 声明局部变量和读取编码器数据 ------------
    float delta_real_center = 0;
    float delta_real_l = 0;
    float delta_real_r = 0;
    Encoder_Get(ecd);

    // ------------ 根据打滑状态选择左右轮数据处理策略 ------------
    switch(slip_state)
    {
        case NONE:
            delta_real_l = (float)ecd->delta_l*ONE_TICK_DISTANCE;
            delta_real_r = (float)ecd->delta_r*ONE_TICK_DISTANCE;

            break;

        case Left_Slip:
            delta_real_r = (float)ecd->delta_r*ONE_TICK_DISTANCE;
            delta_real_l = delta_real_r;

            break;

        case Right_Slip:
            delta_real_l = (float)ecd->delta_l*ONE_TICK_DISTANCE;
            delta_real_r = delta_real_l;

            break;

        default : break;
    }

    // ------------ 计算中心位移和当前角度（倒车时特殊处理） ------------
    if(!daoche_flag)
    {
        delta_real_center  = (delta_real_l+delta_real_r)/2.0f;
        state->current_state.theta = Yaw_1;
        if (DebugStatus)
            uart_write_printf(DEBUG_UART_INDEX, "Not reverse! Center Displacement = %.5lf, theta = %.5lf\n", delta_real_center, Yaw_1);
    }
    else
    {
        delta_real_center  = -(delta_real_l+delta_real_r)/2.0f;
        state->current_state.theta =Yaw_1+180.0f;
        angle_plan(&state->current_state.theta);
        if (DebugStatus)
            uart_write_printf(DEBUG_UART_INDEX, "Reverse! Center Displacement = %.5lf, theta = %.5lf\n", delta_real_center, state->current_state->theta);
    }

    // ------------ 里程计更新：根据位移和角度累加 x, y 坐标 ------------
    state->current_state.x+=delta_real_center*sinf(state->current_state.theta/180.0f*M_PI);
    state->current_state.y+=delta_real_center*cosf(state->current_state.theta/180.0f*M_PI);
    if (DebugStatus)
        uart_write_printf(DEBUG_UART_INDEX, "Current Coordinate: (%.5lf, %.5lf)\n", state->current_state.x, state->current_state.y);
}

/**
 * @brief 重置第一段路径的状态（里程计清零、编码器清零、停止电机）
 * @note 调用后 INS 的 current_state 全部归零，电机和舵机输出置零，编码器计数清零
 */
void portion_1_reset(void)
{
    // ------------ 将第一段路径相关的所有标志和状态清零 ------------
    portion1_state_flag = 0;
    portion1_finally_length = 0;
    INS.current_point_index = 0;
    daoche_flag = 0;
    out_v_l = 0;
    out_v_r = 0;
    out_servo = 0;
    INS.current_state.x = 0.0f;
    INS.current_state.y = 0.0f;
    INS.current_state.theta = 0.0f;

    // ------------ 编码器计数清零并停止后轮电机 ------------
    Encoder_count_init(&guandao_ecd);
    Encoder_count_init(&Speed_ecd);
    encoder_clear_count(ENCODER_QUADDEC);
    rear_motor_stop();
}

/**
 * @brief 第一段路径跟踪：从里程计出发，执行纯追踪控制到达终点
 * @note 首次进入时确定终点位置（优先使用 KEY1 记录的倒车点），到达终点后切换回管道模式
 */
void portion_1(void)
{
    // ------------ 更新里程计位置 ------------
    update_state(&INS, &guandao_ecd);

    // ------------ 首次进入时确定终点目标 ------------
    if(portion1_state_flag == 0)
    {
        if (DebugStatus) uart_write_printf(DEBUG_UART_INDEX, "First Entered! Confirm Dst!\n");

        portion1_finally_length = INS.length_index;
        if(daoche_point_length > 0 && daoche_point_length < portion1_finally_length)
        {
            INS.length_index = daoche_point_length;                // KEY1记录的倒车点为第一段终点
        }
        else
        {
            INS.length_index = portion1_finally_length;            // 没有倒车点时就用INS整条路径
        }
        portion1_state_flag = 1;
    }

    // ------------ 执行纯追踪控制 ------------
    pursuit_control_mode(&INS ,&out_v_l ,&out_v_r ,&out_servo);

    // ------------ 到达终点时停车并切换模式 ------------
    if(INS.current_point_index >= INS.length_index)
    {
        if (DebugStatus) uart_write_string(DEBUG_UART_INDEX, "Reach dst! Mode switched to GUANDAO.\n");
        out_v_l = 0;
        out_v_r = 0;
        out_servo = 0;
        control_mode = GUANDAO;
    }

    // ------------ 屏幕显示追踪状态 ------------
    follow_points_show(&INS);
}

/**
 * @brief 按距离阈值间隔自动记录航点
 * @param state 指向待记录航点的管道状态结构体的指针
 * @note 首点直接记录；后续点仅在移动距离超过 record_threshold 时才记录；支持 KEY1 记录倒车点
 */
void record_waypoint(guandao_state * state)
{
    // ------------ 越界保护 ------------
    if(state -> length_index >= MAX_LENGTH_INDEX) return;

    // ------------ 首个点直接记录 ------------
    if(state ->length_index == 0)
    {
        state->record_map[state->length_index] = state->current_state;
        state->length_index++;
        return;
    }

    // ------------ 距离超过阈值时记录航点 ------------
    state_t last_record =  state->record_map[state->length_index-1];
    float dist = get_distance(state->current_state, last_record );

    if(dist >= record_threshold) // 相邻两点距离超过 0.4m 存点
    {
        state->record_map[state->length_index] =state->current_state;
        state->length_index++;
    }

    // ------------ KEY1 按键或遥控触发记录倒车点 ------------
    static uint8 dche_flag = 1;
    if(state == &INS && (key1_flag == 1|| x6f_out[3] ==200) && dche_flag ==1)  //遥控触发记录
    {
        key1_flag =0;
        dche_flag =0;
        state->record_map[state->length_index] =state->current_state;
        daoche_point_length = state->length_index;
        state->length_index++;
        daoche_flag =1;
        daoche_flash_cheack =1;
    }
}

/**
 * @brief 第二段路径点的记录，由按键触发后按指定长度记录航点
 * @note KEY1 按下时记录起始点，后续自动按 PORTION_TWO_INDEX 长度记录，使用 passage 结构
 */
void portion2_points_record(void)
{
    // ------------ KEY1 按下时记录起始点 ------------
    static int16 p2p_r_flag1= 0 ;
//    static uint8 p2p_r_flag2= 0;
    if(key1_flag == 1)
    {
        key1_flag = 0;
        p2p_r_flag1 = passage.length_index;
        Key_Record_Point(&passage);

    }

    // ------------ 按指定长度自动记录后续航点 ------------
    if(passage.length_index - p2p_r_flag1<=PORTION_TWO_INDEX -1)
    {
        record_waypoint(&passage);
    }


}

/**
 * @brief 纯追踪（Pure Pursuit）路径跟踪控制算法的核心实现
 * @param state 指向管道状态结构体的指针，包含当前位姿和路径数据
 * @param out_v_l 指向左轮输出速度的指针，函数将计算结果写入此处
 * @param out_v_r 指向右轮输出速度的指针，函数将计算结果写入此处
 * @param out_servo 指向舵机输出角度的指针，函数将计算结果写入此处
 * @note 函数包含到达判断、预瞄计算、转向计算、终点减速、以及差动速度分解；调用前需确保 record_map 已填充有效数据
 */
void pursuit_control_mode(guandao_state * state,float * out_v_l,float * out_v_r,float *out_servo)
{
    // ------------ 声明局部变量 ------------
    float actual_ld = 0 , preview_alpha =0;
    float actual_ld2 = 0 , preview_alpha2 =0;
    float target_steering = 0;

    // ------------ 路径为空或已到达终点时停止输出 ------------
    if(state->length_index == 0 || state->current_point_index ==state->length_index)
    {
        if (DebugStatus) uart_write_string(DEBUG_UART_INDEX, "Empty Route or Reach Dst!\n");
        * out_v_l = 0;
        * out_v_r = 0;
        *out_servo = 0;
//        Buzzer_check(50);
        return;
    }

    // ------------ 计算到当前目标点的距离和角度偏差 ------------
    state_t current_point = state->current_state;
    state_t target_point = state->record_map[state->current_point_index];

    float dx = target_point.x - current_point.x;
    float dy = target_point.y - current_point.y;
    float distance_to_target = hypotf(dx, dy);
    float angle_to_target = atan2f(dx,dy)/M_PI*180.0f;
    float angle_diff = angle_to_target - state->current_state.theta;

    while (angle_diff > 180.0f) angle_diff -= 360.0f;
    while (angle_diff < -180.0f) angle_diff += 360.0f;

    if (DebugStatus){
        uart_write_printf(DEBUG_UART_INDEX, "delta x = %.5lf, delta y = %.5lf\n", dx, dy);
        uart_write_printf(DEBUG_UART_INDEX,
                "distance to target: %.5lf, angle to target: %.5lf, angle diff: %.5lf\n",
                distance_to_target, angle_to_target, angle_diff);
    }

    // ------------ 到达当前目标点则切换到下一个 ------------
    if(distance_to_target <= persuit_threshold|| fabsf(angle_diff) > 90.0f )//  || fabsf(angle_diff) > 90.0f
    {
        state->current_point_index++;
        if (state->current_point_index >=state->length_index )
        {
            if (DebugStatus) uart_write_string(DEBUG_UART_INDEX, "Dst Reached!\n");
            state->current_point_index = state->length_index;
            * out_v_l = 0;
            * out_v_r = 0;
            *out_servo = 0;
//            Buzzer_check(50);
            return;
        }
        if (DebugStatus) uart_write_string(DEBUG_UART_INDEX, "Current Point Reached! Switch to next point!\n");
    }

    // ------------ 计算两个预瞄点的角度和距离 ------------
    pursuit_midhandle(state , &current_point , preview_spets , &preview_alpha , &actual_ld);
    pursuit_midhandle(state , &current_point , 5 , &preview_alpha2 , &actual_ld2);

    // ------------ 根据预瞄角计算转向系数 k ------------
    float k = 0;
    k = -0.0038*fabs(preview_alpha2) + 1;
    Value_Limit_float(&k ,0.6 ,1);
    if (DebugStatus)
        uart_write_printf(DEBUG_UART_INDEX, "Steering coefficient = %.5lf\n", k);

    // ------------ 根据角度偏差分三档计算目标转向角 ------------
    if(angle_diff >=90)
    {
       target_steering = 30.0f;
       if (DebugStatus)
           uart_write_string(DEBUG_UART_INDEX, "Angle diff >= 90! Target Steering = 30\n");
   }
   else if(angle_diff <= -90)
   {
       target_steering = -30.0f;
       if (DebugStatus)
          uart_write_string(DEBUG_UART_INDEX, "Angle diff <= -90! Target Steering = -30\n");
   }
   else if(fabsf(angle_diff) <=90)
   {
       target_steering = 3.0f*atan2f(2.0f * WHEEL_BASE * sinf(preview_alpha/180.0f*M_PI), actual_ld)/M_PI*180.0f;
       if (DebugStatus)
          uart_write_printf(DEBUG_UART_INDEX, "abs(Angle diff) <= 90! Target Steering Angle = %.5f\n", target_steering);
   }

   // ------------ 转向角限幅 ------------
   Value_Limit_float(&target_steering ,-MAX_STEERING_RAD,MAX_STEERING_RAD);

   if (DebugStatus)
         uart_write_printf(DEBUG_UART_INDEX, "Switch Target Steering Angle to %.5lf\n", target_steering);
//   slip_cheak(&guandao_ecd,target_steering);

   // ------------ 计算到终点的距离 ------------
   float dist_to_final = get_distance(state->current_state, state->record_map[state->length_index -1]);
   float v_center = base_speed;
   if (DebugStatus)
       uart_write_printf(DEBUG_UART_INDEX, "Distance To dst: %.5lf\n", dist_to_final);

    // ------------ 根据路线选择执行终点航向校正 ------------
   switch (route_setting_choice)
   {
       case 0:{
           azimuth_adjust(state, 1.3, dist_to_final, &target_steering, CORRECT_ANGLE_1);
           break;
       }
       case 2:{
           azimuth_adjust(state, 5.5, dist_to_final, &target_steering, CORRECT_ANGLE_3);
           break;
       }
       case 3:

           break;
       default : break;
   }

    // ------------ 终点减速逻辑 ------------
   if (dist_to_final < final_dsts && state->current_point_index >= state->length_index - 30)
   {

       persuit_threshold = persuit_threshold*(dist_to_final / final_dsts);
       if(persuit_threshold < 0.3f){persuit_threshold = 0.3f;}
       v_center = base_speed * (dist_to_final / final_dsts);
       if (v_center < MIN_SPEED) v_center = MIN_SPEED; // 最低速度限制
       uart_write_printf(DEBUG_UART_INDEX, "Ready to slow down! speed = %.5f\n", v_center);
   }

    // ------------ 差动速度分解：中心速度 + 角速度分量 ------------
   float w = (v_center * tanf(target_steering/3.0f/180.0f*M_PI)) / WHEEL_BASE;
   *out_v_l = v_center + (w * TRACK_WIDTH / 2.0f);
   *out_v_r = v_center - (w * TRACK_WIDTH / 2.0f);

   *out_servo = target_steering;


}

/**
 * @brief 终点航向校正：在接近终点时根据目标航向角修正转向输出
 * @param state 指向管道状态结构体的指针
 * @param start_d 开始航向校正的距离阈值
 * @param dist_to_final 当前到终点的距离
 * @param target_steering 指向当前转向角度的指针，函数会修改其值
 * @param target_yaw 目标航向角（度）
 * @note 校正效果随距离线性减弱；倒车时角度偏差方向取反
 */
void azimuth_adjust(guandao_state * state ,float start_d , float dist_to_final , float * target_steering , float target_yaw )
{
    // ------------ 距离小于阈值时执行航向校正 ------------
    float angle_delta = 0;
    if(dist_to_final < start_d && state->current_point_index >= state->length_index - 30)
    {
        // ------------ 计算航向偏差（倒车时取反） ------------
        if(!daoche_flag)angle_delta  = target_yaw - Yaw_1;
        else angle_delta  = -(target_yaw - Yaw_1);
        angle_plan(&angle_delta);

        // ------------ 线性插值计算校正系数 ------------
        float kp_d = (1 - ANGLE_CORRECT_KP)/start_d*dist_to_final +ANGLE_CORRECT_KP;
        float kp_y = (ANGLE_CORRECT_KP -1)*(dist_to_final/start_d - 1);

        * target_steering = kp_d*(* target_steering) + kp_y *angle_delta;
    }
}

/**
 * @brief 计算预瞄点的角度和距离，用于纯追踪控制
 * @param state 指向管道状态结构体的指针
 * @param current_state 指向当前状态点的指针
 * @param index 预瞄步数（超前当前目标点的点数）
 * @param angle 指向输出角度的指针，存储预瞄点相对于当前位置的偏角
 * @param distance 指向输出距离的指针，存储到预瞄点的距离
 * @note 记录模式下预瞄点固定为最后一个点；距离小于 0.1 时会强制设为 0.1 防止除零
 */
void pursuit_midhandle(guandao_state * state ,state_t * current_state , int index ,float * angle , float * distance)
{
    // ------------ 根据模式确定预瞄点索引 ------------
    int preview_index = 0;
    if(main_mode == Guandao_Record_Mode)
    {
        preview_index = state->length_index - 1;
    }
    else
    {
        preview_index = state->current_point_index+index;
    }

   if(preview_index >=state->length_index)preview_index = state->length_index - 1;

   // ------------ 获取预瞄点坐标 ------------
   state_t preview_point = state->record_map[preview_index];
   if (DebugStatus) uart_write_printf(DEBUG_UART_INDEX, "Preview Point: (%.5lf, %.5lf), theta: %.5lf\n",
           preview_point.x, preview_point.y, preview_point.theta);

   // ------------ 计算预瞄点相对当前点的角度和距离 ------------
   float p_dx = preview_point.x - current_state->x;
   float p_dy = preview_point.y - current_state->y;
    * angle = atan2f(p_dx,p_dy)/M_PI*180.0f - state->current_state.theta;

   while (* angle > 180.0f) * angle -= 360.0f;
   while (* angle < -180.0f) * angle += 360.0f;

    * distance = hypotf(p_dx, p_dy);
    if(*distance < 0.1f) *distance = 0.1f;

    if (DebugStatus){
        uart_write_printf(DEBUG_UART_INDEX, "Compare preview point and Current point: \n\tAngle: %.5lf\n\t", angle);
        uart_write_printf(DEBUG_UART_INDEX, "Distance = %.5lf\n\tdx = %.5lf, dy = %.5lf\n", *distance, p_dx, p_dy);
    }

}

/**
 * @brief 手动构建测试用地图数据
 * @param state 指向管道状态结构体的指针，地图数据将写入其 record_map 数组
 * @note 该函数生成一个锯齿形测试地图（20个点），并将倒车点长度设为 20；仅供调试使用
 */
void build_map_text(guandao_state * state)
{
    // ------------ 分段构建锯齿形测试地图 ------------
    float length = 0.176f *6.0;
    for(int i = 0 ; i<=5 ;i++)
    {
        state->record_map[i].x = length * i;
        state->record_map[i].y = length * i;
        state->record_map[i].theta = 45.0f;
    }
    for(int i = 6 ; i<=10 ;i++)
    {
        state->record_map[i].x = length* 10 - length * i;
        state->record_map[i].y = length * i;
        state->record_map[i].theta = 45.0f;
    }
    for(int i = 11 ; i<=15 ;i++)
    {
        state->record_map[i].x =length* 10 - length* i;
        state->record_map[i].y = length* 20 - length * i;
        state->record_map[i].theta = 45.0f;
    }
    for(int i = 16 ; i<=20 ;i++)
    {
        state->record_map[i].x =-length* 20 + length * i;
        state->record_map[i].y = length* 20 - length* i;
        state->record_map[i].theta = 45.0f;
    }

    // ------------ 设置路径长度和倒车点 ------------
    state->length_index = 20;
    daoche_point_length =20;

//    for(int i = 0 ; i<20 ;i++)
//    {
//        state->record_map[i].x = 0;
//        state->record_map[i].y = length * i;
//        state->record_map[i].theta = 0.0f;
//    }

}

float out_v_l = 0;
float out_v_r = 0;
float out_servo = 0;

/**
 * @brief 管道路径记录的核心函数，支持多路线选择和 Flash 存储
 * @param state 指向管道状态链表头节点的指针
 * @note 根据 route_setting_choice 遍历链表定位目标节点；长按 KEY1（>1.5s）触发 Flash 存储；passage 路线使用 portion2_points_record，其他使用 record_waypoint
 */
void guandao_record(guandao_state * state)
{
    // ------------ 静态标志位初始化 ------------
    static uint8 flag0 = 1;                                                 // 首次调用标志，1=首次，0=已初始化，用于执行一次性初始化
    static uint8 flag1 = 1;                                                 // Flash存储标志，1=允许存储，0=已存储，防止重复写入
    static uint32 key1_save_start_ms = 0;
    static uint8 key1_save_wait_release = 0;
    uint32 now_ms = 0;
    int choice_flag = 0;                                                    // 路线选择计数器，用于遍历链表找到目标路线

    // ------------ 遍历链表找到目标路线节点 ------------
    guandao_state * p = state;                                          // 临时指针，指向当前路线节点，用于遍历链表
    while(choice_flag < route_setting_choice)               // 根据route_setting_choice的值，遍历链表选择目标路线
                                                                                        // route_setting_choice: 0=INS, 1=passage, 2=portion_3, 3=portion_2
    {
        p = p->next;                                                        // 指针后移，指向下一个路线节点
        if(p == NULL)return;                                            // 空指针保护，如果下一个为空则提前退出
        choice_flag++;                                                  // 空指针保护，如果下一个为空则提前退出
    }

    // ------------ 首次调用时初始化目标路线 ------------
    if(flag0){  guandao_state_init(p); daoche_point_length = 0; daoche_flash_cheack = 0;  flag0 =0;}          // 清空路线数据，倒车点相关变量归零

    // ------------ 更新里程计位置 ------------
    update_state(p  , &guandao_ecd);                        // 根据编码器数据更新当前车辆位姿（x, y, theta）

    // ------------ KEY1 长按检测和 Flash 存储逻辑 ------------
    if(gpio_get_level(KEY1))
    {
        key1_flag = 0;
        now_ms = system_getval_ms();
        if(key1_save_start_ms == 0) key1_save_start_ms = now_ms;
        if((uint32)(now_ms - key1_save_start_ms) > 1500 && flag1)
        {
            Flash_Store_Mode(route_setting_choice);
            Buzzer_check(50);
            flag1 = 0;
            key1_save_wait_release = 1;
        }
        return;
    }
    else
    {
        // ------------ 短按松手后记录倒车点 ------------
        if(key1_save_start_ms != 0 && key1_save_wait_release == 0)
        {
            key1_flag = 1;                 // 短按松开才记录倒车点
        }
        key1_save_start_ms = 0;

        // ------------ 长按松手等待释放期间不处理 ------------
        if(key1_save_wait_release)
        {
            key1_save_wait_release = 0;
            return;
        }
    }

    // ------------ 根据路线类型调用不同的记录函数 ------------
    if( p == &passage)portion2_points_record();     // passage路线使用手动记录，适合人工控制节奏
    else record_waypoint(p);                                         // 其他路线等距自动记录，移动超过阈值自动记录

//    guandao_show(p);                                                // 在IPS200屏幕上显示路线信息（长度、位姿等）

    // ------------ GPS辅助记录 ------------
    if(GPS_WORK_FLAG){if(key2_flag == 1){ key2_flag = 0 ; record_gps(p);  }}        // GPS工作标志有效且按键2按下时，记录GPS数据

    // ------------ 遥控/备用通道触发Flash存储 ------------
    if((x6f_out[2] == 200)&&flag1){   Flash_Store_Mode(route_setting_choice);  Buzzer_check(50);  flag1 = 0; };    // Flash存储备用触发：遥控通道2值为200时
    // flag1确保只存储一次，防止重复写入

}

/**
 * @brief 构建对称的第二段路径（8 字形重复路径拼接）
 * @retval 1 表示构建成功
 * @note 该函数将 passage 的第一段路径进行多段复制和镜像拼接，最终生成 8 段路径（PORTION_TWO_INDEX * 8 个点）
 */
uint8 portion2_points_build(void)
{
    // ------------ 多段路径复制和镜像拼接 ------------
    for( uint8 i = PORTION_TWO_INDEX*5 , j =PORTION_TWO_INDEX*0 ; i <PORTION_TWO_INDEX*5 +PORTION_TWO_INDEX; i++ , j++)
    {
        passage.record_map[i].x =  passage.record_map[j].x;
        passage.record_map[i].y =  passage.record_map[j].y;
    }
    for( uint8 i = PORTION_TWO_INDEX*6  , j =PORTION_TWO_INDEX*2 ; i <PORTION_TWO_INDEX*6 +PORTION_TWO_INDEX; i++ ,j++)
    {
        passage.record_map[i].x =  passage.record_map[j].x;
        passage.record_map[i].y =  passage.record_map[j].y;
    }
    for( uint8 i = PORTION_TWO_INDEX*7 , j =PORTION_TWO_INDEX*4; i <PORTION_TWO_INDEX*7 +PORTION_TWO_INDEX; i++ , j++)
    {
        passage.record_map[i].x =  passage.record_map[j].x;
        passage.record_map[i].y =  passage.record_map[j].y;
    }

    // ------------ 镜像复制（倒序） ------------
    for( uint8 i = PORTION_TWO_INDEX*0 , j =PORTION_TWO_INDEX*4-1; i <PORTION_TWO_INDEX*0 +PORTION_TWO_INDEX; i++ ,j--)
    {
        passage.record_map[i].x =  passage.record_map[j].x;
        passage.record_map[i].y =  passage.record_map[j].y;
    }
    for( uint8 i = PORTION_TWO_INDEX*3  , j =PORTION_TWO_INDEX*0 ; i <PORTION_TWO_INDEX*3 +PORTION_TWO_INDEX; i++ ,j++)
    {
        passage.record_map[i].x =  passage.record_map[j].x;
        passage.record_map[i].y =  passage.record_map[j].y;
    }
    for( uint8 i = PORTION_TWO_INDEX*1  , j =PORTION_TWO_INDEX*1; i <PORTION_TWO_INDEX*1 +PORTION_TWO_INDEX; i++,j++)
    {
        passage.record_map[i].x =  passage.record_map[j].x;
        passage.record_map[i].y =  passage.record_map[j].y;
    }

    // ------------ 线性插值填充过渡区域 ------------
    float index = (passage.record_map[PORTION_TWO_INDEX * 3].x - passage.record_map[PORTION_TWO_INDEX].x)/2.0f;
    for(uint8 i = 0 ,j = PORTION_TWO_INDEX *2 , m = PORTION_TWO_INDEX *4; i<PORTION_TWO_INDEX ; i++ ,j++ , m++)
    {
        passage.record_map[i].x =  passage.record_map[PORTION_TWO_INDEX].x - index;
        passage.record_map[i].y =  passage.record_map[PORTION_TWO_INDEX].y +record_threshold*i;
        passage.record_map[j].x =  passage.record_map[PORTION_TWO_INDEX].x + index;
        passage.record_map[j].y =  passage.record_map[PORTION_TWO_INDEX].y+record_threshold*i;
        passage.record_map[m].x =  passage.record_map[3*PORTION_TWO_INDEX].x + index;
        passage.record_map[m].y =  passage.record_map[PORTION_TWO_INDEX].y+record_threshold*i;
    }

    // ------------ 设置最终路径长度 ------------
    passage.length_index = PORTION_TWO_INDEX*8;
    return 1;

}

/**
 * @brief 语音/通道控制的第二段路径多段跟踪，使用状态机切换不同路径段
 * @param channal1 第一通道选择（控制第二段路径段）
 * @param channal2 第二通道选择（控制第五段路径段）
 * @param state 外部触发信号，用于状态机推进
 * @note 状态机有 4 个阶段：构建 initial 段 -> 追踪 INS -> 构建 return 段 -> 追踪 INS
 */
void portion2_points_trace(uint8 channal1 , uint8 channal2 ,uint8 state )
{
    // ------------ 状态机管理多段路径的构建和追踪 ------------
    static uint8 p2p_state = 0;
    switch(p2p_state)
    {
        case 0:
            // ------------ 构建 initial 段路径（三段拼接） ------------
            for(uint8 i  = 0 , j = PORTION_TWO_INDEX*5 ; i <PORTION_TWO_INDEX ; i ++ ,j++)
            {
                portion_2.record_map[i].x = passage.record_map[j].x;
                portion_2.record_map[i].y = passage.record_map[j].y;
            }
            for(uint8 i  = PORTION_TWO_INDEX*1 , j = PORTION_TWO_INDEX*channal1 ; i <PORTION_TWO_INDEX*1 +PORTION_TWO_INDEX ; i ++ ,j++)
            {
                portion_2.record_map[i].x = passage.record_map[j].x;
                portion_2.record_map[i].y = passage.record_map[j].y;
            }
            for(uint8 i  = PORTION_TWO_INDEX*2 , j = PORTION_TWO_INDEX*6 ; i <PORTION_TWO_INDEX*2 +PORTION_TWO_INDEX ; i ++ ,j++)
            {
                portion_2.record_map[i].x = passage.record_map[j].x;
                portion_2.record_map[i].y = passage.record_map[j].y;
            }
            portion_2.length_index = PORTION_TWO_INDEX*3;
            p2p_state++;
               break;
        case 1:
            // ------------ 追踪 INS ------------
            guandao_trace(&INS);
            if(state) p2p_state++;
            break;

        case 2:
            // ------------ 构建 return 段路径（倒序+补充） ------------
            for(uint8 i  = PORTION_TWO_INDEX*3 , j = PORTION_TWO_INDEX*(channal2+1)-1 ; i <PORTION_TWO_INDEX*3 +PORTION_TWO_INDEX ; i ++ ,j--)
            {
                portion_2.record_map[i].x = passage.record_map[j].x;
                portion_2.record_map[i].y = passage.record_map[j].y;
            }
            for(uint8 i  = PORTION_TWO_INDEX*4 , j = PORTION_TWO_INDEX*7 ; i <PORTION_TWO_INDEX*4 +PORTION_TWO_INDEX ; i ++ ,j++)
            {
                portion_2.record_map[i].x = passage.record_map[j].x;
                portion_2.record_map[i].y = passage.record_map[j].y;
            }
            portion_2.length_index = PORTION_TWO_INDEX*5;
            p2p_state++;
            break;
        case 3:
            // ------------ 再次追踪 INS ------------
            guandao_trace(&INS);
            break;

        default :break;
    }

}

/**
 * @brief 主路径跟踪函数，根据路线选择标志执行纯追踪控制及 GPS 轨迹跟踪
 * @param state 指向管道状态链表头节点的指针
 * @note 根据 route_setting_choice 遍历链表定位目标节点；调用 pursuit_control_mode 计算输出；若 GPS_WORK_FLAG 有效则同步 GPS 跟踪
 */
void guandao_trace(guandao_state * state)
{
//    static uint8 flag2 = 1;
    int choice_flag = 0;                           // 路线选择计数器，用于遍历链表找到目标路线

    // ------------ 遍历链表找到目标路线节点 ------------
    guandao_state * p = state;                // 临时指针，指向当前路线节点，用于遍历链表
    while(choice_flag < route_setting_choice)
    {
        p = p->next;                             // 指针后移，指向下一个路线节点
        if(p == NULL)return;              // 空指针保护，如果下一个为空则提前退出
        choice_flag++;
    }

    // ------------ 更新里程计位置 ------------
    update_state(p,&guandao_ecd); // 根据编码器数据更新当前车辆位姿（x, y, theta）

    // ------------ 纯追踪控制 ------------
    pursuit_control_mode(p ,&out_v_l ,&out_v_r ,&out_servo);

    // ------------ GPS 轨迹跟踪 ------------
    if(GPS_WORK_FLAG)trace_gps(p);                              // 激活GPS轨迹跟踪函数

    // ------------ 屏幕显示追踪状态 ------------
    follow_points_show(p);
//    follow_points_show();


}

/**
 * @brief 根据编码器计算实际车速
 * @param ecd 指向编码器结构体的指针，包含左右轮增量
 * @param time_tick 时间间隔（秒）
 * @retval 实际车速（float，m/s）
 * @note 取左右轮平均增量除以时间间隔；需配合外部定时器按固定周期调用
 */
float speed_calculate(Encoder_t * ecd , float time_tick)
{
    // ------------ 左右轮平均速度 = (ΔL + ΔR) * 每个tic距离 / 时间 / 2 ------------
    float v_speed = (ecd->delta_l +ecd->delta_r)*ONE_TICK_DISTANCE/time_tick/2.0f;
            return v_speed;
}

/**
 * @brief 轮子打滑检测，通过比较理论角速度与 IMU 实测角速度判断打滑方向
 * @param ecd 指向编码器结构体的指针
 * @param steer_angle 当前转向角（度）
 * @note 仅在车速 >= 1m/s 且角速度差值超过阈值时才判定为打滑；根据左右编码增量大小确定左滑/右滑
 */
void slip_cheak(Encoder_t * ecd,float steer_angle)
{
    // ------------ 计算理论角速度和实际角速度的差值 ------------
    static int flag = 0;

    float v_speed = 0 ;
    v_speed =speed_calculate(ecd , 0.007);
    float w = (v_speed * tanf(steer_angle/180.0f*M_PI)) / WHEEL_BASE;


    float slip_index = fabs(w - IMU_Data.gyro_z);

//    ips200_show_int(X(1),  Y(1) ,flag, 5);
//    ips200_show_float(X(1),  Y(2) ,v_speed,3, 2);
//    ips200_show_float(X(1),  Y(3) ,w,3, 2);
//    ips200_show_float(X(1),  Y(4) ,slip_index,3, 2);

    // ------------ 判断打滑方向和类型 ------------
    if(v_speed >= 1.0 &&  slip_index >= SLIP_CHEAK_INDEX)
    {
        flag++;

        if(ecd->delta_l > ecd->delta_r){ slip_state =Left_Slip; }
        else if(ecd->delta_r >=ecd->delta_l ) {slip_state =Right_Slip;}
    }
    else slip_state =NONE;
}

uint16 portion3_foint_flag = 0;

/**
 * @brief 对第三段路径进行翻转和镜像处理，生成回程轨迹
 * @retval 1 表示转换成功；0 表示路径长度超限
 * @note 函数会在路径末尾追加轴距长度的倒退点，然后将整条路径关于 x 轴翻转并倒序排列
 */
uint8 portion3_points_switch(void)
{
    // ------------ 计算并追加轴距长度的倒退点 ------------
    float x_delta = portion_3.record_map[portion_3.length_index -1 ].x ;
    int8 cut_length =(uint8)(WHEEL_BASE/record_threshold);
    if(cut_length < 1)cut_length = 1;

    portion3_foint_flag = cut_length;

    for(uint8 i = 1 ; i<=cut_length ; i++ )
    {
        portion_3.record_map[portion_3.length_index].x = portion_3.record_map[portion_3.length_index - 1].x ;
        portion_3.record_map[portion_3.length_index].y = portion_3.record_map[portion_3.length_index - 1].y - i*record_threshold;
        portion_3.length_index ++;

        if(portion_3.length_index >MAX_LENGTH_INDEX)return 0;
    }

    // ------------ 将 x 坐标关于终点 x 做镜像 ------------
    for(int i =cut_length ; i < portion_3.length_index ; i++)
    {
        portion_3.record_map[i].x -= x_delta;
    }

    // ------------ 倒序排列路径点（临时缓冲） ------------
    float * p = (float *)malloc(sizeof(portion_3.record_map[0].x)*portion_3.length_index*2);
    for( int i = portion_3.length_index -1 , j = 1 ; i >= cut_length ; i-- , j++)
    {
        p[2*j - 2] =  portion_3.record_map[i].x;
        p[2*j -1] =  portion_3.record_map[i].y;
    }
    portion_3.length_index -=cut_length;
    for(int i = 0  , j = 1; i < portion_3.length_index-1 ; i++ , j++)
    {
        portion_3.record_map[i].x = p[2*j - 2];
        portion_3.record_map[i].y = p[2*j -1];
    }
    free(p);

    return 1;
}

/**
 * @brief 在 IPS200 屏幕上绘制管道路线地图及进度条动画
 * @param e 指向管道状态链表头节点的指针
 * @note 根据 route_setting_choice 定位目标节点；自动缩放适配屏幕；同时绘制白色路线轨迹和绿色进度条
 */
void Guandao_Points_Show(guandao_state * e)
{
    // ------------ 遍历链表找到目标路线节点 ------------
    int choice_flag = 0;

    guandao_state * p = e;
    while(choice_flag < route_setting_choice)
    {
        p = p->next;
        if(p == NULL)return;
        choice_flag++;
    }
    if(p->length_index == 0)return;

    // ------------ 计算坐标边界 ------------
    float Max_R_Line = -10000.0f , Max_D_Line = -10000.0f , Min_L_Line = 10000.0f , Min_U_Line = 10000.0f;
    float Center_H = 0 ,  Center_W = 0,  INDEX_H = 0,  INDEX_W = 0 , INDEX_Progress = 0;
    uint16 GD_Show [2][p->length_index] ; uint16 Progress_Show [2][p->length_index] ;
    for( int i = 0 ; i< p->length_index ;i ++)
    {
        if(p->record_map[i].x < Min_L_Line)Min_L_Line =p->record_map[i].x;
        if(p->record_map[i].x > Max_R_Line)Max_R_Line =p->record_map[i].x;
        if(p->record_map[i].y < Min_U_Line)Min_U_Line =p->record_map[i].y;
        if(p->record_map[i].y > Max_D_Line)Max_D_Line =p->record_map[i].y;
    }

    // ------------ 计算地图中心和缩放比例 ------------
    Center_W = (Max_R_Line + Min_L_Line)/2.0f;
    Center_H = (Max_D_Line + Min_U_Line)/2.0f;
    INDEX_W = Max_R_Line - Min_L_Line;
    INDEX_H = Max_D_Line - Min_U_Line;

    // ------------ 将里程计坐标映射到屏幕像素 ------------
    for(  int i = 0 ; i< p->length_index ; i ++ )
    {
        GD_Show[0][i] = 110.0f + (p->record_map[i].x - Center_W)*(200.0f/INDEX_W);
        GD_Show[1][i] = 150.0f - (p->record_map[i].y - Center_H)*(280.0f/INDEX_H);

    }

    // ------------ 构建进度条坐标（沿屏幕边框一周） ------------
    INDEX_Progress = 1040.0f/p->length_index;
    for(int i = 0 ; i <p->length_index*300/1040 ; i++){Progress_Show [0][i] = 0;Progress_Show [1][i]= 300 - i*INDEX_Progress; }
    for(int i = p->length_index*300/1040 , j =0 ; i <p->length_index/2 ; i++ , j++){Progress_Show [0][i] = j*INDEX_Progress ;Progress_Show [1][i] = 0; }
    for(int i = p->length_index/2 , j =0 ; i <p->length_index*820/1040 ; i++ , j++){Progress_Show [0][i] = 220 ;Progress_Show [1][i] = j *INDEX_Progress; }
    for(int i = p->length_index*820/1040 , j =0 ; i <p->length_index ; i++ , j++){Progress_Show [0][i] = 220 - j*INDEX_Progress ;Progress_Show [1][i] = 300; }

    // ------------ 逐点绘制路线（白色）和进度条（绿色） ------------
    for(int i = 0 ; i< p->length_index - 1  ; i ++ )
    {
        ips200_draw_point(GD_Show[0][i],GD_Show[1][i],RGB565_WHITE);
        ips200_draw_line(GD_Show[0][i] ,GD_Show[1][i] ,GD_Show[0][i+1] ,GD_Show[1][i+1] , RGB565_WHITE);
        ips200_draw_line(Progress_Show[0][i] ,Progress_Show[1][i] ,Progress_Show[0][i+1] ,Progress_Show[1][i+1] , RGB565_GREEN);
        system_delay_ms(20);

    }



}

/**
 * @brief 在 IPS200 屏幕上显示记录的管道位置信息
 * @param p 指向管道状态结构体的指针
 * @note 显示内容：路径长度、当前角度、第一个记录点的 x/y 坐标、当前点的 x/y 坐标
 */
void guandao_show(guandao_state * p)
{
    // ------------ 在屏幕上显示管道状态信息 ------------
    ips200_show_int(X(1),  Y(0) ,p->length_index, 5);                                              ips200_show_float(X(10),Y(0),p->current_state.theta,3,2);
    ips200_show_float(X(1),  Y(3) ,p->record_map[INS.length_index-1].x, 5,2);     ips200_show_float(X(10),  Y(3) ,p->record_map[INS.length_index-1].y, 5,2);
    ips200_show_float(X(1),  Y(4) ,p->current_state.x, 5,2);                                     ips200_show_float(X(10),  Y(4) ,p->current_state.y, 5,2);


}

/**
 * @brief 在 IPS200 屏幕上显示追踪管道位置信息（当前点与目标点）
 * @param p 指向管道状态结构体的指针
 * @note 显示内容：目标点坐标、当前坐标、当前点索引、路径总长度、当前角度
 */
void follow_points_show(guandao_state * p)
{
    // ------------ 在屏幕上显示追踪状态信息 ------------
    ips200_show_float(X(10),Y(10),p->record_map[INS.current_point_index].x,3,2); ips200_show_float(X(15),Y(10),p->record_map[INS.current_point_index].y,3,2);
    ips200_show_float(X(10),Y(11),p->current_state.x,3,2);                                         ips200_show_float(X(15),Y(11),p->current_state.y,3,2);
    ips200_show_int(X(10),  Y(15) ,p->current_point_index, 5);                                   ips200_show_int(X(15),  Y(15) ,p->length_index, 5);
    ips200_show_float(X(15),Y(16),p->current_state.theta,3,2);
}

/**
 * @brief 手动按键在当前管道位置记录一个点
 * @param e 指向管道状态结构体的指针
 * @note 若已超过 MAX_LENGTH_INDEX 上限则不记录；将当前状态追加到 record_map 末尾
 */
void Key_Record_Point(guandao_state * e)
{
    // ------------ 越界检查 ------------
    if(e->length_index >MAX_LENGTH_INDEX) return;

    // ------------ 记录当前状态到路径末尾 ------------
    e->record_map[e->length_index] =e->current_state;
    e->length_index  ++;


}


//void guandao_mode_record(void)
//{
//    static uint8 flag0 = 1;
//    static uint8 flag1 = 1;
//    update_state(&INS,&guandao_ecd);
//    if(flag0){  guandao_state_init(&INS);   flag0 =0;}
//
//     record_waypoint(&INS);
////     guandao_show();
//
//
////    if(gpio_get_level(SWITCH1)&&flag1){   Flash_Write_mappoints();  Buzzer_check(50);  flag1 = 0; };
//
//}

//void guandao_mode_trace(void)
//{
//
////    static uint8 flag2 = 1;
////    while(flag2){Guandao_Points_Show();ips200_clear();flag2 =0;}  //轨迹显示
//
//     update_state(&INS,&guandao_ecd);
//     pursuit_control_mode(&INS ,&out_v_l ,&out_v_r ,&out_servo);
//     follow_points_show();
////        Steer_UpPID(&SteerUpPID ,out_servo);
//      Steer_PID(&SteerPID,out_servo);
//}
