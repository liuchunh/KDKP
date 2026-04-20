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
 * 文件名：[ky_vi5300.c]
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
#include "ky_vi5300.h"
#include "ky_config.h"

#include "ky_soft_i2c.h"
#include "ky_delay.h"
#include <stdio.h>
#include <string.h>

#define vi5300_device_addr          0xD8

#define vi5300_reg_mcu_cfg          0x00
#define vi5300_ret_int_status       0x03
#define vi5300_reg_sys_cfg          0x01
#define vi5300_reg_pw_ctrl          0x07
#define vi5300_reg_cmd              0x0a
#define vi5300_reg_size             0x0b
#define vi5300_reg_scratch_pad_base 0x0c 

#define vi5300_writefw_cmd          0x03
#define vi5300_user_cfg_cmd         0x09
#define vi5300_start_rang_cmd       0x0E

#define vi5300_check_ret(a)   if(a != vi5300_ok) return a

typedef struct 
{
    int16_t millimeter;     
    uint32_t peak;          
    uint16_t noise;         
    uint32_t confidence;    
} vi5300_internal_dist_t;

typedef struct 
{
    int8_t xtalk_cal;
    uint16_t xtalk_peak;
} vi5300_xtalk__calib__data;

typedef struct
{
    int16_t offset_cal;
} vi5300_offset__calib__data;

static SoftI2c_Info_t g_vi5300_i2c; 
static uint8_t chip_reg = 0;

static vi5300_xtalk__calib__data xtalk_cal; 



/***************************************************************
  *  @brief     VI5300延时毫秒
  *  @param     ms     [ms description]
  *  @Sample usage:     vi5300_delay_ms(ms);
 **************************************************************/
static void vi5300_delay_ms(uint32_t ms)
{
    delay_ms((unsigned short)ms);
}

/***************************************************************
  *  @brief     I2CWRITEXBYTES
  *  @param     startaddr     [startaddr description]
  *  @param     buf     [buf description]
  *  @param     len     [len description]
  *  @Sample usage:     I2C_WriteXBytes(startaddr, buf, len);
 **************************************************************/
static vi5300_status_t I2C_WriteXBytes(uint8_t startaddr, const uint8_t *buf, uint8_t len)
{
    soft_i2c_start(&g_vi5300_i2c);
    if (!soft_i2c_send_byte(&g_vi5300_i2c, vi5300_device_addr)) { soft_i2c_stop(&g_vi5300_i2c); return vi5300_error; }
    if (!soft_i2c_send_byte(&g_vi5300_i2c, startaddr)) { soft_i2c_stop(&g_vi5300_i2c); return vi5300_error; }
    for (uint8_t i = 0; i < len; i++) {
        if (!soft_i2c_send_byte(&g_vi5300_i2c, (uint8_t)buf[i])) { soft_i2c_stop(&g_vi5300_i2c); return vi5300_error; }
    }
    soft_i2c_stop(&g_vi5300_i2c);
    return vi5300_ok;
}

/***************************************************************
  *  @brief     I2CREADXBYTES
  *  @param     startaddr     [startaddr description]
  *  @param     buf     [buf description]
  *  @param     len     [len description]
  *  @Sample usage:     I2C_ReadXBytes(startaddr, buf, len);
 **************************************************************/
static vi5300_status_t I2C_ReadXBytes(uint8_t startaddr, uint8_t *buf, uint8_t len)
{
    soft_i2c_start(&g_vi5300_i2c);
    if (!soft_i2c_send_byte(&g_vi5300_i2c, vi5300_device_addr)) { soft_i2c_stop(&g_vi5300_i2c); return vi5300_error; }
    if (!soft_i2c_send_byte(&g_vi5300_i2c, startaddr)) { soft_i2c_stop(&g_vi5300_i2c); return vi5300_error; }
    soft_i2c_start(&g_vi5300_i2c); 
    if (!soft_i2c_send_byte(&g_vi5300_i2c, vi5300_device_addr | 0x01)) { soft_i2c_stop(&g_vi5300_i2c); return vi5300_error; }
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = soft_i2c_read_byte(&g_vi5300_i2c, (i == len - 1) ? 1 : 0);
    }
    soft_i2c_stop(&g_vi5300_i2c);
    return vi5300_ok;
}

