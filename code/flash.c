/*
 * flash.c
 *
 *  Created on: 2025年11月23日
 *      Author: 18905
 */
#include "zf_common_headfile.h"

float speed_pid[6]={0};
int16 control[5];
float kp;
float ki;
float kd;


/**
 * @brief 从 Flash 读取 PID 参数和控制参数
 * @param 无
 * @note 读取失败（flash_check 返回 false）时保留默认值；读取成功后自动赋值到全局变量
 */
void Flash_Read_pid(void)
{
    // ------------ 检查 Flash 页有效性后读取 ------------
    if(flash_check(FLASH_SECTION_INDEX,SPEED_PID_PAGE_INDEX))
    {
        flash_buffer_clear();
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, SPEED_PID_PAGE_INDEX);

        // ------------ 读取6个 PID 速度参数 ------------
        for(uint8 i = 0  ; i<6 ; i++)
        {
            speed_pid[i] = flash_union_buffer[i].float_type;
        }

        // ------------ 赋值到左右电机 PID 结构体 ------------
        MoterPID_L.Kp = speed_pid[0];
        MoterPID_R.Kp = speed_pid[0];
        MoterPID_L.Ki = speed_pid[1];
        MoterPID_R.Ki = speed_pid[1];
        MoterPID_L.Kd = speed_pid[2];
        MoterPID_R.Kd = speed_pid[2];

        // ------------ 赋值阈值和减速参数 ------------
        record_threshold = speed_pid[3];
        persuit_threshold = speed_pid[4];
        final_dsts = speed_pid[5];

        // ------------ 读取控制参数数组 ------------
        for(uint8 i = 12  ,j =0; i<16 ; i++ ,j++)
        {
            control[j] = flash_union_buffer[i].int16_type;
        }

        // ------------ 赋值控制参数全局变量 ------------
        base_speed = (float)control[0];
        daoche_speed = (float)control[1];
        preview_spets = control[2];
    }




}

/**
 * @brief 将 PID 参数和控制参数写入 Flash
 * @param 无
 * @note 先同步全局变量到数组，再擦除旧页写入新页；写入失败不阻塞
 */
void Flash_Write_pid(void)
{
    // ------------ 清空缓冲区 ------------
    flash_buffer_clear();

    // ------------ 同步 PID 与阈值到结构体 ------------
    MoterPID_L.Kp = speed_pid[0];
    MoterPID_R.Kp = speed_pid[0];
    MoterPID_L.Ki = speed_pid[1];
    MoterPID_R.Ki = speed_pid[1];
    MoterPID_L.Kd = speed_pid[2];
    MoterPID_R.Kd = speed_pid[2];
    record_threshold = speed_pid[3];
    persuit_threshold = speed_pid[4];
    final_dsts = speed_pid[5];
    base_speed = (float)control[0];
    daoche_speed = (float)control[1];
    preview_spets = control[2];

    // ------------ 将 PID 参数填入缓冲区 ------------
    for(uint8 i = 0  ; i<6 ; i++)
    {
        flash_union_buffer[i].float_type = speed_pid[i];
    }

    // ------------ 将控制参数填入缓冲区 ------------
    for(uint8 i = 12 ,j=0 ; i<16 ; i++ ,j++)
    {
        flash_union_buffer[i].int16_type =control[j] ;
    }

    // ------------ 擦除旧页并写入新数据 ------------
      if(flash_check(FLASH_SECTION_INDEX,SPEED_PID_PAGE_INDEX))
      {
          flash_erase_page(FLASH_SECTION_INDEX,SPEED_PID_PAGE_INDEX) ;
      }
      flash_write_page_from_buffer(FLASH_SECTION_INDEX,SPEED_PID_PAGE_INDEX);

}

/**
 * @brief 根据 route_choice 存储对应路线数据
 * @param route_choice 路线类型：0=INS, 1=passage, 2=portion_3
 * @note 记录完成后自动写入 Flash；GPS 工作时额外写入 GPS 校验标志
 */
