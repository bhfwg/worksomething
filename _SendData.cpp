/*
 * _SendData.cpp
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#include "_SendData.h"

_SendData::_SendData() {
	iLength = 0;
	bNeedAckFlag = ACK0;
    bDestCode = BOARD_BROADCAST;
    bZongXian = CAN0;//Ĭ��CAN0,��ʵ������ʱ����޸�
	bPriority = PRIORITY0;
	bType = SHORTFRAME;
}

_SendData::~_SendData() {
}
