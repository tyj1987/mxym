// ServerSystem.h: interface for the CServerSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERSYSTEM_H__FD3EBFC3_EE3D_4505_A5A1_24DA471D20AB__INCLUDED_)
#define AFX_SERVERSYSTEM_H__FD3EBFC3_EE3D_4505_A5A1_24DA471D20AB__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 服务器专用：确保只在服务器编译时定义这些类型
#ifdef _MAPSERVER_

#include "MHMap.h"
#include "D:\mxym\[Lib]YHLibrary\IndexGenerator.h"
#include "D:\mxym\[Lib]YHLibrary\PtrList.h"
class CObject;
class CGridSystem;

class CPlayer;
class CTitan;
class CPet;
class CMonster;
class CNpc;
class CTacticObject;
class CTacticStartInfo;
class CSkillObject;
class CBossMonster;
class CMapObject;

// �ʵ庸�� - 05.12 �̿���
class CFieldBossMonster;
class CFieldSubMonster;

enum eNATION
{
	eNATION_KOREA,
	eNATION_CHINA,
};

enum{ePET_FROM_DB, ePET_FROM_ITEM};		//DB�κ��� �����, ��ȯ������ ù������� �ֻ���

enum eMapKind
{
	eNormalMap,
	eSurvival,
	//eRunningMap,
	eGTMap,	//��������
	eSGMap, //��������
};

struct WAIT_EXIT_PLAYER_INFO
{
	DWORD dwAgentConnectionIndex;
	DWORD dwPlayerID;
	DWORD dwStartTime;
};


class CServerSystem  
{
//	CYHHashTable<CItemObject> m_ItemObjectTable;
	CGridSystem* m_pGridSystem;
	CMHMap m_Map;
	
	friend class CCharMove;

	friend void GameProcess();

	BOOL m_start;
//	CIndexGenerator m_objectIdxGen;
	
	WORD	m_wMapNum;

	//SW061019 �ʼ���
	WORD	m_iMapKind;
	
//����
	int		m_Nation;

	cPtrList									m_listWaitExitPlayer;
	CMemoryPoolTempl<WAIT_EXIT_PLAYER_INFO>*	m_mpWaitExitPlayer;
	
	DWORD	m_dwQuestTime;
	BOOL	m_bQuestTime;

	BOOL	m_bCompletionChrUpdate;
//--check process time
	DWORD	m_dwMainProcessTime;


public:
	DWORD GetMainProcessTime() { return m_dwMainProcessTime; }

	// RaMa - ����Ȯ�ο�
	MSG_PROSESSINFO		m_ProcessInfo;
	BOOL				m_bCheckInfo;
	void CheckServerInfo( DWORD ObjectID );
	//

public:

#ifdef _CHINA_LOCAL_
//���
 	SYSTEMTIME		m_NoPV;
 	int				m_NoPCode;
 	char			m_NoP[128];
//---
#endif


	CServerSystem();
	virtual ~CServerSystem();

	void AddWaitExitPlayer( DWORD dwAgentConnetionIndex, DWORD dwPlayerID );
	void ProcessWaitExitPlayer();
//����
	void SetNation();
	int GetNation() { return m_Nation; }

	void _5minProcess();
	void Process();
	void Start(WORD ServerNum);
	void End();		

	CMHMap* GetMap()	{ return &m_Map;	}
	WORD	GetMapNum()	{ return m_wMapNum;	}
	
	BOOL GetStart()		{ return m_start; 	}
	void SetStart(BOOL state);

	void	SetMapKind( WORD wMapNum );
	BOOL	CheckMapKindIs( int eMapkind );
	/*
	CObject* GetObject(DWORD dwObjectID);
	CPlayer* GetPlayer(DWORD dwPlayerID);
	CMonster* GetMonster(DWORD dwMonsterID);
	CNpc* GetNpc(DWORD dwNpcID);
	*/
//	CItemObject* GetItemObject(DWORD dwItemID);
	
	CPlayer* AddPlayer(DWORD dwPlayerID, DWORD dwAgentNum,DWORD UniqueIDinAgent,int ChannelNum);
	CPlayer* InitPlayerInfo(BASEOBJECT_INFO* pBaseObjectInfo,CHARACTER_TOTALINFO* pTotalInfo,HERO_TOTALINFO* pHeroInfo);

	CTitan*	MakeTitan(CPlayer* pOwner);
	TITAN_TOTALINFO* MakeTitanInfo(TITAN_TOTALINFO* pInfo);
	void	RemoveTitanInfo(TITAN_TOTALINFO* pInfo);

