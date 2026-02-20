#ifndef __COMMON_INCLUDES_H__
#define __COMMON_INCLUDES_H__

// 禁用Windows SDK的min/max宏，避免与标准库冲突
#define NOMINMAX

// 禁用安全CRT警告
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// 包含标准库头文件
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

// 包含Windows头文件
#include <windows.h>
#include <winsock2.h>
#include <mmsystem.h>

// 包含项目基础头文件
#include "BasicTypes.h"
#include "CommonDefine.h"
#include "CommonGameFunc.h"
#include "CommonGameDefine.h"
#include "CommonStruct.h"
#include "CommonGameStruct.h"
#include "ServerGameDefine.h"
#include "ServerGameStruct.h"

// 包含辅助函数头文件
#include "HelperFunctions.h"

// 包含库头文件
#include "..\[Lib]YHLibrary\PtrList.h"
#include "..\[Lib]YHLibrary\Array.h"
#include "..\[Lib]YHLibrary\Strclass.h"
#include "..\[Lib]YHLibrary\Fileio.h"

#endif // __COMMON_INCLUDES_H__