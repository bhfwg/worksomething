/*
 * _AT_COMMAND.cpp
 *
 *  Created on: 2016-5-26
 *      Author: lenovo
 */

#include "_AtCommand.h"
_AtCommand::_AtCommand(std::string str,int time, byte code)
{
	bDeviceCode = code;
	iSendTimes = time;
	strAtCommand = str;
}
_AtCommand::_AtCommand()
{
	bDeviceCode = BOARD_ZH_ALL;
	iSendTimes = 0;
	strAtCommand = "";
}
_AtCommand::~_AtCommand() {
}
