/*
 * gps.c
 *
 *  Created on: 2026年4月23日
 *      Author: 18905
 */

#include "zf_common_headfile.h"

GPS_work gps_work;
Lost_Point lost_judge;

/**
 * @brief 将角度归一化到 [-180, 180] 范围内
 * @param angle 指向待归一化角度的指针，函数直接修改其值
 * @note 调用前确保传入的指针非空；此函数会直接修改原值而非返回新值
 */
void angle_plan(float * angle)
{
    if(* angle >= 180){* angle -= 360 ;}
    else if(* angle < -180){* angle += 360 ;}

}

/**
 * @brief 交替记录 GPS 航点数据到指定的管道状态结构中
 * @param state 指向管道状态结构体的指针，航点数据将写入其 record_gpsmap 数组
 * @note 使用静态标志位 rcd_gps_flag 在两种记录模式间切换；偶数次记录纬度/经度/偏航角，奇数次记录纬度/经度/偏航角差值
 */
void record_gps(guandao_state * state)
{
    // ------------ 通过静态标志位交替记录两种 GPS 航点 ------------
    static uint8 rcd_gps_flag = 0 ;
    switch(rcd_gps_flag)
    {
        case 0 :
            state->record_gpsmap[state->gps_record_length].lat = gnss.latitude;
            state->record_gpsmap[state->gps_record_length].lon = gnss.longitude;
            state->record_gpsmap[state->gps_record_length].theta = Yaw_1;
            state->record_gpsmap[state->gps_record_length].cheak_flag = state->length_index;
            state->gps_record_length ++;
            rcd_gps_flag =1;
            break;
        case 1 :
            state->record_gpsmap[state->gps_record_length].lat = gnss.latitude;
            state->record_gpsmap[state->gps_record_length].lon = gnss.longitude;
            state->record_gpsmap[state->gps_record_length].theta =fabs(state->record_gpsmap[state->gps_record_length - 1].theta - Yaw_1) ;
            state->record_gpsmap[state->gps_record_length].cheak_flag = state->length_index;
            state->gps_record_length ++;
            rcd_gps_flag =0;
            break;
    }

}

/**
 * @brief 检查 GPS 目标点是否到达并切换到下一个目标点
 * @retval 1 表示奇数索引点切换成功；0 表示未切换或偶数索引点刚切换
 * @note 偶数索引点切换后不返回 1，需等待车辆转过目标角度的一半后 flag 复位才允许奇数点切换
 */
uint8 switch_gps(void)
{
    // ------------ 检查偶数索引到达条件：距离小于阈值时切换 ------------
    static float loop_theta = 0;
    static uint8 loop_theta_flag = 1;
    if(gps_work.distance_gps <= GPS_SWITCH_DISTANCE && gps_work.gps_current_point%2 ==0)
    {
        loop_theta = Yaw_1;
        loop_theta_flag = 0;
        gps_work.gps_current_point++;
        gps_work.azimuth_gps = get_two_points_azimuth(gnss.latitude,   gnss.longitude,  gps_work.points[gps_work.gps_current_point  ].lat , gps_work.points[gps_work.gps_current_point  ].lon);
        angle_plan(&gps_work.azimuth_gps);
        gps_work.angle_gps = gps_work.azimuth_gps - Yaw_1;
        angle_plan(&gps_work.angle_gps);
        gps_work.distance_gps =   get_two_points_distance(gnss.latitude,   gnss.longitude,  gps_work.points[gps_work.gps_current_point  ].lat , gps_work.points[gps_work.gps_current_point  ].lon);

    }

    // ------------ 检查奇数索引到达条件：需 flag 已复位 ------------
    if(gps_work.distance_gps <= GPS_SWITCH_DISTANCE && gps_work.gps_current_point%2 !=0 &&loop_theta_flag == 1)
    {
        gps_work.gps_current_point++;
        gps_work.azimuth_gps = get_two_points_azimuth(gnss.latitude,   gnss.longitude,  gps_work.points[gps_work.gps_current_point  ].lat , gps_work.points[gps_work.gps_current_point  ].lon);
        angle_plan(&gps_work.azimuth_gps);
        gps_work.angle_gps = gps_work.azimuth_gps - Yaw_1;
        angle_plan(&gps_work.angle_gps);
        gps_work.distance_gps =   get_two_points_distance(gnss.latitude,   gnss.longitude,  gps_work.points[gps_work.gps_current_point  ].lat , gps_work.points[gps_work.gps_current_point  ].lon);
        return 1;
    }

    // ------------ 车辆转过目标角度一半后复位 flag，允许下一次奇数点切换 ------------
    if(fabs(Yaw_1 - loop_theta) == gps_work.points[gps_work.gps_current_point].theta/2.0f && loop_theta_flag == 0)
    {
        loop_theta_flag = 1;
    }
    return 0 ;

}

/**
 * @brief GPS 轨迹跟踪，包含丢点与找回点的检测逻辑
 * @param e 指向管道状态结构体的指针，用于校验 cheak_flag 判断丢点/找回
 * @note 该函数依赖 gps_work.gps_update_flag 作为触发信号，调用前需确保 update_gpsinformation 已置位此标志
 */
