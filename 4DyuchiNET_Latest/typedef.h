#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include "stdafx.h"
#include "net_define.h"

// 基本类型定义
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef long			LONG;

// 前向声明
class CNetwork;

// 函数指针类型定义
typedef BOOL (*OnIntialFunc)(DWORD dwBufferSize);
typedef void (*OnAcceptFunc)(DWORD dwConnectID, DWORD dwIP, DWORD dwSendPort, DWORD dwRecvPort);
typedef void (*OnConnectFunc)(DWORD dwConnectID);
typedef void (*OnDisconnectFunc)(DWORD dwConnectID);
typedef void (*OnRecvFunc)(DWORD dwConnectID, char* pMsg, DWORD dwSize);
typedef void (*OnErrorFunc)(DWORD dwErrorCode, char* pErrorMsg);

// 额外的函数指针类型
typedef void (*RECVFUNC)(DWORD dwConnectID, char* pMsg, DWORD dwSize);
typedef void (*DISCONNECTFUNC)(DWORD dwConnectID);
typedef void (*ACCEPTFUNC)(DWORD dwConnectID);
typedef void (*CONNECTSUCCESSFUNC)(DWORD dwConnectID);
typedef void (*CONNECTFAILFUNC)(DWORD dwConnectID, DWORD dwErrorCode);

// 事件回调函数指针类型
typedef void (*EVENTCALLBACK)(DWORD dwEventIndex);
typedef void (*VOIDFUNC)(void);

#endif
