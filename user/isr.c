/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          isr
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.10.2
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding            first version
********************************************************************************************************************/

#include "isr_config.h"
#include "isr.h"
#include "rear_motor/rear_motor.h"

// 对于TC系列默认是不支持中断嵌套的，希望支持中断嵌套需要在中断内使用 interrupt_global_enable(0); 来开启中断嵌套
// 简单点说实际上进入中断后TC系列的硬件自动调用了 interrupt_global_disable(); 来拒绝响应任何的中断，因此需要我们自己手动调用 interrupt_global_enable(0); 来开启中断的响应。






/**
 * @brief CCU6_0 PIT通道1中断服务函数
 *
 * 清除CCU60通道1 PIT中断标志。
 * 该中断曾用于遥控器信号扫描(x6f_scan), 当前已禁用。
 */
IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH1);
//    x6f_scan();        // RackTest禁用遥控器扫描，释放P33_6/P33_7给TIM2编码器


}

/**
 * @brief CCU6_1 PIT通道0中断服务函数(1ms定时)
 *
 * 1ms周期定时中断，主要功能:
 * - 每10ms执行舵机转向控制(根据control_mode选择控制策略)
 * - 在管道/倒车/测试模式下每10ms更新后轮编码器
 * - 每100ms解析GPS/GNSS定位数据
 *
 * @note 该中断是系统核心定时基准, 舵机控制和GPS解析均依赖此中断的计数分频
 */
IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH0);
    // ------------ 定时计数更新 ------------
    TIM_FLAG2 ++;
    TIM_FLAG3 ++;

    // ------------ 舵机转向控制(每10ms) ------------
    if(TIM_FLAG3 > 10)
    {
        TIM_FLAG3 = 0 ;
//        Steer_Moter_control(30);
        switch(control_mode)
        {
            case IDLE:
                break;

            case YAOKONG:
                Steer_Moter_control(hot_rc_steer);
                break;

            case GUANDAO:
                Steer_Moter_control(out_servo);
                break;
            case DAOCHE :
                Steer_Moter_control(-out_servo);
                break;
            case RACK_TEST:
                if(rack_test_stage == 1)Steer_Moter_control((float)rack_test_steer_target);
                else if(rack_test_stage == 3)
                {
                    Rack_Straight_Update();
                    Steer_Moter_control(rack_straight_steer_target);
                }
                else VeerMoter_Set(0);
                break;

            default : break;
        }


    }

    /* 后轮独立模块: ISR 只做10ms编码器采样, PID在主循环处理 */
    // ------------ 后轮编码器更新(管道/倒车/测试模式, 每10ms) ------------
    if(control_mode == RACK_TEST || control_mode == GUANDAO || control_mode == DAOCHE)
    {
        static uint8 rear_tick = 0;
        rear_tick++;
        if(rear_tick >= 10)
        {
            rear_tick = 0;
            rear_motor_encoder_update_10ms();
        }
    }

    // ------------ GPS数据解析(每100ms) ------------
    if(GPS_WORK_FLAG)
    {
        if(TIM_FLAG2 > 100 )
        {
            TIM_FLAG2 = 0;
            if(gnss_flag)
            {
                gnss_flag = 0 ;
                gnss_data_parse();           //开始解析数据
//                klm_handle();
                // 检查标志位 开始解析
                if(Main_Key_Flag ){update_gpsinformation();}
            }
        }
    }

}

/**
 * @brief CCU6_1 PIT通道1中断服务函数(7ms定时)
 *
 * 7ms周期定时中断，主要功能:
 * - 每7ms执行按键扫描(Key_Scan)与IMU传感器读取(imu963ra_get_gyro, IMU_GetValues)
 * - 根据控制模式(control_mode)执行速度控制(遥控模式)
 * - 当Main_Key_Flag为0时停止所有电机
 *
 * @note 该中断与cc61_pit_ch0_isr配合使用, 分别处理不同的控制周期
 */
IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH1);
    // ------------ 定时计数更新 ------------
    TIM_FLAG1++;
