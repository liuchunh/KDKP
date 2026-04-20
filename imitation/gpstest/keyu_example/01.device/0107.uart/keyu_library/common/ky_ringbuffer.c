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
 * 文件名：[ky_ringbuffer.c]
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
#include "ky_ringbuffer.h"
#include <string.h>

/***************************************************************
  *  @brief     检查一个数是否是2的幂次
 **************************************************************/
static uint8_t is_power_of_2(uint32_t n)
{
    return (n != 0) && ((n & (n - 1)) == 0);
}

/***************************************************************
  *  @brief     初始化环形缓冲区
 **************************************************************/
uint8_t ringbuffer_init(ringbuffer *rb, uint8_t *buffer, uint32_t size)
{
    if (rb == NULL || buffer == NULL || !is_power_of_2(size))
    {
        return 0; // 失败
    }
    
    rb->buffer = buffer;
    rb->size = size;
    rb->mask = size - 1;
    rb->in = 0;
    rb->out = 0;
    
    return 1; // 成功
}

/***************************************************************
  *  @brief     向环形缓冲区写入一个字节
 **************************************************************/
uint8_t ringbuffer_putc(ringbuffer *rb, uint8_t dat)
{
    if (ringbuffer_is_full(rb))
    {
        return 0; // 满了
    }
    
    rb->buffer[rb->in & rb->mask] = dat;
    rb->in++;
    
    return 1; // 成功
}

/***************************************************************
  *  @brief     从环形缓冲区读取一个字节
 **************************************************************/
uint8_t ringbuffer_getc(ringbuffer *rb, uint8_t *dat)
{
    if (ringbuffer_is_empty(rb))
    {
        return 0; // 空了
    }
    
    *dat = rb->buffer[rb->out & rb->mask];
    rb->out++;
    
    return 1; // 成功
}

/***************************************************************
  *  @brief     从环形缓冲区读取多个字节
 **************************************************************/
uint32_t ringbuffer_read(ringbuffer *rb, uint8_t *target, uint32_t len)
{
    uint32_t available = ringbuffer_len(rb);
    uint32_t to_read = (len < available) ? len : available;
    uint32_t off = rb->out & rb->mask;
    uint32_t l;
    
    if (to_read == 0) return 0;
    
    // 计算第一段数据长度 (从out到缓冲区末尾)
    l = (off + to_read > rb->size) ? (rb->size - off) : to_read;
    
    // 复制第一段数据
    memcpy(target, rb->buffer + off, l);
    
    // 如果有第二段数据 (从缓冲区开头)
    if (to_read > l)
    {
        memcpy(target + l, rb->buffer, to_read - l);
    }
    
    rb->out += to_read;
    
    return to_read;
}

/***************************************************************
  *  @brief     向环形缓冲区写入多个字节
 **************************************************************/
uint32_t ringbuffer_write(ringbuffer *rb, const uint8_t *source, uint32_t len)
{
    uint32_t available = rb->size - ringbuffer_len(rb);
    uint32_t to_write = (len < available) ? len : available;
    uint32_t off = rb->in & rb->mask;
    uint32_t l;
    
    if (to_write == 0) return 0;
    
    // 计算第一段数据长度 (从in到缓冲区末尾)
    l = (off + to_write > rb->size) ? (rb->size - off) : to_write;
    
    // 复制第一段数据
    memcpy(rb->buffer + off, source, l);
    
    // 如果有第二段数据 (从缓冲区开头)
    if (to_write > l)
    {
        memcpy(rb->buffer, source + l, to_write - l);
    }
    
    rb->in += to_write;
    
    return to_write;
}

/***************************************************************
  *  @brief     获取环形缓冲区中的数据长度
 **************************************************************/
uint32_t ringbuffer_len(ringbuffer *rb)
{
    return rb->in - rb->out;
}

/***************************************************************
  *  @brief     清空环形缓冲区
 **************************************************************/
void ringbuffer_clear(ringbuffer *rb)
{
    rb->in = 0;
    rb->out = 0;
}

/***************************************************************
  *  @brief     检查环形缓冲区是否为空
 **************************************************************/
uint8_t ringbuffer_is_empty(ringbuffer *rb)
{
    return (rb->in == rb->out) ? 1 : 0;
}

/***************************************************************
  *  @brief     检查环形缓冲区是否已满
 **************************************************************/
uint8_t ringbuffer_is_full(ringbuffer *rb)
{
    return (ringbuffer_len(rb) >= rb->size) ? 1 : 0;
}
