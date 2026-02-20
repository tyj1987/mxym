// AbilityUpdater_Interface.cpp: implementation of the CAbilityUpdater_Interface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AbilityUpdater_Interface.h"
#ifdef _MHCLIENT_
#include "SuryunDialog.h"
#include "GameIn.h"
#include "QuickManager.h"
#include "SkillPointRedist.h"
#endif
#include "AbilityGroup.h"
#include "AbilityManager.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAbilityUpdater_Interface::CAbilityUpdater_Interface()
{

}

CAbilityUpdater_Interface::~CAbilityUpdater_Interface()
{

}

void CAbilityUpdater_Interface::Update(DWORD& Flag,DWORD Param,CAbilityGroup* pGroup,CYHHashTable<CAbilityInfo>* pInfoTable)
{
#ifdef _MHCLIENT_
	if(Flag == ABILITYUPDATE_ABILITYQUICK_CHANGED)
		return;

	BYTE Level;
	int State;
	CAbility* pAbility;
	CAbilityInfo* pInfo;
	CSuryunDialog* pDlg = GAMEIN->GetSuryunDialog();
	pDlg->SetAbilityExp(pGroup->GetAbilityExp());
	CSkillPointRedist* pSkDlg = GAMEIN->GetSkPointDlg();
	pSkDlg->SetAbilityExp(pGroup->GetAbilityExp());

	if(Flag != ABILITYUPDATE_ABILITYEXP_CHANGED)
	{
		// to each Ability
		pGroup->SetPositionHead();
		while(pAbility = pGroup->GetNextAbility(eAGK_Max))
		{
			State = eAIS_Usable;
			Level = pAbility->GetLevel();
			pDlg->UpdateAbility(State,Level,pAbility->GetAbilityInfo());
		}

		// each AbilityInfo
		pInfoTable->SetPositionHead();
		while(pInfo = pInfoTable->GetData())
		{
			State = ABILITYMGR->GetState(pGroup,pInfo);
			Level = 0;
			// 06. 01. 수련창이 활성화 비활성화 체크 - 띠아로
			// 수련창은 단순 스킬창이지만 스킬은 아니다
			if(State == eAIS_OnlyVisible || State == eAIS_NotAvailable)
				pDlg->UpdateAbility(State,Level,pInfo);
		}
	}

	if(Flag == ABILITYUPDATE_ABILITYINITED)
	{
		pGroup->SetPositionHead();
		while(pAbility = pGroup->GetNextAbility(eAGK_Max))
		{
			if(pAbility->GetQuickPosition() == 0)
				continue;

			// 06. 01 띠아로 - 퀵창 초기화
			// 초기화할때 생성할것 AddQuickItemReal -> SetQuickItemReal
			// add quick item
			QUICKMGR->SetQuickItemReal(pAbility->GetQuickPosition(),
				pAbility->GetAbilityInfo()->GetAbsPosition(), pAbility->GetIndex() + MIN_ABILITY_INDEX );
/*
			CAbilityIcon* pAbilityIcon = GAMEIN->GetSuryunDialog()->GetAbilityIconAbs(pAbility->GetAbilityInfo()->GetAbsPosition());
			ASSERT(pAbilityIcon);
			if(!pAbilityIcon) continue;

			CQuickItem * pNewQItem = QUICKMGR->NewQuickItem((CBaseItem*)pAbilityIcon);

			pNewQItem->SetPosition(QuickPos);
			GAMEIN->GetQuickDialog()->AddQuickItem(QuickPos, pNewQItem);	*/
		}
	}

	// 06. 01. 수련창이 활성화 비활성화 체크 - 띠아로
	// 수련창에는 갱신해야할게 남아있다
	if(Flag == ABILITYUPDATE_CHARACTERLEVEL_CHANGED)
	{
		pGroup->SetPositionHead();
		while(pAbility = pGroup->GetNextAbility(eAGK_Max))
		{
			pDlg->UpdateAbility(pAbility->GetAbilityInfo());
		}

		pInfoTable->SetPositionHead();
		while(pInfo = pInfoTable->GetData())
		{
			pDlg->UpdateAbility(pInfo);
		}

	}
#else
	// Server-side: empty implementation
#endif
}
