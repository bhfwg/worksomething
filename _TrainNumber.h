/*
 * _TrainNumber.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _TRAINNUMBER_H_
#define _TRAINNUMBER_H_
#include "HeadFileForAll.h"


class _TrainNumber {
public:
    byte arrNumber[7];
	byte bTaxOrManualFlag; // 0:从TAX箱得到 1:从MMI人工输入
	byte bBenBuStatus; // 本务机补机01234
public:
	_TrainNumber();
	virtual ~_TrainNumber();
};

#endif /* _TRAINNUMBER_H_ */
