/*
 * _FNStruct.cpp
 *
 *  Created on: 2016-5-25
 *      Author: lenovo
 */

#include "_FnStruct.h"

_FnStruct::_FnStruct() {
	strFunctionNumber = ""; // ���ι��ܺ�
	isRegisterFlag = 0; // 0:ע����1:ע��
	bTaxOrManualFlag = 0; // 0:��TAX��õ� 1:��MMI�˹�����
	bBenBuStatus = 0; // ����״̬
	bRegisterSuccessed = 0; // ��ǵ�ǰ���κ�ע��ע���Ƿ�ɹ�,0δ�ɹ�1�ѳɹ�
}

_FnStruct::~_FnStruct() {
}
