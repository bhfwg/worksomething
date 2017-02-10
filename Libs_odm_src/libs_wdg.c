/*
 * libs_wdg.c
 *
 *  Created on: 2011-2-16
 *      Author: Administrator
 */

#include "libs_emfuture_odm.h"

#ifdef CFG_WDG_LIB

#define WATCHDOGDEV	"/dev/watchdog"
static int _fd_wdg = -1;

//*------------------------------------------------------------------------------------------------
//* 函数名称:  EnableWtd
//* 功能描述:  使能看门狗设备
//* 入口参数:
//*           -- TimeOut:     - 指定看门狗的最大喂狗间隔（秒）
//*
//* 返回值：      成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:     无
//*------------------------------------------------------------------------------------------------
int EnableWtd(int TimeOut)
{
	int cmd = WDIOS_ENABLECARD;

	if (TimeOut <= 0)
		return ERROR_INPARA;

	if(_fd_wdg<0){
		_fd_wdg = open(WATCHDOGDEV, O_RDWR);
		if (_fd_wdg < 0){
			TRACE("open watchdog %s error\n\r",WATCHDOGDEV);
			return ERROR_SYS;
		}
	}
	//set timeout
	if (ioctl(_fd_wdg, WDIOC_SETTIMEOUT, &TimeOut)) {
		TRACE("seting WDIOC_SETTIMEOUT error\n\r");
		return ERROR_SYS;
	}

	//enable
	if (ioctl(_fd_wdg, WDIOC_SETOPTIONS, &cmd)) {
		TRACE("enable WDIOC_SETOPTIONS error\n\r");
		return ERROR_SYS;
	}

	return ERROR_OK;
}

//*------------------------------------------------------------------------------------------------
//* 函数名称:  FeedWtd
//* 功能描述:  喂狗操作
//* 入口参数:  无
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
int FeedWtd(void)
{
	if(_fd_wdg<0)
		return ERROR_FAIL;

	if (ioctl(_fd_wdg, WDIOC_KEEPALIVE, 0)){
		TRACE("keepalive WDIOC_KEEPALIVE error\n\r");
		return ERROR_SYS;
	}else
		return ERROR_OK;
}


//*------------------------------------------------------------------------------------------------
//* 函数名称:  DisableWtd
//* 功能描述:  关闭看门狗
//* 入口参数:  无
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
int DisableWtd(void)
{
	int cmd = WDIOS_DISABLECARD;

	if(_fd_wdg<0)
		return ERROR_FAIL;

	if (ioctl(_fd_wdg, WDIOC_SETOPTIONS, &cmd))
		return ERROR_SYS;

	return ERROR_OK;

}

#endif
