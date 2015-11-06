/*
 * Driver for the TECHWELL TW8816 - LCD Falt Panel Processor (I2C bus)
 *
 * Copyright (C) 2015 Span Liu, Mitac Inc.
 *
 * Licensed under the GPL-2 or later.
 * Modifier     Date            Sign                                Description
 * =====================================================================================================
 * 
*/

#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#ifdef CONFIG_ARCH_VEXPRESS
#include <linux/platform_device.h>
#else
#endif	/* CONFIG_ARCH_VEXPRESS */
#include <linux/i2c.h>
#include <linux/sysfs.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/module.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include "tw8816_init.h"

MODULE_LICENSE("Dual BSD/GPL");



u8 event_mode_flag=0; /* ugly!! i2c-core.c needs it!! */

#ifdef CONFIG_TW8816_400X240_INIT
	#define	_USE_400x240_INIT_
#endif

#define 	_USE_SYSFS_
///#define		_USE_TW8816FREE_MODE_
#define		_USE_3_10_52_

#define 	MX6_TW8816_LCD_PWR_ON	IMX_GPIO_NR(3, 20)	/* EIM_D20 - active high */
#define 	MX6_TW8816_VP_RESET		IMX_GPIO_NR(7, 12)	/* GPIO_17 - active low */


typedef struct t_tw8816_drvdata {
	struct i2c_client	*client;
	struct input_dev	*input_dev;
	struct work_struct 	work;
#ifdef	_USE_3_10_52_
	/* of gpio pins */
	///struct device *dev;
	struct device *pdev;
	int	chip_id;	
	int gpio_reset;
	int gpio_lcdpwr;
#endif
} tw8816drvData;




#ifdef CONFIG_ARCH_VEXPRESS
#else
static tREGDATA tw8816_reg_Read(struct i2c_client *client, tREGADDR regID);
static tREGDATA tw8816_reg_Write(struct i2c_client *client, tREGADDR regID, tREGDATA regWrite);
static void init_TW8816reg(struct i2c_client *client, char cCmd);
static void osd_TW8816reg(struct i2c_client *client, tREGADDR type);
static void tw8816_chip_RESET(void);
#endif


static tw8816CtlSet g_usrCMD;
static tw8816drvData g_8816DRVdata;

#include "tw8816_init_table.c"

#ifdef _USE_SYSFS_

#define MY_MAJOR        40
struct class    *sysfs_class;


