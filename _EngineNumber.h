/*
 * _EngineNumber.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _ENGINENUMBER_H_
#define _ENGINENUMBER_H_

#include "HeadFileForAll.h"

class _EngineNumber {

public:
	byte bArrEngineNumber[8];
	byte bTaxOrManualFlag; // 0:从TAX箱得到 1:从MMI人工输入

public:
	_EngineNumber();
	virtual ~_EngineNumber();
};

#endif /* _ENGINENUMBER_H_ */
