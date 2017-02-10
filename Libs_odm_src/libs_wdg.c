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
//* ��������:  EnableWtd
//* ��������:  ʹ�ܿ��Ź��豸
//* ��ڲ���:
//*           -- TimeOut:     - ָ�����Ź������ι��������룩
//*
//* ����ֵ��      �ɹ�����ERROR_OK(==0)��ʧ�ܷ���(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ��ע:     ��
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
//* ��������:  FeedWtd
//* ��������:  ι������
//* ��ڲ���:  ��
//*
//* ����ֵ��        �ɹ�����ERROR_OK(==0)��ʧ�ܷ���(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ��ע:      ��
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
//* ��������:  DisableWtd
//* ��������:  �رտ��Ź�
//* ��ڲ���:  ��
//*
//* ����ֵ��        �ɹ�����ERROR_OK(==0)��ʧ�ܷ���(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ��ע:      ��
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
