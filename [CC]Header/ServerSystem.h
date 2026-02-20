// ServerSystem.h - 客户端存根文件
// 为客户端编译提供必要的ServerSystem声明

#pragma once

// 对于客户端，使用MAP宏而不是g_pServerSystem
// 此存根仅用于编译，不应实际使用

#ifdef _MAPSERVER_
	// 服务器端：包含服务器版本的MHMap.h和ServerSystem.h
	#include "..\[Server]Map\MHMap.h"
	#include "..\[Server]Map\ServerSystem.h"
#else
	// 客户端：包含客户端版本的MHMap.h以获取eMapKind和CMHMap定义
	// 库编译：使用存根定义，不包含客户端头文件
	#ifndef _MHCLIENT_LIBRARY_
		#include "..\[Client]MH\MHMap.h"
	#endif

	// 客户端：提供最小声明以通过编译
	// 注意：客户端代码应使用MAP宏，而不是g_pServerSystem

	// 前向声明CServerSystem（客户端不实际使用）
	class CServerSystem
	{
	public:
		class CMHMap* GetMap();
		MAPTYPE GetMapNum();
		WORD GetMapKind();
	};

	// 空指针（客户端不应使用）
	extern CServerSystem* g_pServerSystem;
#endif
