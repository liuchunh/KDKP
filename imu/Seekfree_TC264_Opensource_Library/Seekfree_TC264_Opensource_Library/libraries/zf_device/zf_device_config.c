/*
 * zf_device_config.c
 * Stub implementations for symbols declared in zf_device_config.h
 * that are not provided by the Seekfree open-source library release.
 *
 * If you need actual DL1B/IMU660RA/SCC8660/MT9V03X support,
 * replace these stubs with real implementations from Seekfree.
 */

#include "zf_device_config.h"

/* Config data blobs (zeroed stubs) */
const unsigned char imu660ra_config_file[8192] = {0};
const unsigned char dl1b_config_file[135] = {0};

/* SCC8660 SCCB stub functions */
unsigned char scc8660_set_config_sccb(void *soft_iic_obj, short int buff[10][2])
{
    (void)soft_iic_obj; (void)buff;
    return 1; /* fail */
}

unsigned char scc8660_set_brightness_sccb(unsigned short int brightness)
{
    (void)brightness;
    return 1;
}

unsigned char scc8660_set_manual_wb_sccb(unsigned short int manual_wb)
{
    (void)manual_wb;
    return 1;
}

unsigned char scc8660_set_reg_sccb(unsigned char reg, unsigned short int data)
{
    (void)reg; (void)data;
    return 1;
}

/* MT9V03X double-camera SCCB stub functions */
unsigned char mt9v03x_set_config_sccb_1(void *soft_iic_obj, short int buff[10][2])
{
    (void)soft_iic_obj; (void)buff;
    return 1;
}

unsigned char mt9v03x_set_exposure_time_sccb_1(unsigned short int light)
{
    (void)light;
    return 1;
}

unsigned char mt9v03x_set_config_sccb_2(void *soft_iic_obj, short int buff[10][2])
{
    (void)soft_iic_obj; (void)buff;
    return 1;
}

unsigned char mt9v03x_set_exposure_time_sccb_2(unsigned short int light)
{
    (void)light;
    return 1;
}
