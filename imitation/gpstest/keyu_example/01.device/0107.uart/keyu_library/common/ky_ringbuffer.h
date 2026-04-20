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
 * 文件名：[ky_ringbuffer.h]
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
#ifndef __KY_RINGBUFFER_H__
#define __KY_RINGBUFFER_H__

#include "ky_typedef.h"

typedef struct {
    uint8_t *buffer;          // 缓冲区指针
    uint32_t size;            // 缓冲区大小 (必须是2的幂次)
    uint32_t in;              // 写入位置
    uint32_t out;             // 读取位置
    uint32_t mask;            // 掩码 (size - 1)
} ringbuffer;
uint8_t ringbuffer_init(ringbuffer *rb, uint8_t *buffer, uint32_t size);        //环形缓冲区初始化
uint8_t ringbuffer_putc(ringbuffer *rb, uint8_t dat);                            //写入单字节
uint8_t ringbuffer_getc(ringbuffer *rb, uint8_t *dat);                           //读取单字节
uint32_t ringbuffer_read(ringbuffer *rb, uint8_t *target, uint32_t len);        //读取多字节
uint32_t ringbuffer_write(ringbuffer *rb, const uint8_t *source, uint32_t len); //写入多字节
uint32_t ringbuffer_len(ringbuffer *rb);                                        //获取数据长度
void ringbuffer_clear(ringbuffer *rb);                                          //清空缓冲区
uint8_t ringbuffer_is_empty(ringbuffer *rb);                                    //判断是否为空
uint8_t ringbuffer_is_full(ringbuffer *rb);                                     //判断是否已满
#endif 
