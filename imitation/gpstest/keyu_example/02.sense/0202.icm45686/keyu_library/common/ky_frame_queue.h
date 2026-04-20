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
 * 文件名：[ky_frame_queue.h]
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
#ifndef KEYU_UTILS_FRAME_QUEUE_H
#define KEYU_UTILS_FRAME_QUEUE_H

#include "ky_typedef.h" 

// 队列句柄结构体
typedef struct {
    uint8_t*   pBufferPool;    // 指向外部缓冲区的指针
    uint16_t   maxFrameSize;   // 单帧最大长度 (Buffer列数)
    uint16_t   frameCount;     // 队列容量 (Buffer行数)
    
    volatile uint16_t head;   
    volatile uint16_t tail;
    uint16_t   currentRxLen;   // 当前正在接收的帧已存入字节数
} FrameQueue_t;
void frame_queue_init(FrameQueue_t* pQueue, void* pBufferPool, uint16_t maxFrameSize, uint16_t frameCount);  //帧队列初始化
void frame_queue_push_byte(FrameQueue_t* pQueue, uint8_t byte, uint8_t delimiter);                           //推入字节
char* frame_queue_peek_latest(FrameQueue_t* pQueue);                                                        //查看最新帧
char* frame_queue_peek_oldest(FrameQueue_t* pQueue);                                                        //查看最旧帧
void frame_queue_pop(FrameQueue_t* pQueue);                                                                 //弹出帧
uint8_t frame_queue_is_empty(FrameQueue_t* pQueue);                                                         //判断队列是否为空
#endif