const tREGDATA regPage0_default[256] = {
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
/* 0x0? */	  0x22, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x20, 0x10, 0xD0, 0xCC, 0x15, 0x00, 0x00, 
/* 0x1? */	  0x00, 0x5C, 0x11, 0x80, 0x00, 0x00, 0x00, 0x30, 0x44, 0x00, 0x00, 0x00, 0x27, 0x7F, 0x08, 0x00, 
/* 0x2? */	  0x50, 0x42, 0xF0, 0xD8, 0xBC, 0xB8, 0x44, 0x2A, 0x00, 0x00, 0x78, 0x44, 0x30, 0x14, 0xA5, 0xE0, 
/* 0x3? */	  0x00, 0x00, 0x00, 0x05, 0x1A, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x4? */	  0x00, 0x20, 0x04, 0x22, 0x08, 0x54, 0x20, 0x00, 0xCF, 0x20, 0x13, 0x13, 0x00, 0x30, 0x00, 0x00, 
/* 0x5? */	  0x00, 0x20, 0xFF, 0x10, 0x20, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
/* 0x6? */	  0xB4, 0x80, 0x50, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x7? */	  0x20, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3F, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x8? */	  0x10, 0x0D, 0x03, 0x00, 0x67, 0x94, 0x18, 0xE8, 0xCA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x9? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 
/* 0xA? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xB? */	  0x00, 0x00, 0x0E, 0x10, 0x1B, 0x00, 0x41, 0x26, 0x06, 0x1F, 0x00, 0x33, 0x00, 0x08, 0x00, 0x00, 
/* 0xC? */	  0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xD? */	  0x00, 0x00, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D, 0xC3, 0xFC, 0x00, 0x00, 0x00, 
/* 0xE? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xF? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0x30, 0x11, 0x00
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
};

const tREGDATA regPage1_default[256] = {
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
/* 0x0? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x1? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x2? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x3? */	  0xF2, 0xAD, 0x04, 0x80, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x4? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x5? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x6? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x7? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x7F, 0x01, 0x00, 0x00, 0x20, 0x01, 0xE2, 
/* 0x8? */	  0x20, 0x00, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x4D, 0x00, 0x06, 0x01, 0xE2, 
/* 0x9? */	  0x02, 0x4D, 0x02, 0xD0, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x90, 
/* 0xA? */	  0x00, 0x00, 0x02, 0x30, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x36, 
/* 0xB? */	  0x02, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xC? */	  0x00, 0x00, 0x00, 0x03, 0x5A, 0x00, 0x20, 0x04, 0x00, 0x06, 0x06, 0x30, 0x00, 0x54, 0x00, 0x00, 
/* 0xD? */	  0x00, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x10, 0x70, 0x00, 0x04, 0x80, 0x80, 0x20, 0x00, 0x00, 0x00, 
/* 0xE? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xF? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static tREGDATA regPage0[256] = {
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
/* 0x0? */	  0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x20, 0x10, 0xD0, 0xCC, 0x15, 0x00, 0x00, 
/* 0x1? */	  0x00, 0x5C, 0x11, 0x80, 0x00, 0x00, 0x00, 0x30, 0x44, 0x00, 0x00, 0x00, 0x27, 0x7F, 0x08, 0x00, 
/* 0x2? */	  0x50, 0x42, 0xF0, 0xD8, 0xBC, 0xB8, 0x44, 0x2A, 0x00, 0x00, 0x78, 0x44, 0x30, 0x14, 0xA5, 0xE0, 
/* 0x3? */	  0x00, 0x00, 0x00, 0x05, 0x1A, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x4? */	  0x00, 0x20, 0x04, 0x22, 0x08, 0x54, 0x20, 0x00, 0xCF, 0x20, 0x13, 0x13, 0x00, 0x30, 0x00, 0x00, 
/* 0x5? */	  0x00, 0x20, 0xFF, 0x10, 0x20, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
/* 0x6? */	  0xB4, 0x80, 0x50, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x7? */	  0x20, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3F, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x8? */	  0x10, 0x0D, 0x03, 0x00, 0x67, 0x94, 0x18, 0xE8, 0xCA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x9? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 
/* 0xA? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xB? */	  0x00, 0x00, 0x0E, 0x10, 0x1B, 0x00, 0x41, 0x26, 0x06, 0x1F, 0x00, 0x33, 0x00, 0x08, 0x00, 0x00, 
/* 0xC? */	  0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xD? */	  0x00, 0x00, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D, 0xC3, 0xFC, 0x00, 0x00, 0x00, 
/* 0xE? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xF? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0x30, 0x11, 0x00
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
};

static tREGDATA regPage1[256] = {
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
/* 0x0? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x1? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x2? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x3? */	  0xF2, 0xAD, 0x04, 0x80, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x4? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x5? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x6? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x7? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x7F, 0x01, 0x00, 0x00, 0x20, 0x01, 0xE2, 
/* 0x8? */	  0x20, 0x00, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x4D, 0x00, 0x06, 0x01, 0xE2, 
/* 0x9? */	  0x02, 0x4D, 0x02, 0xD0, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x90, 
/* 0xA? */	  0x00, 0x00, 0x02, 0x30, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x36, 
/* 0xB? */	  0x02, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xC? */	  0x00, 0x00, 0x00, 0x03, 0x5A, 0x00, 0x20, 0x04, 0x00, 0x06, 0x06, 0x30, 0x00, 0x54, 0x00, 0x00, 
/* 0xD? */	  0x00, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x10, 0x70, 0x00, 0x04, 0x80, 0x80, 0x20, 0x00, 0x00, 0x00, 
/* 0xE? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xF? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static tREGDATA regPage0_store[256] = {
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
/* 0x0? */	  0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x20, 0x10, 0xD0, 0xCC, 0x15, 0x00, 0x00, 
/* 0x1? */	  0x00, 0x5C, 0x11, 0x80, 0x00, 0x00, 0x00, 0x30, 0x44, 0x00, 0x00, 0x00, 0x27, 0x7F, 0x08, 0x00, 
/* 0x2? */	  0x50, 0x42, 0xF0, 0xD8, 0xBC, 0xB8, 0x44, 0x2A, 0x00, 0x00, 0x78, 0x44, 0x30, 0x14, 0xA5, 0xE0, 
/* 0x3? */	  0x00, 0x00, 0x00, 0x05, 0x1A, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x4? */	  0x00, 0x20, 0x04, 0x22, 0x08, 0x54, 0x20, 0x00, 0xCF, 0x20, 0x13, 0x13, 0x00, 0x30, 0x00, 0x00, 
/* 0x5? */	  0x00, 0x20, 0xFF, 0x10, 0x20, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
/* 0x6? */	  0xB4, 0x80, 0x50, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x7? */	  0x20, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3F, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x8? */	  0x10, 0x0D, 0x03, 0x00, 0x67, 0x94, 0x18, 0xE8, 0xCA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x9? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 
/* 0xA? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xB? */	  0x00, 0x00, 0x0E, 0x10, 0x1B, 0x00, 0x41, 0x26, 0x06, 0x1F, 0x00, 0x33, 0x00, 0x08, 0x00, 0x00, 
/* 0xC? */	  0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xD? */	  0x00, 0x00, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D, 0xC3, 0xFC, 0x00, 0x00, 0x00, 
/* 0xE? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xF? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0x30, 0x11, 0x00
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
};

static tREGDATA regPage1_store[256] = {
/*            0x?0  0x?1  0x?2  0x?3  0x?4  0x?5  0x?6  0x?7  0x?8  0x?9  0x?A  0x?B  0x?C  0x?D  0x?E  0x?F  */
/* 0x0? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x1? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x2? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x3? */	  0xF2, 0xAD, 0x04, 0x80, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x4? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x5? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x6? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0x7? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x7F, 0x01, 0x00, 0x00, 0x20, 0x01, 0xE2, 
/* 0x8? */	  0x20, 0x00, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x4D, 0x00, 0x06, 0x01, 0xE2, 
/* 0x9? */	  0x02, 0x4D, 0x02, 0xD0, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x90, 
/* 0xA? */	  0x00, 0x00, 0x02, 0x30, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x36, 
/* 0xB? */	  0x02, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xC? */	  0x00, 0x00, 0x00, 0x03, 0x5A, 0x00, 0x20, 0x04, 0x00, 0x06, 0x06, 0x30, 0x00, 0x54, 0x00, 0x00, 
/* 0xD? */	  0x00, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x10, 0x70, 0x00, 0x04, 0x80, 0x80, 0x20, 0x00, 0x00, 0x00, 
/* 0xE? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0xF? */	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static int sys_checkRegAddrVaild( tREGADDR regAddr)
{
	int iRet = 0;
	
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );


	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	
	return iRet;
}


