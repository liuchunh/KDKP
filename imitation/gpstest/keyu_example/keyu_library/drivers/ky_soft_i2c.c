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
 * 文件名：[ky_soft_i2c.c]
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
#include "ky_soft_i2c.h"
#include "ky_gpio.h"
#include "IfxPort.h"

#define SOFT_I2C_DELAY(x)  do { volatile uint32 i; for(i = 0; i < (x); i++) __asm("nop"); } while(0)

/***************************************************************
 *  @brief     软件I2C短延时
 *  @param     i2c  软件I2C结构体指针
 *  @return    void
 **************************************************************/
void soft_i2c_short_delay(SoftI2c_Info_t *i2c)
{
    volatile uint32 i;
    for(i = 0; i < 500; i++) __asm("nop");
}

/***************************************************************
 *  @brief     软件I2C START信号
 *  @param     i2c  软件I2C结构体指针
 *  @return    void
 **************************************************************/
void soft_i2c_start(SoftI2c_Info_t *i2c)
{
    gpio_set_level(i2c->scl_pin, GPIO_HIGH); 
    gpio_set_level(i2c->sda_pin, GPIO_HIGH); 
    SOFT_I2C_DELAY(i2c->delay);
    
    gpio_set_level(i2c->sda_pin, GPIO_LOW);  
    SOFT_I2C_DELAY(i2c->delay);
    gpio_set_level(i2c->scl_pin, GPIO_LOW);  
    SOFT_I2C_DELAY(i2c->delay);
}

/***************************************************************
 *  @brief     软件I2C STOP信号
 *  @param     i2c  软件I2C结构体指针
 *  @return    void
 **************************************************************/
void soft_i2c_stop(SoftI2c_Info_t *i2c)
{
    gpio_set_level(i2c->sda_pin, GPIO_LOW); 
    gpio_set_level(i2c->scl_pin, GPIO_LOW); 
    SOFT_I2C_DELAY(i2c->delay);
    
    gpio_set_level(i2c->scl_pin, GPIO_HIGH);
    SOFT_I2C_DELAY(i2c->delay);
    gpio_set_level(i2c->sda_pin, GPIO_HIGH);
    SOFT_I2C_DELAY(i2c->delay);
}

/***************************************************************
 *  @brief     软件I2C发送ACK/NACK信号
 *  @param     i2c  软件I2C结构体指针
 *  @param     ack  1-NACK, 0-ACK
 *  @return    void
 **************************************************************/
static void soft_i2c_send_ack(SoftI2c_Info_t *i2c, uint8 ack)
{
    gpio_set_level(i2c->scl_pin, GPIO_LOW);   
    
    if(ack)
        gpio_set_level(i2c->sda_pin, GPIO_HIGH);
    else
        gpio_set_level(i2c->sda_pin, GPIO_LOW); 
    
    SOFT_I2C_DELAY(i2c->delay);
    gpio_set_level(i2c->scl_pin, GPIO_HIGH); 
    SOFT_I2C_DELAY(i2c->delay);
    gpio_set_level(i2c->scl_pin, GPIO_LOW);  
    gpio_set_level(i2c->sda_pin, GPIO_HIGH);  
}

/***************************************************************
 *  @brief     软件I2C等待ACK信号
 *  @param     i2c  软件I2C结构体指针
 *  @return    1-NACK, 0-ACK
 **************************************************************/
uint8 soft_i2c_wait_ack(SoftI2c_Info_t *i2c)
{
    uint8 ack = 0;
    
    gpio_set_level(i2c->scl_pin, GPIO_LOW);   
    gpio_set_level(i2c->sda_pin, GPIO_HIGH);  
    gpio_set_dir(i2c->sda_pin, GPIO_DIR_IN);
    SOFT_I2C_DELAY(i2c->delay);
    gpio_set_level(i2c->scl_pin, GPIO_HIGH);
    SOFT_I2C_DELAY(i2c->delay);
    if(gpio_get_level(i2c->sda_pin) == GPIO_HIGH)
        ack = 1; 
    gpio_set_level(i2c->scl_pin, GPIO_LOW); 
    gpio_set_dir(i2c->sda_pin, GPIO_DIR_OUT);
    SOFT_I2C_DELAY(i2c->delay);
    
    return ack;
}

/***************************************************************
 *  @brief     软件I2C发送一个字节
 *  @param     i2c   软件I2C结构体指针
 *  @param     data  要发送的数据
 *  @return    1-ACK, 0-NACK
 **************************************************************/
