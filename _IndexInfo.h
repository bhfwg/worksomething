/*
 * _Index_Info.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _INDEX_INFO_H_
#define _INDEX_INFO_H_
#include "HeadFileForAll.h"

class _IndexInfo {
public:
	int iPosInFile;// �ļ��е�λ��
	int iInfo; // ��Ϣ����   ԭ����DWORD
public:
	_IndexInfo();
	virtual ~_IndexInfo();
};

#endif /* _INDEX_INFO_H_ */
