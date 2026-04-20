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
 * 文件名：[ky_key.c]
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
#include "ky_key.h"
#include "ky_gpio.h"

// 按键引脚配置（可扩展，只需修改此表）
static const GPIO_Pin_t key_pin_table[KEY_NUM] = 
{
    GPIO_P11_3,     // KEY_0
    GPIO_P11_2,     // KEY_1
    GPIO_P20_7,     // KEY_2
    GPIO_P20_6,     // KEY_3
};

static uint8 key_debounce_cnt[KEY_NUM] = {0};                   // 按键消抖计数器
static Key_State_t key_last_state[KEY_NUM] = {KEY_RELEASED};    // 按键上一次状态
static uint8 key_triggered[KEY_NUM] = {0};                      // 按键触发标志
static uint16 key_press_cnt[KEY_NUM] = {0};                     // 按键按下持续时间计数器
static uint8 key_long_press_triggered[KEY_NUM] = {0};           // 长按触发标志
static uint16 key_release_time[KEY_NUM] = {0};                  // 上次释放时间计数器
static uint8 key_click_count[KEY_NUM] = {0};                    // 连按计数器
static Key_Callback_t key_callbacks[KEY_NUM] = {NULL};          // 按键事件回调函数指针数组
// 每个按键的长按时间阈值（可动态设置）
static uint16 key_long_press_time[KEY_NUM] = {KEY_LONG_PRESS_TIME, KEY_LONG_PRESS_TIME, KEY_LONG_PRESS_TIME, KEY_LONG_PRESS_TIME};

/***************************************************************
 *  @brief     按键初始化
 *  @return    void
 **************************************************************/
void key_init(void)
{
    uint8 i;
    for(i = 0; i < KEY_NUM; i++)
    {
        gpio_init(key_pin_table[i], GPIO_MODE_IN_PU, GPIO_HIGH);
        key_debounce_cnt[i] = 0;
        key_last_state[i] = KEY_RELEASED;
        key_triggered[i] = 0;
        key_press_cnt[i] = 0;
        key_long_press_triggered[i] = 0;
        key_release_time[i] = 0;
        key_click_count[i] = 0;
    }
}

/***************************************************************
 *  @brief     读取按键状态（无消抖）
 *  @param     key_id  按键ID
 *  @return    KEY_PRESSED-按下, KEY_RELEASED-释放
 **************************************************************/
Key_State_t key_read(Key_Id_t key_id)
{
    if(key_id >= KEY_NUM)
    {
        return KEY_RELEASED;
    }
    return (gpio_get_level(key_pin_table[key_id]) == GPIO_LOW) ? KEY_PRESSED : KEY_RELEASED;
}

/***************************************************************
 *  @brief     扫描按键（带消抖）
 *  @param     key_id  按键ID
 *  @return    KEY_PRESSED-检测到按下, KEY_RELEASED-未检测到
 *  @note      需要在主循环中定期调用，返回按下时只触发一次
 **************************************************************/
Key_State_t key_scan(Key_Id_t key_id)
{
    Key_State_t current_state;
    
    if(key_id >= KEY_NUM)
    {
        return KEY_RELEASED;
    }
    current_state = key_read(key_id);
    // 消抖处理
    if(current_state == KEY_PRESSED)
    {
        if(key_debounce_cnt[key_id] < KEY_DEBOUNCE_TIME)
        {
            key_debounce_cnt[key_id]++;
        }
        if(key_debounce_cnt[key_id] >= KEY_DEBOUNCE_TIME && !key_triggered[key_id])
        {
            key_triggered[key_id] = 1;
            key_last_state[key_id] = KEY_PRESSED;
            return KEY_PRESSED;  // 触发按键事件
        }
    }
    else
    {
        // 按键释放，清除计数和触发标志
        key_debounce_cnt[key_id] = 0;
        key_triggered[key_id] = 0;
        key_last_state[key_id] = KEY_RELEASED;
    }
    
    return KEY_RELEASED;
}

/***************************************************************
 *  @brief     获取按键事件（支持短按/长按）
 *  @param     key_id  按键ID
 *  @return    Key_Event_t 按键事件类型
 *  @note      需要在主循环中定期调用（建议10ms调用一次）
 **************************************************************/
