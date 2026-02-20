// stdafx.h - Skill module precompiled header
// This file provides common includes for Skill module compilation

#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// 修复：如果同时定义了_MHCLIENT_和_MHCLIENT_LIBRARY_，取消_MHCLIENT_LIBRARY_
// 因为_MHCLIENT_表示编译客户端代码（使用完整定义），优先级更高
#ifdef _MHCLIENT_
#ifdef _MHCLIENT_LIBRARY_
#undef _MHCLIENT_LIBRARY_
#endif
#endif

// Include common game headers
#include "..\[CC]Header\CommonDefine.h"
#include "..\[CC]Header\CommonGameFunc.h"
#include "..\[CC]Header\CommonGameDefine.h"
#include "..\[CC]Header\CommonStruct.h"
#include "..\[CC]Header\CommonGameStruct.h"
#include "..\[CC]Header\ServerGameDefine.h"
#include "..\[CC]Header\ServerGameStruct.h"

// Include helper functions
#include "..\[CC]Header\HelperFunctions.h"

// Include server module headers for g_DB
#include "..\[CC]ServerModule\DataBase.h"

// 客户端库编译模式：使用存根系统避免客户端依赖
// 注意：只在单独定义_MHCLIENT_LIBRARY_时使用ClientDefs.h
// 如果同时定义了_MHCLIENT_，说明是编译客户端代码，应使用客户端完整定义
#ifdef _MHCLIENT_LIBRARY_
#ifndef _MHCLIENT_
    #include "..\[CC]Header\ClientDefs.h"
#endif
#endif
