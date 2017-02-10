/*
 * _AT_COMMAND.h
 *
 *  Created on: 2016-5-26
 *      Author: lenovo
 */

#ifndef _AT_COMMAND_H_
#define _AT_COMMAND_H_

#include "HeadFileForAll.h"

class _AtCommand {
public:
	byte bDeviceCode;
	int iSendTimes;
	std::string strAtCommand;
public:
	_AtCommand(std::string str,int time, byte code);
	_AtCommand();
	virtual ~_AtCommand();
};

#endif /* _AT_COMMAND_H_ */
