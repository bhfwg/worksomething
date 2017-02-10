/*
 * _GPRSSendData.cpp
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#include "_SendDataNet.h"

_SendDataNet::_SendDataNet() {
	strDestinationIPAddress= "";
	//strDestinationPort = "20001";
	iDestinationPort = 20001;
	lDelayTime = 0;
}

_SendDataNet::~_SendDataNet() {
}
