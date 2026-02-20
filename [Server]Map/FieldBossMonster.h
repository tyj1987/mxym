#ifndef __FIELDBOSSMONSTER_H__
#define __FIELDBOSSMONSTER_H__
#include "..\[CC]Header\CommonStruct.h"

#pragma once
#include "monster.h"
#include "FieldBossMonsterManager.h"

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

class CFieldBossMonster :
	public CMonster
{
	// Ҹ/ȸ  ߰ °
	FIELDMONSTERINFO m_Info;

	//ʺ ġ ε 
	int m_nRegenPosIndex;

public:
	CFieldBossMonster(void);
	virtual ~CFieldBossMonster(void);

	virtual BOOL Init(EObjectKind kind,DWORD AgentNum, BASEOBJECT_INFO* pBaseObjectInfo);

	// CMonster ޼  ش. ȸμ   ʿ
	virtual void SetLife(DWORD Life,BOOL bSendMsg = TRUE);
	virtual void SetShield(DWORD Shield,BOOL bSendMsg = TRUE);

	virtual void DoDie(CObject* pAttacker);

	void SetInfo(FIELDMONSTERINFO* Info)
	{
		memcpy(&m_Info, Info, sizeof(FIELDMONSTERINFO));
	}

	BOOL IsBattle();	// Ȳ üũ
	BOOL IsDistruct();	// Ҹð üũ - Ҹð īƮؼ ҸǾ ϸ TRUE Ѵ

	virtual void AddStatus(CStatus* pStatus);
	virtual void StartSpecialState(DWORD SpecialStateKind,DWORD Time,
						WORD wParam1,WORD wParam2,float fParam3,
						WORD EffectNum,WORD StateIcon);
	
	void Recover();		// ȸ μ

	void SetRegenPosIndex(int nIndex)	{ m_nRegenPosIndex = nIndex; }
	int GetRegenPosIndex()				{ return m_nRegenPosIndex; }	

	virtual BOOL DoWalkAround();
};

#endif
