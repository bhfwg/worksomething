/*
 * _RawInfo.cpp
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#include "_RawInfo.h"

_RawInfo::_RawInfo() {
	iInfoLenth = 0;
	bSourCode = 0;
	bSourAddreLenth = 0;
	bDectCode = 0;
	bDectAddreLenth = 0;
	bServiceType = 0;
	bCommand = 0;
}

_RawInfo::~_RawInfo()
{
}
void _RawInfo::Clear()
{
}
void _RawInfo::AppendData(byte* data, int len)
{
	for(int i=0; i<len; i++)
		vRawInfoData.push_back(*(data+i));
	iDataLenth += len;
}