void Flash_Store_Mode(uint8 route_choice)
{
    // ------------ 根据路线类型记录并存储 ------------
    switch(route_choice)
    {
        case 0:
            Key_Record_Point(&INS); Flash_Write_INSpoints();
            break;
        case 1:
            if(portion2_points_build()){ Key_Record_Point(&passage); Flash_Write_passage_points();}
            break;
        case 2:
            Key_Record_Point(&portion_3);
            if(portion3_points_switch()){  Flash_Write_portion_3points();}
            break;
        default :break;
        // ------------ GPS 工作时额外写入校验标志 ------------
        if(GPS_WORK_FLAG)Flash_Write_gpscheak();
    }


}

/**
 * @brief 开机时从 Flash 读取所有存储数据
 * @param 无
 * @note 依次读取 PID 参数、三种路径点和 GPS 校验标志
 */
void Flash_Main_Read(void)
{
    // ------------ 读取所有 Flash 存储数据 ------------
    Flash_Read_pid();
    Flash_Read_INSpoints();
    Flash_Read_passage_points();
    Flash_Read_portion_3points();
    Flash_Read_gpscheak();


}

/**
 * @brief 将 passage 路径点写入 Flash
 * @param 无
 * @note 最大存储 1020 个单元；先写长度再交错写入 x/y 坐标
 */
void Flash_Write_passage_points(void)
{
    // ------------ 计算存储上限并限制 ------------
    int max_storage = 2 * passage.length_index +2 ;
    if (max_storage >=1020) max_storage = 1020;
    flash_buffer_clear();

    // ------------ 写入长度索引 ------------
    flash_union_buffer[0].int16_type = passage.length_index;

    // ------------ 交错写入 x 坐标 ------------
    for(int i = 2 , j = 0;i < max_storage ; i += 2 , j++)
    {
        flash_union_buffer[i].float_type = passage.record_map[j].x;
    }

    // ------------ 交错写入 y 坐标 ------------
    for(int i = 3 , j = 0;i < max_storage ; i += 2 , j++)
    {
        flash_union_buffer[i].float_type = passage.record_map[j].y;
    }


    // ------------ 擦除旧页并写入 ------------
    if(flash_check(FLASH_SECTION_INDEX,RECORD_PASSAGE))
    {
        flash_erase_page(FLASH_SECTION_INDEX,RECORD_PASSAGE) ;
    }
    flash_write_page_from_buffer(FLASH_SECTION_INDEX,RECORD_PASSAGE);
}

/**
 * @brief 从 Flash 读取 passage 路径点
 * @param 无
 * @note 读取失败时函数静默返回；读取后长度和坐标自动恢复到 passage 结构体
 */
void Flash_Read_passage_points(void)
{
    int get_max_storage = 0;
    // ------------ 检查并读取 Flash 页 ------------
    if(flash_check(FLASH_SECTION_INDEX,RECORD_PASSAGE))
    {
        flash_buffer_clear();
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, RECORD_PASSAGE);

        // ------------ 读取长度索引 ------------
        passage.length_index = flash_union_buffer[0].int16_type;

        // ------------ 计算并读取 x 坐标 ------------
        get_max_storage =2 * passage.length_index +2;
        for(int i = 2 , j = 0;i < get_max_storage ; i += 2 , j++)
        {
            passage.record_map[j].x = flash_union_buffer[i].float_type;
        }

        // ------------ 读取 y 坐标 ------------
        for(int i = 3 , j = 0;i < get_max_storage ; i += 2 , j++)
        {
            passage.record_map[j].y = flash_union_buffer[i].float_type;
        }
    }
}

/**
 * @brief 将 portion_3 路径点写入 Flash
 * @param 无
 * @note 最大存储 1020 个单元；格式与 passage 写入一致
 */
void Flash_Write_portion_3points(void)
{
    // ------------ 计算存储上限 ------------
    int max_storage = 2 * portion_3.length_index +2 ;
    if (max_storage >=1020) max_storage = 1020;
    flash_buffer_clear();

    // ------------ 写入长度索引 ------------
    flash_union_buffer[0].int16_type = portion_3.length_index;

    // ------------ 交错写入 x 坐标 ------------
    for(int i = 2 , j = 0;i < max_storage ; i += 2 , j++)
    {
        flash_union_buffer[i].float_type = portion_3.record_map[j].x;
    }

    // ------------ 交错写入 y 坐标 ------------
    for(int i = 3 , j = 0;i < max_storage ; i += 2 , j++)
    {
        flash_union_buffer[i].float_type = portion_3.record_map[j].y;
    }


    // ------------ 擦除旧页并写入 ------------
    if(flash_check(FLASH_SECTION_INDEX,RECORD_PORTION_THREE))
    {
        flash_erase_page(FLASH_SECTION_INDEX,RECORD_PORTION_THREE) ;
    }
    flash_write_page_from_buffer(FLASH_SECTION_INDEX,RECORD_PORTION_THREE);

}

