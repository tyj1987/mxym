// stdafx.h : Suryun模块预编译头
//

#pragma once

#include <Winsock2.h>
#include <Windows.h>

#include "../[CC]Header/CommonDefine.h"
#include "../[CC]Header/CommonStruct.h"
#include "../[CC]Header/CommonGameFunc.h"
#include "../[CC]Header/CommonGameStruct.h"
#include "../[CC]Header/CommonGameDefine.h"
#include "../[CC]Header/GameResourceStruct.h"
#include "../[CC]Header/GameResourceManager.h"
#include "SuryunDefine.h"
#include "../[CC]Header/ServerSystem.h"

#include "../[CC]ServerModule/DataBase.h"
#include "../[CC]ServerModule/db.h"

// 客户端库编译模式：使用存根系统避免客户端依赖
#ifdef _MHCLIENT_LIBRARY_
    #include "../[CC]Header/ClientDefs.h"
#endif
