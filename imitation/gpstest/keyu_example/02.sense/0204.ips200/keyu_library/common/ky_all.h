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
 * 文件名：[ky_all.h]
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
#ifndef KEYU_LIB_INCLUDE_CORE_KEYU_LIB_H_
#define KEYU_LIB_INCLUDE_CORE_KEYU_LIB_H_

// Keyu 二次开发库
// ke_yu_common (系统通用功能)
#include "ky_typedef.h"
#include "ky_math.h"
#include "ky_frame_queue.h"
#include "ky_utils.h"

// ke_yu_core (核心功能)
#include "ky_sys_clock.h"

// ke_yu_drivers (驱动层)
#include "ky_delay.h"
#include "ky_gpio.h"
#include "ky_pwm.h"
#include "ky_key.h"
#include "ky_uart.h"
#include "ky_spi.h"
#include "ky_i2c.h"
#include "ky_soft_i2c.h"
#include "ky_adc.h"
#include "ky_dma.h"
#include "ky_encoder.h"
#include "ky_eeprom.h"
#include "ky_timer.h"

// ke_yu_devices (设备层)
#include "ky_ips.h"
#include "ky_icm45686.h"
#include "ky_lsm6der.h"
#include "ky_tau1202.h"
#include "ky_font.h"
#include "ky_imu_base.h"
#include "ky_mt9v034.h"
#include "ky_vi5300.h"
// App

// User