//    TIM_FLAG2++;
    // ------------ 按键扫描、IMU读取与速度控制(每7ms) ------------
        if(TIM_FLAG1 > 7 )
    {
        TIM_FLAG1 = 0;
//        Speed_Control(10 , 10);
        if(Main_Key_Flag == 1)
        {
                // ------------ 按键扫描与IMU读取 ------------
                                Key_Scan();

                imu963ra_get_gyro();
                IMU_GetValues();



                switch(control_mode)
                {
                    case IDLE:
                        break;
                    case YAOKONG:
                        Speed_Control(hot_rc_speed +(hot_rc_delta * TRACK_WIDTH / 2.0f) ,hot_rc_speed- (hot_rc_delta * TRACK_WIDTH / 2.0f));
                        break;
                    case GUANDAO:
                        break;
                    case DAOCHE:
                        break;
                    case RACK_TEST:
                        break;
                    default : break;
                }


        }
        // ------------ 未启动时电机停止 ------------
        if(Main_Key_Flag == 0)
        {

                Speed_Control((float)0.0 ,(float) 0.0);
                rear_motor_stop();

        }
            }

    }










// **************************** PIT中断函数 ****************************


// **************************** 外部中断函数 ****************************
/**
 * @brief 外部中断通道0和通道4服务函数
 *
 * 处理两个外部中断源:
 * - 通道4(P33_7): 摄像头1场同步信号(VSYNC), 触发camera_vsync_handler_1
 * - 通道0(P15_4): 预留通道(当前未使用)
 *
 * @note 该ISR为摄像头采集的关键入口, vsync信号触发后开始新一帧图像采集
 */
IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    // ------------ 通道4中断: 摄像头1场同步 ------------
    if(exti_flag_get(ERU_CH4_REQ8_P33_7))           // 通道4中断
    {
        exti_flag_clear(ERU_CH4_REQ8_P33_7);
        camera_vsync_handler_1();                   // 摄像头1 触发采集统一回调函数
    }

    // ------------ 通道0中断: P15_4(预留) ------------
    if(exti_flag_get(ERU_CH0_REQ0_P15_4))           // 通道0中断
    {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容




    }

}

// 由于摄像头pclk引脚默认占用了 1通道，用于触发DMA，因此这里不再定义中断函数
/**
 * @brief 外部中断通道1和通道5服务函数
 *
 * 处理摄像头PCLK(像素时钟)中断信号, 用于DMA采集时序同步。
 * 当前PCLK中断由DMA直接处理, 此ISR内未执行实际采集逻辑, 仅保留中断框架。
 *
 * @note 使用摄像头时请勿修改此中断, PCLK信号由硬件连接至DMA触发
 */
IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

    // ------------ 通道1中断: P14_3(PCLK) ------------
    if(exti_flag_get(ERU_CH1_REQ10_P14_3))          // 通道1中断
    {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容




    }

    // ------------ 通道5中断: P15_8(PCLK) ------------
    if(exti_flag_get(ERU_CH5_REQ1_P15_8))           // 通道5中断
    {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容




    }
}

/**
 * @brief 外部中断通道2和通道6服务函数
 *
 * 处理摄像头PCLK(像素时钟)中断信号, 用于DMA采集时序同步。
 * 当前PCLK中断由DMA直接处理, 此ISR内未执行实际采集逻辑, 仅保留中断框架。
 *
 * @note 使用摄像头时请勿修改此中断, PCLK信号由硬件连接至DMA触发
 */
IFX_INTERRUPT(exti_ch2_ch6_isr, 0, EXTI_CH2_CH6_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    // ------------ 通道2中断: P00_4(PCLK) ------------
    if(exti_flag_get(ERU_CH2_REQ7_P00_4))           // 通道2中断
    {
        exti_flag_clear(ERU_CH2_REQ7_P00_4);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容



    }
    // ------------ 通道6中断: P20_0(PCLK) ------------
    if(exti_flag_get(ERU_CH6_REQ9_P20_0))           // 通道6中断
    {
        exti_flag_clear(ERU_CH6_REQ9_P20_0);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容




    }
}

/**
 * @brief 外部中断通道3和通道7服务函数
 *
 * 处理两个外部中断源:
 * - 通道3(P10_3): 摄像头2场同步信号(VSYNC), 触发camera_vsync_handler_2
 * - 通道7(P15_1): 预留通道(当前未使用)
 *
 * @note 该ISR为摄像头2采集的关键入口, vsync信号触发后开始新一帧图像采集
 */
IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

    // ------------ 通道3中断: 摄像头2场同步 ------------
    if(exti_flag_get(ERU_CH3_REQ3_P10_3))           // 通道3中断
    {
        exti_flag_clear(ERU_CH3_REQ3_P10_3);
        camera_vsync_handler_2();                   // 摄像头2触发采集统一回调函数
    }

    // ------------ 通道7中断: P15_1(预留) ------------
    if(exti_flag_get(ERU_CH7_REQ16_P15_1))          // 通道7中断
    {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容



    }
}
// **************************** 外部中断函数 ****************************