static tREGDATA sys_getRegVal(tREGADDR regAddr)
{
	tREGDATA rRet = 0;
	
	DRVDBG( KERN_INFO"%s:%s:+++ %d[0x%02x]\n", DrvStr, __FUNCTION__, regAddr, regAddr );
	DRVKNOT( "+++ %d[0x%02x]\n", regAddr, regAddr );

	if(0 == sys_checkRegAddrVaild(regAddr)) {
		int iPage = 0;
		char *ptbl = 0;
		
		if( regAddr <= 0xFF ) {
			iPage = 0;
			ptbl = regPage0;
		} else {
			iPage = 1;
			ptbl = regPage1;
		}
			
#ifdef CONFIG_ARCH_VEXPRESS
		rRet = ptbl[(regAddr&0xFF)];
#else
		rRet = tw8816_reg_Read(g_8816DRVdata.client, regAddr);
#endif	/* CONDIF_ARCH_VEXPRESS */
		DRVDBG( KERN_INFO"%s:%s:read 0x%02x%02x = 0x%02x\n", DrvStr, __FUNCTION__, iPage, (regAddr&0xFF), rRet );
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	
	return rRet;
}


static int sys_setRegVal(tREGADDR regAddr, tREGDATA regVal)
{
	int iRet = (-1);
	tREGDATA rRet = 0;

	DRVDBG( KERN_INFO"%s:%s:+++ %d[0x%02x], %d[0x%02x]\n", DrvStr, __FUNCTION__, regAddr, regAddr, regVal, regVal );
	DRVKNOT( "+++ %d[0x%02x], %d[0x%02x]\n", regAddr, regAddr, regVal, regVal );

	if(0 == sys_checkRegAddrVaild(regAddr)) {
		int iPage = 0;
		char *ptbl = 0;
		
		if( regAddr <= 0xFF ) {
			iPage = 0;
			ptbl = regPage0;
		} else {
			iPage = 1;
			ptbl = regPage1;
		}
			
#ifdef CONFIG_ARCH_VEXPRESS
		ptbl[(regAddr&0xFF)] = regVal;
		iRet = 0;
#else
		rRet = tw8816_reg_Write(g_8816DRVdata.client, regAddr, regVal);
		if( 0 != rRet )
			iRet = 0;
#endif	/* CONDIF_ARCH_VEXPRESS */
		
		DRVDBG( KERN_INFO"%s:%s:write 0x%02x%02x = 0x%02x\n", DrvStr, __FUNCTION__, iPage, (regAddr&0xFF), iRet );
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	
	return iRet;
}


static ssize_t dpm_control_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	if( buf ) {
		len = sprintf(buf, "show controls....\n");
		DRVKNOT( "show controls....\n" );
		DRVDBG( KERN_INFO"%s:%s:show controls....\n", DrvStr, __FUNCTION__ );
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	return len;
}


static ssize_t dpm_control_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	ssize_t error = 0;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	if( buf ) {
		DRVKNOT( "get string = %s\n", buf );
		DRVDBG( KERN_INFO"%s:%s:get string = %s\n", DrvStr, __FUNCTION__, buf );
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	error = n;
	return error;
}


static ssize_t dpm_control_rsp(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t error = 0;
	ssize_t len = 0;
	int iRet = 0;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "++++\n" );

	///pre-set shadow
	if( 0x00 == regPage0[0] ) {
		int loop = 0;
		DRVDBG( KERN_INFO"%s:%s:Init reg shadow...\n", DrvStr, __FUNCTION__ );
		DRVKNOT( "Init reg shadow...\n" );
		for( loop = 0; loop < 256; loop++ ) {
			regPage0[loop] = (char)regPage0_default[loop];
			regPage1[loop] = (char)regPage1_default[loop];
		}
	}

	if( buf ) {
		/// do cmd
		if( spriv_IsSysCmdRead(g_usrCMD.cCmd) ) {
			len = sprintf(buf, "%s! read reg[0x%04X]=0x%02X\n", (-1==g_usrCMD.iOk)?"Oops":"Okey", g_usrCMD.iReg, g_usrCMD.iVal);
			error = len;
				
			g_usrCMD.iOk = (-1);	///reset
		} else
		if( spriv_IsSysCmdWrite(g_usrCMD.cCmd) ) {
			len = sprintf(buf, "%s! write reg[0x%04X]=0x%02X\n", (-1==g_usrCMD.iOk)?"Oops":"Okey", g_usrCMD.iReg, g_usrCMD.iVal);
			error = len;
				
			g_usrCMD.iOk = (-1);	///reset
		} else
		if( spriv_IsSysCmdDump(g_usrCMD.cCmd) ) {
			len = sprintf(buf, "TW8816 register dump >>>\n");
			DRVKPOP( "TW8816 register dump ...\n" );
			DRVKPOP( "Page 0:\n" );
			for( iRet = 0; iRet < 0x100; iRet++ ) {
				g_usrCMD.iReg = (tREGADDR)iRet;
				g_usrCMD.iVal =regPage0_store[g_usrCMD.iReg];
				DRVKPOP( "reg[0x%02X] = 0x%02X\n", g_usrCMD.iReg, g_usrCMD.iVal );
			}
			DRVKPOP( "Page 1:\n" );
			for( iRet = 0; iRet < 0x100; iRet++ ) {
				g_usrCMD.iReg = (tREGADDR)iRet;
				g_usrCMD.iVal =regPage1_store[g_usrCMD.iReg];
				DRVKPOP( "reg[0x%02X] = 0x%02X\n", g_usrCMD.iReg, g_usrCMD.iVal );
			}
			error = len;
			
			g_usrCMD.iOk = (-1);	///reset
		} else
		if( spriv_IsSysCmdMsgLv(g_usrCMD.cCmd) ) {
			len = sprintf(buf, "TW8816 OSD msg level mode[%d] >>>\n", g_usrCMD.iReg );
			error = len;
			g_usrCMD.iOk = (-1);	///reset
		} else
		if( spriv_IsSysCmdReset(g_usrCMD.cCmd) ) {
			len = sprintf(buf, "TW8816 OSD chip reset >>>\n" );
			error = len;
			g_usrCMD.iOk = (-1);	///reset
		} else
		if( spriv_IsSysCmdOsdTest(g_usrCMD.cCmd) ) {
			len = sprintf(buf, "TW8816 OSD testing mode[%d] >>>\n", g_usrCMD.iReg );
			error = len;
			g_usrCMD.iOk = (-1);	///reset
		} else
		if( spriv_IsSysCmd800x480(g_usrCMD.cCmd) ) {
			g_usrCMD.iOk = (-1);	///reset
		} else
		if( spriv_IsSysCmd400x240(g_usrCMD.cCmd) ) {
			g_usrCMD.iOk = (-1);	///reset
		} else {
			DRVKNOT( "nothing to do by cmd....\n" );
			len = sprintf(buf, "nothing to do with cmd=%c reg[0x%04X]=0x%02X\n", g_usrCMD.cCmd, g_usrCMD.iReg, g_usrCMD.iVal);
			error = len;
			g_usrCMD.iOk = (-1);
		}
	}	///if( buf )

	DRVKNOT( "----\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	return error;
}


static void init_regShadow(void)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "++++\n" );
	
	if( 0x00 == regPage0[0] ) {
		int loop = 0;
		DRVDBG( KERN_INFO"%s:%s:Init shadow...\n", DrvStr, __FUNCTION__ );
		DRVKNOT( "Init shadow...\n" );
		for( loop = 0; loop < 256; loop++ ) {
			regPage0[loop] = (char)regPage0_default[loop];
			regPage1[loop] = (char)regPage1_default[loop];
		}
	}
	
	DRVKNOT( "----\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

}


static int DoCommands ( tw8816CtlSet *pCtlset )
{
	int iRet = 0;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "++++\n" );
	
	if( pCtlset ) {
		/// do cmd
		if( spriv_IsSysCmdRead(g_usrCMD.cCmd) ) {
			pCtlset->iVal = sys_getRegVal(pCtlset->iReg);
			pCtlset->iOk = 0;
		} else
		if( spriv_IsSysCmdWrite(g_usrCMD.cCmd) ) {
			iRet = sys_setRegVal(pCtlset->iReg, pCtlset->iVal);
			if( (-1) == iRet ) {
				pCtlset->iOk = (-1);
			} else {
				pCtlset->iOk = 0;
			}
		} else
		if( spriv_IsSysCmdDump(g_usrCMD.cCmd) ) {
			/// page 0
			for( iRet = 0; iRet < 0x100; iRet++ ) {
				pCtlset->iReg = (tREGADDR)iRet;
				pCtlset->iVal = sys_getRegVal(pCtlset->iReg);
				regPage0_store[pCtlset->iReg] = pCtlset->iVal;
			}
			/// page 1
			for( iRet = 0; iRet < 0x100; iRet++ ) {
				pCtlset->iReg = 0x0100|(tREGADDR)iRet;
				pCtlset->iVal = sys_getRegVal(pCtlset->iReg);
				regPage1_store[pCtlset->iReg&0x00FF] = pCtlset->iVal;
			}
			pCtlset->iOk = 0;
			iRet = 0;
		} else
		if( spriv_IsSysCmdMsgLv(g_usrCMD.cCmd) ) {
			spriv_setMsgLv(g_usrCMD.iReg);
			pCtlset->iOk = 0;
		} else
		if( spriv_IsSysCmdReset(g_usrCMD.cCmd) ) {
			tw8816_chip_RESET();
			pCtlset->iOk = 0;
		} else
		if( spriv_IsSysCmdOsdTest(g_usrCMD.cCmd) ) {
			osd_TW8816reg(g_8816DRVdata.client, pCtlset->iReg);
			pCtlset->iOk = 0;
		} else
		if( spriv_IsSysCmd800x480(g_usrCMD.cCmd) ) {
			init_TW8816reg(g_8816DRVdata.client, 's');
			pCtlset->iOk = 0;
		} else
		if( spriv_IsSysCmd400x240(g_usrCMD.cCmd) ) {
			init_TW8816reg(g_8816DRVdata.client, 'S');
			pCtlset->iOk = 0;
		} else {
			DRVKNOT( "nothing to do by cmd [%c]....\n", g_usrCMD.cCmd );
			pCtlset->iOk = (-1);
		}
	}

	DRVKNOT( "----\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
		
	return iRet;
}


static ssize_t dpm_control_cmd(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	ssize_t error = 0;
	int iRet = 0;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "++++\n" );

	///pre-set shadow
	init_regShadow();

	if( buf ) {
		char *pCh = (char *)buf;
		enum SYSCMD_STAT stat = csCMD;
		int iCnt = n+1;
		
		DRVKNOT( "get cmd = %s\n", buf );
		DRVDBG( KERN_INFO"%s:%s:get cmd = %s\n", DrvStr, __FUNCTION__, buf );
		
		while( pCh ) {

			if( 0 == iCnt-- ) {
				DRVKNOT( "overrun !!!\n" );
				break;						
			}
			if( spriv_IsSysCmdEnd(*pCh) ) {
				DRVKNOT( "END char = %c\n", *pCh );
				break;
			}
			if( csEND == stat ) {
				DRVKNOT( "END state\n" );
				break;
			}
					
			switch(stat)
			{
				case csCMD:
				{
					if( spriv_IsSysCmdS(*pCh) ) {
						g_usrCMD.cCmd = *pCh;
						DRVKNOT( "get CMD = %c\n", g_usrCMD.cCmd );
						spriv_CMDnextStat( pCh, &stat );
						pCh++;
						continue;
					} else {
						DRVDBG( KERN_INFO"%s:%s:skip char = %c\n", DrvStr, __FUNCTION__, *pCh );
						DRVKNOT( "skip char = %c\n", *pCh );
						pCh++;
						continue;
					}	
				}	///case
					break;
				
				case csREG:
				{
					if( spriv_IsSysCmdOsdTest(g_usrCMD.cCmd) )
						g_usrCMD.iReg = 0;		/// clear it first
					
					DRVDBG( KERN_INFO"%s:%s:REG = %c%c%c%c\n", DrvStr, __FUNCTION__, *(pCh+0), *(pCh+1), *(pCh+2), *(pCh+3) );
					DRVKNOT( "REG = %c%c%c%c\n", *(pCh+0), *(pCh+1), *(pCh+2), *(pCh+3) );
					g_usrCMD.iReg = (spriv_char2int(*(pCh+0))*0x1000) + (spriv_char2int(*(pCh+1))*0x0100) + (spriv_char2int(*(pCh+2))*0x0010) + (spriv_char2int(*(pCh+3))*0x0001);
					DRVDBG( KERN_INFO"%s:%s:REG = %d = 0x%04x\n", DrvStr, __FUNCTION__, g_usrCMD.iReg, g_usrCMD.iReg );
					DRVKNOT( "REG = %d = 0x%04x\n", g_usrCMD.iReg, g_usrCMD.iReg );
					
					if( spriv_IsSysRegS(g_usrCMD.cCmd) ) {
						stat = csEND;
					} else {
						stat = csSET2;
					}
					pCh = pCh + 4;	///shift
				}	///case
					break;
					
				case csVAL:
				{
					DRVDBG( KERN_INFO"%s:%s:VAL = %c%c\n", DrvStr, __FUNCTION__, *(pCh+0), *(pCh+1) );
					DRVKNOT( "VAL = %c%c\n", *(pCh+0), *(pCh+1) );
					g_usrCMD.iVal = (spriv_char2int(*(pCh+0))*0x10) + (spriv_char2int(*(pCh+1))*0x01);
					DRVDBG( KERN_INFO"%s:%s:VAL = %d = 0x%02x\n", DrvStr, __FUNCTION__, g_usrCMD.iVal, g_usrCMD.iVal );
					DRVKNOT( "VAL = %d = 0x%02x\n", g_usrCMD.iVal, g_usrCMD.iVal );

					stat = csSET3;
					pCh = pCh + 2;	///shift
				}	///case 
					break;
					
				default:
				{
					if( spriv_IsSysCmdSparS(*pCh) ) {
						spriv_STATnextStat( &stat );
						pCh++;
						continue;
					} else {						
						DRVDBG( KERN_INFO"%s:%s:skip char = %c\n", DrvStr, __FUNCTION__, *pCh );
						DRVKNOT( "skip char = %c\n", *pCh );
						pCh++;
						continue;
					}
				}	///default
					break;
			}	///switch
			
		}	///while
		
		iRet = DoCommands( &g_usrCMD );
		if( 0 == iRet ) {
			error = n;
		} else {
			error = n;
		}
		
	}	///if( buf )

	DRVKNOT( "----\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	///error = n;
	return error;
}


static struct device_attribute sysfs_attrs[] = {
///        __ATTR(name, S_IRUGO, sysfs_show_name, NULL),
        __ATTR(show, S_IRUGO, dpm_control_show, NULL),
///        __ATTR(time, S_IRUGO, sysfs_show_time, NULL),
        __ATTR(store, S_IWUSR, NULL, dpm_control_store),
        ///__ATTR(cmd, S_IRUGO | S_IWUSR, dpm_control_get, dpm_control_cmd),
        __ATTR(cmd, S_IWUSR, NULL, dpm_control_cmd),
        __ATTR(rsp, S_IRUGO, dpm_control_rsp, NULL),
        { },
};


static int sysfs_suspend(struct device *dev, pm_message_t mesg)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	return 0;
}


