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
	std::vector<byte> vSendData;// ����//char
	int iLength;
	byte bNeedAckFlag; // 0:��Ӧ����Ϣ,���ط�����; ����: Ӧ����Ϣ,���ʹ���ΪACK_Flag
	byte bDestCode;
	byte bZongXian;
	byte bPriority;
	byte bType;// 0������֡ 1������֡
public:
	_SendData();
	virtual ~_SendData();
};

#endif /* _SENDDATA_H_ */
