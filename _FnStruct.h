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
	std::string strFunctionNumber ; // ���ι��ܺ�
	bool isRegisterFlag; // 0:ע����1:ע��
	byte arrFunctionNumber[10] ; // ԭʼ���κţ�ת���ɹ��ܺŸ�ʽ֮ǰ��
	byte bTaxOrManualFlag; // 0:��TAX��õ� 1:��MMI�˹�����
	byte bBenBuStatus; // ����״̬
	byte bRegisterSuccessed; // ��ǵ�ǰ���κ�ע��ע���Ƿ�ɹ�,0δ�ɹ�1�ѳɹ�
public:
	_FnStruct();
	virtual ~_FnStruct();
};

#endif /* _FNSTRUCT_H_ */