	void CreateNewPet(CPlayer* pMaster, DWORD dwItemIdx, DWORD SummonItemDBIdx, WORD wPetKind, WORD PetGrade = 1);
	CPet*	AddPet(CPlayer* pMaster, DWORD dwPetObjID, PET_TOTALINFO* pTotalInfo,VECTOR3* pPos);
//	CPet*	AddPet(DWORD dwPetObjID, CPlayer* pMaster, BASEOBJECT_INFO* pBaseObjectInfo, PET_TOTALINFO* pTotalInfo, WORD wPetKind, int flag = ePET_FROM_DB);
	CMonster* AddMonster(DWORD dwSubID, BASEOBJECT_INFO* pBaseObjectInfo,MONSTER_TOTALINFO* pTotalInfo,VECTOR3* pPos,WORD wObjectKind = 32);
	CBossMonster* AddBossMonster(DWORD dwSubID, BASEOBJECT_INFO* pBaseObjectInfo,MONSTER_TOTALINFO* pTotalInfo,VECTOR3* pPos);
	
	// �ʵ庸�� - 05.12 �̿���
	CFieldBossMonster* AddFieldBossMonster(DWORD dwSubID, BASEOBJECT_INFO* pBaseObjectInfo,MONSTER_TOTALINFO* pTotalInfo,VECTOR3* pPos);
	CFieldSubMonster* AddFieldSubMonster(DWORD dwSubID, BASEOBJECT_INFO* pBaseObjectInfo,MONSTER_TOTALINFO* pTotalInfo,VECTOR3* pPos);
	
	//CNpc* AddNpc(BASEOBJECT_INFO* pBaseObjectInfo,NPC_TOTALINFO* pTotalInfo,VECTOR3* pPos);
	//SW070626 ���λ�NPC
	CNpc* AddNpc(BASEOBJECT_INFO* pBaseObjectInfo,NPC_TOTALINFO* pTotalInfo,VECTOR3* pPos,float angle = 0);

	CTacticObject* AddTacticObject(BASEOBJECT_INFO* pBaseObjectInfo,CTacticStartInfo* pInfo,CPlayer* pOperator,DIRINDEX Direction);
	CSkillObject* AddSkillObject(CSkillObject* pSkillObj,VECTOR3* pPos);
	CMapObject* AddMapObject(DWORD Kind, BASEOBJECT_INFO* pBaseObjectInfo, MAPOBJECT_INFO* pMOInfo, VECTOR3* pPos);

	void RemovePlayer(DWORD dwPlayerID, BOOL bRemoveFromUserTable = TRUE);
	void RemoveTitan(CTitan* pTitan);
	void RemovePet(DWORD dwPetObjectID);
	void RemoveMonster(DWORD dwPlayerID);
	void RemoveBossMonster(DWORD dwMonster, BOOL bDisappear=FALSE);
	void RemoveNpc(DWORD dwPlayerID);
	void RemoveTacticObject(DWORD dwTacticObjID);
	void RemoveItemObject(DWORD ItemID);
	void RemoveSkillObject(DWORD SkillObjectID);
	void RemoveMapObject( DWORD MapObjID );

	
	inline CGridSystem* GetGridSystem()	{	return m_pGridSystem;	}

	void SendToOne(CObject* pObject,void* pMsg,int MsgLen);
	
	void ReloadResourceData();
	
	//////////////////////////////////////////////////////////////////////////
	// Npc AI���� �Լ���
	//void MonsterStateProc();	

	void	SetCharUpdateCompletion(BOOL bVal)	{ m_bCompletionChrUpdate = bVal;	}
	BOOL	GetCharUpdateCompletion()	{	return m_bCompletionChrUpdate;	}
	void	RemoveServerForKind( WORD wSrvKind );
	void	HandlingBeforeServerEND();

	void	LoadHackCheck();
};

#endif // _MAPSERVER_

void GameProcess();
void _5minGameProcess();
extern CServerSystem * g_pServerSystem;
void OnConnectServerSuccess(DWORD dwIndex, void* pVoid);
void OnConnectServerFail(void* pVoid);
BOOL LoadEventRate(char* strFileName);
BOOL LoadPartyPlustime(char* strFileName);


#endif // !defined(AFX_SERVERSYSTEM_H__FD3EBFC3_EE3D_4505_A5A1_24DA471D20AB__INCLUDED_)
