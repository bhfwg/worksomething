/*
 * CanPackage.cpp
 *
 *  Created on: 2016-8-24
 *      Author: lenovo
 */

#include "CanPackage.h"

CanPackage::CanPackage() {
	bZongXian = CAN0;//默认CAN0,在实例化的时候会修改
	iLen = 0;
}

CanPackage::~CanPackage() {
}
