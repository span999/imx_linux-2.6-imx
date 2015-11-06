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

#ifndef __TW8816_INIT_H__
#define __TW8816_INIT_H__



#define		_TW8816DRVNAME_			"tw8816-init"
#define		DrvStr 					"tw8816"
#define 	DRVDEBUG

#ifdef DRVDEBUG
	#if 0
		#define DRVINF(p,...)		;
		#define DRVDBG(p,...) 		;
	#else
		#define DRVINF 				printk
		#define DRVDBG 				printk
	#endif
	#define DRVPWR 					printk
	#define DRVMON 					printk
	#define DRVERR 					printk
	#define DRVKPOP(fmt, args...)	printk( KERN_WARNING "[%s]:" fmt, DrvStr, ##args)
	#define DRVKPOPS(fmt, args...)	printk( KERN_WARNING "[%s]:************************************", DrvStr);	\
									printk( KERN_WARNING "[%s]:" fmt, DrvStr, ##args);	\
									printk( KERN_WARNING "[%s]:************************************", DrvStr)
	#define DRVKERR(fmt, args...)	printk( KERN_WARNING "[%s]%s:%s:%d " fmt, DrvStr, __FILE__, __FUNCTION__, __LINE__, ##args)
	#define DRVKNOT(fmt, args...) 	printk( KERN_NOTICE "[%s]%s:%s:%d " fmt, DrvStr, __FILE__, __FUNCTION__, __LINE__, ##args)
#else
	#define DRVINF(p,...)			;
	#define DRVDBG(p,...) 			;
	#define DRVPWR(p,...)			;
	#define DRVMON(p,...)			;
	#define DRVERR 					printk
	#define DRVKPOP(fmt, args...)	printk( KERN_WARNING "[%s]:" fmt, DrvStr, ##args)
	#define DRVKPOPS(fmt, args...)	printk( KERN_WARNING "[%s]:************************************", DrvStr);	\
									printk( KERN_WARNING "[%s]:" fmt, DrvStr, ##args);	\
									printk( KERN_WARNING "[%s]:************************************", DrvStr)
	#define DRVKERR(fmt, args...)	printk( KERN_WARNING "[%s]%s:%s:%d " fmt, DrvStr, __FILE__, __FUNCTION__, __LINE__, ##args)
	#define DRVKNOT(fmt, args...) 	printk( KERN_NOTICE "[%s]%s:%s:%d " fmt, DrvStr, __FILE__, __FUNCTION__, __LINE__, ##args)
///	#define DRVKNOT(fmt, args...) 	;
#endif


typedef		uint16_t	tREGADDR;
typedef		uint16_t	tREGMSGLV;
typedef		uint8_t		tREGDATA;

typedef struct t_tw8816_reg_date {
	tREGADDR			regAddr;
	tREGDATA			regData;
} tw8816RegDataSet;

typedef struct t_tw8816_ctl_set {
	char 				cCmd;
	tREGADDR 			iReg;
	tREGDATA 			iVal;
	int 				iOk;
} tw8816CtlSet;


enum SYSCMD_STAT{
	csNOT = 0,
	csCMD,
	csSET1,
	csREG,
	csSET2,
	csVAL,
	csSET3,
	csEND
};


int spriv_char2int(char ch);
int spriv_STATnextStat(enum SYSCMD_STAT *pState);
int spriv_CMDnextStat(char *pCmd, enum SYSCMD_STAT *pState);
int spriv_IsSysCmdEnd(char ch);
int spriv_IsSysCmdS(char ch);
int spriv_IsSysRegS(char ch);
int spriv_IsSysCmdOsdTest(char ch);
int spriv_IsSysCmdDump(char ch);
int spriv_IsSysCmdMsgLv(char ch);
int spriv_IsSysCmdReset(char ch);
int spriv_IsSysCmdRead(char ch);
int spriv_IsSysCmdWrite(char ch);
int spriv_IsSysCmdSparS(char ch);
int spriv_IsSysCmd400x240(char ch);
int spriv_IsSysCmd800x480(char ch);
int spriv_LIBdate(void);
int spriv_IsMsgLvRegReadWrite(void);
int spriv_setMsgLv(tREGMSGLV msgLv);

extern const tw8816RegDataSet preloadtbl_osd400x240[];
extern const tw8816RegDataSet preloadtbl_osd400x240_full[];
extern const tw8816RegDataSet preloadtbl_osd800x480[];
extern const tw8816RegDataSet preloadtbl_osd800x480_full[];

#endif	/* __TW8816_INIT_H__ */
/* _E_O_F_ */
