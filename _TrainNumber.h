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
	byte bTaxOrManualFlag; // 0:��TAX��õ� 1:��MMI�˹�����
	byte bBenBuStatus; // ���������01234
public:
	_TrainNumber();
	virtual ~_TrainNumber();
};

#endif /* _TRAINNUMBER_H_ */