static vi5300_status_t WriteOneReg(uint8_t addr, uint8_t value) { return I2C_WriteXBytes(addr, &value, 1); }
static vi5300_status_t ReadOneReg(uint8_t addr, uint8_t *value) { return I2C_ReadXBytes(addr, value, 1); }

/***************************************************************
  *  @brief     VI5300设置DIGITALCLOCKDUTYCYCLE
  *  @param     None
  *  @Sample usage:     vi5300_set__digital__clock__dutycycle();
 **************************************************************/
static vi5300_status_t vi5300_set__digital__clock__dutycycle(void)
{
    vi5300_status_t ret = vi5300_ok;
    ret = WriteOneReg(vi5300_reg_pw_ctrl, 0x0F);
    ret = WriteOneReg(vi5300_reg_pw_ctrl, 0x0E);
    vi5300_check_ret(ret);
    if(chip_reg==0x00)
    {
        vi5300_delay_ms(1);
        ret = WriteOneReg(0xE9, 0x24);
        vi5300_check_ret(ret);
        ret = WriteOneReg(0xEE, 0x00);
        vi5300_check_ret(ret);
        ret = WriteOneReg(0xF5, 0x00);
        vi5300_check_ret(ret);
    }
    vi5300_delay_ms(4);
    return ret;  
}

/***************************************************************
  *  @brief     VI5300写入FIRMWAREPRE配置
  *  @param     None
  *  @Sample usage:     vi5300_write_firmware_pre_config();
 **************************************************************/
static vi5300_status_t vi5300_write_firmware_pre_config(void)
{
    vi5300_status_t ret = vi5300_ok;
    uint8_t reg_sys_cfg = 0;

    vi5300_set__digital__clock__dutycycle();       
    
    ret = WriteOneReg(vi5300_reg_pw_ctrl, 0x08); vi5300_check_ret(ret);  
    ret = WriteOneReg(vi5300_reg_pw_ctrl, 0x0a); vi5300_check_ret(ret);  
    ret = WriteOneReg(vi5300_reg_mcu_cfg, 0x06); vi5300_check_ret(ret);  
    ret = ReadOneReg (vi5300_reg_sys_cfg, &reg_sys_cfg); vi5300_check_ret(ret);  
    ret = WriteOneReg(vi5300_reg_sys_cfg, reg_sys_cfg | (0x01<<0)); vi5300_check_ret(ret);
    if(chip_reg==0x00)
    {
        ret = WriteOneReg(0x38, 0x30); vi5300_check_ret(ret);
        ret = WriteOneReg(0x3A, 0x30); vi5300_check_ret(ret);
    }
    ret = WriteOneReg(vi5300_reg_cmd, 0x01); vi5300_check_ret(ret);  
    ret = WriteOneReg(vi5300_reg_size, 0x02); vi5300_check_ret(ret);  
    ret = WriteOneReg(vi5300_reg_scratch_pad_base+0x00, 0x00); vi5300_check_ret(ret);  
    ret = WriteOneReg(vi5300_reg_scratch_pad_base+0x01, 0x00); vi5300_check_ret(ret);

    return vi5300_ok;
}

/***************************************************************
  *  @brief     VI5300写入FIRMWAREPOST配置
  *  @param     None
  *  @Sample usage:     vi5300_write_firmware_post_config();
 **************************************************************/