static int sysfs_resume(struct device *dev)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	return 0;
}

#endif /* #ifdef _USE_SYSFS_ */

#define 	TW8816_I2C_aDDR					0x45
#define 	TW8816_I2C_wRITE				0x0
#define 	TW8816_I2C_rEAD					0x1

#define		TW8816_IDX_BASE					0x0000
#define		TW8816_IDX_PRODUCT_ID			(TW8816_IDX_BASE+0x00)
#define		TW8816_IDX_CHIP_STATUS			(TW8816_IDX_BASE+0x01)
#define		TW8816_IDX_PAGESET				0xFF


#ifdef CONFIG_ARCH_VEXPRESS
#else
static tREGDATA tw8816_reg_Read(struct i2c_client *client, tREGADDR regID)
{
	#define	R_TTL_BYTES		3
	tREGDATA u8Ret = 0;
	struct i2c_msg this_msg[R_TTL_BYTES];
	u8 cmd = 0x00;
	u8 rsp = 0x00;
	tREGDATA page0[2] = {TW8816_IDX_PAGESET, 0};
	tREGDATA page1[2] = {TW8816_IDX_PAGESET, 1};
	int iRet = -1;

	if( spriv_IsMsgLvRegReadWrite() ) {
		DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
		DRVKNOT( "+++\n" );
	}

	if( (regID & 0x0100) > 0 ) {	
		/// page 1
		/// Write 2 byte 
		this_msg[0].addr = TW8816_I2C_aDDR;
		this_msg[0].flags = TW8816_I2C_wRITE;
		this_msg[0].buf = page1;
		this_msg[0].len = 2;
	} 
	else {
		/// page 0
		/// Write 2 byte 
		this_msg[0].addr = TW8816_I2C_aDDR;
		this_msg[0].flags = TW8816_I2C_wRITE;
		this_msg[0].buf = page0;
		this_msg[0].len = 2;
	}
	
	cmd = (regID & 0x00FF);

	/// Write 1 byte 
	this_msg[1].addr = TW8816_I2C_aDDR;
	this_msg[1].flags = TW8816_I2C_wRITE;
	this_msg[1].buf = &cmd;
	this_msg[1].len = sizeof(cmd);
	/// Read 1 byte
	this_msg[2].addr = TW8816_I2C_aDDR;
	this_msg[2].flags = TW8816_I2C_rEAD;
	this_msg[2].buf = &rsp;
	this_msg[2].len = sizeof(rsp);
	/// Transfer
	iRet = i2c_transfer(client->adapter, this_msg, R_TTL_BYTES);
	if( iRet == R_TTL_BYTES ){
		if( spriv_IsMsgLvRegReadWrite() ) {
			DRVDBG( KERN_INFO"%s:%s: %s reg: 0x%02X, value: 0x%02X.\n", DrvStr, __FUNCTION__, ((regID&0x0100)>0)?"Page1":"Page0", cmd, rsp);
		}
		u8Ret = rsp;
	}else{
		DRVKERR( "failed to read [%d]\n", iRet );
	}

	if( spriv_IsMsgLvRegReadWrite() ) {
		DRVKNOT( "---\n" );
		DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	}
	return u8Ret;
}
#endif	/* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
#else
static tREGDATA tw8816_reg_Write(struct i2c_client *client, tREGADDR regID, tREGDATA regWrite)
{
	#define	W_TTL_BYTES		2
	tREGDATA u8Ret = 0;
	struct i2c_msg this_msg[W_TTL_BYTES];
	u8 cmd[2] = {0x00, 0x00};
	///u8 rsp = 0x00;
	u8 page0[2] = {TW8816_IDX_PAGESET, 0};
	u8 page1[2] = {TW8816_IDX_PAGESET, 1};
	int iRet = -1;

	if( spriv_IsMsgLvRegReadWrite() ) {
		DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
		DRVKNOT( "+++\n" );
	}

	if( (regID & 0x0100) > 0 ) {	
		/// page 1
		/// Write 2 byte 
		this_msg[0].addr = TW8816_I2C_aDDR;
		this_msg[0].flags = TW8816_I2C_wRITE;
		this_msg[0].buf = page1;
		this_msg[0].len = 2;
	} 
	else {
		/// page 0
		/// Write 2 byte 
		this_msg[0].addr = TW8816_I2C_aDDR;
		this_msg[0].flags = TW8816_I2C_wRITE;
		this_msg[0].buf = page0;
		this_msg[0].len = 2;
	}
	
	cmd[0] = (regID & 0x00FF);
	cmd[1] = regWrite;

	/// Write 2 byte 
	this_msg[1].addr = TW8816_I2C_aDDR;
	this_msg[1].flags = TW8816_I2C_wRITE;
	this_msg[1].buf = cmd;
	this_msg[1].len = 2;
#ifdef _USE_TW8816FREE_MODE_
	if(1) { 
		DRVDBG( KERN_INFO"%s:%s: reg: 0x%04X, value: 0x%02X \n", DrvStr, __FUNCTION__, regID, regWrite);
		DRVKNOT( "reg: 0x%04X, value: 0x%02X \n", regID, regWrite );
		iRet = W_TTL_BYTES;
		u8Ret = W_TTL_BYTES;
	}
#else	
	/// Transfer
	iRet = i2c_transfer(client->adapter, this_msg, W_TTL_BYTES);
	if( iRet == W_TTL_BYTES ){
		if( spriv_IsMsgLvRegReadWrite() ) {
			///DRVDBG( KERN_INFO"%s:%s: cmd: %x, value: %x.\n", DrvStr, __FUNCTION__, cmd[0], cmd[1]);
			DRVDBG( KERN_INFO"%s:%s: %s reg: 0x%02X, value: 0x%02X.\n", DrvStr, __FUNCTION__, ((regID&0x0100)>0)?"Page1":"Page0", cmd[0], cmd[1]);
		}
		u8Ret = W_TTL_BYTES;
	}else{
		DRVKERR( "failed to write [%d]\n", iRet );
	}
#endif	/* _USE_TW8816FREE_MODE_ */

	if( spriv_IsMsgLvRegReadWrite() ) {
		DRVKNOT( "---\n" );
		DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	}
	return u8Ret;
}


