/*
 * _RawInfo.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _RAWINFO_H_
#define _RAWINFO_H_


#include "HeadFileForAll.h"
#include <vector>
class _RawInfo {
public:
	byte bFeature;//00：2.0帧   01：3.0数据帧   10：3.0语音帧
	byte bType;// 0：基础帧 1：精简帧
	byte bZongXian;// CAN0 CAN1
	byte bNeedAck; //0：不要求应答 1：要求应答
	byte bSequence;//发送方数据流水号

	int iInfoLenth; // 信息长度
	byte bSourCode; // 源端口代码
	byte bSourAddreLenth; // 源通信地址长度
	byte arrSourAddre[6]; // 源通信地址//char
	byte bDectCode; // 目的端口代码
	byte bDectAddreLenth; // 目的通信地址长度
	byte arrDectAddre[6]; // 目的通信地址//char
	byte bServiceType; // 业务类型
	byte bCommand; // 命令
	std::vector<byte> vRawInfoData;// 数据//char
	int iDataLenth; // 信息长度
public:
	_RawInfo();
	void Clear();
	virtual ~_RawInfo();

	void AppendData(byte* data, int len);
};

#endif /* _RAWINFO_H_ */
