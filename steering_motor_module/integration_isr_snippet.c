// ============================================================
// isr.c 转向控制插件 (CCU6_0 CH0 PIT 中断)
// ============================================================
// 头文件
#include "angle_control.h"

// CCU6_0 CH0 中断服务函数，10ms
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    static uint8 in_update = 0;
    if (in_update) return;                          // 防止重入
    in_update = 1;

    pit_clear_flag(CCU60_CH0);                      // 清除中断标志

    angle_control_update();                         // 转向角度更新

    in_update = 0;
}