/**
 * @brief 从 Flash 读取 portion_3 路径点
 * @param 无
 * @note 读取后 length_index 减 1（协议差异修正）；坐标自动恢复到 portion_3 结构体
 */
void Flash_Read_portion_3points(void)
{
    int get_max_storage = 0;
    // ------------ 检查并读取 Flash 页 ------------
    if(flash_check(FLASH_SECTION_INDEX,RECORD_PORTION_THREE))
    {
        flash_buffer_clear();
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, RECORD_PORTION_THREE);

        // ------------ 读取长度索引（减1修正） ------------
        portion_3.length_index = flash_union_buffer[0].int16_type -1 ;

        // ------------ 读取 x 坐标 ------------
        get_max_storage =2 * portion_3.length_index +2;
        for(int i = 2 , j = 0;i < get_max_storage ; i += 2 , j++)
        {
            portion_3.record_map[j].x = flash_union_buffer[i].float_type;
        }

        // ------------ 读取 y 坐标 ------------
        for(int i = 3 , j = 0;i < get_max_storage ; i += 2 , j++)
        {
            portion_3.record_map[j].y = flash_union_buffer[i].float_type;
        }
    }
}

/**
 * @brief 将 INS 路径点（含 GPS 数据）写入 Flash
 * @param 无
 * @note 先写入路径 x/y 坐标，再追加 GPS 经纬度数据；存储上限受 MAX_LENGTH_INDEX 和 MAX_GPS_RECORD 约束
 */
void Flash_Write_INSpoints(void)
{
    // ------------ 计算存储上限 ------------
    int max_storage = 2 * INS.length_index +2 ;
    if (max_storage >=MAX_LENGTH_INDEX*2) max_storage = MAX_LENGTH_INDEX*2;
    int gps_max_storage = max_storage +INS.gps_record_length*2 + 2 ;
    if(gps_max_storage >= max_storage +MAX_GPS_RECORD*2 +2)gps_max_storage = max_storage +MAX_GPS_RECORD*2+2;
    flash_buffer_clear();

    // ------------ 写入长度索引（含倒车点长度） ------------
    flash_union_buffer[0].int16_type = INS.length_index;
    if(daoche_flash_cheack)flash_union_buffer[1].int16_type = daoche_point_length;
    else{flash_union_buffer[1].int16_type = INS.length_index;}

    // ------------ 交错写入 x 坐标 ------------
    for(int i = 2 , j = 0;i < max_storage ; i += 2 , j++)
    {
        flash_union_buffer[i].float_type = INS.record_map[j].x;
    }

    // ------------ 交错写入 y 坐标 ------------
    for(int i = 3 , j = 0;i < max_storage ; i += 2 , j++)
    {
        flash_union_buffer[i].float_type = INS.record_map[j].y;
    }

    // ------------ 写入 GPS 记录长度和经纬度 ------------
    flash_union_buffer[max_storage].int16_type = INS.gps_record_length;
    for(int i = max_storage+2 , j = 0; i < gps_max_storage ; i+=2 , j++)
    {
        flash_union_buffer[i].int32_type = double_to_int32(INS.record_gpsmap[j].lat);
    }
    for(int i = max_storage+3 , j = 0; i < gps_max_storage ; i+=2 , j++)
    {
        flash_union_buffer[i].int32_type = double_to_int32(INS.record_gpsmap[j].lon);
    }


    // ------------ 擦除旧页并写入 ------------
    if(flash_check(FLASH_SECTION_INDEX,RECORD_MAP_POINTS_INDEX))
    {
        flash_erase_page(FLASH_SECTION_INDEX,RECORD_MAP_POINTS_INDEX) ;
    }
    flash_write_page_from_buffer(FLASH_SECTION_INDEX,RECORD_MAP_POINTS_INDEX);

}