Key_Event_t key_get_event(Key_Id_t key_id)
{
    Key_State_t current_state;
    Key_Event_t event = KEY_EVENT_NONE;
    
    if(key_id >= KEY_NUM)
    {
        return KEY_EVENT_NONE;
    }
    // 读取当前按键状态
    current_state = key_read(key_id);
    // 状态机处理
    if(current_state == KEY_PRESSED)
    {
        if(key_debounce_cnt[key_id] < KEY_DEBOUNCE_TIME)
        {
            key_debounce_cnt[key_id]++;
        }
        else
        {
            if(key_press_cnt[key_id] < 0xFFFF)
            {
                key_press_cnt[key_id]++;
            }
            if(key_press_cnt[key_id] >= key_long_press_time[key_id])
            {
                if(!key_long_press_triggered[key_id])
                {
                    key_long_press_triggered[key_id] = 1;
                    event = KEY_EVENT_LONG_PRESS;
                }
                else
                {
                    event = KEY_EVENT_CONTINUOUS_PRESS;
                }
            }
        }
    }
    else
    {
        if(key_debounce_cnt[key_id] >= KEY_DEBOUNCE_TIME)
        {
            if(key_press_cnt[key_id] < key_long_press_time[key_id] && key_press_cnt[key_id] > 0)
            {
                event = KEY_EVENT_SHORT_PRESS;
            }
        }
        key_debounce_cnt[key_id] = 0;
        key_press_cnt[key_id] = 0;
        key_long_press_triggered[key_id] = 0;
        if(key_release_time[key_id] < KEY_DOUBLE_CLICK_TIME)
        {
            key_click_count[key_id]++;
            if(key_click_count[key_id] >= 2)
            {
                event = KEY_EVENT_DOUBLE_CLICK;
                key_click_count[key_id] = 0;
                key_release_time[key_id] = 0xFFFF; 
            }
        }
        else
        {
            key_click_count[key_id] = 1; 
        }
        
        key_release_time[key_id] = 0;
    }
    if(current_state == KEY_RELEASED && key_release_time[key_id] < 0xFFFF)
    {
        key_release_time[key_id]++;
    }
    if(event != KEY_EVENT_NONE && key_callbacks[key_id] != NULL)
    {
        key_callbacks[key_id](key_id, event);
    }
    return event;
}

/***************************************************************
 *  @brief     初始化单个按键
 *  @param     key_id  按键ID
 *  @return    void
 **************************************************************/
void key_init_single(Key_Id_t key_id)
{
    if(key_id >= KEY_NUM)
    {
        return;
    }
    
    gpio_init(key_pin_table[key_id], GPIO_MODE_IN_PU, GPIO_HIGH);
    key_debounce_cnt[key_id] = 0;
    key_last_state[key_id] = KEY_RELEASED;
    key_triggered[key_id] = 0;
    key_press_cnt[key_id] = 0;
    key_long_press_triggered[key_id] = 0;
    key_release_time[key_id] = 0;
    key_click_count[key_id] = 0;
}

/***************************************************************
 *  @brief     清空所有按键状态
 *  @return    void
 **************************************************************/
void key_clear_all(void)
{
    uint8 i;
    
    for(i = 0; i < KEY_NUM; i++)
    {
        key_debounce_cnt[i] = 0;
        key_last_state[i] = KEY_RELEASED;
        key_triggered[i] = 0;
        key_press_cnt[i] = 0;
        key_long_press_triggered[i] = 0;
        key_release_time[i] = 0;
        key_click_count[i] = 0;
    }
}

/***************************************************************
 *  @brief     清空单个按键状态
 *  @param     key_id  按键ID
 *  @return    void
 **************************************************************/
void key_clear_single(Key_Id_t key_id)
{
    if(key_id >= KEY_NUM)
    {
        return;
    }
    
    key_debounce_cnt[key_id] = 0;
    key_last_state[key_id] = KEY_RELEASED;
    key_triggered[key_id] = 0;
    key_press_cnt[key_id] = 0;
    key_long_press_triggered[key_id] = 0;
    key_release_time[key_id] = 0;
    key_click_count[key_id] = 0;
}

/***************************************************************
 *  @brief     查询按键当前是否按下
 *  @param     key_id  按键ID
 *  @return    1-按下, 0-释放
 **************************************************************/
uint8 key_is_pressed(Key_Id_t key_id)
{
    if(key_id >= KEY_NUM)
    {
        return 0;
    }
    
    return (key_read(key_id) == KEY_PRESSED) ? 1 : 0;
}

/***************************************************************
 *  @brief     检测按键组合（多个按键同时按下）
 *  @param     key_mask  按键掩码（位0-3对应KEY_0-KEY_3）
 *  @return    1-所有指定按键都按下, 0-未全部按下
 **************************************************************/
uint8 key_is_combination(uint8 key_mask)
{
    uint8 i;
    
    for(i = 0; i < KEY_NUM; i++)
    {
        if(key_mask & (1 << i))
        {
            if(key_read(i) != KEY_PRESSED)
            {
                return 0; 
            }
        }
    }
    
    return 1;
}

/***************************************************************
 *  @brief     设置单个按键的长按时间
 *  @param     key_id  按键ID
 *  @param     time_ms  长按时间（毫秒，假设10ms扫描周期）
 *  @return    void
 **************************************************************/
void key_set_long_press_time(Key_Id_t key_id, uint16 time_ms)
{
    if(key_id >= KEY_NUM)
    {
        return;
    }
    key_long_press_time[key_id] = time_ms / 10;
}

/***************************************************************
 *  @brief     注册按键事件回调函数
 *  @param     key_id  按键ID
 *  @param     callback  回调函数指针
 *  @return    void
 **************************************************************/
void key_register_callback(Key_Id_t key_id, Key_Callback_t callback)
{
    if(key_id >= KEY_NUM)
    {
        return;
    }
    
    key_callbacks[key_id] = callback;
}
