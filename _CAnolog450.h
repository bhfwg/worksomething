/*
 * _CAnolog450.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _CANOLOG450_H_
#define _CANOLOG450_H_

#include "HeadFileForAll.h"

class _CAnolog450 {
public:
    byte bCallStatusIn450; // 0x00:������ ����:ͨ����
	byte bCallStatusIn400;
public:
	_CAnolog450();
	virtual ~_CAnolog450();

};

#endif /* _CANOLOG450_H_ */