/**
 * @brief 从 Flash 读取 INS 路径点（含 GPS 数据）
 * @param 无
 * @note GPS 记录长度超过 MAX_GPS_RECORD 时自动截断；经纬度通过 int32/double 转换恢复
 */
void Flash_Read_INSpoints(void)
{
    int get_max_storage = 0;
    // ------------ 检查并读取 Flash 页 ------------
    if(flash_check(FLASH_SECTION_INDEX,RECORD_MAP_POINTS_INDEX))
    {
        flash_buffer_clear();
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, RECORD_MAP_POINTS_INDEX);

        // ------------ 读取长度和倒车点长度 ------------
        INS.length_index = flash_union_buffer[0].int16_type;
        daoche_point_length = flash_union_buffer[1].int16_type ;

        // ------------ 读取 x 坐标 ------------
        get_max_storage =2 * INS.length_index +2;
        for(int i = 2 , j = 0;i < get_max_storage ; i += 2 , j++)
        {
            INS.record_map[j].x = flash_union_buffer[i].float_type;
        }

        // ------------ 读取 y 坐标 ------------
        for(int i = 3 , j = 0;i < get_max_storage ; i += 2 , j++)
        {
            INS.record_map[j].y = flash_union_buffer[i].float_type;
        }

        // ------------ 读取 GPS 记录并限幅 ------------
        INS.gps_record_length = flash_union_buffer[get_max_storage].int16_type;
        if(INS.gps_record_length >MAX_GPS_RECORD)INS.gps_record_length = MAX_GPS_RECORD;

        // ------------ 读取 GPS 经纬度 ------------
        int gps_max_storage = get_max_storage +INS.gps_record_length*2 + 2 ;
        for(int i = get_max_storage+2 , j = 0; i < gps_max_storage ; i+=2 , j++)
        {
            INS.record_gpsmap[j].lat = int32_to_double(flash_union_buffer[i].int32_type);
        }
        for(int i = get_max_storage+3 , j = 0; i < gps_max_storage ; i+=2 , j++)
        {
            INS.record_gpsmap[j].lon = int32_to_double(flash_union_buffer[i].int32_type);
        }
    }

}

/**
 * @brief 从 Flash 读取 GPS 校验标志
 * @param 无
 * @note 读取 cheak_flag 和 theta 角度信息；页不存在时静默返回
 */
void Flash_Read_gpscheak(void)
{
    // ------------ 检查并读取 Flash 页 ------------
    if(flash_check(FLASH_SECTION_INDEX,GPS_CHEAK_FLAG))
    {
        flash_buffer_clear();
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, GPS_CHEAK_FLAG);

        // ------------ 读取校验标志 ------------
        for(int i = 0 ; i < MAX_GPS_RECORD ; i++)
        {
            INS.record_gpsmap[i].cheak_flag = flash_union_buffer[i].int16_type;
        }

        // ------------ 读取 theta 角度 ------------
        for(int i = MAX_GPS_RECORD , j = 1; i < MAX_GPS_RECORD*3/2 ; i++ , j+=2)
        {
            INS.record_gpsmap[j].theta =flash_union_buffer[i].float_type ;
        }

    }

}

/**
 * @brief 将 GPS 校验标志写入 Flash
 * @param 无
 * @note 写入 cheak_flag 和 theta 角度信息；先擦除旧页再写入
 */
void Flash_Write_gpscheak(void)
{
    // ------------ 清空缓冲区 ------------
    flash_buffer_clear();

    // ------------ 写入校验标志 ------------
    for(int i = 0 ; i < MAX_GPS_RECORD ; i++)
    {
        flash_union_buffer[i].int16_type = INS.record_gpsmap[i].cheak_flag;
    }

    // ------------ 写入 theta 角度 ------------
    for(int i = MAX_GPS_RECORD , j = 1; i < MAX_GPS_RECORD*3/2 ; i++ , j+=2)
    {
        flash_union_buffer[i].float_type = INS.record_gpsmap[j].theta;
    }


    // ------------ 擦除旧页并写入 ------------
    if(flash_check(FLASH_SECTION_INDEX,GPS_CHEAK_FLAG))
    {
        flash_erase_page(FLASH_SECTION_INDEX,GPS_CHEAK_FLAG) ;
    }
    flash_write_page_from_buffer(FLASH_SECTION_INDEX,GPS_CHEAK_FLAG);

}
