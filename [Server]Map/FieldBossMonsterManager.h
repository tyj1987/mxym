#ifndef __FIELDBOSSMONSTERMANAGER_H__
#define __FIELDBOSSMONSTERMANAGER_H__
#include "..\[CC]Header\CommonStruct.h"

#pragma once

#define MAX_FIELDBOSS_DROPITEM_GROUP_NUM 5
#define MAX_FIELDBOSS_DROPITEM_NUM 10

class CFieldBossMonster;
class CFieldSubMonster;

typedef struct stFieldMonsterInfo
{
	//    
	WORD	m_MonsterKind;
	WORD	m_ObjectKind;
	WORD	m_SubKind;
	WORD	m_SubCount;

	// ð
	DWORD	m_dwRegenTimeMin;
	DWORD	m_dwRegenTimeMax;

	// Ҹ  
	DWORD	m_dwDistructTime;
	DWORD	m_dwCountTime;
	DWORD	m_dwLastCheckTime;

	// ȸ  
	DWORD	m_dwPieceTime;
	DWORD	m_dwRecoverStartTime;
	DWORD	m_dwRecoverDelayTime;
	float	m_fLifeRate;
	float	m_fShieldRate;

	// ʱȭ
	stFieldMonsterInfo()
	{
		m_MonsterKind = 0;
		m_ObjectKind = 0;
		m_SubKind = 0;
		m_SubCount = 0;
		
		m_dwDistructTime = 0;
		m_dwCountTime = 0;
		m_dwLastCheckTime = 0;

		m_dwRegenTimeMin = 0;
		m_dwRegenTimeMax = 0;

		m_dwPieceTime = 0;
		m_dwRecoverStartTime = 0;
		m_dwRecoverDelayTime = 0;
		m_fLifeRate = 0.0f;
		m_fShieldRate = 0.0f;
	}

} FIELDMONSTERINFO;

typedef struct stFieldGroup
{
	WORD	m_GroupID;
	DWORD	m_GridID;
	DWORD	m_BossObjectID;

	CFieldBossMonster*	m_pBoss;
	cPtrList			m_SubList;

	DWORD	m_dwRegenTimeMin;
	DWORD	m_dwRegenTimeMax;

	stFieldGroup()
	{
		m_GroupID = 0;
		m_GridID = 0;

		m_pBoss = NULL;

		m_dwRegenTimeMin = 0;
		m_dwRegenTimeMax = 0;
	}
} FIELDGROUP;

// ʵ    
typedef struct stFieldBossDropItem
{
	WORD	m_wItemIndex[MAX_FIELDBOSS_DROPITEM_NUM];
	WORD	m_wItemCount[MAX_FIELDBOSS_DROPITEM_NUM];
	
	WORD	m_wCount;
	WORD	m_wDropRate;
	WORD	m_wDamageRate;

	stFieldBossDropItem()
	{
		memset( m_wItemIndex, 0, sizeof(WORD) * MAX_FIELDBOSS_DROPITEM_NUM );
		memset( m_wItemCount, 0, sizeof(WORD) * MAX_FIELDBOSS_DROPITEM_NUM );
		
		m_wCount = 0;
		m_wDropRate = 0;
		m_wDamageRate = 0;
	}
} FIELDBOSSDROPITEM;

// ʵ    Ʈ
typedef struct stFieldBossDropItemList
{
	WORD	m_wMonsterKind;

	FIELDBOSSDROPITEM m_ItemList[MAX_FIELDBOSS_DROPITEM_GROUP_NUM];

	stFieldBossDropItemList()
	{
		m_wMonsterKind = 0;
	}
} FIELDBOSSDROPITEMLIST;

#define FIELDBOSSMONMGR USINGTON(CFieldBossMonsterManager)
#define MAX_REGENPOS_NUM 10

/**
 * \ingroup MapServer
 *
 *
 * \par requirements
 * win98 or later\n
 * win2k or later\n
 * MFC\n
 *
 * \version 1.0
 * first version
 *
 * \date 2005-12-14
 *
 * \author Administrator
 *
 * \par license
 * This code is absolutely free to use and modify. The code is provided "as is" with
 * no expressed or implied warranty. The author accepts no liability if it causes
 * any damage to your computer, causes your pet to fall ill, increases baldness
 * or makes your car start emitting strange noises when you start it up.
 * This code has no bugs, just undocumented features!
 * 
 * \todo 
 *
 * \bug 
 *
 */

class CFieldBossMonsterManager
{
	CYHHashTable<FIELDGROUP>					m_FieldGroupTable;
	CMemoryPoolTempl<FIELDGROUP>*				m_pFieldGroupPool;

	CYHHashTable<FIELDMONSTERINFO>				m_FieldMonsterInfoTable;
	CMemoryPoolTempl<FIELDMONSTERINFO>*			m_pFieldMonsterInfoPool;

	CYHHashTable<FIELDBOSSDROPITEMLIST>			m_FieldBossDropItemTable;
	CMemoryPoolTempl<FIELDBOSSDROPITEMLIST>*	m_pFieldBossDropItemPool;

	VECTOR3 m_RegenPos[ MAX_REGENPOS_NUM ];
	WORD m_RegnePosCount;

	char m_LogFile[256];

public:
	CFieldBossMonsterManager(void);
	virtual ~CFieldBossMonsterManager(void);

	void Init();	// ʵ    ε

	void AddRegenPosition(float x, float z);	//  ġ Ʈ ߰
	VECTOR3 GetRegenPosition(int index = -1);	//  ġ ´ -1  
	WORD GetRegenPositionCount() { return m_RegnePosCount; }
	
	void BossDead(CFieldBossMonster* pBoss);	//  ׾
	void SubDead(CFieldSubMonster* pSub);		//  ׾

	void AddFieldBossMonster( CFieldBossMonster* pBoss );	//  ʵ庸߰
	
	void Process();

	FIELDBOSSDROPITEMLIST* GetFieldBossDropItemList( WORD MonsterKind );
};

EXTERNGLOBALTON(CFieldBossMonsterManager);

#endif