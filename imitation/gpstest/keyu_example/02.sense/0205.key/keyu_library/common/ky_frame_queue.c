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
 * 文件名：[ky_frame_queue.c]
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
#include "ky_frame_queue.h"
#include <string.h>

/***************************************************************
  *  @brief     初始化帧队列
  *  @param     pQueue          指向帧队列控制结构的指针
  *  @param     pBufferPool     指向用于存储数据的缓冲池的指针
  *  @param     maxFrameSize    单个数据帧的最大字节大小
  *  @param     frameCount      缓冲区中可容纳的最大帧数
  *  @Sample usage:     frame_queue_init(&myQueue, buffer, 64, 10);
 **************************************************************/
void frame_queue_init(FrameQueue_t* pQueue, void* pBufferPool, uint16_t maxFrameSize, uint16_t frameCount)
{
    pQueue->pBufferPool  = (uint8_t*)pBufferPool;
    pQueue->maxFrameSize = maxFrameSize;
    pQueue->frameCount   = frameCount;
    pQueue->head         = 0;
    pQueue->tail         = 0;
    pQueue->currentRxLen = 0;
}

/***************************************************************
  *  @brief     向帧队列中压入一个字节
  *  @param     pQueue      指向帧队列控制结构的指针
  *  @param     byte        要压入的字节数据
  *  @param     delimiter   帧结束分隔符（若为0x00，则填满最大长度时分帧）
  *  @Sample usage:     frame_queue_push_byte(&myQueue, rxByte, 0x0A);
 **************************************************************/
void frame_queue_push_byte(FrameQueue_t* pQueue, uint8_t byte, uint8_t delimiter)
{
    uint16_t nextHead;
    uint8_t isFrameEnd = 0;
    uint32_t offset = (uint32_t)pQueue->head * pQueue->maxFrameSize + pQueue->currentRxLen;
    
    if (pQueue->currentRxLen < pQueue->maxFrameSize - 1)
    {
        pQueue->pBufferPool[offset] = byte;
        pQueue->currentRxLen++;
        
        if (delimiter != 0x00) {
            if (byte == delimiter) isFrameEnd = 1;
        } else { 
            if (pQueue->currentRxLen >= pQueue->maxFrameSize - 1) isFrameEnd = 1;
        }

        if (isFrameEnd)
        {
            pQueue->pBufferPool[offset + 1] = '\0';
            
            pQueue->currentRxLen = 0;
            
            nextHead = (pQueue->head + 1) % pQueue->frameCount;
            
            if (nextHead == pQueue->tail)
            {
                pQueue->tail = (pQueue->tail + 1) % pQueue->frameCount;
            }
            
            pQueue->head = nextHead;
        }
    }
    else
    {
        pQueue->currentRxLen = 0;
    }
}

/***************************************************************
  *  @brief     查看队列中最新的一帧数据
  *  @param     pQueue      指向帧队列控制结构的指针
  *  @return    char*       返回指向最新帧数据的指针，若队列为空则返回NULL
  *  @Sample usage:     char* pData = frame_queue_peek_latest(&myQueue);
 **************************************************************/
char* frame_queue_peek_latest(FrameQueue_t* pQueue)
{
    uint16_t count;
    uint32_t offset;
    if (pQueue->head == pQueue->tail) return NULL;

    count = (pQueue->head + pQueue->frameCount - pQueue->tail) % pQueue->frameCount;

    if (count > 1)
    {
        pQueue->tail = (pQueue->head + pQueue->frameCount - 1) % pQueue->frameCount;
    }

    offset = (uint32_t)pQueue->tail * pQueue->maxFrameSize;
    return (char*)&pQueue->pBufferPool[offset];
}

/***************************************************************
  *  @brief     查看队列中最旧的一帧数据
  *  @param     pQueue      指向帧队列控制结构的指针
  *  @return    char*       返回指向最旧帧数据的指针，若队列为空则返回NULL
  *  @Sample usage:     char* pData = frame_queue_peek_oldest(&myQueue);
 **************************************************************/
char* frame_queue_peek_oldest(FrameQueue_t* pQueue)
{
    uint32_t offset;
    if (pQueue->head == pQueue->tail) return NULL;

    offset = (uint32_t)pQueue->tail * pQueue->maxFrameSize;
    return (char*)&pQueue->pBufferPool[offset];
}

/***************************************************************
  *  @brief     从队列中弹出一帧数据（丢弃最旧的一帧）
  *  @param     pQueue      指向帧队列控制结构的指针
  *  @Sample usage:     frame_queue_pop(&myQueue);
 **************************************************************/
void frame_queue_pop(FrameQueue_t* pQueue)
{
    if (pQueue->head != pQueue->tail)
    {
        pQueue->tail = (pQueue->tail + 1) % pQueue->frameCount;
    }
}

/***************************************************************
  *  @brief     检查队列是否为空
  *  @param     pQueue      指向帧队列控制结构的指针
  *  @return    uint8_t     如果队列为空返回1，否则返回0
  *  @Sample usage:     if (frame_queue_is_empty(&myQueue)) { ... }
 **************************************************************/
uint8_t frame_queue_is_empty(FrameQueue_t* pQueue)
{
    return (pQueue->head == pQueue->tail);
}
