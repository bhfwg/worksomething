/*
 * _FNStruct.cpp
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#include "_FnStruct.h"

_FnStruct::_FnStruct() {
	strFunctionNumber = ""; // 车次功能号
	isRegisterFlag = 0; // 0:注销，1:注册
	bTaxOrManualFlag = 0; // 0:从TAX箱得到 1:从MMI人工输入
	bBenBuStatus = 0; // 本补状态
	bRegisterSuccessed = 0; // 标记当前车次号注册注销是否成功,0未成功1已成功
}

_FnStruct::~_FnStruct() {
}