void trace_gps(guandao_state * e)
{
    // ------------ 检查 GPS 更新标志，执行目标点切换 ------------
    if( gps_work.gps_update_flag == 1)
    {
        gps_work.gps_update_flag = 0;
        if(switch_gps())
        {
            // ------------ 根据 cheak_flag 判断丢点或找回 ------------
            if(e->record_gpsmap[gps_work.gps_current_point - 1].cheak_flag == e->current_point_index)  //e->record_gpsmap[gps_work.gps_current_point - 1].cheak_flag == e->current_point_index + 1
            {
                lost_judge = FIND;
            }
            else
            {
                lost_judge = LOST;
            }

            // ------------ 大角度偏差时跳过当前点，标记为丢失 ------------
            if(fabs(gps_work.angle_gps) >=90)
            {
                if(gps_work.gps_current_point%2 !=0)
                {
                    lost_judge = LOST;
                }
                gps_work.gps_current_point++;
            }
        }
    }

}

/**
 * @brief 更新当前 GPS 目标点的方位角、偏航角差和距离信息
 * @note 调用前需确保 gps_work.gps_current_point 未越界；函数会置位 gps_update_flag 供 trace_gps 使用
 */
void update_gpsinformation(void)
{
    // ------------ 越界保护：当前点索引超过路线长度则返回 ------------
    if(gps_work.gps_current_point >= gps_work.work_gps_length)return;

    // ------------ 计算到当前目标点的方位角和距离 ------------
    gps_work.azimuth_gps = get_two_points_azimuth(gnss.latitude,   gnss.longitude,  gps_work.points[gps_work.gps_current_point  ].lat , gps_work.points[gps_work.gps_current_point  ].lon);
    angle_plan(&gps_work.azimuth_gps);
    gps_work.angle_gps = gps_work.azimuth_gps - Yaw_1;
    angle_plan(&gps_work.angle_gps);
    gps_work.distance_gps =   get_two_points_distance(gnss.latitude,   gnss.longitude,  gps_work.points[gps_work.gps_current_point  ].lat , gps_work.points[gps_work.gps_current_point  ].lon);

    // ------------ 置位更新标志 ------------
    gps_work.gps_update_flag = 1;
}

/**
 * @brief 将双精度浮点数转换为固定点 int32 格式（乘以 1e7）
 * @param y 待转换的双精度浮点数
 * @retval 转换后的 int32 固定点数值（x1e7）
 * @note 注意 double 转 int32 的精度损失；超出 int32 范围的值会产生截断
 */
int32 double_to_int32(double y)
{
    // ------------ double 乘以 1e7 后强制转换为 int32 ------------
    int32 x = 0;
    x = (int32)(y*10000000);
    return x;
}

/**
 * @brief 将固定点 int32 格式转换回双精度浮点数（除以 1e7）
 * @param y 待转换的 int32 固定点数值
 * @retval 还原后的双精度浮点数
 * @note 这是 double_to_int32 的逆操作，用于从存储格式恢复原始坐标值
 */
double int32_to_double(int32 y)
{
    // ------------ int32 除以 1e7 恢复为 double ------------
    double x = 0;
    x = (double)y * 1.0f / 10000000;
    return x;
}

/**
 * @brief 将链表中的 GPS 路线数据复制到 GPS 工作缓冲区中
 * @param e 指向管道状态链表头节点的指针，根据 route_setting_choice 定位目标节点
 * @note 如果选中路径的 GPS 记录长度为奇数，会触发蜂鸣器告警（500ms）；复制时会初始化所有点的 theta 为零
 */
void GPS_WorkMap_Copy(guandao_state * e)
{
    // ------------ 遍历链表找到目标节点 ------------
    int choice_flag = 0;
    guandao_state * p = e;

    while(choice_flag < route_setting_choice && p != NULL)
    {
        p = p->next;
        choice_flag++;
    }

    // ------------ 奇数长度告警 ------------
    if(p->gps_record_length%2 != 0)Buzzer_check(500);

    // ------------ 复制 GPS 路线数据到工作缓冲区 ------------
    gps_work.work_gps_length = p->gps_record_length;
    for( int i = 0 ; i <p->gps_record_length ; i++)gps_work.points[i].theta= 0.0f;
    for( int i = 0 ; i <p->gps_record_length/2 ; i+=2)gps_work.points[i].theta= p->record_gpsmap[i].theta;

    for( int i = 0 ; i <p->gps_record_length ; i++)
    {
        gps_work.points[i].lat = p->record_gpsmap[i].lat;
        gps_work.points[i].lon = p->record_gpsmap[i].lon;
        gps_work.points[i].gps_cheak_flag = p->record_gpsmap[i].cheak_flag;
    }


}

/**
 * @brief 在 IPS200 屏幕上绘制 GPS 路线点之间的连线
 * @param e 指向管道状态结构体的指针，包含待显示的 GPS 路线数据
 * @note 屏幕绘制尺寸固定（200x280），坐标超出此范围的点将被裁剪；每两点之间延迟 20ms
 */
