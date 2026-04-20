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
 * 文件名：[ky_gpio.c]
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
#include "ky_gpio.h"

/***************************************************************
  *  @brief     根据端口号获取端口模块指针
  *  @param     portNum    端口号
  *  @return    Ifx_P*     端口模块指针
  **************************************************************/
static Ifx_P* get_port_module(uint8 portNum)
{
    switch(portNum)
    {
        case 0x00: return &MODULE_P00;
        case 0x02: return &MODULE_P02;
        case 0x10: return &MODULE_P10;
        case 0x11: return &MODULE_P11;
        case 0x13: return &MODULE_P13;
        case 0x14: return &MODULE_P14;
        case 0x15: return &MODULE_P15;
        case 0x20: return &MODULE_P20;
        case 0x21: return &MODULE_P21;
        case 0x22: return &MODULE_P22;
        case 0x23: return &MODULE_P23;
        case 0x32: return &MODULE_P32;
        case 0x33: return &MODULE_P33;
        case 0x40: return &MODULE_P40;
        default: return NULL;
    }
}

/***************************************************************
  *  @brief     从 GPIO_Pin_t 枚举值中提取端口号和引脚号
  *  @param     pin        GPIO引脚枚举值
  *  @param     portNum    输出参数：端口号
  *  @param     pinNum     输出参数：引脚号
  **************************************************************/
static void extract_port_pin(GPIO_Pin_t pin, uint8 *portNum, uint8 *pinNum)
{
    *portNum = (uint8)((pin >> 8) & 0xFF);
    *pinNum = (uint8)(pin & 0xFF);
}

/***************************************************************
  *  @brief     将 GPIO_Mode_t 转换为 iLLD 的 IfxPort_Mode
  *  @param     mode    GPIO模式
  *  @return    IfxPort_Mode    iLLD端口模式
  **************************************************************/
static IfxPort_Mode convert_gpio_mode(GPIO_Mode_t mode)
{
    switch(mode)
    {
        case GPIO_MODE_IN_FLOATING:
            return IfxPort_Mode_inputNoPullDevice;
        case GPIO_MODE_IN_PD:
            return IfxPort_Mode_inputPullDown;
        case GPIO_MODE_IN_PU:
            return IfxPort_Mode_inputPullUp;
        case GPIO_MODE_OUT_PP:
            return IfxPort_Mode_outputPushPullGeneral;
        case GPIO_MODE_OUT_OD:
            return IfxPort_Mode_outputOpenDrainGeneral;
        case GPIO_MODE_AF_PP:
            return IfxPort_Mode_outputPushPullAlt1; 
        case GPIO_MODE_AF_OD:
            return IfxPort_Mode_outputOpenDrainAlt1; 
        default:
            return IfxPort_Mode_inputNoPullDevice;
    }
}

/***************************************************************
  *  @brief     将 GPIO_Speed_t 转换为 iLLD 的 IfxPort_PadDriver
  *  @param     speed    GPIO速度
  *  @return    IfxPort_PadDriver    iLLD Pad驱动模式
  **************************************************************/
static IfxPort_PadDriver convert_gpio_speed(GPIO_Speed_t speed)
{
    switch(speed)
    {
        case GPIO_SPEED_FAST:
            return IfxPort_PadDriver_cmosAutomotiveSpeed1;
        case GPIO_SPEED_SECOND:
            return IfxPort_PadDriver_cmosAutomotiveSpeed2;
        case GPIO_SPEED_TIHRD:
            return IfxPort_PadDriver_cmosAutomotiveSpeed3;
        case GPIO_SPEED_LOW:
            return IfxPort_PadDriver_cmosAutomotiveSpeed4;
        default:
            return IfxPort_PadDriver_cmosAutomotiveSpeed1;
    }
}

/***************************************************************
  *  @brief     初始化一个GPIO引脚
  *  @param     pin     指定的GPIO引脚, 例如: GPIO_P10
  *  @param     mode    要配置的GPIO模式, 来自 GPIO_Mode_t 枚举
  *  @param     level   初始化输出电平
  *  @Sample usage:     gpio_init(GPIO_P10_1, GPIO_MODE_OUT_PP, GPIO_HIGH);
 **************************************************************/
