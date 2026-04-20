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
 * 文件名：[ky_eeprom.h]
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
#ifndef __KY_EEPROM_H__
#define __KY_EEPROM_H__

#include "ky_typedef.h"
#include "IfxFlash.h"

#define EEPROM_BASE_ADDR    0xAF000000    // TC264 Data Flash 0 起始地址
#define EEPROM_SECTOR_SIZE  0x2000        // 扇区大小 8KB
#define EEPROM_PAGE_SIZE    8             // 页大小 8字节 (64-bit)

void eeprom_init(void);                                                     //EEPROM初始化
uint8_t eeprom_read_byte(uint32_t addr);                                    //读取单字节
void eeprom_write_byte(uint32_t addr, uint8_t dat);                         //写入单字节
void eeprom_read_buffer(uint32_t addr, uint8_t *dat, uint16_t length);      //读取缓冲区
void eeprom_write_buffer(uint32_t addr, uint8_t *buffer, uint16_t length);  //写入缓冲区
void eeprom_erase_sector(uint32_t addr);                                    //擦除扇区
uint8_t eeprom_get_status(void);                                            //获取状态
#endif /* __KY_EEPROM_H__ */