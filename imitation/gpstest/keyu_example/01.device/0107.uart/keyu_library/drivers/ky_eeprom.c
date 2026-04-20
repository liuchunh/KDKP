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
 * 文件名：[ky_eeprom.c]
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
#include "ky_eeprom.h"
#include "IfxFlash.h"
#include "Cpu/Std/IfxCpu.h"
#include "string.h"


/***************************************************************
  *  @brief     初始化EEPROM模块
  *  @param     None
  *  @Sample usage:     eeprom_init();
 **************************************************************/
void eeprom_init(void)
{
    IfxFlash_clearStatus(0);
}

/***************************************************************
  *  @brief     从EEPROM中读取一个字节
  *  @param     addr    偏移地址
  *  @return    uint8_t 数据
 **************************************************************/
uint8_t eeprom_read_byte(uint32_t addr)
{
    return *(volatile uint8_t *)(EEPROM_BASE_ADDR + addr);
}

/***************************************************************
  *  @brief     从EEPROM中读取多个字节
  *  @param     addr    偏移地址
  *  @param     buffer  接收缓冲区
  *  @param     length  长度
 **************************************************************/
void eeprom_read_buffer(uint32_t addr, uint8_t *buffer, uint16_t length)
{
    uint8_t *src = (uint8_t *)(EEPROM_BASE_ADDR + addr);
    while (length--)
    {
        *buffer++ = *src++;
    }
}

/***************************************************************
  *  @brief     向EEPROM写入多个字节
  *  @param     addr    偏移地址
  *  @param     buffer  数据缓冲区
  *  @param     length  长度
  *  @note      按Page(8字节)进行 Read-Modify-Write
 **************************************************************/
void eeprom_write_buffer(uint32_t addr, uint8_t *buffer, uint16_t length)
{
    while (length > 0)
    {
        uint32_t pageAddr = (EEPROM_BASE_ADDR + addr) & ~(EEPROM_PAGE_SIZE - 1);
        uint32_t offset = (EEPROM_BASE_ADDR + addr) & (EEPROM_PAGE_SIZE - 1);
        uint32_t bytesToWrite = EEPROM_PAGE_SIZE - offset;
        
        if (bytesToWrite > length)
        {
            bytesToWrite = length;
        }
        
        uint32_t wordL = *(volatile uint32_t*)(pageAddr);
        uint32_t wordU = *(volatile uint32_t*)(pageAddr + 4);
        
        union {
            uint32_t words[2];
            uint8_t bytes[8];
        } pageData;
        
        pageData.words[0] = wordL;
        pageData.words[1] = wordU;

        uint32_t i;
        for (i = 0; i < bytesToWrite; i++)
        {
            pageData.bytes[offset + i] = *buffer++;
        }
        
        boolean interruptState = IfxCpu_disableInterrupts();
        
        IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);
        IfxFlash_clearStatus(0);
        IfxFlash_enterPageMode(pageAddr);
        IfxFlash_loadPage2X32(pageAddr, pageData.words[0], pageData.words[1]);
        IfxFlash_writePage(pageAddr);
        IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);
        
        IfxCpu_restoreInterrupts(interruptState);
        
        addr += bytesToWrite;
        length -= bytesToWrite;
    }
}

/***************************************************************
  *  @brief     向EEPROM写入一个字节
  *  @param     addr    偏移地址
  *  @param     dat     数据
  *  @Sample usage:     eeprom_write_byte(0x00, 0x55);
 **************************************************************/
void eeprom_write_byte(uint32_t addr, uint8_t dat)
{
    eeprom_write_buffer(addr, &dat, 1);
}

/***************************************************************
  *  @brief     擦除EEPROM扇区
  *  @param     addr    偏移地址 (该地址所在的扇区将被擦除)
  *  @Sample usage:     eeprom_erase_sector(0x00);
 **************************************************************/
void eeprom_erase_sector(uint32_t addr)
{
    uint32_t sectorAddr = (EEPROM_BASE_ADDR + addr) & ~(EEPROM_SECTOR_SIZE - 1);
    
    boolean interruptState = IfxCpu_disableInterrupts();
    
    IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);
    IfxFlash_clearStatus(0);
    IfxFlash_eraseSector(sectorAddr);
    IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);
    
    IfxCpu_restoreInterrupts(interruptState);
}

/***************************************************************
  *  @brief     获取EEPROM状态
  *  @param     None
  *  @return    uint8_t 0: 正常; 1: 错误
 **************************************************************/
uint8_t eeprom_get_status(void)
{
    if (FLASH0_FSR.U & 0x000E3000) 
    {
        return 1;
    }
    return 0;
}