static vi5300_status_t vi5300_write_firmware_post_config(void)
{
    vi5300_status_t ret = vi5300_ok;

    ret = WriteOneReg(vi5300_reg_sys_cfg, 0x0c); vi5300_check_ret(ret);
    ret = WriteOneReg(vi5300_reg_mcu_cfg, 0x06); vi5300_check_ret(ret);
    ret = WriteOneReg(0x3B, 0xA0); vi5300_check_ret(ret);
    ret = WriteOneReg(0x3B, 0x80); vi5300_check_ret(ret);
    
    if(chip_reg==0x00)
    {
        ret = WriteOneReg(0xE9, 0x24); vi5300_check_ret(ret);
        ret = WriteOneReg(0xEE, 0x00); vi5300_check_ret(ret);
        ret = WriteOneReg(0xF5, 0x00); vi5300_check_ret(ret);
    }
    
    ret = WriteOneReg(vi5300_reg_mcu_cfg, 0x07); vi5300_check_ret(ret);
    vi5300_delay_ms(5);
    ret = WriteOneReg(vi5300_reg_pw_ctrl, 0x02); vi5300_check_ret(ret);  
    ret = WriteOneReg(vi5300_reg_pw_ctrl, 0x00); vi5300_check_ret(ret);
    return vi5300_ok;
}

/***************************************************************
  *  @brief     VI5300写入FIRMWARE32字节
  *  @param     len     [len description]
  *  @param     data     [data description]
  *  @Sample usage:     vi5300_write_firmware32_byte(len, data);
 **************************************************************/
static vi5300_status_t vi5300_write_firmware32_byte(uint8_t len, const uint8_t *data)
{
    vi5300_status_t ret = vi5300_ok;
    uint8_t cmd_size[2];

    if(len > 32) return vi5300_error;

    cmd_size[0] = vi5300_writefw_cmd;
    cmd_size[1] = len;
    
    ret = I2C_WriteXBytes(vi5300_reg_cmd, cmd_size, 2);
    if(ret != vi5300_ok) return ret;
    
    ret = I2C_WriteXBytes(vi5300_reg_scratch_pad_base, data, len);
    if(ret != vi5300_ok) return ret;

    return vi5300_ok; 
}

/***************************************************************
  *  @brief     VI5300DOWNLOADFIRMWARE
  *  @param     buf     [buf description]
  *  @param     size     [size description]
  *  @Sample usage:     vi5300_download_firmware(buf, size);
 **************************************************************/
static vi5300_status_t vi5300_download_firmware(const uint8_t *buf, uint16_t size)
{
    vi5300_status_t ret = vi5300_ok;
    uint8_t page = 0;

    ret = vi5300_write_firmware_pre_config();
    if(ret != vi5300_ok) return ret;

    while(size >= 32)
    {
        ret = vi5300_write_firmware32_byte(32, (const uint8_t*)(buf + page * 32));
        if(ret != vi5300_ok) return ret;
        size -= 32;
        page++;
    }

    if(size > 0)
    {
        ret = vi5300_write_firmware32_byte((uint8_t)size, (const uint8_t*)(buf + page * 32));
        if(ret != vi5300_ok) return ret;
    }
    
    ret = vi5300_write_firmware_post_config();
    if(ret != vi5300_ok) return ret;

    return vi5300_ok;
}

/***************************************************************
  *  @brief     VI5300初始化FIRMWARE
  *  @param     None
  *  @Sample usage:     vi5300_init__firmware();
 **************************************************************/
static vi5300_status_t vi5300_init__firmware(void)
{
	vi5300_status_t ret;

	ret = vi5300_download_firmware(vi5300_firmware_ranging, vi5300_firmware_ranging__size);
	return ret;
}

/***************************************************************
  *  @brief     VI5300等待FORCPUREADY
  *  @param     None
  *  @Sample usage:     vi5300_wait__for_cpu__ready();
 **************************************************************/
static uint8_t vi5300_wait__for_cpu__ready(void)
{
	vi5300_status_t Status = vi5300_ok;
	uint8_t stat = 0xFF;
	int retry = 0;

	do {
		vi5300_delay_ms(1);
		Status = ReadOneReg(0x02, &stat);
		if(Status != vi5300_ok) return vi5300_error;
		retry++;
	}while((retry < 20) && (stat & 0x01));
	
	if(retry >= 20) return 1;
	return Status;
}