void gpio_init(GPIO_Pin_t pin, GPIO_Mode_t mode, GPIO_Level_t level)
{
    uint8 portNum, pinNum;
    Ifx_P *port;
    
    extract_port_pin(pin, &portNum, &pinNum);
    port = get_port_module(portNum);
    
    if(port == NULL)
    {
        return; 
    }
    
    IfxPort_Mode portMode = convert_gpio_mode(mode);
    IfxPort_setPinMode(port, pinNum, portMode);

    if(mode < GPIO_MODE_OUT_PP) 
    {
        if(portNum == 0 || portNum == 13 || portNum == 14 || portNum == 15 || 
           portNum == 20 || portNum == 21 || portNum == 22 || portNum == 23 ||
           portNum == 32 || portNum == 33 || portNum == 40 || portNum == 41 || portNum == 2)
        {

             if(portNum == 0) MODULE_P00.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 2) MODULE_P02.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 13) MODULE_P13.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 14) MODULE_P14.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 15) MODULE_P15.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 20) MODULE_P20.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 21) MODULE_P21.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 22) MODULE_P22.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 23) MODULE_P23.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 32) MODULE_P32.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 33) MODULE_P33.PDISC.U &= ~(1 << pinNum);
             else if(portNum == 40) MODULE_P40.PDISC.U &= ~(1 << pinNum);
        }
    }

    if(mode >= GPIO_MODE_OUT_PP)
    {
        if(level == GPIO_HIGH)
        {
            IfxPort_setPinHigh(port, pinNum);
        }
        else
        {
            IfxPort_setPinLow(port, pinNum);
        }
        
        IfxPort_setPinPadDriver(port, pinNum, IfxPort_PadDriver_cmosAutomotiveSpeed2);
    }
}

/***************************************************************
  *  @brief     设置指定GPIO引脚的输出电平
  *  @param     pin     指定的GPIO引脚, 例如: GPIO_P12
  *  @param     level   要设置的电平 (GPIO_HIGH 或 GPIO_LOW)
  *  @Sample usage:     gpio_set_level(GPIO_P10_1, GPIO_HIGH);
 **************************************************************/
void gpio_set_level(GPIO_Pin_t pin, GPIO_Level_t level)
{
    uint8 portNum, pinNum;
    Ifx_P *port;
    
    extract_port_pin(pin, &portNum, &pinNum);
    port = get_port_module(portNum);
    
    if(port == NULL)
    {
        return; 
    }
    
    if(level == GPIO_HIGH)
    {
        IfxPort_setPinHigh(port, pinNum);
    }
    else
    {
        IfxPort_setPinLow(port, pinNum);
    }
}

/***************************************************************
  *  @brief     翻转指定GPIO引脚的输出电平
  *  @param     pin     指定的GPIO引脚, 例如: GPIO_P12
  *  @Sample usage:     gpio_toggle_level(GPIO_P10_1);
 **************************************************************/
void gpio_toggle_level(GPIO_Pin_t pin)
{
    uint8 portNum, pinNum;
    Ifx_P *port;
    
    extract_port_pin(pin, &portNum, &pinNum);
    port = get_port_module(portNum);
    
    if(port == NULL)
    {
        return; 
    }
    
    IfxPort_togglePin(port, pinNum);
}

/***************************************************************
  *  @brief     获取指定GPIO引脚的当前输入电平
  *  @param     pin     指定的GPIO引脚, 例如: GPIO_P30
  *  @return    GPIO_Level_t
  *  @Sample usage:     if(gpio_get_level(GPIO_P10_1) == GPIO_HIGH) 
 **************************************************************/
GPIO_Level_t gpio_get_level(GPIO_Pin_t pin)
{
    uint8 portNum, pinNum;
    Ifx_P *port;
    
    extract_port_pin(pin, &portNum, &pinNum);
    port = get_port_module(portNum);
    
    if(port == NULL)
    {
        return GPIO_LOW; 
    }
    
    boolean state = IfxPort_getPinState(port, pinNum);
    return (state == TRUE) ? GPIO_HIGH : GPIO_LOW;
}

/***************************************************************
  *  @brief     设置指定GPIO引脚的输出转换速率
  *  @param     pin     指定的GPIO引脚, 例如: GPIO_P54
  *  @param     speed   要设置的转换速率
  *  @Sample usage:     gpio_set_speed(GPIO_P10_1, GPIO_SPEED_LOW);
 **************************************************************/
void gpio_set_speed(GPIO_Pin_t pin, GPIO_Speed_t speed)
{
    uint8 portNum, pinNum;
    Ifx_P *port;
    
    extract_port_pin(pin, &portNum, &pinNum);
    port = get_port_module(portNum);
    
    if(port == NULL)
    {
        return;
    }
    
    IfxPort_PadDriver padDriver = convert_gpio_speed(speed);
    IfxPort_setPinPadDriver(port, pinNum, padDriver);
}

/***************************************************************
  *  @brief     设置GPIO引脚方向 (输入/输出)
  *  @param     pin     指定的GPIO引脚
  *  *  @param     dir     方向 (GPIO_DIR_IN 或 GPIO_DIR_OUT)
  *  @Sample usage:     gpio_set_dir(GPIO_P02_2, GPIO_DIR_IN);
 **************************************************************/
void gpio_set_dir(GPIO_Pin_t pin, GPIO_Dir_t dir)
{
    uint8 portNum, pinNum;
    Ifx_P *port;
    
    extract_port_pin(pin, &portNum, &pinNum);
    port = get_port_module(portNum);
    
    if(port == NULL)
    {
        return;
    }
    
    if(dir == GPIO_DIR_IN)
    {
        IfxPort_setPinMode(port, pinNum, IfxPort_Mode_inputNoPullDevice);
    }
    else
    {
        IfxPort_setPinMode(port, pinNum, IfxPort_Mode_outputOpenDrainGeneral);
    }
}

