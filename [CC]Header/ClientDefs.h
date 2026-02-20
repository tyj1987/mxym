#ifndef _CLIENTDEFS_STUB_H
#define _CLIENTDEFS_STUB_H

// CC模块客户端库编译时的全局定义和宏
// 所有存根定义都内联在这个文件中，避免外部依赖

#include "CommonStruct.h"

// ========== 前向声明 ==========

// 游戏对象类 - 使用前向声明，避免与服务器版本冲突
class CObject;
class CTileManager;
class CHero;
class CMHCamera;
class CMainGame;
class CPet;

// 客户端管理器
class CObjectStateManager;
class CFONT_OBJ;
class CTC_TOWHOLE;
class CPKManager;
class CChatManager;
class cScriptManager;

// 游戏管理器
class CQuickManager;
class CPetManager;
class CGameIn;
class CSuryunDialog;

// ========== 特效和UI类 ==========
// 特效类型定义
typedef DWORD HEFFPROC;

class CEffect;
class CDamageNumber;
class CEngineObject;
class CEffectManager;
class CStatusIconDlg;

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

// ========== 聊天类型枚举 ==========
enum
{
	CTC_OPERATOR,
	CTC_GENERALCHAT,
	CTC_PARTYCHAT,
	CTC_GUILDCHAT,
	CTC_WHISPER,
	CTC_ATTACK,
	CTC_ATTACKED,
	CTC_DEFENCE,
	CTC_DEFENCED,
	CTC_KILLED,
	CTC_MPWARNING,
	CTC_HPWARNING,
	CTC_GETITEM,
	CTC_GETMONEY,
	CTC_SYSMSG,
	CTC_TOWHOLE,
	CTC_TOPARTY,
	CTC_TOGUILD,
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

	void AddMsg(int nClass, int nMsgNum, ...)
	{
		// 空实现
	}

	void AddMsg(int nClass, char* str, ...)
	{
		// 空实现
	}

	char* GetChatMsg(int nMsgNum)
	{
		return NULL;
	}
};

// ========== CQuickManager 存根 ==========
class CQuickManager
{
public:
	static CQuickManager* GetInstance()
	{
		static CQuickManager mgr;
		return &mgr;
	}

	// static void AddAbilityQuickPosition(BYTE Kind, BYTE Pos, POSTYPE QuickPos, ABILITY_TOTALINFO* pOutAbilityTotalInfo)
	static void AddAbilityQuickPosition(BYTE Kind, BYTE Pos, POSTYPE QuickPos, void* pOutAbilityTotalInfo)
	{
		// 空实现
	}

	// static POSTYPE GetAbilityQuickPosition(BYTE Kind, BYTE Pos, ABILITY_TOTALINFO* pOutAbilityTotalInfo)
	static POSTYPE GetAbilityQuickPosition(BYTE Kind, BYTE Pos, void* pOutAbilityTotalInfo)
	{
		return 0;
	}
};

// ========== CPetManager 存根 ==========
class CPetManager
{
public:
	static CPetManager* GetInstance()
	{
		static CPetManager mgr;
		return &mgr;
	}

	// CPet* GetCurSummonPet() 方法存根
	CPet* GetCurSummonPet()
	{
		return NULL;
	}
};

// ========== CGameIn 存根 ==========
class CGameIn
{
public:
	static CGameIn* GetInstance()
	{
		static CGameIn gameIn;
		return &gameIn;
	}
};

// ========== CSuryunDialog 存根 ==========
class CSuryunDialog
{
public:
	static CSuryunDialog* GetInstance()
	{
		static CSuryunDialog dlg;
		return &dlg;
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

// ========== CEffectManager 存根 ==========
class CEffectManager
{
public:
	static CEffectManager* GetInstance()
	{
		static CEffectManager mgr;
		return &mgr;
	}

	// 特效管理方法存根
	void AddEffect(CEffect* pEffect) {}
	void RemoveEffect(CEffect* pEffect) {}
};

// ========== CStatusIconDlg 存根 ==========
class CStatusIconDlg
{
public:
	static CStatusIconDlg* GetInstance()
	{
		static CStatusIconDlg dlg;
		return &dlg;
	}

	void AddIcon(DWORD dwIconType, void* pOwner) {}
	void RemoveIcon(void* pOwner) {}
};

// ========== 全局单例宏 ==========
#define HERO ((CHero*)NULL)
#define CAMERA ((CMHCamera*)NULL)
#define MAINGAME ((CMainGame*)NULL)
#define OBJECTMGR (CObjectManager::GetInstance())
#define MAP (CMHMap::GetInstance())
#define CHATMGR (CChatManager::GetInstance())
#define QUICKMGR (CQuickManager::GetInstance())
#define PETMGR (CPetManager::GetInstance())
#define GAMEIN (CGameIn::GetInstance())
#define SCRIPTMGR (cScriptManager::GetInstance())
#define OBJECTSTATEMGR ((CObjectStateManager*)NULL)

// ========== 路径类型枚举 ==========
enum
{
	PFT_HARDPATH,
	PFT_TEXTURE,
};

#endif
