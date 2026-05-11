#include "zf_common_typedef.h"
#include "zf_driver_soft_iic.h"
#include "zf_device_config.h"

#define MT9V03X_SCCB_ADDR       (0x5C)
#define SCC8660_SCCB_ADDR        (0x78)

static soft_iic_info_struct *mt9v03x_iic_obj_1  = NULL;
static soft_iic_info_struct *mt9v03x_iic_obj_2  = NULL;
static soft_iic_info_struct *scc8660_iic_obj    = NULL;

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 1 config via SCCB
//-------------------------------------------------------------------------------------------------------------------
void mt9v03x_sccb_check_id_1 (void *soft_iic_obj)
{
    (void)soft_iic_obj;
}

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 1 set config via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char mt9v03x_set_config_sccb_1 (void *soft_iic_obj, short int buff[10][2])
{
    mt9v03x_iic_obj_1 = (soft_iic_info_struct *)soft_iic_obj;
    mt9v03x_iic_obj_1->addr = MT9V03X_SCCB_ADDR;

    for(uint32 i = 1; i < 10; i++)
    {
        soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFE, (uint8)(buff[i][0] >> 8));
        soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFE, (uint8)(buff[i][0] & 0xFF));
        soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFF, (uint8)(buff[i][1] >> 8));
        soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFF, (uint8)(buff[i][1] & 0xFF));
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 1 set exposure time via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char mt9v03x_set_exposure_time_sccb_1 (unsigned short int light)
{
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFE, 0x00);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFE, 0xF0);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFF, (uint8)(light >> 8));
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFF, (uint8)(light & 0xFF));
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 1 set register via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char mt9v03x_set_reg_sccb_1 (unsigned char addr, unsigned short int data)
{
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFE, 0x00);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFE, addr);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFF, (uint8)(data >> 8));
    soft_iic_sccb_write_register(mt9v03x_iic_obj_1, 0xFF, (uint8)(data & 0xFF));
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 2 config via SCCB
//-------------------------------------------------------------------------------------------------------------------
void mt9v03x_sccb_check_id_2 (void *soft_iic_obj)
{
    (void)soft_iic_obj;
}

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 2 set config via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char mt9v03x_set_config_sccb_2 (void *soft_iic_obj, short int buff[10][2])
{
    mt9v03x_iic_obj_2 = (soft_iic_info_struct *)soft_iic_obj;
    mt9v03x_iic_obj_2->addr = MT9V03X_SCCB_ADDR;

    for(uint32 i = 1; i < 10; i++)
    {
        soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFE, (uint8)(buff[i][0] >> 8));
        soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFE, (uint8)(buff[i][0] & 0xFF));
        soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFF, (uint8)(buff[i][1] >> 8));
        soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFF, (uint8)(buff[i][1] & 0xFF));
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 2 set exposure time via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char mt9v03x_set_exposure_time_sccb_2 (unsigned short int light)
{
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFE, 0x00);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFE, 0xF0);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFF, (uint8)(light >> 8));
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFF, (uint8)(light & 0xFF));
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// mt9v03x camera 2 set register via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char mt9v03x_set_reg_sccb_2 (unsigned char addr, unsigned short int data)
{
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFE, 0x00);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFE, addr);
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFF, (uint8)(data >> 8));
    soft_iic_sccb_write_register(mt9v03x_iic_obj_2, 0xFF, (uint8)(data & 0xFF));
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// scc8660 config via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char scc8660_set_config_sccb (void *soft_iic_obj, short int buff[10][2])
{
    scc8660_iic_obj = (soft_iic_info_struct *)soft_iic_obj;
    scc8660_iic_obj->addr = SCC8660_SCCB_ADDR;

    for(uint32 i = 1; i < 10; i++)
    {
        soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, (uint8)(buff[i][0] >> 8));
        soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, (uint8)(buff[i][0] & 0xFF));
        soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(buff[i][1] >> 8));
        soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(buff[i][1] & 0xFF));
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// scc8660 set brightness via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char scc8660_set_brightness_sccb (unsigned short int brightness)
{
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, 0x00);
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, 0xF0);
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(brightness >> 8));
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(brightness & 0xFF));
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// scc8660 set manual white balance via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char scc8660_set_manual_wb_sccb (unsigned short int manual_wb)
{
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, 0x00);
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, 0xF3);
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(manual_wb >> 8));
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(manual_wb & 0xFF));
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// scc8660 set register via SCCB
//-------------------------------------------------------------------------------------------------------------------
unsigned char scc8660_set_reg_sccb (unsigned char reg, unsigned short int data)
{
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, 0x00);
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFE, reg);
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(data >> 8));
    soft_iic_sccb_write_register(scc8660_iic_obj, 0xFF, (uint8)(data & 0xFF));
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// IMU660RA config file - replace with actual config data from IMU660RA SDK
//-------------------------------------------------------------------------------------------------------------------
const unsigned char imu660ra_config_file[8192] = {0};

//-------------------------------------------------------------------------------------------------------------------
// DL1B config file - replace with actual config data from DL1B SDK
//-------------------------------------------------------------------------------------------------------------------
const unsigned char dl1b_config_file[135] = {0};
