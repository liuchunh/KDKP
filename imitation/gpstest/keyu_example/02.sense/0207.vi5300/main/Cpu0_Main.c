/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的传感器应用演示 - VI5300测距例程
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
 * 文件功能：VI5300距离传感器例程 - 在IPS200屏幕上显示距离数据
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
 * 2026年01月10日      毛毛    V3.1     增加VI5300测距例程
 *
 * === 代码说明 ===
 * 1. 硬件连接：
 *    - VI5300传感器: I2C接口 ToF激光测距传感器
 *    - I2C引脚: SCL:P21_3, SDA:P21_2
 *    - 中断引脚: XSHUT:P21_4 (用于传感器复位)
 *    - 显示屏幕: IPS200 2.0寸 SPI屏幕
 * 2. 测量功能：
 *    - 测量范围: 0mm ~ 4000mm (0-4米)
 *    - 测量精度: ±5%典型值
 *    - 置信度输出: 0-100%，数值越大越可信
 *    - I2C通信: 标准I2C协议读写距离和置信度数据
 * 3. 实验现象：
 *    - 屏幕实时显示测量距离（毫米）
 *    - 显示测量置信度和状态
 *    - 串口同步输出测量数据
 *    - 适用于智能车循迹、障碍物检测等场景
 */

#include "ky_all.h"
#include "ky_vi5300.h"
#include "ky_ips.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

vi5300_data_t vi5300_data;
int counter = 0;

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
    ips_show_string(10, 5, "VI5300 Distance Sensor");
    ips_show_string(10, 25, "Initializing...");

    // 初始化VI5300距离传感器
    vi5300_status_t status = vi5300_init();
    if(status == vi5300_ok)
    {
        printf("VI5300初始化成功\r\n");
        ips_show_string(10, 45, "Sensor OK!");
    }
    else
    {
        printf("VI5300初始化失败, 状态: %d\r\n", status);
        ips_show_string(10, 45, "Sensor Error!");
        while(1);  // 初始化失败则停止
    }

    /* 初始化结束 */
    IfxCpu_enableInterrupts(); /* 开启全局中断 */

    printf("开始测量距离...\r\n");
    ips_show_string(10, 65, "Running...");

    while(1)
    {
        /* 主循环开始 */

        // 获取距离数据
        status = vi5300_get_distance(&vi5300_data);

        // 在屏幕上显示距离数据
        ips_set_color(WHITE, BLACK);

        // 显示标题
        ips_show_string(10, 90, "Distance Measurement");
        ips_show_string(10, 110, "------------------");

        // 显示距离（毫米）
        ips_show_string(10, 135, "Distance:");
        if(vi5300_data.is_valid)
        {
            ips_show_u_int(80, 135, vi5300_data.distance_mm, 5);
            ips_show_string(140, 135, "mm");
        }
        else
        {
            ips_show_string(80, 135, "Invalid");
        }

        // 显示置信度
        ips_show_string(10, 160, "Confidence:");
        ips_show_u_int(90, 160, vi5300_data.confidence, 3);
        ips_show_string(120, 160, "%");

        // 显示状态
        ips_show_string(10, 185, "Status:");
        if(vi5300_data.is_valid)
        {
            ips_show_string(60, 185, "Valid");
        }
        else
        {
            ips_show_string(60, 185, "No Signal");
        }

        // 串口打印（每秒一次）
        if(counter % 2 == 0)
        {
            if(vi5300_data.is_valid)
            {
                printf("距离: %d mm, 置信度: %d%%\r\n", vi5300_data.distance_mm, vi5300_data.confidence);
            }
            else
            {
                printf("无效数据\r\n");
            }
        }

        counter++;
        delay_ms(500);

        /* 主循环结束 */
    }
}
