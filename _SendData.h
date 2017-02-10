/*
 * _SendData.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _SENDDATA_H_
#define _SENDDATA_H_

#include "HeadFileForAll.h"
#include <vector>
class _SendData {
public:
	std::vector<byte> vSendData;// 数据//char
	int iLength;
	byte bNeedAckFlag; // 0:非应答信息,无重发机制; 其他: 应答信息,发送次数为ACK_Flag
	byte bDestCode;
	byte bZongXian;
	byte bPriority;
	byte bType;// 0：基础帧 1：精简帧
public:
	_SendData();
	virtual ~_SendData();
};

#endif /* _SENDDATA_H_ */