static int tw8816_reg_Fill(struct i2c_client *client, tw8816RegDataSet *pTable)
{
	tREGDATA u8Ret = 0;
	tw8816RegDataSet *pRegSet = NULL;
	
	int iRet = -1;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	if( pTable ) {
		pRegSet = (tw8816RegDataSet *)&pTable[0];
		iRet = 0;
		while ( 0 == (pRegSet->regAddr & 0x1000) ) {
			u8Ret = tw8816_reg_Write(client, pRegSet->regAddr, pRegSet->regData);
			if( 0 == u8Ret ) {
				iRet = (-1);
				break;
			}
			pRegSet++;
		}
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return iRet;
}
#endif	/* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
#else
static int tw8816_open(struct inode *inode, struct file *file)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
#endif	 /* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
#else
static int tw8816_release(struct inode *inode, struct file *file)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
#endif	/* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
#else
static long tw8816_ioctl( struct file *file,
        unsigned int cmd, unsigned long arg)
{

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	switch (cmd) 
	{
		case 0:
			break;
		default:
			DRVKERR( "fails in switch case!\n" );
			return -ENOTTY;
    }

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
    return 0;
}
#endif	/* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
static int tw8816_init_drv_resume(struct platform_device *dev)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
#else
static int tw8816_init_pm_resume(struct device *dev)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
/*
static int tw8816_init_drv_resume(struct i2c_client *client)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
*/
#endif	/* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
static int tw8816_init_drv_suspend(struct platform_device *dev, pm_message_t state)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
#else
static int tw8816_init_pm_suspend(struct device *dev)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
/*
static int tw8816_init_drv_suspend(struct i2c_client *client, pm_message_t mesg)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return 0;
}
*/
#endif	/* #ifdef CONFIG_ARCH_VEXPRESS */



#ifdef CONFIG_ARCH_VEXPRESS
#else
static const struct i2c_device_id tw8816_id[] = {
	{_TW8816DRVNAME_, 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, tw8816_id);


static const struct file_operations tw8816_init_fops = {
	.owner = THIS_MODULE,
	.open = tw8816_open,
	.release = tw8816_release,
	.unlocked_ioctl = tw8816_ioctl,
};
#endif	 /* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
#else
static void init_TW8816reg(struct i2c_client *client, char cCmd)
{
	tREGDATA u8Data = 0;
	int iRet = 5;

	DRVDBG( KERN_INFO"%s:%s:[%c]+++\n", DrvStr, __FUNCTION__, cCmd );
	DRVKNOT( "[%c]+++\n", cCmd );

	do {
		u8Data = tw8816_reg_Read(client, TW8816_IDX_PRODUCT_ID);
		DRVDBG( KERN_INFO"%s:%s:chip ID[0x%04X]=0x%02X\n", DrvStr, __FUNCTION__, TW8816_IDX_PRODUCT_ID, u8Data );
		if( 0x22 != u8Data ) {
			msleep(200);
			iRet--;
			DRVDBG( KERN_INFO"%s:%s:Wrong chip ID=0x%02X\n", DrvStr, __FUNCTION__, u8Data );
		} else {
			iRet = 0;		/// get chip ID, stop retry
		}
	} while( (0 < iRet) );

#ifdef _USE_TW8816FREE_MODE_
	if( 1 ) {
#else
	if( regPage0_default[TW8816_IDX_PRODUCT_ID] == u8Data ) {
#endif	/* _USE_TW8816FREE_MODE_ */
		
		u8Data = tw8816_reg_Read(client, TW8816_IDX_CHIP_STATUS);
		DRVDBG( KERN_INFO"%s:%s:chip status=0x%02x\n", DrvStr, __FUNCTION__, u8Data );
		
		if( 'S' == cCmd ) {
			/// load 400x240 TW8816 mode register setting
			DRVKPOPS( "TW8816 chip initial as 400x240 mode ... \n" );
			iRet = tw8816_reg_Fill( client, (tw8816RegDataSet *)preloadtbl_400x240 );
			if( -1 == iRet ) {
				DRVKPOPS( "TW8816 chip initial as 400x240 mode ... fail! \n" );
			} else {
				DRVKPOPS( "TW8816 chip initial as 400x240 mode ... done. \n" );
			}
		} else {
			/// load bypass TW8816 mode register setting
			DRVKPOPS( "TW8816 chip initial as bypass mode ... \n" );
			iRet = tw8816_reg_Fill( client, (tw8816RegDataSet *)preloadtbl_bypass );
			if( -1 == iRet ) {
				DRVKPOPS( "TW8816 chip initial as bypass mode ... fail! \n" );
			} else {
				DRVKPOPS( "TW8816 chip initial as bypass mode ... done. \n" );
			}
		}
	} else {
		DRVKPOPS( "TW8816 chip not found !!! \n" );
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	return;
}

static void osd_TW8816reg(struct i2c_client *client, tREGADDR type)
{
	tREGDATA u8Data = 0;
	int iRet = 5;

	DRVDBG( KERN_INFO"%s:%s:[%c]+++\n", DrvStr, __FUNCTION__, type );
	DRVKNOT( "[%c]+++\n", type );

	do {
		u8Data = tw8816_reg_Read(client, TW8816_IDX_PRODUCT_ID);
		DRVDBG( KERN_INFO"%s:%s:chip ID[0x%04X]=0x%02X\n", DrvStr, __FUNCTION__, TW8816_IDX_PRODUCT_ID, u8Data );
		msleep(50);
	} while( (0 < iRet--) && (0x22 != u8Data) );

	if( regPage0_default[TW8816_IDX_PRODUCT_ID] != u8Data ) {
		DRVKPOPS( "TW8816 chip not found !!! \n" );
	} else {
		u8Data = tw8816_reg_Read(client, TW8816_IDX_CHIP_STATUS);
		DRVDBG( KERN_INFO"%s:%s:chip status=0x%02x\n", DrvStr, __FUNCTION__, u8Data );
		
		switch (type) {
			case 0:
				tw8816_reg_Write(client, 0x9F, 0x00);	/// disable OSD
				break;
			case 4:
				DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				iRet = tw8816_reg_Fill(client, (tw8816RegDataSet *)preloadtbl_osd400x240);			/// load OSD for 400x240
				if( -1 == iRet ) {
					DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				}
				break;
			case 5:
				DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				iRet = tw8816_reg_Fill(client, (tw8816RegDataSet *)preloadtbl_osd400x240_full);			/// load OSD for 400x240
				if( -1 == iRet ) {
					DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				}
				break;
			case 8:
				DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				iRet = tw8816_reg_Fill(client, (tw8816RegDataSet *)preloadtbl_osd800x480);			/// load OSD for 400x240
				if( -1 == iRet ) {
					DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				}
				break;
			case 9:
				DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				iRet = tw8816_reg_Fill(client, (tw8816RegDataSet *)preloadtbl_osd800x480_full);			/// load OSD for 800x480
				if( -1 == iRet ) {
					DRVKPOPS( "TW8816 chip OSD test mode(%d)\n", type );
				}
				break;
			default:
				break;
		}	///switch		
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

	return;
}
#endif	/* #ifdef CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
static int tw8816_init_probe(struct platform_device *dev)
{
	tw8816drvData *pdata;
	int iRet = 0;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	pdata = kzalloc(sizeof(tw8816drvData), GFP_KERNEL);
	if (!pdata) {
		DRVKERR( "fails to allocate memory\n" );
		return -ENOMEM;
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return iRet;
}
#else
static int __init tw8816_init_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device_node *of_node = client->dev.of_node;
	tw8816drvData *pdata;
	int iRet = 0;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	pdata = kzalloc(sizeof(tw8816drvData), GFP_KERNEL);
	if (!pdata) {
		DRVKERR( "fails to allocate memory\n" );
		return -ENOMEM;
	}

	pdata->client = client;
	g_8816DRVdata.client = client;
	if (of_node) {
		pdata->pdev = &client->dev;
		g_8816DRVdata.pdev = &client->dev;
		pdata->gpio_reset = of_get_named_gpio(of_node, "tw8816reset-gpios", 0);
		pdata->gpio_lcdpwr = of_get_named_gpio(of_node, "lcdpoweron-gpios", 0);
		g_8816DRVdata.gpio_reset = of_get_named_gpio(of_node, "tw8816reset-gpios", 0);
		g_8816DRVdata.gpio_lcdpwr = of_get_named_gpio(of_node, "lcdpoweron-gpios", 0);
	}

	tw8816_chip_RESET();
	
	DRVKPOPS( "TW8816 chip initial driver built date=[%s,%s] \n", __DATE__, __TIME__ );
	spriv_LIBdate();

#ifdef _USE_SYSFS_
	sysfs_class = class_create(THIS_MODULE, "tw8816.class");
	if (IS_ERR(sysfs_class)) {
		DRVDBG( KERN_INFO"%s:%s:couldn't create class error !!\n", DrvStr, __FUNCTION__ );
		DRVKERR( "couldn't create class error !!\n" );
		return PTR_ERR(sysfs_class);
	}
	DRVDBG( KERN_INFO"%s:%s:sysfs class create OK !!\n", DrvStr, __FUNCTION__ );
	DRVKERR( "sysfs class create OK !!\n" );
        
	sysfs_class->suspend = sysfs_suspend;        
	sysfs_class->resume = sysfs_resume;
	sysfs_class->dev_attrs = sysfs_attrs;
        
	device_create(sysfs_class, NULL, MKDEV(200, 0), NULL, "tw8816.0");
#endif /* #ifdef _USE_SYSFS_ */
	
#ifdef _USE_400x240_INIT_
	/* 400x240, upper_margin=29, lower_margin=16 */
	init_TW8816reg(client, 'S');
#else 
	/* 800x480, upper_margin=13, lower_margin=32 */
	init_TW8816reg(client, '0');
#endif

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return iRet;
}
#endif	/* CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
static int tw8816_init_remove(struct platform_device *dev)
{
	return 0;	
}
#else
static int __exit tw8816_init_remove(struct i2c_client *client)
{
	
#ifdef _USE_SYSFS_
	device_destroy(sysfs_class, MKDEV(200, 0));
	class_destroy(sysfs_class);
#endif /* #ifdef _USE_SYSFS_ */

	g_8816DRVdata.client = 0;
	return 0;	
}
#endif	/* CONFIG_ARCH_VEXPRESS */


#ifdef CONFIG_ARCH_VEXPRESS
static const struct of_device_id tw8816drv_dt_ids[] = {
	{ .compatible = "fsl,tw8816drv", .data = &tw8816_id[0]},
	{ }
};
MODULE_DEVICE_TABLE(of, tw8816drv_dt_ids);

static struct platform_driver tw8816_driver = {
	.probe		= tw8816_init_probe,
	.remove		= tw8816_init_remove,
	.suspend 	= tw8816_init_drv_suspend,
	.resume 	= tw8816_init_drv_resume,
	.driver 	= {
		.name		= _TW8816DRVNAME_,
		.owner		= THIS_MODULE,
///		.pm			= &tw8816_init_pm_ops,
		.of_match_table = tw8816drv_dt_ids,	/* add for device tree */
	},
};
module_platform_driver(tw8816_driver);
#else

/* add for device tree */
static const struct of_device_id tw8816drv_dt_ids[] = {
	{ .compatible = "fsl,tw8816drv", .data = &tw8816_id[0]},
	{ }
};
MODULE_DEVICE_TABLE(of, tw8816drv_dt_ids);

static SIMPLE_DEV_PM_OPS(tw8816_init_pm_ops, tw8816_init_pm_suspend, tw8816_init_pm_resume);
static struct i2c_driver tw8816_driver = {
	.driver = {
		.name	= _TW8816DRVNAME_,
		.owner	= THIS_MODULE,
		.pm	= &tw8816_init_pm_ops,
		.of_match_table = tw8816drv_dt_ids,	/* add for device tree */
	},
	.probe	= tw8816_init_probe,
	.remove	= tw8816_init_remove,
///	.suspend = tw8816_init_drv_suspend,
///	.resume = tw8816_init_drv_resume,
	.id_table	= tw8816_id,
};
module_i2c_driver(tw8816_driver);
#endif	/* CONFIG_ARCH_VEXPRESS */

static void tw8816_chip_RESET(void)
{
#ifdef _USE_3_10_52_
	int ret;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

	if (!gpio_is_valid(g_8816DRVdata.gpio_reset)) {
		DRVKERR( "gpio_reset[%d] is not valid\n", g_8816DRVdata.gpio_reset );
	}
	if (!gpio_is_valid(g_8816DRVdata.gpio_lcdpwr)) {
		DRVKERR( "gpio_lcdpwr[%d] is not valid\n", g_8816DRVdata.gpio_lcdpwr );
	}


	ret = devm_gpio_request(g_8816DRVdata.pdev, g_8816DRVdata.gpio_reset, "tw8816_codec_reset");
	if (ret < 0) {
		DRVKERR( "devm_gpio_request fail [%d,%s]\n", ret, "tw8816_codec_reset" );
		return ret;
	}
	ret = gpio_direction_output(g_8816DRVdata.gpio_reset, 1);
	if (ret < 0) {
		DRVKERR( "gpio_direction_output fail [%d,%s]\n", ret, "tw8816_codec_reset" );
		///return ret;
	} else {
		gpio_set_value(g_8816DRVdata.gpio_reset, 0);
		gpio_free(g_8816DRVdata.gpio_reset);
		DRVKPOPS( "MX6_TW8816_VP_RESET pin set low OK !!\n" );
	}

	ret = devm_gpio_request(g_8816DRVdata.pdev, g_8816DRVdata.gpio_lcdpwr, "tw8816_codec_pwr_on");
	if (ret < 0) {
		DRVKERR( "devm_gpio_request fail [%d,%s]\n", ret, "tw8816_codec_pwr_on" );
		return ret;
	}
	ret = gpio_direction_output(g_8816DRVdata.gpio_lcdpwr, 1);
	if (ret < 0) {
		DRVKERR( "gpio_direction_output fail [%d,%s]\n", ret, "tw8816_codec_pwr_on" );
		///return ret;
	} else {
		gpio_set_value(g_8816DRVdata.gpio_lcdpwr, 1);
		gpio_free(g_8816DRVdata.gpio_lcdpwr);
		DRVKPOPS( "MX6_TW8816_LCD_PWR_ON pin set high OK !!\n" );
	}

	msleep(100);	/// reset time

	ret = devm_gpio_request(g_8816DRVdata.pdev, g_8816DRVdata.gpio_reset, "tw8816_codec_reset");
	if (ret < 0) {
		DRVKERR( "devm_gpio_request fail [%d,%s]\n", ret, "tw8816_codec_reset" );
		return ret;
	}
	ret = gpio_direction_output(g_8816DRVdata.gpio_reset, 1);
	if (ret < 0) {
		DRVKERR( "gpio_direction_output fail [%d,%s]\n", ret, "tw8816_codec_reset" );
		///return ret;
	} else {
		gpio_set_value(g_8816DRVdata.gpio_reset, 1);
		gpio_free(g_8816DRVdata.gpio_reset);
		DRVKPOPS( "MX6_TW8816_VP_RESET pin set high OK !!\n" );
	}

	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );

#else	
	int ret;
	ret = gpio_request(MX6_TW8816_VP_RESET, "tw8816_codec_reset");
	if( ! unlikely(ret) ) {
		gpio_direction_output(MX6_TW8816_VP_RESET, 1);
		gpio_set_value(MX6_TW8816_VP_RESET, 0);
		gpio_free(MX6_TW8816_VP_RESET);
		DRVKPOPS( "MX6_TW8816_VP_RESET pin set low OK !!\n" );
	} else {
		DRVKPOPS( "MX6_TW8816_VP_RESET pin set low Fail(%d) !!\n", ret );
	}

	ret = gpio_request(MX6_TW8816_LCD_PWR_ON, "tw8816_codec_pwr_on");
	if( ! unlikely(ret) ) {
		gpio_direction_output(MX6_TW8816_LCD_PWR_ON, 1);
		gpio_set_value(MX6_TW8816_LCD_PWR_ON, 1);
		gpio_free(MX6_TW8816_LCD_PWR_ON);
		DRVKPOPS( "MX6_TW8816_LCD_PWR_ON pin set high OK !!\n" );
	} else {
		DRVKPOPS( "MX6_TW8816_LCD_PWR_ON pin set high Fail(%d) !!\n", ret );
	}

	msleep(100);	/// reset time
	
	ret = gpio_request(MX6_TW8816_VP_RESET, "tw8816_codec_reset");
	if( ! unlikely(ret) ) {
		gpio_direction_output(MX6_TW8816_VP_RESET, 1);
		gpio_set_value(MX6_TW8816_VP_RESET, 1);
		gpio_free(MX6_TW8816_VP_RESET);
		DRVKPOPS( "MX6_TW8816_VP_RESET pin set high OK !!\n" );
	} else {
		DRVKPOPS( "MX6_TW8816_VP_RESET pin set high Fail(%d) !!\n", ret );
	}
#endif	
	msleep(150);	/// wait for statable
}

#if 0
static int __init tw8816_init_init(void)
{
	int ret;

	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

#ifdef CONFIG_ARCH_VEXPRESS
#else
	tw8816_chip_RESET();
#endif	/* CONFIG_ARCH_VEXPRESS */

#ifdef CONFIG_ARCH_VEXPRESS
	ret = platform_driver_register(&tw8816_driver);
#else
	ret = i2c_add_driver(&tw8816_driver);
#endif	/* CONFIG_ARCH_VEXPRESS */

#ifdef _USE_SYSFS_
	sysfs_class = class_create(THIS_MODULE, "tw8816.class");
	if (IS_ERR(sysfs_class)) {
		DRVDBG( KERN_INFO"%s:%s:couldn't create class error !!\n", DrvStr, __FUNCTION__ );
		DRVKERR( "couldn't create class error !!\n" );
		return PTR_ERR(sysfs_class);
	}
	DRVDBG( KERN_INFO"%s:%s:sysfs class create OK !!\n", DrvStr, __FUNCTION__ );
	DRVKERR( "sysfs class create OK !!\n" );
        
	sysfs_class->suspend = sysfs_suspend;        
	sysfs_class->resume = sysfs_resume;
	sysfs_class->dev_attrs = sysfs_attrs;
        
	device_create(sysfs_class, NULL, MKDEV(200, 0), NULL, "tw8816.0");
#endif /* #ifdef _USE_SYSFS_ */
	
	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
	return ret;

}
module_init(tw8816_init_init);


static void __exit tw8816_init_exit(void)
{
	DRVDBG( KERN_INFO"%s:%s:+++\n", DrvStr, __FUNCTION__ );
	DRVKNOT( "+++\n" );

#ifdef _USE_SYSFS_
	device_destroy(sysfs_class, MKDEV(200, 0));
	class_destroy(sysfs_class);
#endif /* #ifdef _USE_SYSFS_ */

#ifdef CONFIG_ARCH_VEXPRESS
	platform_driver_unregister(&tw8816_driver);
#else
	i2c_del_driver(&tw8816_driver);
#ifdef _USE_3_10_52_
#else	
	gpio_free(MX6_TW8816_VP_RESET);
	gpio_free(MX6_TW8816_LCD_PWR_ON);	
#endif
#endif	/* CONFIG_ARCH_VEXPRESS */


	DRVKNOT( "---\n" );
	DRVDBG( KERN_INFO"%s:%s:---\n", DrvStr, __FUNCTION__ );
}
module_exit(tw8816_init_exit);
#endif

MODULE_AUTHOR("span liu <spanliu@mic.com.tw>");
MODULE_DESCRIPTION("TW8816 video codec init driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS(_TW8816DRVNAME_);
/* _E_O_F_ */
