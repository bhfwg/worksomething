/*
 * _Infotofile.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _INFOTOFILE_H_
#define _INFOTOFILE_H_

#include "HeadFileForAll.h"
#include <vector>
class _InfoToFile {
public:
	int iLost;
	std::vector<byte> vInfoData;// ÄÚÈÝ
public:
	_InfoToFile();
	void Clear();
	virtual ~_InfoToFile();
};

#endif /* _INFOTOFILE_H_ */