uint8 soft_i2c_send_byte(SoftI2c_Info_t *i2c, const uint8 data)
{
    uint8 mask = 0x80;
    
    while(mask)
    {
        gpio_set_level(i2c->sda_pin, (data & mask) ? GPIO_HIGH : GPIO_LOW);
        mask >>= 1;
        
        SOFT_I2C_DELAY(i2c->delay / 2);
        gpio_set_level(i2c->scl_pin, GPIO_HIGH); 
        SOFT_I2C_DELAY(i2c->delay);
        gpio_set_level(i2c->scl_pin, GPIO_LOW);  
        SOFT_I2C_DELAY(i2c->delay / 2);
    }
    
    return (soft_i2c_wait_ack(i2c) == 0) ? 1 : 0;  
}

/***************************************************************
 *  @brief     软件I2C读取一个字节
 *  @param     i2c  软件I2C结构体指针
 *  @param     ack  1-发送NACK, 0-发送ACK
 *  @return    读取到的数据
 **************************************************************/
uint8 soft_i2c_read_byte(SoftI2c_Info_t *i2c, uint8 ack)
{
    uint8 data = 0x00;
    uint8 bits = 8;
    
    gpio_set_level(i2c->scl_pin, GPIO_LOW);  
    gpio_set_level(i2c->sda_pin, GPIO_HIGH); 
    
    gpio_set_dir(i2c->sda_pin, GPIO_DIR_IN);
    
    SOFT_I2C_DELAY(i2c->delay);
    
    while(bits--)
    {
        gpio_set_level(i2c->scl_pin, GPIO_LOW);  
        SOFT_I2C_DELAY(i2c->delay);
        gpio_set_level(i2c->scl_pin, GPIO_HIGH); 
        SOFT_I2C_DELAY(i2c->delay);
        data = ((data << 1) | gpio_get_level(i2c->sda_pin));
    }
    
    gpio_set_level(i2c->scl_pin, GPIO_LOW);
    gpio_set_dir(i2c->sda_pin, GPIO_DIR_OUT);
    SOFT_I2C_DELAY(i2c->delay);
    soft_i2c_send_ack(i2c, ack);
    return data;
}

/***************************************************************
 *  @brief     写8位寄存器
 *  @param     i2c  软件I2C结构体指针
 *  @param     reg  寄存器地址
 *  @param     data 要写入的数据
 *  @return    void
 **************************************************************/
void soft_i2c_write_reg8(SoftI2c_Info_t *i2c, const uint8 reg, const uint8 data)
{
    soft_i2c_start(i2c);
    soft_i2c_send_byte(i2c, i2c->addr << 1); 
    soft_i2c_send_byte(i2c, reg);            
    soft_i2c_send_byte(i2c, data);           
    soft_i2c_stop(i2c);
}

/***************************************************************
 *  @brief     读8位寄存器
 *  @param     i2c  软件I2C结构体指针
 *  @param     reg  寄存器地址
 *  @return    读取到的数据
 **************************************************************/
uint8 soft_i2c_read_reg8(SoftI2c_Info_t *i2c, const uint8 reg)
{
    uint8 data = 0;
    
    soft_i2c_start(i2c);
    soft_i2c_send_byte(i2c, i2c->addr << 1);      
    soft_i2c_send_byte(i2c, reg);                
    
    soft_i2c_start(i2c);                        
    soft_i2c_send_byte(i2c, (i2c->addr << 1) | 0x01);  
    data = soft_i2c_read_byte(i2c, 1);           
    soft_i2c_stop(i2c);
    
    return data;
}

/***************************************************************
 *  @brief     写16位寄存器 (高字节在前)
 *  @param     i2c  软件I2C结构体指针
 *  @param     reg  寄存器地址
 *  @param     data 要写入的16位数据
 *  @return    void
 **************************************************************/
void soft_i2c_write_reg16(SoftI2c_Info_t *i2c, const uint8 reg, const uint16 data)
{
    soft_i2c_start(i2c);
    if(soft_i2c_send_byte(i2c, i2c->addr << 1) == 0) { soft_i2c_stop(i2c); return; } 
    if(soft_i2c_send_byte(i2c, reg) == 0)             { soft_i2c_stop(i2c); return; } 
    if(soft_i2c_send_byte(i2c, (uint8)(data >> 8)) == 0) { soft_i2c_stop(i2c); return; } 
    soft_i2c_send_byte(i2c, (uint8)(data & 0xFF));   
    soft_i2c_stop(i2c);
}

