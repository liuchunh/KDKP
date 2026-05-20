/*
 * gps.h
 *
 *  Created on: 2026年4月23日
 *      Author: 18905
 */

#ifndef CODE_GPS_H_
#define CODE_GPS_H_

// 宏定义

#define GPS_WORK_NUM                            30
#define GPS_WORK_FLAG                            1
#define GPS_SWITCH_DISTANCE                  2.0f
//外部变量

typedef struct
{
        double lat;
        double lon;
        float theta;
        int16 gps_cheak_flag;

}GPS_points;

typedef struct
{
        GPS_points points[GPS_WORK_NUM];
        float angle_gps;
        float azimuth_gps;
        double distance_gps ;

        uint8 gps_update_flag;
        int16 work_gps_length;
        uint16 gps_current_point;

}GPS_work;

typedef enum{
    FIND,
    LOST,
}Lost_Point;
//函数声明
void gps_work_init(void);

/**
 * @brief 将角度归一化到 [-180, 180] 范围内
 */
void angle_plan(float * angle);

/**
 * @brief 交替记录 GPS 航点数据到指定的管道状态结构中
 */
void record_gps(guandao_state * state);

/**
 * @brief 检查 GPS 目标点是否到达并切换到下一个目标点
 */
uint8 switch_gps(void);

/**
 * @brief GPS 轨迹跟踪，包含丢点与找回点的检测逻辑
 */
void trace_gps(guandao_state * e);

/**
 * @brief 更新当前 GPS 目标点的方位角、偏航角差和距离信息
 */
void update_gpsinformation(void);

/**
 * @brief 将双精度浮点数转换为固定点 int32 格式（乘以 1e7）
 */
int32 double_to_int32(double y);

/**
 * @brief 将固定点 int32 格式转换回双精度浮点数（除以 1e7）
 */
int32_to_double(int32 y);

/**
 * @brief 将链表中的 GPS 路线数据复制到 GPS 工作缓冲区中
 */
void GPS_WorkMap_Copy(guandao_state * e);

/**
 * @brief 在 IPS200 屏幕上绘制 GPS 路线点之间的连线
 */
void GPS_Points_Show(guandao_state * e);

/**
 * @brief 在 IPS200 屏幕上显示当前工作的 GPS 路线
 */
void GPS_Work_SHOW(void);
#endif /* CODE_GPS_H_ */