// 英飞凌 iLLD 基础库头文件
// CPU 核心模块
#include "Cpu/Std/Ifx_Types.h"
#include "Cpu/Std/IfxCpu.h"
#include "Cpu/Irq/IfxCpu_Irq.h"
#include "Cpu/Trap/IfxCpu_Trap.h"
// 系统控制单元 (SCU)
#include "Scu/Std/IfxScuCcu.h"
#include "Scu/Std/IfxScuWdt.h"
#include "Scu/Std/IfxScuEru.h"
// 端口/GPIO
#include "Port/Std/IfxPort.h"
#include "Port/Io/IfxPort_Io.h"
// 系统定时器 (STM)
#include "Stm/Std/IfxStm.h"
#include "Stm/Timer/IfxStm_Timer.h"
// 中断源控制 (SRC)
#include "Src/Std/IfxSrc.h"
// DMA
#include "Dma/Std/IfxDma.h"
#include "Dma/Dma/IfxDma_Dma.h"
// 异步串行通信接口 (ASCLIN) - UART/SPI/LIN
#include "Asclin/Std/IfxAsclin.h"
#include "Asclin/Asc/IfxAsclin_Asc.h"
#include "Asclin/Spi/IfxAsclin_Spi.h"
#include "Asclin/Lin/IfxAsclin_Lin.h"
// CAN 总线 (MultiCAN)
#include "Multican/Std/IfxMultican.h"
#include "Multican/Can/IfxMultican_Can.h"
// I2C
#include "I2c/Std/IfxI2c.h"
#include "I2c/I2c/IfxI2c_I2c.h"
// QSPI
#include "Qspi/Std/IfxQspi.h"
#include "Qspi/SpiMaster/IfxQspi_SpiMaster.h"
#include "Qspi/SpiSlave/IfxQspi_SpiSlave.h"
// ADC (VADC)
#include "Vadc/Std/IfxVadc.h"
#include "Vadc/Adc/IfxVadc_Adc.h"
// 差分信号ADC (DSADC)
#include "Dsadc/Std/IfxDsadc.h"
#include "Dsadc/Dsadc/IfxDsadc_Dsadc.h"
#include "Dsadc/Rdc/IfxDsadc_Rdc.h"
// CCU6 (捕获比较单元) - PWM/定时器
#include "Ccu6/Std/IfxCcu6.h"
#include "Ccu6/Icu/IfxCcu6_Icu.h"
#include "Ccu6/PwmBc/IfxCcu6_PwmBc.h"
#include "Ccu6/PwmHl/IfxCcu6_PwmHl.h"
#include "Ccu6/Timer/IfxCcu6_Timer.h"
#include "Ccu6/TimerWithTrigger/IfxCcu6_TimerWithTrigger.h"
#include "Ccu6/TPwm/IfxCcu6_TPwm.h"
// GPT12 (通用定时器)
#include "Gpt12/Std/IfxGpt12.h"
#include "Gpt12/IncrEnc/IfxGpt12_IncrEnc.h"
// GTM (通用定时器模块)
#include "Gtm/Std/IfxGtm.h"
#include "Gtm/Std/IfxGtm_Atom.h"
#include "Gtm/Std/IfxGtm_Tim.h"
#include "Gtm/Std/IfxGtm_Tom.h"
#include "Gtm/Std/IfxGtm_Tbu.h"
#include "Gtm/Std/IfxGtm_Cmu.h"
#include "Gtm/Std/IfxGtm_Dpll.h"
#include "Gtm/Atom/Pwm/IfxGtm_Atom_Pwm.h"
#include "Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.h"
#include "Gtm/Atom/Timer/IfxGtm_Atom_Timer.h"
#include "Gtm/Tom/Pwm/IfxGtm_Tom_Pwm.h"
#include "Gtm/Tom/PwmHl/IfxGtm_Tom_PwmHl.h"
#include "Gtm/Tom/Timer/IfxGtm_Tom_Timer.h"
#include "Gtm/Tim/In/IfxGtm_Tim_In.h"
#include "Gtm/Trig/IfxGtm_Trig.h"
// Flash
#include "Flash/Std/IfxFlash.h"
// E-Ray (FlexRay)
#include "Eray/Std/IfxEray.h"
#include "Eray/Eray/IfxEray_Eray.h"
// Ethernet
#include "Eth/Std/IfxEth.h"
#include "Eth/Phy_Pef7071/IfxEth_Phy_Pef7071.h"
// PSI5/PSI5S
#include "Psi5/Std/IfxPsi5.h"
#include "Psi5/Psi5/IfxPsi5_Psi5.h"
#include "Psi5s/Std/IfxPsi5s.h"
#include "Psi5s/Psi5s/IfxPsi5s_Psi5s.h"
// SENT (单边半字节传输)
#include "Sent/Std/IfxSent.h"
#include "Sent/Sent/IfxSent_Sent.h"
// MSC (多通道串行接口)
#include "Msc/Std/IfxMsc.h"
#include "Msc/Msc/IfxMsc_Msc.h"
// HSSL (高速串行链路)
#include "Hssl/Std/IfxHssl.h"
#include "Hssl/Hssl/IfxHssl_Hssl.h"
// IOM (输入输出多路复用器)
#include "Iom/Std/IfxIom.h"
#include "Iom/Driver/IfxIom_Driver.h"
// CIF (摄像头接口)
#include "Cif/Std/IfxCif.h"
#include "Cif/Cam/IfxCif_Cam.h"
// FCE (快速CRC引擎)
#include "Fce/Std/IfxFce.h"
#include "Fce/Crc/IfxFce_Crc.h"
// FFT (快速傅里叶变换)
#include "Fft/Std/IfxFft.h"
#include "Fft/Fft/IfxFft_Fft.h"
// DTS (数字温度传感器)
#include "Dts/Std/IfxDts.h"
#include "Dts/Dts/IfxDts_Dts.h"
// EMEM (扩展内存)
#include "Emem/Std/IfxEmem.h"
// MTU (内存测试单元)
#include "Mtu/Std/IfxMtu.h"
// SMU (安全监控单元)
#include "Smu/Std/IfxSmu.h"
// PinMap (引脚映射)
#include "_PinMap/Ifx_PinMap.h"
#include "_PinMap/IfxPort_PinMap.h"
#include "_PinMap/IfxAsclin_PinMap.h"
#include "_PinMap/IfxCcu6_PinMap.h"
#include "_PinMap/IfxMultican_PinMap.h"
#include "_PinMap/IfxI2c_PinMap.h"
#include "_PinMap/IfxQspi_PinMap.h"
#include "_PinMap/IfxVadc_PinMap.h"
#include "_PinMap/IfxGtm_PinMap.h"
#include "_PinMap/IfxGpt12_PinMap.h"
#include "_PinMap/IfxSent_PinMap.h"
#include "_PinMap/IfxScu_PinMap.h"
// 数据结构和工具库
#include "_Lib/DataHandling/Ifx_Fifo.h"
#include "_Lib/DataHandling/Ifx_CircularBuffer.h"
#include "_Lib/InternalMux/Ifx_InternalMux.h"

#endif /* KEYU_LIB_INCLUDE_CORE_KEYU_LIB_H_ */