// **************************** DMA中断函数 ****************************
/**
 * @brief DMA通道6中断服务函数
 *
 * 处理摄像头1的DMA数据传输完成中断。
 * 当一帧图像数据通过DMA传输完成后触发, 调用camera_dma_handler_1进行图像数据处理。
 *
 * @note 该中断优先级与DMA_INT_PRIO_1对应, 需确保摄像头数据处理在中断内完成
 */
IFX_INTERRUPT(dma_ch6_isr, 0, DMA_INT_PRIO_1)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    camera_dma_handler_1();                         // 摄像头1 采集完成统一回调函数
}

/**
 * @brief DMA通道7中断服务函数
 *
 * 处理摄像头2的DMA数据传输完成中断。
 * 当一帧图像数据通过DMA传输完成后触发, 调用camera_dma_handler_2进行图像数据处理。
 *
 * @note 该中断优先级与DMA_INT_PRIO_2对应, 需确保摄像头数据处理在中断内完成
 */
IFX_INTERRUPT(dma_ch7_isr, 0, DMA_INT_PRIO_2)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    camera_dma_handler_2();                         // 摄像头2 采集完成统一回调函数
}
// **************************** DMA中断函数 ****************************


// **************************** 串口中断函数 ****************************
// 串口0默认作为调试串口
/**
 * @brief 串口0(UART0)发送中断服务函数
 *
 * 串口0默认为调试串口(printf输出), 当前发送中断内未执行额外处理。
 */
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套



}
/**
 * @brief 串口0(UART0)接收中断服务函数
 *
 * 串口0默认为调试串口, 若启用DEBUG_UART_USE_INTERRUPT则通过debug_interrupr_handler处理接收数据。
 *
 * @note 若需修改调试串口号, 请修改DEBUG_UART_INDEX宏并将对应串口的接收中断处理函数放入此处
 */
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

#if DEBUG_UART_USE_INTERRUPT                        // 如果开启 debug 串口中断
        debug_interrupr_handler();                  // 调用 debug 串口接收处理函数 数据会被 debug 环形缓冲区读取
#endif                                              // 如果修改了 DEBUG_UART_INDEX 那这段代码需要放到对应的串口中断去
}


/**
 * @brief 串口1(UART1)发送中断服务函数
 *
 * 串口1默认连接摄像头配置串口, 当前发送中断内未执行额外处理。
 */
// 串口1默认连接到摄像头配置串口
IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套




}
/**
 * @brief 串口1(UART1)接收中断服务函数
 *
 * 串口1默认连接摄像头配置串口, 接收数据通过camera_uart_handler_1处理。
 */
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    camera_uart_handler_1();                        // 摄像头参数配置统一回调函数
}

// 串口2默认连接到无线转串口模块
/**
 * @brief 串口2(UART2)发送中断服务函数
 *
 * 串口2默认连接无线转接模块, 当前发送中断内未执行额外处理。
 */
IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套



}

/**
 * @brief 串口2(UART2)接收中断服务函数
 *
 * 串口2默认连接无线转接模块, 接收数据通过wireless_module_uart_handler处理。
 */
IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    wireless_module_uart_handler();                 // 无线模块统一回调函数



}
/**
 * @brief 串口3(UART3)发送中断服务函数
 *
 * 串口3默认连接GPS/GNSS定位模块, 当前发送中断内未执行额外处理。
 */
// 串口3默认连接到GPS定位模块
IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套



}

/**
 * @brief 串口3(UART3)接收中断服务函数
 *
 * 串口3默认连接GPS/GNSS定位模块, 接收数据通过gnss_uart_callback处理。
 */
IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

    gnss_uart_callback();                           // GNSS串口回调函数



}

// 串口通讯错误中断
/**
 * @brief 串口0(UART0)错误中断服务函数
 *
 * 调用IfxAsclin_Asc_isrError处理串口0的通信错误(如帧错误、奇偶校验错误等)。
 */
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart0_handle);
}
/**
 * @brief 串口1(UART1)错误中断服务函数
 *
 * 调用IfxAsclin_Asc_isrError处理串口1的通信错误。
 */
IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart1_handle);
}
/**
 * @brief 串口2(UART2)错误中断服务函数
 *
 * 调用IfxAsclin_Asc_isrError处理串口2的通信错误。
 */
IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart2_handle);
}
/**
 * @brief 串口3(UART3)错误中断服务函数
 *
 * 调用IfxAsclin_Asc_isrError处理串口3的通信错误。
 */
IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart3_handle);
}
