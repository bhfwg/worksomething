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
	byte bFeature;//00��2.0֡   01��3.0����֡   10��3.0����֡
	byte bType;// 0������֡ 1������֡
	byte bZongXian;// CAN0 CAN1
	byte bNeedAck; //0����Ҫ��Ӧ�� 1��Ҫ��Ӧ��
	byte bSequence;//���ͷ�������ˮ��

	int iInfoLenth; // ��Ϣ����
	byte bSourCode; // Դ�˿ڴ���
	byte bSourAddreLenth; // Դͨ�ŵ�ַ����
	byte arrSourAddre[6]; // Դͨ�ŵ�ַ//char
	byte bDectCode; // Ŀ�Ķ˿ڴ���
	byte bDectAddreLenth; // Ŀ��ͨ�ŵ�ַ����
	byte arrDectAddre[6]; // Ŀ��ͨ�ŵ�ַ//char
	byte bServiceType; // ҵ������
	byte bCommand; // ����
	std::vector<byte> vRawInfoData;// ����//char
	int iDataLenth; // ��Ϣ����
public:
	_RawInfo();
	void Clear();
	virtual ~_RawInfo();

	void AppendData(byte* data, int len);
};

#endif /* _RAWINFO_H_ */
