#pragma once

#define WIN32_LEAN_AND_MEAN

// 保护标准C++库不受宏定义影响
// 在包含Windows头文件之前保护可能冲突的标识符
#pragma push_macro("abs")
#pragma push_macro("exit")
#ifdef abs
#undef abs
#endif
#ifdef exit
#undef exit
#endif

#include <winsock2.h>
#include <windows.h>
#include <ole2.h>
#include <initguid.h>
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>
#include <time.h>

#pragma pop_macro("exit")
#pragma pop_macro("abs")

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// 定义加密检查宏（用于MSGROOT的Code成员）
#define _CRYPTCHECK_

// 常用定义
#define GUID_SIZE 128
#define MAX_STRING_LENGTH 256
typedef void** PPVOID;

// ASSERT宏
#ifdef _DEBUG
#define ASSERT(condition) \
	do { \
		if(!(condition)) { \
			_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #condition); \
			_CrtDbgBreak(); \
		} \
	} while(0)
#else
#define ASSERT(condition) ((void)0)
#endif

// ASSERTMSG宏 - Only define if not already defined
#ifndef ASSERTMSG
#ifdef _DEBUG
#define ASSERTMSG(condition, msg) \
do { \
	if(!(condition)) { \
		_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, msg); \
		_CrtDbgBreak(); \
	} \
} while(0)
#else
#define ASSERTMSG(condition, msg) ((void)0)
#endif
#endif

// GetCurTime和GetCurTimeToString在CommonGameFunc.cpp中定义

// 包含游戏引擎核心头文件（用于共享模块编译）
#include "D:\mxym\[Lib]YHLibrary\YHLibrary.h"
#include "..\[CC]Header\vector.h"
#include "..\[CC]Header\protocol.h"
#include "..\[CC]Header\CommonDefine.h"
#include "..\[CC]Header\CommonGameDefine.h"
#include "..\[CC]Header\ServerGameDefine.h"
#include "..\[CC]Header\CommonGameStruct.h"
#include "..\[CC]Header\CommonStruct.h"
#include "..\[CC]Header\ServerGameStruct.h"
#include "..\[CC]Header\CommonGameFunc.h"
#include "..\[CC]Header\ServerTable.h"
#include "..\[CC]Header\ServerSystem.h"

// 前向声明服务器特定的全局变量和函数
class CServerTable;
class CServerSystem;
class CTrafficLog;
class CConsole;

// 声明服务器特定的全局变量（在实际服务器项目中定义）
#ifdef __AGENTSERVER__
extern CServerTable * g_pServerTable;
extern CServerSystem * g_pServerSystem;
extern CConsole g_Console;
#define TRAFFIC (g_pServerSystem->GetTrafficLog())
#else
extern CServerTable * g_pServerTable;
extern CServerSystem * g_pServerSystem;
extern CConsole g_Console;
#endif

// 声明连接回调函数
#ifdef __AGENTSERVER__
void OnConnectServerSuccess(DWORD dwConnectionIndex, void* pVoid);
void OnConnectServerFail(void* pVoid);
#endif
