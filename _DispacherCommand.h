/*
 * _DispacherCommand.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _DISPACHERCOMMAND_H_
#define _DISPACHERCOMMAND_H_

#include "HeadFileForAll.h"
#include <vector>
class _DispacherCommand {
public:
	std::vector<byte> vDispatchData;
	int iDispatchLength;
public:
	_DispacherCommand();
	virtual ~_DispacherCommand();
};

#endif /* _DISPACHERCOMMAND_H_ */
