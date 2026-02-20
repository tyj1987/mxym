// stdafx.h - Ability module precompiled header
// This file provides common includes for Ability module compilation

#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Ensure ASSERT macros are always defined
#ifndef ASSERT
#define ASSERT(condition) ((void)0)
#endif

#ifndef ASSERTMSG
#define ASSERTMSG(condition, msg) ((void)0)
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
#ifdef _MHCLIENT_LIBRARY_
    #include "..\[CC]Header\ClientDefs.h"
#endif
