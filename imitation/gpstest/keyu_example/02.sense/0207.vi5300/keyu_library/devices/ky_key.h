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
 * 文件名：[ky_key.h]
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
#ifndef __KY_KEY_H__
#define __KY_KEY_H__

#include "ky_typedef.h"
#include "ky_gpio.h"

#define KEY_NUM                 4   // 按键数量
#define KEY_DEBOUNCE_TIME       3   // 消抖时间（单位：扫描次数）
#define KEY_LONG_PRESS_TIME     100 // 长按时间（单位：扫描次数，假设10ms扫描一次，则100次=1秒）
#define KEY_DOUBLE_CLICK_TIME   50  // 连按间隔时间（单位：扫描次数，50次=500ms）

typedef enum {
    KEY_0 = 0,                       // 按键0 (P11.2)
    KEY_1 = 1,                       // 按键1 (P11.3)
    KEY_2 = 2,                       // 按键2 (P20.6)
    KEY_3 = 3,                       // 按键3 (P20.7)
} Key_Id_t;         
typedef enum {           
    KEY_RELEASED = 0,               // 按键释放
    KEY_PRESSED  = 1,               // 按键按下
} Key_State_t;
typedef enum {
    KEY_EVENT_NONE = 0,             // 无事件
    KEY_EVENT_SHORT_PRESS,          // 短按事件
    KEY_EVENT_LONG_PRESS,           // 长按事件（触发一次）
    KEY_EVENT_CONTINUOUS_PRESS,     // 连续按下（长按后持续触发）
    KEY_EVENT_DOUBLE_CLICK,         // 连按事件（双击）
} Key_Event_t;
typedef void (*Key_Callback_t)(Key_Id_t key_id, Key_Event_t event);
void key_init(void);                                                //按键初始化
Key_State_t key_read(Key_Id_t key_id);                              //读取按键状态
Key_State_t key_scan(Key_Id_t key_id);                              //扫描按键
Key_Event_t key_get_event(Key_Id_t key_id);                         //获取按键事件
void key_init_single(Key_Id_t key_id);                              //初始化单个按键
void key_clear_all(void);                                           //清除所有按键状态
void key_clear_single(Key_Id_t key_id);                             //清除单个按键状态
uint8 key_is_pressed(Key_Id_t key_id);                              //判断按键是否按下
uint8 key_is_combination(uint8 key_mask);                           //判断组合按键
void key_set_long_press_time(Key_Id_t key_id, uint16 time_ms);      //设置长按时间
void key_register_callback(Key_Id_t key_id, Key_Callback_t callback);  //注册按键回调
#endif /* __KY_KEY_H__ */