void GPS_Points_Show(guandao_state * e)
{
    // ------------ 计算 GPS 坐标的地理边界（经纬度最大最小值） ------------
    double Max_R_Line = -10000.0f , Max_D_Line = -10000.0f , Min_L_Line = 10000.0f , Min_U_Line = 10000.0f;
    double Center_H = 0 ,  Center_W = 0,  INDEX_H = 0,  INDEX_W = 0 ;
    uint16 GD_Show [2][e->gps_record_length] ;
    if(e->gps_record_length ==0)return;
    for( int i = 0 ; i< e->gps_record_length ;i ++)
    {
        if(e ->record_gpsmap[i].lat < Min_L_Line)Min_L_Line =e ->record_gpsmap[i].lat  ;
        if(e ->record_gpsmap[i].lat   > Max_R_Line)Max_R_Line =e ->record_gpsmap[i].lat  ;
        if(e->record_gpsmap[i].lon < Min_U_Line)Min_U_Line =e->record_gpsmap[i].lon;
        if(e->record_gpsmap[i].lon > Max_D_Line)Max_D_Line =e->record_gpsmap[i].lon;
    }

    // ------------ 计算地图中心和缩放比例 ------------
    Center_W = (Max_R_Line + Min_L_Line)/2.0f;
    Center_H = (Max_D_Line + Min_U_Line)/2.0f;
    INDEX_W = (Max_R_Line - Min_L_Line);
    INDEX_H = (Max_D_Line - Min_U_Line);

    // ------------ 将地理坐标映射到屏幕像素坐标 ------------
    for(  int i = 0 ; i< e->gps_record_length ; i ++ )
    {
        GD_Show[0][i] = 110.0f + (e ->record_gpsmap[i].lat  - Center_W)*(200.0f/INDEX_W);
        GD_Show[1][i] = 150.0f - (e->record_gpsmap[i].lon - Center_H)*(280.0f/INDEX_H);

    }

    // ------------ 逐点绘制路线点和连线 ------------
    for(int i = 0 ; i< e->gps_record_length - 1  ; i ++ )
    {
        ips200_draw_point((uint16)GD_Show[0][i],(uint16)GD_Show[1][i],RGB565_WHITE);
        ips200_draw_line((uint16)GD_Show[0][i] ,(uint16)GD_Show[1][i] ,(uint16)GD_Show[0][i+1] ,(uint16)GD_Show[1][i+1] , RGB565_WHITE);
        system_delay_ms(20);
    }

}

/**
 * @brief 在 IPS200 屏幕上显示当前工作的 GPS 路线
 * @note 与 GPS_Points_Show 类似，但数据来源于全局 gps_work 缓冲区；每两点之间延迟 50ms
 */
void GPS_Work_SHOW(void)
{
    // ------------ 计算 GPS 坐标的地理边界（经纬度最大最小值） ------------
    double Max_R_Line = -10000.0f , Max_D_Line = -10000.0f , Min_L_Line = 10000.0f , Min_U_Line = 10000.0f;
    double Center_H = 0 ,  Center_W = 0,  INDEX_H = 0,  INDEX_W = 0 ;
    uint16 GD_Show [2][gps_work.work_gps_length] ;
    if(gps_work.work_gps_length ==0)return;
    for( int i = 0 ; i< gps_work.work_gps_length ;i ++)
    {
        if( gps_work.points[i].lat< Min_L_Line)Min_L_Line =gps_work.points[i].lat  ;
        if(gps_work.points[i].lat   > Max_R_Line)Max_R_Line =gps_work.points[i].lat  ;
        if(gps_work.points[i].lon < Min_U_Line)Min_U_Line =gps_work.points[i].lon;
        if(gps_work.points[i].lon > Max_D_Line)Max_D_Line =gps_work.points[i].lon;
    }

    // ------------ 计算地图中心和缩放比例 ------------
    Center_W = (Max_R_Line + Min_L_Line)/2.0f;
    Center_H = (Max_D_Line + Min_U_Line)/2.0f;
    INDEX_W = (Max_R_Line - Min_L_Line);
    INDEX_H = (Max_D_Line - Min_U_Line);

    // ------------ 将地理坐标映射到屏幕像素坐标 ------------
    for(  int i = 0 ; i< gps_work.work_gps_length ; i ++ )
    {
        GD_Show[0][i] = 110.0f + (gps_work.points[i].lat  - Center_W)*(200.0f/INDEX_W);
        GD_Show[1][i] = 150.0f - (gps_work.points[i].lon - Center_H)*(280.0f/INDEX_H);

    }

    // ------------ 逐点绘制路线点和连线 ------------
    for(int i = 0 ; i<gps_work.work_gps_length - 1  ; i ++ )
    {
        ips200_draw_point((uint16)GD_Show[0][i],(uint16)GD_Show[1][i],RGB565_WHITE);
        ips200_draw_line((uint16)GD_Show[0][i] ,(uint16)GD_Show[1][i] ,(uint16)GD_Show[0][i+1] ,(uint16)GD_Show[1][i+1] , RGB565_WHITE);
        system_delay_ms(50);
    }


}
