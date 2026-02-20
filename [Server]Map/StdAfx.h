// stdafx.h : test version - adding Map headers
#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#define _CRYPTCHECK_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define __MAPSERVER__
#pragma warning(disable : 4786)

#include <windows.h>
#include <winsock2.h>
#include <ole2.h>
#include <initguid.h>
#include <intrin.h>
#include <stdio.h>
#include <assert.h>

#include "DataBase.h"
#include "Console.h"

#include <yhlibrary.h>
#undef min
#undef max
#undef abs

#include <vector>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <algorithm>

// 使用标准库的min/max替代宏
using std::min;
using std::max;

#include "..\[CC]Header\vector.h"
#include "..\[CC]Header\protocol.h"
#include "..\[CC]Header\CommonDefine.h"
#include "..\[CC]Header\CommonGameFunc.h"
#include "..\[CC]Header\CommonGameDefine.h"
#include "..\[CC]Header\ServerGameDefine.h"
#include "..\[CC]Header\CommonGameStruct.h"
#include "..\[CC]Header\CommonStruct.h"
#include "..\[CC]Header\ServerGameStruct.h"

#include ".\Pet.h"
#include ".\BossMonster.h"
#include ".\PetManager.h"
#include ".\Player.h"
#include ".\ServerSystem.h"

extern CServerTable * g_pServerTable;
extern CServerSystem * g_pServerSystem;
extern CConsole g_Console;

// 全局变量 extern 声明
extern class CDataBase g_DB;
extern DWORD gCurTime;

// 确保标准库函数可用（在所有包含之后）
#include <cstring>
#include <cstdio>

#endif
