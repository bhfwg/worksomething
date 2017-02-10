/*
 * _GPRSSendData.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _GPRSSENDDATA_H_
#define _GPRSSENDDATA_H_

#include "HeadFileForAll.h"
#include <vector>
class _SendDataNet {
public:
	std::vector<byte> vSendData;// Êý¾Ý//char
	std::string strDestinationIPAddress;
	//std::string strDestinationPort;
	int iDestinationPort;
	long lDelayTime;
public:
	_SendDataNet();
	virtual ~_SendDataNet();
};

#endif /* _GPRSSENDDATA_H_ */
