/*
 * flash.h
 *
 *  Created on: 2025年11月23日
 *      Author: 18905
 */

#ifndef CODE_FLASH_H_
#define CODE_FLASH_H_

//外部变量

extern float speed_pid[6];
extern int16 control[5];
extern float kp ;
extern float ki ;
extern float kd ;

//宏定义
#define FLASH_SECTION_INDEX             (0)    //存储使用的扇区号
#define SPEED_PID_PAGE_INDEX            (11)   //参数页
#define RECORD_MAP_POINTS_INDEX   (10)   //记录地图点位
#define RECORD_PASSAGE                  (9)   //记录地图点位
#define RECORD_PASSAGE_TWO                  (8)   //记录地图点位
#define RECORD_PASSAGE_THREE                  (7)   //记录地图点位
#define RECORD_PASSAGE_FOUR                  (6)   //记录地图点位
#define RECORD_PASSAGE_FIF                  (5)   //记录地图点位
#define RECORD_PORTION_THREE                  (4)   //记录地图点位
#define GPS_CHEAK_FLAG                            (3)   //记录地图点位
//函数

/**
 * @brief 从 Flash 读取 GPS 校验标志位
 */
void Flash_Read_gpscheak(void);

/**
 * @brief 将 GPS 校验标志位写入 Flash
 */
void Flash_Write_gpscheak(void);

/**
 * @brief 从 Flash 读取 PID 参数和控制参数
 */
void Flash_Read_pid(void);

/**
 * @brief 将 PID 参数和控制参数写入 Flash
 */
void Flash_Write_pid(void);

/**
 * @brief 将 INS 路径点（含 GPS 数据）写入 Flash
 */
void Flash_Write_INSpoints(void);

/**
 * @brief 从 Flash 读取 INS 路径点（含 GPS 数据）
 */
void Flash_Read_INSpoints(void);

/**
 * @brief 将 passage 路径点写入 Flash
 */
void Flash_Write_passage_points(void);

/**
 * @brief 从 Flash 读取 passage 路径点
 */
void Flash_Read_passage_points(void);

/**
 * @brief 将 portion_3 路径点写入 Flash
 */
void Flash_Write_portion_3points(void);

/**
 * @brief 从 Flash 读取 portion_3 路径点
 */
void Flash_Read_portion_3points(void);

/**
 * @brief 开机时从 Flash 读取所有存储数据
 */
void Flash_Main_Read(void);

/**
 * @brief 根据 route_choice 存储对应路线数据
 */
void Flash_Store_Mode(uint8 route_setting_choice);

#endif /* CODE_FLASH_H_ */
