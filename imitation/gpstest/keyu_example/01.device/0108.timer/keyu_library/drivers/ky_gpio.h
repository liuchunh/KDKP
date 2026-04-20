/*********************************************************************************
 * 项目名称：[Keyu_TC264DA_Open_Source_Library] 开源组件库
 * 版权所有：[2025] [北京科宇通博科技有限公司]
 *
 * 许可协议：采用 GNU GPL v3.0 开源许可
 * 您可依据协议进行二次开发、传播，但须保留原始版权信息
 * 协议详情参见：https://www.gnu.org/licenses/gpl-3.0.html
 *
 * 免责声明：本组件库仅提供技术参考，使用方需自行验证适用性
 *
 * 协议文件：GPL v3.0 完整文本位于根目录下
 *
 * === 文件信息 ===
 * 文件名：[ky_gpio.h]
 * 开发单位：北京科宇通博科技有限公司
 * 适用环境：[AURIX TC264DA]
 * 官方渠道：
 *   - 代码仓库：[https://gitee.com/beijing-keyu---jiangxi/Keyu_TC264DA_Open_Source_Library]
 *   - 淘宝店铺：https://kyznc.taobao.com/
 *   - 技术支持：QQ群 974530818
 *
 * === 修订记录 ===
 * 日期       |  开发者  | 变更说明
 * -----------|----------|----------------------
 * 2026.01.07 |   毛毛   | V3.0
 *********************************************************************************/
#ifndef __KY_GPIO_H__
#define __KY_GPIO_H__

#include "ky_typedef.h"
#include "Port/Std/IfxPort.h"

// GPIO 端口定义
typedef enum {
    /* PORT 00 */
    GPIO_P00 = 0x0000, 
    GPIO_P01, GPIO_P02, GPIO_P03, GPIO_P04, GPIO_P05, GPIO_P06, GPIO_P07, GPIO_P08, GPIO_P09,
    GPIO_P00_12 = 0x000C,
    /* PORT 02 */
    GPIO_P02_0  = 0x0200,GPIO_P02_1,GPIO_P02_2,GPIO_P02_3,GPIO_P02_4,
    GPIO_P02_5  = 0x0205,GPIO_P02_6,GPIO_P02_7,GPIO_P02_8,
    /* PORT 10 */
    GPIO_P10_1  = 0x1001,GPIO_P10_2,GPIO_P10_3,
    GPIO_P10_5  = 0x1005,GPIO_P10_6,
    /* PORT 11 */
    GPIO_P11_2  = 0x1102,GPIO_P11_3,
    GPIO_P11_6  = 0x1106,
    GPIO_P11_9  = 0x1109,GPIO_P11_10,GPIO_P11_11,GPIO_P11_12,
    /* PORT 13 */
    GPIO_P13_0  = 0x1300,GPIO_P13_1,GPIO_P13_2,GPIO_P13_3,
    /* PORT 14 */
    GPIO_P14_0  = 0x1400,GPIO_P14_1,GPIO_P14_2,GPIO_P14_3,GPIO_P14_4,
    GPIO_P14_5  = 0x1405,GPIO_P14_6,
    /* PORT 15 */
    GPIO_P15_0  = 0x1500,GPIO_P15_1,GPIO_P15_2,GPIO_P15_3,GPIO_P15_4,
    GPIO_P15_5  = 0x1505,GPIO_P15_6,GPIO_P15_7,GPIO_P15_8,
    /* PORT 20 */
    GPIO_P20_0  = 0x2000,
    GPIO_P20_2  = 0x2002,GPIO_P20_3,
    GPIO_P20_6  = 0x2006,GPIO_P20_7,GPIO_P20_8,GPIO_P20_9,GPIO_P20_10,
    GPIO_P20_11 = 0x200B,GPIO_P20_12,GPIO_P20_13,GPIO_P20_14,
    /* PORT 21 */
    GPIO_P21_2  = 0x2102,GPIO_P21_3,GPIO_P21_4,GPIO_P21_5,GPIO_P21_6,
    GPIO_P21_7  = 0x2107,
    /* PORT 22 */
    GPIO_P22_0 = 0x2200,GPIO_P22_1,GPIO_P22_2,GPIO_P22_3,
    /* PORT 23 */
    GPIO_P23_1 = 0x2301,
    /* PORT 32 */
    GPIO_P32_4 = 0x3204,
    /* PORT 33 */
    GPIO_P33_4  = 0x3304,GPIO_P33_5,GPIO_P33_6,GPIO_P33_7,GPIO_P33_8,    
    GPIO_P33_9  = 0x3309,GPIO_P33_10,GPIO_P33_11,GPIO_P33_12,GPIO_P33_13,
    GPIO_PIN_MAX_NUM
} GPIO_Pin_t;
// GPIO 模式定义 (适配 TC264 IfxPort 模式)
typedef enum {
    // --- 输入模式 ---
    GPIO_MODE_IN_FLOATING, // 浮空输入 (高阻态，无上下拉) 
    GPIO_MODE_IN_PD,       // 下拉输入 (高阻态 + 内部下拉) 
    GPIO_MODE_IN_PU,       // 上拉输入 (高阻态 + 内部上拉) 
    // --- 输出模式 ---
    GPIO_MODE_OUT_PP,      // 推挽输出  
    GPIO_MODE_OUT_OD,      // 开漏输出  
    // --- 复用模式 ---
    GPIO_MODE_AF_PP,       // 复用推挽输出
    GPIO_MODE_AF_OD,       // 复用开漏输出
    // --- 兼容性定义 ---
    GPIO_MODE_OUT_OD_PU = GPIO_MODE_OUT_OD, // 开漏输出 + 内部上拉 (TC264通常不直接支持，映射为普通开漏) 
    GPIO_MODE_QUASI = GPIO_MODE_OUT_PP,     // 准双向 (映射为推挽)
    GPIO_MODE_QUASI_PU = GPIO_MODE_OUT_PP   // 准双向 + 内部强上拉 (映射为推挽)
} GPIO_Mode_t;
// GPIO 电平定义
typedef enum {
    GPIO_LOW = 0,  // 低电平
    GPIO_HIGH = 1, // 高电平

} GPIO_Level_t;
// GPIO 速率定义
typedef enum {
    GPIO_SPEED_FAST = 0,    // 转换速度最快
    GPIO_SPEED_SECOND = 1,  // 转换速度第二
    GPIO_SPEED_TIHRD = 2,   // 转换速度第三
    GPIO_SPEED_LOW = 3,     // 转换速度最低

} GPIO_Speed_t;
// GPIO 方向定义 (用于动态切换输入/输出)
typedef enum {
    GPIO_DIR_IN = 0,   // 输入方向
    GPIO_DIR_OUT = 1,  // 输出方向
} GPIO_Dir_t;
void gpio_init(GPIO_Pin_t pin, GPIO_Mode_t mode, GPIO_Level_t level);   //GPIO初始化
void gpio_set_dir(GPIO_Pin_t pin, GPIO_Dir_t dir);                      //设置GPIO方向
void gpio_set_level(GPIO_Pin_t pin, GPIO_Level_t level);                //设置GPIO电平
void gpio_toggle_level(GPIO_Pin_t pin);                                 //翻转GPIO电平
GPIO_Level_t gpio_get_level(GPIO_Pin_t pin);                           //获取GPIO电平
void gpio_set_speed(GPIO_Pin_t pin, GPIO_Speed_t speed);                //设置GPIO速率
#endif /* __KY_GPIO_H__ */
