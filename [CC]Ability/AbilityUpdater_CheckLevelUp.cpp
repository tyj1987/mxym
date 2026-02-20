// AbilityUpdater_CheckLevelUp.cpp: implementation of the CAbilityUpdater_CheckLevelUp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AbilityUpdater_CheckLevelUp.h"
#include "AbilityGroup.h"
#include "AbilityCommonHeader.h"
#include "..\[Server]Map\Player.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAbilityUpdater_CheckLevelUp::CAbilityUpdater_CheckLevelUp()
{

}

CAbilityUpdater_CheckLevelUp::~CAbilityUpdater_CheckLevelUp()
{

}

BOOL CAbilityUpdater_CheckLevelUp::GetAbility(CAbilityGroup* pGroup,CAbilityInfo* pAbilityInfo)
{
	BYTE InitLevel = pAbilityInfo->GetInfo()->Ability_AcquireInitLevel;
	if(InitLevel == 0)
		return FALSE;

	pGroup->AddAbility(pAbilityInfo,InitLevel,0);
	return TRUE;
}

void CAbilityUpdater_CheckLevelUp::Update(DWORD& UpdateCmd,DWORD Param,CAbilityGroup* pGroup,CYHHashTable<CAbilityInfo>* pInfoTable)
{
	BOOL bAbilityLevelChanged = FALSE;
	CPlayer* pObj = pGroup->GetOwenerObject();
	CAbilityInfo* pAbilityInfo;
	ABILITY_INFO* pInfo;
	pInfoTable->SetPositionHead();
	while(pAbilityInfo = pInfoTable->GetData())
	{
		//  0 Ư⿡ ؼ ˻ Ѵ.
		if(pGroup->GetAbility(pAbilityInfo->GetIdx()))
			continue;

		pInfo = pAbilityInfo->GetInfo();

		// ɸ  ȭ
		if( UpdateCmd == ABILITYUPDATE_CHARACTERLEVEL_CHANGED &&
			pInfo->Ability_AcquireKind == eAAK_CharacterLevel )
		{
			LEVELTYPE Level = (LEVELTYPE)Param;
			if(Level >= pInfo->Ability_AcquireParam1)
				bAbilityLevelChanged |= GetAbility(pGroup,pAbilityInfo);
		}

		// ٸ Ư  ȭ
		if( UpdateCmd == ABILITYUPDATE_ABILITYLEVEL_CHANGED &&
			pInfo->Ability_AcquireKind == eAAK_OtherAbility )
		{
			WORD NeedAbilityIdx = pInfo->Ability_AcquireParam1;
			BYTE NeedAbilityLevel = (BYTE)pInfo->Ability_AcquireParam2;
			CAbility* pNeedAbility = pGroup->GetAbility(NeedAbilityIdx);
			if(pNeedAbility == NULL)
				continue;
			if(pNeedAbility->GetLevel() >= NeedAbilityLevel)
				bAbilityLevelChanged |= GetAbility(pGroup,pAbilityInfo);
		}
		
		//  
		if( UpdateCmd == ABILITYUPDATE_ABILITYUPITEM_USE &&
			pInfo->Ability_AcquireKind == eAAK_Item )
		{
			WORD UsedItemIdx = (WORD)Param;
			WORD NeedItemIdx = pInfo->Ability_AcquireParam1;
			if(UsedItemIdx == NeedItemIdx)
				bAbilityLevelChanged |= GetAbility(pGroup,pAbilityInfo);
		}

		// Ʈ... -_-a
	}

	if(bAbilityLevelChanged)
	{
		UpdateCmd = ABILITYUPDATE_ABILITYLEVEL_CHANGED;
		Update(UpdateCmd,0,pGroup,pInfoTable);
	}
}
