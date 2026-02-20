// stdafx.h : Quest模块预编译头
//

#pragma once

// 先包含Winsock2.h避免与Windows.h冲突
#include <Winsock2.h>
#include <Windows.h>

// 核心头文件
#include "../[CC]Header/CommonDefine.h"
#include "../[CC]Header/CommonStruct.h"
#include "../[CC]Header/CommonGameFunc.h"
#include "../[CC]Header/CommonGameStruct.h"
#include "../[CC]Header/CommonGameDefine.h"

// 其他头文件由各源文件自行包含

// 客户端库编译模式：使用存根系统避免客户端依赖
#ifdef _MHCLIENT_LIBRARY_
    #include "../[CC]Header/ClientDefs.h"
#endif

