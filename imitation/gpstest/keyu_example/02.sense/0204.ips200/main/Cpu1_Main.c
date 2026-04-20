/*
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 项目定位：基于[TC264DA核心技术/依赖]的基础功能空库
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
 * 文件名：Cpu1_Main.c
 * 文件功能：cpu1主函数，主要用于初始化及主循环
 * 开发单位：北京科宇通博科技有限公司江西分公司
 * 适用环境：[AURIX™ Development Studio V1.10.28]
 * 官方渠道：代码仓库 https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library.git | 淘宝店铺 https://kyznc.taobao.com/ | 技术支持 QQ群 974530818
 * 
 * === 修订记录 ===
 * 日期         开发者    变更说明
 * -------------------------------------------
 * 2026年01月07日      毛毛    V3.0
 */
 
 #include "ky_all.h"

extern IfxCpu_syncEvent cpuSyncEvent;

void core1_main(void)
{
    /* 系统初始化（中断、看门狗等） */
    systemInit(KEYU_CPU_CORE_1);
    /* CPU同步等待（等待所有核心初始化完成） */
    cpuSyncWait(&cpuSyncEvent, 1);
    
    /* 初始化开始 */

    //用户书写自己的初始化代码

    /* 初始化结束 */
    while(1)
    {
        /* 主循环开始 */

        //用户书写自己的主循环代码

        /* 主循环结束 */
    }
}