/***************************************************************
  *  @brief     VI5300配置XTALKPARAMETER
  *  @param     None
  *  @Sample usage:     vi5300_config_x_talk__parameter();
 **************************************************************/
static vi5300_status_t vi5300_config_x_talk__parameter(void)
{
	vi5300_status_t ret = vi5300_ok;
	vi5300_wait__for_cpu__ready();
    vi5300_set__digital__clock__dutycycle();       
	WriteOneReg(0x0C, 0x01);
	WriteOneReg(0x0D, 0x01);
	WriteOneReg(0x0E, 0x00);
	WriteOneReg(0x0F, xtalk_cal.xtalk_cal);
	WriteOneReg(0x0A, 0x09);
	return ret;
}

/***************************************************************
  *  @brief     VI5300获取RESULT
  *  @param     val     [val description]
  *  @Sample usage:     vi5300_get__result(val);
 **************************************************************/
static vi5300_status_t vi5300_get__result(vi5300_internal_dist_t *val)
{
    vi5300_status_t ret = vi5300_ok;
    uint8_t buf[26];
    
    ret = I2C_ReadXBytes(0x14, buf, 26);
    if(ret != vi5300_ok) return ret;

    val->millimeter = (int16_t)(((uint16_t)buf[11]<<8) | buf[10]);
    val->peak = (uint32_t)(((uint32_t)buf[1]<<24) | ((uint32_t)buf[2]<<16) | ((uint32_t)buf[3]<<8) | buf[0]);
    val->noise = (uint16_t)(((uint16_t)buf[5]<<8) | buf[4]);
    val->confidence = (uint32_t)(((uint32_t)buf[6]<<8) | buf[7]);
    
    return vi5300_ok;
}

/***************************************************************
  *  @brief     VI5300获取DISTANCE
  *  @param     data     [data description]
  *  @Sample usage:     vi5300_get__distance(data);
 **************************************************************/
vi5300_status_t vi5300_get__distance(vi5300_data_t *data)
{
    vi5300_status_t ret;
    vi5300_internal_dist_t raw_dist;
    uint8_t int_status = 0;

    ret = ReadOneReg(vi5300_ret_int_status, &int_status);
    if(ret != vi5300_ok) return ret;

    if((int_status & 0x7E) == 0x20)
    {
        ret = vi5300_get__result(&raw_dist);
        if(ret == vi5300_ok)
        {
            if(raw_dist.millimeter > 2000) raw_dist.millimeter = 2000;
            if(raw_dist.millimeter < 0) raw_dist.millimeter = 0;
            
            data->distance_mm = raw_dist.millimeter;
            data->confidence = (uint8_t)((raw_dist.confidence > 100) ? 100 : raw_dist.confidence);
            data->is_valid = 1;
            

            WriteOneReg(vi5300_ret_int_status, 0x00);
            return vi5300_ok;
        }
    }
    
    return vi5300_busy; 
}

/***************************************************************
  *  @brief     初始化VI5300激光测距传感器
  *  @param     None
  *  @Sample usage:     vi5300_init();
 **************************************************************/
vi5300_status_t vi5300_init(void)
{
    vi5300_status_t ret = vi5300_ok;
    int retry = 0;


    soft_i2c_init(&g_vi5300_i2c, vi5300_device_addr, 500, vi5300_scl_pin, vi5300_sda_pin);
    

    gpio_init(vi5300_xsh_pin, GPIO_MODE_OUT_PP, GPIO_HIGH);
    vi5300_delay_ms(10);
    

    do {
        ret = vi5300_init__firmware();
        if(ret == vi5300_ok) break;
        retry++;
        vi5300_delay_ms(10);
    } while(retry < 3);
    
    if(ret != vi5300_ok) return ret;

    vi5300_wait__for_cpu__ready();
    

    xtalk_cal.xtalk_cal = 0;
    vi5300_config_x_talk__parameter();
    

    WriteOneReg(vi5300_reg_cmd, vi5300_start_rang_cmd);
    
    return vi5300_ok;
}
