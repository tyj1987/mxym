#ifndef _CLIENT_STUBS_H
#define _CLIENT_STUBS_H

// CC模块客户端编译时的存根定义
// 避免包含客户端具体实现

#include "../[CC]Header/CommonStruct.h"
#include "../[CC]Header/CommonGameStruct.h"

// 前向声明客户端类
class CMHCamera;

// 重要：区分客户端库编译和客户端exe编译
// _MHCLIENT_LIBRARY_: 客户端库编译，使用存根定义
// _MHCLIENT_: 客户端exe编译，使用客户端完整定义（不定义存根）
#ifdef _MHCLIENT_LIBRARY_

// CObject基类的存根定义（仅在客户端库编译时使用）
class CEngineObject;
class CObject
{
protected:
	CEngineObject* m_pEngineObject;
	BOOL m_bInited;

public:
	CObject() : m_pEngineObject(NULL), m_bInited(FALSE) {}
	virtual ~CObject() {}

	// 客户端库需要的成员函数
	CEngineObject* GetEngineObject() { return m_pEngineObject; }
	void SetEngineObject(CEngineObject* pEngineObj) { m_pEngineObject = pEngineObj; }
	virtual VECTOR3* GetCurPosition() { static VECTOR3 v = {0,0,0}; return &v; }
	BOOL IsDied() { return FALSE; }
	BOOL IsInited() { return m_bInited; }
	int GetDirectionIndex() { return 0; }
};

// CHero类的存根定义（仅在客户端库编译时使用）
class CHero : public CObject
{
public:
	CHero() : m_fAngle(0.0f) { memset(&m_vPos, 0, sizeof(m_vPos)); }
	virtual ~CHero() {}

	// 客户端库需要的成员函数
	VECTOR3* GetCurPosition() { return &m_vPos; }
	float GetAngleDeg() { return m_fAngle * 57.2957795131f; }
	void SetPosition(const VECTOR3* pPos) { if(pPos) m_vPos = *pPos; }

protected:
	VECTOR3 m_vPos;
	float m_fAngle;
};

#else // _MHCLIENT_ - 客户端exe编译，使用前向声明，由客户端头文件提供完整定义

// 客户端exe编译：只提供前向声明，完整定义由[Client]MH/Object.h等提供
class CEngineObject;
class CObject;
class CHero;

#endif // _MHCLIENT_LIBRARY_

// 简化的管理器类声明（客户端库和客户端exe都需要）
// 注意：客户端exe有自己版本的这些类，所以只在必要时定义

#ifdef _MHCLIENT_LIBRARY_

class CObjectManager
{
public:
	static CObject* GetObject(DWORD dwID) { return NULL; }
};

class CVimuManager
{
public:
	void NetworkMsgParse(BYTE Protocol, void* pMsg) {}
};

class CChatManager
{
};

class CItemManager
{
public:
	int m_nItemUseCount;
	CItemManager() : m_nItemUseCount(0) {}
	static CItemManager* GetInstance() { static CItemManager instance; return &instance; }
};

class CTacticManager
{
public:
	void HeroTacticStart() {}
};

class CEffectManager
{
public:
	DWORD GetOperatorAnimatioEndTime(int EffectNum, int EffectGenderKind, CEngineObject* pEngineObject) { return 0; }
};

#define EFFECTMGR ((CEffectManager*)NULL)

#else // _MHCLIENT_ - 客户端exe编译，这些类由客户端头文件提供

// 客户端exe编译：只提供前向声明
class CObjectManager;
class CVimuManager;
class CChatManager;
class CItemManager;
class CTacticManager;
class CEffectManager;

#endif // _MHCLIENT_LIBRARY_

// CMHMap和CMainGame由客户端头文件提供，此处不定义

class MSGBASE {};

// 简化的消息结构
struct MSG_DWORD2 { DWORD dwData1; DWORD dwData2; };
struct MSG_DWORD3 { DWORD dwData1; DWORD dwData2; DWORD dwData3; };
struct MSG_DWORD4 { DWORD dwObjectID; DWORD dwData1; DWORD dwData2; DWORD dwData3; DWORD dwData4; };
struct MSG_BATTLE_INFO {};
struct MSG_BATTLE_TEAMMEMBER_ADDDELETE
{
	BYTE Team;
	struct { DWORD MemberID; char MemberName[32]; } Member;
};
struct SEND_SW_BTGUILDLIST
{
	BYTE Team;
};

// 宏定义
#define HERO ((CHero*)NULL)
#define OBJECTMGR ((CObjectManager*)NULL)
#define VIMUMGR ((CVimuManager*)NULL)
#define MAP ((CMHMap*)NULL)
#define CAMERA ((CMHCamera*)NULL)
#define MAINGAME ((CMainGame*)NULL)

// 战斗相关枚举
enum { eBattleTeam1 = 0, eBattleTeam2 = 1 };
enum BATTLE_KIND { eBATTLE_KIND_NONE = 0, eBATTLE_KIND_VIMUSTREET, eBATTLE_KIND_MURIMFIELD, 
                   eBATTLE_KIND_MUNPAFIELD, eBATTLE_KIND_SURYUN, eBATTLE_KIND_GTOURNAMENT, 
                   eBATTLE_KIND_SIEGEWAR, eBATTLE_KIND_MAX };
typedef DWORD MAPTYPE;

// 效果相关
struct OBJECTEFFECTDESC
{
	OBJECTEFFECTDESC(DWORD effectNum) : m_dwEffectNum(effectNum) {}
	DWORD m_dwEffectNum;
};
DWORD FindEffectNum(const char* name) { return 0; }

#endif
