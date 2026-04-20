/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的图像采集演示 - MT9V034摄像头例程
 * 版权所有：[2025] [北京科宇通博科技有限公司江西分公司]
 *
 * 许可协议：采用 GNU GPL v3.0 开源许可
 * 您可依据协议进行二次开发、传播，但须保留原始版权信息
 * 协议详情参见：https://www.gnu.org/licenses/gpl-3.0.html
 *
 * 免责声明：本组件库仅提供技术参考，使用方需自行验证适用性
 *
 * 协议文件：GPL v3.0 完整文本位于 [Keyu_TC264DA_Open_Source_Library] 目录下
 *
 * === 文件信息 ===
 * 文件名：Cpu0_Main.c
 * 文件功能：MT9V034摄像头例程 - 在IPS200屏幕上显示摄像头图像
 * 开发单位：北京科宇通博科技有限公司江西分公司
 * 适用环境：[AURIX™ Development Studio V1.10.28]
 * 官方渠道：
        代码仓库 https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library.git
        淘宝店铺 https://kyznc.taobao.com/
        技术支持 QQ群 974530818
 *
 * === 修订记录 ===
 * 日期         开发者    变更说明
 * -------------------------------------------
 * 2026年01月07日      毛毛    V3.0
 * 2026年01月10日      毛毛    V3.1     增加MT9V034摄像头例程
 *
 * === 代码说明 ===
 * 1. 硬件连接：
 *    - MT9V034摄像头: 并口接口 (分辨率:188x120)
 *    - 数据引脚: D0-D7 (P00_0 - P00_7)
 *    - 控制引脚: PCLK:P02_1, VSYNC:P02_0
 *    - I2C配置: SCL:P02_3, SDA:P02_2
 *    - 显示屏幕: IPS200 2.0寸 SPI屏幕
 * 2. 功能说明：
 *    - DMA采集: 使用DMA通道5自动采集图像数据
 *    - 场同步触发: VSYNC中断触发新帧采集
 *    - 图像显示: 实时在IPS200屏幕上显示灰度图像
 * 3. 实验现象：
 *    - 屏幕实时显示摄像头采集的灰度图像
 *    - 图像居中显示在IPS200屏幕上
 *    - 支持调整曝光时间和增益参数
 */

#include "ky_all.h"
#include "ky_mt9v034.h"
#include "ky_ips.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

volatile uint8_t new_frame_flag = 0;  // 新帧标志

void core0_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_0);
    cpuSyncWait(&cpuSyncEvent, 1);
    uart_debug_init();
    /* 初始化开始 */

    // 初始化IPS200屏幕
    ips_init(IPS_TYPE_200);
    ips_clear(BLACK);
    ips_show_string(10, 5, "MT9V034 Camera");
    ips_show_string(10, 25, "Initializing...");

    // 初始化MT9V034摄像头
    if(mt9_v034_init() == mt9_v034_err_none)
    {
        printf("MT9V034初始化成功\r\n");
        ips_show_string(10, 45, "Camera OK!");
    }
    else
    {
        printf("MT9V034初始化失败\r\n");
        ips_show_string(10, 45, "Camera Error!");
        while(1);  // 初始化失败则停止
    }

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    printf("开始采集图像...\r\n");
    ips_show_string(10, 65, "Running...");

    while(1)
    {
        /* 主循环开始 */

        // 获取摄像头图像
        if(mt9_v034_get_image() == mt9_v034_err_none)
        {
            // 在IPS200屏幕上显示灰度图像
            // MT9V034: 188x120, 居中显示在IPS200 (320x240)
            ips_show_gray_image(66, 80, mt9_v034_width, mt9_v034_height, (const uint8_t*)mt9_v034__image);

            // 显示帧信息
            ips_show_string(10, 210, "Size: 188x120");
            ips_show_string(10, 230, "FPS: Calculating...");
        }

        /* 主循环结束 */
    }
}
