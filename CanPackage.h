/*
 * CanPackage.h
 *
 *  Created on: 2016-8-24
 *      Author: lenovo
 */

#ifndef CANPACKAGE_H_
#define CANPACKAGE_H_
#include <iostream>
#include <iomanip>
#include <vector>
#include "HeadFileForAll.h"

class CanPackage {
public:
	CanPackage();
	virtual ~CanPackage();

	byte bZongXian;
	int iLen;
	std::vector<byte> vData;
};

#endif /* CANPACKAGE_H_ */
