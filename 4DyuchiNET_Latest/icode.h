#ifndef _ICODE_H
#define _ICODE_H

#include "code_guid.h"

// ICode接口定义
interface ICode
{
public:
	virtual void SetCodeKey(BYTE* pKey, DWORD dwSize) = 0;
	virtual void Encode(char* pSrc, DWORD dwSrcSize, char** ppDest, DWORD* pdwDestSize) = 0;
	virtual void Decode(char* pSrc, DWORD dwSrcSize, char** ppDest, DWORD* pdwDestSize) = 0;
	virtual void Release() = 0;
};

#endif
