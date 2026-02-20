#ifndef _CLIENTDEFS_STUB_H
#define _CLIENTDEFS_STUB_H

// CC模块客户端编译时的全局定义和宏
// 所有存根定义都内联在这个文件中，避免外部依赖

#include "../[CC]Header/CommonStruct.h"
// #include "cImage.h"  // Don't include directly - cImage.h has its own include guards
#include "PtrList.h"

// CTileManager前向声明
class CTileManager;

// CObject类前向声明
class CObject;

// ========== 客户端类前向声明 ==========
class CHero;
class CMHCamera;
class CMainGame;
class CObjectStateManager;
class CFONT_OBJ;
class CTC_TOWHOLE;
class CPKManager;

// ========== ObjectManager 存根 ==========
class CObjectManager
{
public:
	static CObjectManager* GetInstance()
	{
		static CObjectManager mgr;
		return &mgr;
	}

	CObject* GetObject(DWORD dwID)
	{
		// 空实现
		return NULL;
	}
};

// ========== CMHMap 存根 ==========
class CMHMap
{
public:
	static CMHMap* GetInstance()
	{
		static CMHMap map;
		return &map;
	}

	MAPTYPE GetMapNum()
	{
		return 0;
	}

	CTileManager* GetTileManager()
	{
		return NULL;
	}
};

// ========== CChatManager 存根 ==========
class CChatManager
{
public:
	static CChatManager* GetInstance()
	{
		static CChatManager mgr;
		return &mgr;
	}
};

// ========== cScriptManager 存根 ==========
class cScriptManager
{
public:
	static cScriptManager* GetInstance()
	{
		static cScriptManager mgr;
		return &mgr;
	}
};

// ========== CFONT_OBJ 存根 ==========
class CFONT_OBJ
{
public:
	static CFONT_OBJ* GetInstance()
	{
		static CFONT_OBJ font;
		return &font;
	}
};

// ========== CObjectStateManager 存根 ==========
class CObjectStateManager
{
public:
};

// ========== 全局单例宏 ==========
#define HERO ((CHero*)NULL)
#define CAMERA ((CMHCamera*)NULL)
#define MAINGAME ((CMainGame*)NULL)
#define OBJECTMGR (CObjectManager::GetInstance())
#define MAP (CMHMap::GetInstance())
#define CHATMGR (CChatManager::GetInstance())
#define SCRIPTMGR (cScriptManager::GetInstance())
#define OBJECTSTATEMGR ((CObjectStateManager*)NULL)

// ========== 路径类型枚举 ==========
enum
{
	PFT_HARDPATH,
	PFT_TEXTURE,
};

#endif