/***************************************************************
 *  @brief     读16位寄存器 (高字节在前)
 *  @param     i2c  软件I2C结构体指针
 *  @param     reg  寄存器地址
 *  @return    读取到的16位数据
 **************************************************************/
uint16 soft_i2c_read_reg16(SoftI2c_Info_t *i2c, const uint8 reg)
{
    uint16 data = 0;
    
    soft_i2c_start(i2c);
    soft_i2c_send_byte(i2c, i2c->addr << 1);  
    soft_i2c_send_byte(i2c, reg);  
    soft_i2c_stop(i2c); 
    soft_i2c_short_delay(i2c);
    
    soft_i2c_start(i2c);                       
    soft_i2c_send_byte(i2c, (i2c->addr << 1) | 0x01);  
    
    data = soft_i2c_read_byte(i2c, 0);           
    data = (data << 8) | soft_i2c_read_byte(i2c, 1);  
    soft_i2c_stop(i2c);
    
    return data;
}

/***************************************************************
 *  @brief     软件I2C初始化
 *  @param     i2c      软件I2C结构体指针
 *  @param     addr     器件地址(7位)
 *  @param     delay    延时时长
 *  @param     scl_pin  SCL引脚
 *  @param     sda_pin  SDA引脚
 *  @return    void
 **************************************************************/
void soft_i2c_init(SoftI2c_Info_t *i2c, uint8 addr, uint32 delay, GPIO_Pin_t scl_pin, GPIO_Pin_t sda_pin)
{
    i2c->scl_pin = scl_pin;
    i2c->sda_pin = sda_pin;
    i2c->addr = addr;
    i2c->delay = delay;
    
    gpio_init(scl_pin, GPIO_MODE_OUT_PP, GPIO_HIGH);
    gpio_init(sda_pin, GPIO_MODE_OUT_OD, GPIO_HIGH);
    
    uint8 port_num = (scl_pin >> 8) & 0xFF;

    
    Ifx_P *port_scl = NULL_PTR;
    Ifx_P *port_sda = NULL_PTR;
    
    switch(port_num)
    {
        case 0:  port_scl = &MODULE_P00; break;
        case 2:  port_scl = &MODULE_P02; break;
        case 10: port_scl = &MODULE_P10; break;
        case 11: port_scl = &MODULE_P11; break;
        case 13: port_scl = &MODULE_P13; break;
        case 14: port_scl = &MODULE_P14; break;
        case 15: port_scl = &MODULE_P15; break;
        case 20: port_scl = &MODULE_P20; break;
        case 21: port_scl = &MODULE_P21; break;
        case 22: port_scl = &MODULE_P22; break;
        case 23: port_scl = &MODULE_P23; break;
        case 32: port_scl = &MODULE_P32; break;
        case 33: port_scl = &MODULE_P33; break;
        default: break;
    }
    
    port_num = (sda_pin >> 8) & 0xFF;
    switch(port_num)
    {
        case 0:  port_sda = &MODULE_P00; break;
        case 2:  port_sda = &MODULE_P02; break;
        case 10: port_sda = &MODULE_P10; break;
        case 11: port_sda = &MODULE_P11; break;
        case 13: port_sda = &MODULE_P13; break;
        case 14: port_sda = &MODULE_P14; break;
        case 15: port_sda = &MODULE_P15; break;
        case 20: port_sda = &MODULE_P20; break;
        case 21: port_sda = &MODULE_P21; break;
        case 22: port_sda = &MODULE_P22; break;
        case 23: port_sda = &MODULE_P23; break;
        case 32: port_sda = &MODULE_P32; break;
        case 33: port_sda = &MODULE_P33; break;
        default: break;
    }
    
    if(port_scl != NULL_PTR)
    {
        uint8 scl_pin_num = scl_pin & 0xFF;
        IfxPort_setPinMode(port_scl, scl_pin_num, IfxPort_Mode_outputPushPullGeneral);
        IfxPort_setPinPadDriver(port_scl, scl_pin_num, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }
    
    if(port_sda != NULL_PTR)
    {
        uint8 sda_pin_num = sda_pin & 0xFF;
        IfxPort_setPinMode(port_sda, sda_pin_num, IfxPort_Mode_outputOpenDrainGeneral);
        IfxPort_setPinPadDriver(port_sda, sda_pin_num, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }
}
