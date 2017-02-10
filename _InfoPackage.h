/*
 * _InfoPackage.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _INFOPACKAGE_H_
#define _INFOPACKAGE_H_

#include "HeadFileForAll.h"
#include <vector>
class _InfoPackage {
public:
	std::vector<byte> vInfoData;// ÄÚÈÝ
public:
	_InfoPackage();
	virtual ~_InfoPackage();
};

#endif /* _INFOPACKAGE_H_ */
