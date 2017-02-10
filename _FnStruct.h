/*
 * _FNStruct.h
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#ifndef _FNSTRUCT_H_
#define _FNSTRUCT_H_

#include "HeadFileForAll.h"

class _FnStruct {
public:
	std::string strFunctionNumber ; // 车次功能号
	bool isRegisterFlag; // 0:注销，1:注册
	byte arrFunctionNumber[10] ; // 原始车次号（转换成功能号格式之前）
	byte bTaxOrManualFlag; // 0:从TAX箱得到 1:从MMI人工输入
	byte bBenBuStatus; // 本补状态
	byte bRegisterSuccessed; // 标记当前车次号注册注销是否成功,0未成功1已成功
public:
	_FnStruct();
	virtual ~_FnStruct();
};

#endif /* _FNSTRUCT_H_ */
