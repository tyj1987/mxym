// SkillManager.cpp: implementation of the CSkillManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _MHCLIENT_

// 包含适配器层 - 必须在其他包含之前
#include "..\\[CC]Header\\Client\\ClientObjectAdapter.h"

// 重要：必须先包含Object.h，让编译器看到完整的CObject定义
#include "..\\[Client]MH\\Object.h"

#include "SkillManager_client.h"
#include "ActionTarget.h"

// 现在可以安全地包含Hero.h，它依赖于Object.h
#include "Hero.h"
#include "MoveManager.h"
#include "QuickManager.h"
#include "Gamein.h"
#include "QuickItem.h"
#include "..\\[Client]MH\\ObjectStateManager.h"
#include "..\\[Client]MH\\Effect\\Effect.h"
#include "..\\[Client]MH\\Effect\\Effect.h"
#include "..\\[Client]MH\\Effect\\EffectManager.h"
#include "ObjectManager.h"
// 客户端使用客户端版本的TacticManager
#include "..\\[Client]MH\\TacticManager.h"

#include "ChatManager.h"
#include "ExchangeManager.h"

#include "GameResourceManager.h"
#include "QuickDialog.h"

#include "PKManager.h"
#include "ObjectActionManager.h"
#include "BattleSystem_Client.h"
#include "../[CC]BattleSystem/GTournament/Battle_GTournament.h"

#include "PeaceWarModeManager.h"

#include "MAINGAME.h"

#include "SkillDelayManager.h"

#include "PeaceWarModeManager.h"
#include "InventoryExDialog.h"
#include "Item.h"
#include "PartyWar.h"
// 客户端使用客户端版本的ItemManager
#include "..\\[Client]MH\\ItemManager.h"

#include "MHNetwork.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//GLOBALTON(CSkillManager);
CSkillManager::CSkillManager()
{
	g_PoolSTLIST.Init(100,50,"g_PoolSTLIST");
	m_pSkillInfo = NULL;

	m_HwaKyungSkillTable.Initialize(30);
	m_GeukMaSkillTable.Initialize(30);

	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ���� ��ȯ �߰�
	m_SkillOptionTable.Initialize(30);
	m_SkillOptionByItemTable.Initialize(30);
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	m_SpecialStateInfoTable.Initialize(3);
	//////////////////////////////////////////////////////////////////////////

	// debug��
	m_nSkillUseCount = 0;
}

CSkillManager::~CSkillManager()
{
	Release();
	g_PoolSTLIST.Release();
}

void CSkillManager::Init()
{
	m_SkillInfoTable.Initialize(2000);
//	m_DummySkillInfoTable.Initialize(64);
	m_SkillObjectTable.Initialize(512);
	LoadSkillInfoList();
	LoadSkillChangeInfoList();

	m_SkillAreaMgr.LoadSkillAreaList();

	m_JobSkillProbabilityTable.Initialize(MAX_JOBLEVEL_NUM);	// 2007. 6. 28. CBH - ������� Ȯ�� ���̺� �ʱ�ȭ /////////////////
	LoadJobSkillProbability();				// 2007. 6. 28. CBH - ������� Ȯ�� ���ҽ� �ε� /////////////////

	// debug��
	m_nSkillUseCount = 0;
}
void CSkillManager::Release()
{
	CSkillInfo* pSInfo = NULL;

	m_SkillInfoTable.SetPositionHead();
	while(pSInfo = m_SkillInfoTable.GetData())
	{
		delete pSInfo;
	}
	m_SkillInfoTable.RemoveAll();
/*	
	m_DummySkillInfoTable.SetPositionHead();
	while(p = m_DummySkillInfoTable.GetData())
	{
		delete p;
	}
	m_DummySkillInfoTable.RemoveAll();
*/	
	CSkillObject* pSObj = NULL;

	m_SkillObjectTable.SetPositionHead();
	while(pSObj = m_SkillObjectTable.GetData())
	{
		OBJECTMGR->AddGarbageObject((CObject*)pSObj);
		delete pSObj;
	}
	m_SkillObjectTable.RemoveAll();

	m_SkillAreaMgr.Release();

	PTRLISTSEARCHSTART(m_SkillChangeList,SKILL_CHANGE_INFO*,pInfo)
		delete pInfo;
	PTRLISTSEARCHEND
	m_SkillChangeList.RemoveAll();

	m_HwaKyungSkillTable.RemoveAll();
	m_GeukMaSkillTable.RemoveAll();

	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ���� ��ȯ �߰�
	SKILLOPTION* pSOpt = NULL;

	m_SkillOptionTable.SetPositionHead();
	while(pSOpt = m_SkillOptionTable.GetData())
	{
		delete pSOpt;
	}
	m_SkillOptionTable.RemoveAll();
	m_SkillOptionByItemTable.RemoveAll();
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	SPECIAL_STATE_INFO* pStateInfo = NULL;

	m_SpecialStateInfoTable.SetPositionHead();
	while(pStateInfo = m_SpecialStateInfoTable.GetData())
	{
		delete pStateInfo;
	}
	m_SpecialStateInfoTable.RemoveAll();
	//////////////////////////////////////////////////////////////////////////

	////// 2007. 6. 28. CBH - ������� Ȯ�� ����Ʈ ���� /////////////////
	JOB_SKILL_PROBABILITY_INFO* pJobSkillInfo = NULL;

	m_JobSkillProbabilityTable.SetPositionHead();
	while(pJobSkillInfo = m_JobSkillProbabilityTable.GetData())
	{
		SAFE_DELETE(pJobSkillInfo);		
	}
	m_JobSkillProbabilityTable.RemoveAll();
	////////////////////////////////////////////////////////////////////////

	// debug��
	m_nSkillUseCount = 0;
}

void CSkillManager::LoadSkillInfoList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	file.Init("Resource/SkillList.bin","rb");
#else
	file.Init("Resource/SkillList.txt","rt");
#endif
	if(file.IsInited() == FALSE)
	{
		//ASSERTMSG(0,"SkillList�� �ε����� ���߽��ϴ�.");
		return;
	}

	while(1)
	{
		if(file.IsEOF() != FALSE)
			break;

		CSkillInfo* pInfo = new CSkillInfo;
//		CSkillInfo* pDummyInfo = new CSkillInfo;
		pInfo->InitSkillInfo(&file);

		//	2005 ũ�������� �̺�Ʈ �ڵ�
		//////////////////////////////////////////////////////////////////////////
		//	��ų �� ���� üũ��...
		WORD SkillIndex = pInfo->GetSkillIndex();
	
		//////////////////////////////////////////////////////////////////////////
		
//		pDummyInfo->InitDummySkillInfo(pInfo);

		ASSERT(m_SkillInfoTable.GetData(pInfo->GetSkillIndex()) == NULL);
		m_SkillInfoTable.Add(pInfo,pInfo->GetSkillIndex());
//		m_DummySkillInfoTable.Add(pDummyInfo,pInfo->GetSkillIndex());
	}

	file.Release();

	file.Init("Resource/SAT.bin","rb");
	if(file.IsInited() == FALSE)
	{
		MessageBox(0,"SAT.bin is not found",0,0);
		return;
	}

	int count = file.GetDword();
	for(int n=0;n<count;++n)
	{
		WORD skillIdx = file.GetWord();
		DWORD aniTimeMale = file.GetDword();
		DWORD aniTimeFemale = file.GetDword();

		CSkillInfo* pSkillInfo = GetSkillInfo(skillIdx);
		if(pSkillInfo == NULL)
			continue;

		pSkillInfo->SetSkillOperateAnimationTime(aniTimeMale,aniTimeFemale);
	}
	file.Release();

	LoadSkillTreeList();

	// ȭ��, �ظ� ����Ʈ
	LoadJobSkillList();
	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ���� ��ȯ �߰�
	LoadSkillOptionList();
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	LoadStateList();
	//////////////////////////////////////////////////////////////////////////
}

void CSkillManager::LoadSkillTreeList()
{
	CMHFile file;
	file.Init("Resource/SkillTree.bin","rb");

	if(file.IsInited() == FALSE)
	{
		return;
	}

	while(1)
	{
		if(file.IsEOF() != FALSE)
			break;

		WORD Array[5];

		file.GetString();
		file.GetWord(Array,5);
		
		for(int n=0;n<5;++n)
		{
			WORD Before = 0,After = 0;
			WORD Cur = Array[n];
			if(n != 0) Before = Array[n-1];
			if(n != 4) After = Array[n+1];
			
			CSkillInfo* pInfo = GetSkillInfo(Cur);
			if(pInfo == NULL)
				continue;

			pInfo->SetSkillTree(Before,After,Array);
		}
	}
	file.Release();
}


void CSkillManager::LoadJobSkillList()
{
	CMHFile file;
	file.Init("Resource/JobSkillList.bin","rb");

	if(file.IsInited() == FALSE)
	{
		return;
	}


	char buf[32];
	int Count = 0;
	DWORD SkillIdx = 0;

	while(1)
	{
		if(file.IsEOF() != FALSE)
			break;

		file.GetString(buf);

		if( strcmp( buf, "#HWAKUNG" ) == 0 )
		{
			Count = file.GetInt();

			for(int i=0; i<Count; ++i)
			{
				SkillIdx = file.GetDword();
				CSkillInfo* pInfo = m_SkillInfoTable.GetData( SkillIdx );
				if( !pInfo )		continue;

				m_HwaKyungSkillTable.Add( pInfo, SkillIdx );
			}
		}
		if( strcmp( buf, "#GEUKMA" ) == 0 )
		{
			Count = file.GetInt();

			for(int i=0; i<Count; ++i)
			{
				SkillIdx = file.GetDword();
				CSkillInfo* pInfo = m_SkillInfoTable.GetData( SkillIdx );
				if( !pInfo )		continue;

				m_GeukMaSkillTable.Add( pInfo, SkillIdx );
			}
		}
	}

	file.Release();
}


void CSkillManager::LoadSkillChangeInfoList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	file.Init("Resource/SkillChangeList.bin","rb");
#else
	file.Init("Resource/SkillChangeList.txt","rt");
#endif
	if(file.IsInited() == FALSE)
	{
		ASSERTMSG(0,"SkillChangeList�� �ε����� ���߽��ϴ�.");
		return;
	}

	while(1)
	{
		if(file.IsEOF() != FALSE)
			break;
		SKILL_CHANGE_INFO * pInfo = new SKILL_CHANGE_INFO;
		pInfo->wMugongIdx = file.GetWord();
		pInfo->wChangeRate = file.GetByte();
		pInfo->wTargetMugongIdx = file.GetWord();

		m_SkillChangeList.AddTail(pInfo);
	}
}

//////////////////////////////////////////////////////////////////////////
// 06. 06. 2�� ���� - �̿���
// ���� ��ȯ �߰�
void CSkillManager::LoadSkillOptionList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	file.Init("Resource/SkillOptionList.bin","rb");
#else
	file.Init("Resource/SkillOptionList.txt","rt");
#endif
	if(file.IsInited() == FALSE)
	{
		ASSERTMSG(0,"SkillOptionList�� �ε����� ���߽��ϴ�.");
		return;
	}

	while(1)
	{
		if(file.IsEOF() != FALSE)
			break;

		SKILLOPTION* pSOpt = new SKILLOPTION;

		memset(pSOpt, 0, sizeof(SKILLOPTION));

		pSOpt->Index		= file.GetWord();
		pSOpt->SkillKind	= file.GetWord();
		pSOpt->OptionKind	= file.GetWord();
		pSOpt->OptionGrade	= file.GetWord();
		pSOpt->ItemIndex	= file.GetWord();
		
		for(int i = 0; i < MAX_SKILLOPTION_COUNT; i++)
		{
			WORD Kind = file.GetWord();
			
			switch(Kind)
			{
			case eSkillOption_Range:
				pSOpt->Range = file.GetInt();
				break;
			
			case eSkillOption_ReduceNaeRyuk:
				pSOpt->ReduceNaeRyuk = file.GetFloat();
				break;
			
			case eSkillOption_PhyAtk:
				pSOpt->PhyAtk = file.GetFloat();
				break;
				
			case eSkillOption_BaseAtk:
				pSOpt->BaseAtk = file.GetFloat();
				break;
			
			case eSkillOption_AttAtk:
				pSOpt->AttAtk = file.GetFloat();
				break;

			case eSkillOption_Life:
				pSOpt->Life = file.GetInt();
				break;

			case eSkillOption_NaeRyuk:
				pSOpt->NaeRyuk = file.GetInt();
				break;

			case eSkillOption_Shield:
				pSOpt->Shield = file.GetInt();
				break;

			case eSkillOption_PhyDef:
				pSOpt->PhyDef = file.GetFloat();
				break;

			case eSkillOption_AttDef:
				pSOpt->AttDef = file.GetFloat();
				break;

			case eSkillOption_Duration:
				pSOpt->Duration = file.GetDword();
				break;

			case eSkillOption_None:
			default:
				file.GetWord();
				break;
			}
		}

		m_SkillOptionTable.Add(pSOpt, pSOpt->Index);
		m_SkillOptionByItemTable.Add(pSOpt, pSOpt->ItemIndex);
	}
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// 06. 06. 2�� ���� - �̿���
// ����/����
void CSkillManager::LoadStateList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	file.Init("Resource/StateInfo.bin","rb");
#else
	file.Init("Resource/StateInfo.txt","rt");
#endif
	if(file.IsInited() == FALSE)
	{
		ASSERTMSG(0,"StateList�� �ε����� ���߽��ϴ�.");
		return;
	}

	while(1)
	{
		if(file.IsEOF() != FALSE)
			break;

		SPECIAL_STATE_INFO* pInfo = new SPECIAL_STATE_INFO;

		memset(pInfo, 0, sizeof(SPECIAL_STATE_INFO));

		pInfo->Index = file.GetWord();
		pInfo->IconIdx = file.GetWord();
		pInfo->TickTime = file.GetDword();
		pInfo->NaeRyuk = file.GetWord();
		pInfo->DelayTime = file.GetDword();
		pInfo->DurationTime = file.GetDword();

		m_SpecialStateInfoTable.Add(pInfo, pInfo->Index);
	}
}
//////////////////////////////////////////////////////////////////////////

CSkillInfo*	CSkillManager::GetSkillInfo(WORD SkillInfoIdx)
{
	return (CSkillInfo*)m_SkillInfoTable.GetData(SkillInfoIdx);
}
/*
CSkillInfo*	CSkillManager::GetDummySkillInfo(WORD SkillInfoIdx)
{
	return (CSkillInfo*)m_DummySkillInfoTable.GetData(SkillInfoIdx);
}
*/
WORD CSkillManager::GetSkillTooltipInfo(WORD SkillInfoIdx)
{
	SKILLINFO* skInfo = ((CSkillInfo*)m_SkillInfoTable.GetData(SkillInfoIdx))->GetSkillInfo();
	if(!skInfo)
		return 0;
	return skInfo->SkillTooltipIdx;
}
CSkillObject* CSkillManager::GetSkillObject(DWORD SkillObjectID)
{
	return (CSkillObject*)m_SkillObjectTable.GetData(SkillObjectID);
}
WORD CSkillManager::GetComboSkillIdx(CHero* pHero)
{
	WORD SkillNum = 0;
	int WeaponEquipType = pHero->GetWeaponEquipType();
	int CurComboNum = pHero->GetCurComboNum();
	if( CurComboNum == SKILL_COMBO_NUM || 
		CurComboNum == MAX_COMBO_NUM ||
		CurComboNum >= pHero->GetLevel() * 2)
	{
		CurComboNum = 0;
		pHero->SetCurComboNum(0);
	}

	// ���� �޺�+1 ���� �޺��� ���ù�ȣ�� ���;� ������
	// SkillNum = COMBO_???_MIN + �޺���ȣ - 1 = COMBO_???_MIN + CurComboNum +1 -1
	// so... COMBO_???_MIN + CurComboNum �̴�.
	switch(WeaponEquipType)
	{
	case WP_GUM:
		SkillNum = COMBO_GUM_MIN + CurComboNum;
		break;
	case WP_GWUN:
		SkillNum = COMBO_GWUN_MIN + CurComboNum;
		break;
	case WP_DO:
		SkillNum = COMBO_DO_MIN + CurComboNum;
		break;
	case WP_CHANG:
		SkillNum = COMBO_CHANG_MIN + CurComboNum;
		break;
	case WP_GUNG:
		SkillNum = COMBO_GUNG_MIN + CurComboNum;
		break;
	case WP_AMGI:
		SkillNum = COMBO_AMGI_MIN + CurComboNum;
		break;
	//	2005 ũ�������� �̺�Ʈ �ڵ�
	case WP_EVENT:
		SkillNum = COMBO_EVENT_MIN;
		break;
	// 2006 �߼�
	case WP_EVENT_HAMMER:
		SkillNum = COMBO_EVENT_HAMMER;
		break;
	}

	// !!!!!!!!!!! magi82 - ������ Ÿ��ź �޺��� ���� ������������ ������ ���ؼ� �ϴ� �̷��� �ӽ÷� �� !!!!!!!!!!!!!!!!!!1
	if(pHero->InTitan())
		SkillNum += 10000;

	return SkillNum;
}

//////////////////////////////////////////////////////////////////////////
// ���� ������ ���Ŵ������� ���õ� ������ ������
// ����â���� ����Ŭ������ ����ҽ� ����â���� ���Ǿ��� ������
// ���Ǵ� ���׷� ��ų �Ŵ����� ���������� ����� ��ų�� �����ϵ��� �Ͽ�
// ������� �ʴ� �Լ�
/*
WORD CSkillManager::GetMugongSkillIdx(CHero* pHero)
{	
	WORD SkillNum = 0;
#ifdef _TESTCLIENT_
	SkillNum = GAMERESRCMNGR->m_TestClientInfo.MugongIdx;
#else
	int abspos = QUICKMGR->GetSelectedQuickAbsPos();
	CQuickItem* pQuickItem;
	pQuickItem = GAMEIN->GetQuickDialog()->GetQuickItem(abspos);
	if(pQuickItem == NULL)
		return FALSE;
	SkillNum = pQuickItem->GetSrcIdx();
#endif
	return SkillNum;
}
*/
//////////////////////////////////////////////////////////////////////////

// ��ų�� ���
BOOL CSkillManager::ExcuteSkillSYN(CHero* pHero,CActionTarget* pTarget,BOOL bMugong)
{
/*
	WORD SkillNum;
	if(bMugong == FALSE)
		SkillNum = GetComboSkillIdx(pHero);
	else
		SkillNum = GetMugongSkillIdx(pHero);

	CSkillInfo* pSkillInfo = GetSkillInfo(SkillNum);
*/
	WORD SkillNum;
	CSkillInfo* pSkillInfo;

	if(bMugong == FALSE)
	{
		SkillNum = GetComboSkillIdx(pHero);
		pSkillInfo = GetSkillInfo(SkillNum);
	}
	else
		pSkillInfo = m_pSkillInfo;

	//ASSERT(pSkillInfo);
	if(pSkillInfo == NULL) 
		return FALSE;

	return ExcuteSkillSYN(pHero,pTarget,pSkillInfo);
}

BOOL CSkillManager::ExcuteSkillSYN(CHero* pHero,CActionTarget* pTarget,CSkillInfo* pSkillInfo)
{
	CActionTarget target;
	target.CopyFrom(pTarget);
	SKILLOPTION* pSkillOption = NULL;

	//ASSERT(pSkillInfo);
	if(pSkillInfo == NULL)
		return FALSE;
	
	int SkillLevel = 0;
	//if( pHero->InTitan() && pSkillInfo->GetSkillIndex() > SKILLNUM_TO_TITAN )
	//	SkillLevel = pHero->GetMugongLevel( pSkillInfo->GetSkillIndex() - SKILLNUM_TO_TITAN );
	//else
		SkillLevel = pHero->GetMugongLevel(pSkillInfo->GetSkillIndex());
	
	//SW070127 Ÿ��ź
	if( 0 == SkillLevel )
		return FALSE;

	WORD wSkillKind = pSkillInfo->GetSkillKind();

	// magi82 - Titan(070912) Ÿ��ź ����������Ʈ
	// ���� Ÿ��ź ����� ĳ������ ����� �����̴�.(���Ⱑ ���� �ٸ��ٰ� �ؼ� ��ų�� �ȳ����°� �ƴ�)
	//if( (pHero->InTitan() == TRUE) && (CheckSkillKind(wSkillKind) == FALSE) )
	//{
	//	WORD weapon = pHero->GetWeaponEquipType();
	//	WORD titanWeapon = pHero->GetTitanWeaponEquipType();
	//	if(weapon != titanWeapon)
	//	{
	//		pHero->DisableAutoAttack(); //���� ��ų�� ���� ������ �ڵ����� ����� ����.		
	//		CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1644));
	//		return FALSE;
	//	}
	//}

	WORD SkillOptionIndex = pHero->GetSkillOptionIndex(pSkillInfo->GetSkillIndex());
	
	if(SkillOptionIndex)
		pSkillOption = m_SkillOptionTable.GetData(SkillOptionIndex);

	if(pSkillInfo->IsExcutableSkillState(pHero,SkillLevel,pSkillOption) == FALSE)
	{
		WORD wSkillKind = pSkillInfo->GetSkillKind();
		if( CheckSkillKind(wSkillKind) == FALSE )
		{
			pHero->SetCurComboNum(0);
		}	
		//"���ݸ���!"
		return FALSE;
	}
	
	// ���� ����̸� ���� �޴��� ������ ������.
	if(pSkillInfo->GetSkillKind() == SKILLKIND_JINBUB)
	{
/*
				if( pTarget->GetTargetID() != 0 )
				{
					CObject* pTargetObj = OBJECTMGR->GetObject( pTarget->GetTargetID() );
					if( pTargetObj )
					{
		                if( !PARTYWAR->IsEnemy( (CPlayer*)pTargetObj ) )	return FALSE;
					}
				}
*/		

		// ����Start��ȣ�� ����Skill��ȣ�� ����.
		TACTICMGR->HeroTacticStart(pSkillInfo->GetSkillIndex());
		return FALSE;
	}

	if(pSkillInfo->ComboTest(pHero) == FALSE)
	{
		return FALSE;
	}
	
	if(pSkillInfo->ConvertTargetToSkill(pHero,&target) == FALSE)
	{
		return FALSE;
	}

	// 2007. 7. 3. CBH - ������ų�ߵ��� ���Ϳ��� ���� ó�� �Լ� �߰�
	if(!IsJobSkill(pHero, pTarget, pSkillInfo))
	{
		return FALSE;
	}

	if(pSkillInfo->IsValidTarget(pHero, &target) == FALSE)
		return FALSE;

	// magi82(5) - Titan(071023) Ÿ��ź ������ ���� üũ�Ҷ� Ÿ�� üũ���� �ڿ� ���ƾ� Ÿ���� ���Ͱ� �ƴҶ� ä�ø޼����� �����ʴ´�.
	if(CheckTitanWeapon(pHero, pSkillInfo->GetSkillInfo()) == FALSE)
	{
		return FALSE;
	}

//KES 040308

	if(PEACEWARMGR->IsPeaceMode(pHero) == TRUE)		//KES�ű�
		PEACEWARMGR->ToggleHeroPeace_WarMode();
	
// RaMa ȭ��, �ظ� üũ
	if( pHero->GetStage() & eStage_Hwa )
	{
		if( m_GeukMaSkillTable.GetData( pSkillInfo->GetSkillIndex() ) )
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1144), CHATMGR->GetChatMsg(892) );
			return FALSE;
		}
	}
	else if( pHero->GetStage() & eStage_Geuk )
	{
		if( m_HwaKyungSkillTable.GetData( pSkillInfo->GetSkillIndex() ) )
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1144), CHATMGR->GetChatMsg(890) );
			return FALSE;
		}
	}

	
//
	VECTOR3* pTargetPos = target.GetTargetPosition();
	if(pTargetPos == NULL)
		return FALSE;
	
	if(pSkillInfo->IsInSkillRange(pHero,&target,pSkillOption) == TRUE)
	{	// ����		
		MOVEMGR->HeroMoveStop();
		
		//�ڽ��� Ÿ���� ��쿡�� ���� ������ �ٲ��� �ʴ´�.
		if( pSkillInfo->GetSkillInfo()->TargetKind != 1 )
			MOVEMGR->SetLookatPos(pHero,pTargetPos,0,gCurTime);
				
		pHero->SetMovingAction(NULL);		

		////////////////////////////////////////////////////////
		//06. 06 2�� ���� - �̿���
		//����Ʈ ����(����)
		//���� ���¿��� ���� ���� ������ ���� �������� ����
		if( pHero->IsSkipSkill() == eSkipEffect_Start && pSkillInfo->CanSkipEffect() )
			return RealExcuteSkillSYN(pHero,&target,pSkillInfo);
		////////////////////////////////////////////////////////		

		if( pSkillInfo->GetSkillInfo()->EffectStart != 0 &&
			pSkillInfo->GetSkillInfo()->EffectStartTime != 0)	// ���������� ���� ���
		{
			return HeroSkillStartEffect(pHero,&target,pSkillInfo);
		}
		else
		{
			return RealExcuteSkillSYN(pHero,&target,pSkillInfo);
		}
	}
	else
	{	// ����
		pHero->SetCurComboNum(0);
		CAction MoveAction;
		if(pSkillInfo->IsMugong() == FALSE)
		{
			pSkillInfo = GetSkillInfo(GetComboSkillIdx(pHero));
		}
		MoveAction.InitSkillAction(pSkillInfo,&target);
		MOVEMGR->SetHeroActionMove(&target,&MoveAction);
		return FALSE;
	}
	

	return TRUE;
}

BOOL CSkillManager::ExcuteTacticSkillSYN(CHero* pHero,TACTIC_TOTALINFO* pTInfo,BYTE OperNum)
{
	CActionTarget target;
	target.InitActionTarget(&pTInfo->Pos,NULL);

	CSkillInfo* pSkillInfo = GetSkillInfo(pTInfo->TacticId);
	//ASSERT(pSkillInfo);
	if(pSkillInfo == NULL)
		return FALSE;

	return RealExcuteSkillSYN(pHero,&target,pSkillInfo);
}

void CSkillManager::GetMultiTargetList(CSkillInfo* pSkillInfo,CHero* pHero,CActionTarget* pTarget)
{
	WORD Radius = pSkillInfo->GetSkillInfo()->TargetRange;
	WORD AreaNum = pSkillInfo->GetSkillInfo()->TargetAreaIdx;
	if(AreaNum != 0)
	{
		CSkillArea* pSkillArea = GetSkillArea(pHero,pTarget,pSkillInfo);	// Area�� �߽���ǥ���� ���õǾ��� �´�.
		pTarget->SetTargetObjectsInArea(pHero,pSkillArea);
	}
	else if(Radius != 0)
	{
		/// 06. 08. �ڱ��߽ɹ����� ��ų ���� ���� - �̿���
		/// �ڱ� �߽� ������ ��쿡 ���ݰ� ���� ��ȯ�� ���� �����Ÿ� ȿ����
		/// ���� ������ ����Ǿ�� �Ѵ�.
		if( pSkillInfo->GetSkillInfo()->TargetAreaPivot == 1 && pSkillInfo->GetSkillInfo()->TargetRange != 0 )
		{
			Radius += (WORD)HERO->GetAddAttackRange();

			WORD SkillOptionIndex = HERO->GetSkillOptionIndex(pSkillInfo->GetSkillIndex());
			SKILLOPTION* pSkillOption = NULL;

			if(SkillOptionIndex)
			{
				pSkillOption = m_SkillOptionTable.GetData(SkillOptionIndex);
			}

			if(pSkillOption)
			{
				Radius += pSkillOption->Range;
			}	
		}

		pTarget->SetTargetObjectsInRange(pSkillInfo->GetTargetAreaPivotPos(&pHero->GetCurPosition(),pTarget->GetTargetPosition()),Radius);
	}
}

BOOL CSkillManager::RealExcuteSkillSYN(CHero* pHero,CActionTarget* pTarget,CSkillInfo* pSkillInfo)
{
/*
	CSkillInfo* pDummyInfo = GetDummySkillInfo(pSkillInfo->GetSkillIndex());
	if(pDummyInfo->CheckOriginal(pSkillInfo) == FALSE)
	{
		MSGBASE msg;
		SetProtocol(&msg,MP_USERCONN,MP_USERCONN_CHEAT_USING);
		msg.dwObjectID = HEROID;
		NETWORK->Send(&msg,sizeof(msg));

//		MAINGAME->SetGameState(eGAMESTATE_END);
		
		return FALSE;
	}
*/
	SKILLOPTION* pSkillOption = NULL;

	if(pSkillInfo == NULL)
		return FALSE;
		
	int SkillLevel = pHero->GetMugongLevel(pSkillInfo->GetSkillIndex());

	WORD SkillOptionIndex = pHero->GetSkillOptionIndex(pSkillInfo->GetSkillIndex());
	
	if(SkillOptionIndex)
		pSkillOption = m_SkillOptionTable.GetData(SkillOptionIndex);

	if(pSkillInfo->IsExcutableSkillState(pHero,SkillLevel,pSkillOption) == FALSE)
	{
		pHero->SetCurComboNum(0);
		//"���ݸ���!"
		return FALSE;
	}

	// 2005 ũ�������� �̺�Ʈ
	if( pHero->GetWeaponEquipType() == WP_EVENT && pSkillInfo->GetSkillIndex() == COMBO_EVENT_MIN )
	{
		//CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemLike( EVENT_ITEM_SNOW );
		//SW061211 ũ���������̺�Ʈ
		CItem* pItem = GAMEIN->GetInventoryDialog()->GetPriorityItemForCristmasEvent();
		
		if( !pItem )
		{
		//	CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(583) );
			return	FALSE;
		}

//		GAMEIN->GetInventoryDialog()->UseItem( pItem );
		
		// ���� Ŭ������ ���Ǿ����� �Ͷ����� ���� ����
		MSG_ITEM_USE_SYN msg;

		msg.Category = MP_ITEM;
		msg.Protocol = MP_ITEM_USE_SYN;
		msg.dwObjectID = HEROID;
		msg.wItemIdx = pItem->GetItemIdx();
		msg.TargetPos = pItem->GetPosition();

		NETWORK->Send(&msg,sizeof(msg));

		// debug��
		ITEMMGR->m_nItemUseCount++;
	}

	// ������ ���º���
	if(pSkillInfo->GetSkillKind() == SKILLKIND_JINBUB && pHero->GetNaeRyuk() < pSkillInfo->GetNeedNaeRyuk(1))
	{
		CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(401) );		
		return FALSE;
	}

	//�����ϸ� pk���ӽð��� �����Ѵ�.
	if( HERO->IsPKMode() )
	{
		CObject* pObject = OBJECTMGR->GetObject(pTarget->GetTargetID());
		if( pObject )
		if( pObject->GetObjectKind() == eObjectKind_Player )
		{
			int SkillLevel = pHero->GetMugongLevel(pSkillInfo->GetSkillIndex());
			if( pSkillInfo->GetFirstPhyAttack( SkillLevel ) ||
				pSkillInfo->GetFirstAttAttack( SkillLevel ) )
			{
				PKMGR->SetPKStartTimeReset();
			}
		}
	}

	HERO->SetNextAction(NULL);

	if(pSkillInfo->IsMultipleTargetSkill() == TRUE)
	{
		pTarget->ConvertMainTargetToPosition(pHero,pSkillInfo->GetSkillRange());
		GetMultiTargetList(pSkillInfo,pHero,pTarget);
	}

	CSkillObject* pSObj = CreateTempSkillObject(pSkillInfo,pHero,pTarget);
	if(pSObj == NULL)
		return FALSE;
	
	pSkillInfo->SendMsgToServer(pHero,pTarget);
	
	pHero->SetSkillCoolTime(pSkillInfo->GetSkillInfo()->DelayTime);

	if(pHero->IsSkipSkill())
	{
		pHero->GetDelayGroup()->AddDelay(
			CDelayGroup::eDK_Skill,pSkillInfo->GetSkillIndex(),
			pSkillInfo->GetSkillInfo()->DelayTime - pSkillInfo->GetSkillOperateAnimationTime( 1 ));
	}
	else
	{
		pHero->GetDelayGroup()->AddDelay(
			CDelayGroup::eDK_Skill,pSkillInfo->GetSkillIndex(),
			pSkillInfo->GetSkillInfo()->DelayTime);
	}
	//���⼭ ������ ��ų�� ������ ���������.
	SKILLDELAYMGR->AddSkillDelay( pSkillInfo->GetSkillIndex() );


	pHero->SetCurComboNum(pSkillInfo->GetSkillInfo()->ComboNum);

	m_pSkillInfo = NULL;

#ifdef _TESTCLIENT_
	static IDDDD = 0;
	MSG_SKILLOBJECT_ADD msg;
	SKILLOBJECT_INFO info;
	memcpy(&info,pSObj->GetSkillObjectInfo(),sizeof(SKILLOBJECT_INFO));
	info.SkillObjectIdx = SKILLOBJECT_ID_START+IDDDD++;
	info.StartTime = gCurTime;
	info.SkillLevel = 5;
	msg.InitMsg(&info,TRUE);
	CTargetListIterator iter(&msg.TargetList);
	pTarget->SetPositionFirstTargetObject();
	CBattle* pBattle = BATTLESYSTEM->GetBattle(HERO);
	while(CObject* pObject = pTarget->GetNextTargetObject())
	{
		RESULTINFO dinfo;
		dinfo.Clear();
		if(pBattle->IsEnemy(pHero,pObject) == TRUE)
		{
			dinfo.bCritical = 1;//rand()%5 ? FALSE : TRUE;
			dinfo.RealDamage = rand() % 50;
			dinfo.ShieldDamage = rand() % 30;
			dinfo.CounterDamage = 0;
			dinfo.StunTime = 1000;//(rand() % 30 == 0) ? 2000 : 0;
			iter.AddTargetWithResultInfo(pObject->GetID(),SKILLRESULTKIND_NEGATIVE,&dinfo);
		}
		else
		{
			iter.AddTargetWithResultInfo(pObject->GetID(),SKILLRESULTKIND_POSITIVE,&dinfo);
		}

		
	}

	iter.Release();

	NetworkMsgParse(MP_SKILL_SKILLOBJECT_ADD,&msg);
#endif

	//SW05810 ��ȭ��� �ڵ���ȯ �۾�
	PEACEWARMGR->SetCheckTime(gCurTime);

	return TRUE;
}

// ���� �޺� ��ų�� ����
void CSkillManager::SetNextComboSkill(CHero* pHero,CActionTarget* pTarget,BOOL bMugong)
{
	CSkillInfo* pNextSkill;
	WORD NextSkillIdx;
/*
	if(bMugong)
		NextSkillIdx = GetMugongSkillIdx(pHero);
	else
		NextSkillIdx = GetComboSkillIdx(pHero);
	pNextSkill = GetSkillInfo(NextSkillIdx);
*/ //GetMugongSkillIdx() �Լ��� ������� �ʰ� �Ǿ� ����
	if(bMugong)
		pNextSkill = m_pSkillInfo;
	else
	{
		NextSkillIdx = GetComboSkillIdx(pHero);
		pNextSkill = GetSkillInfo(NextSkillIdx);
	}
	
	if(pNextSkill == NULL)
		return;
	CAction act;
	act.InitSkillAction(pNextSkill,pTarget);
	pHero->SetNextAction(&act);
}

// �̺�Ʈ �ڵ� �Լ���
BOOL CSkillManager::OnSkillCommand(CHero* pHero,CActionTarget* pTarget,BOOL bMugong)
{
	//////////////////////////////////////////////////////////////////////////
	// ���� ����� �����Ҽ� ����
	if(pTarget->GetTargetID() != 0)
	{
		CObject* pTObj = OBJECTMGR->GetObject(pTarget->GetTargetID());
		if(pTObj == NULL)
			return FALSE;
		if(pTObj->IsDied() == TRUE)
			return FALSE;

		if( pHero->GetWeaponEquipType() != WP_EVENT )
		{
			if( pTObj->GetObjectKind() & eObjectKind_Monster )
			{
				CMonster* pMonster = (CMonster*)pTObj;
				if(	pMonster->GetMonsterKind() == EVENT_FIELDBOSS_SNOWMAN_SM || pMonster->GetMonsterKind() == EVENT_FIELDSUB_SNOWMAN_SM
					|| pMonster->GetMonsterKind() == EVENT_FIELDBOSS_SNOWMAN_MD || pMonster->GetMonsterKind() == EVENT_FIELDSUB_SNOWMAN_MD
					|| pMonster->GetMonsterKind() == EVENT_FIELDBOSS_SNOWMAN_LG || pMonster->GetMonsterKind() == EVENT_FIELDSUB_SNOWMAN_LG
/*					|| pMonster->GetMonsterKind() == EVENT_SNOWMAN_SM
					|| pMonster->GetMonsterKind() == EVENT_SNOWMAN_MD
					|| pMonster->GetMonsterKind() == EVENT_SNOWMAN_LG*/ )
					return FALSE;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	

	// Guild Tournament�� ���������� Observer�̸� ���Ұ�
	CBattle* pBattle = BATTLESYSTEM->GetBattle( HERO );
	if( pBattle && pBattle->GetBattleKind() == eBATTLE_KIND_GTOURNAMENT ||
		pBattle && pBattle->GetBattleKind() == eBATTLE_KIND_SIEGEWAR )
	{
		if( pHero->GetBattleTeam() == 2 )
			return FALSE;	
	}

	// 2005 ũ�������� �̺�Ʈ �ڵ�
	// �̺�Ʈ ���� ������ �����̰� ������ ���ݺҰ� ���� ��� �Ұ�
	if( pHero->GetWeaponEquipType() == WP_EVENT )
	{
		if( bMugong )
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(586) );	
			return FALSE;
		}
		else
		{
			//CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemLike( EVENT_ITEM_SNOW );
			//SW061211 ũ���������̺�Ʈ
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetPriorityItemForCristmasEvent();

			if( !pItem )
			{
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(583) );
				return	FALSE;
			}
		}
	}
	else if( pHero->GetWeaponEquipType() == WP_EVENT_HAMMER )
	{
		if( bMugong )
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(798) );	
			return FALSE;
		}
	}

	if( pHero->GetState() == eObjectState_SkillSyn ||
		pHero->GetState() == eObjectState_SkillUsing)
	{
		// 2007. 7. 6. CBH - ������ų�� �ڵ� ������ ���ƾ��Ѵ�.
		CObject* pObject = OBJECTMGR->GetObject(pTarget->GetTargetID());
		if(pObject == NULL) //����ó��
		{
			return FALSE;
		}

		if( GetObjectKindGroup(pObject->GetObjectKind()) == eOBJECTKINDGROUP_JOB )
		{
			pHero->SetStage(eObjectState_None);
			return FALSE;
		}
		else
		{
			SetNextComboSkill(pHero,pTarget,bMugong);
		}		
	}
	else
		ExcuteSkillSYN(pHero,pTarget,bMugong);		//return FALSE ó��....����.. ��������?
	return TRUE;
}
DWORD GetComboDelayTime(WORD WeaponKind)
{
	DWORD time = 0;
	ySWITCH(WeaponKind)
		yCASE(WP_GUM)	time = 120;
		yCASE(WP_GWUN)	time = 150;
		yCASE(WP_DO)	time = 150;
		yCASE(WP_CHANG)	time = 150;
		yCASE(WP_GUNG)	time = 50;
		yCASE(WP_AMGI)	time = 80;
	yENDSWITCH
	return time;
}
void CSkillManager::OnComboTurningPoint(CHero* pHero)
{
	if(pHero->GetNextAction()->HasAction())
	{
		if(pHero->GetNextAction()->GetActionKind() != eActionKind_Skill)
			pHero->SetCurComboNum(0);

		pHero->GetNextAction()->ExcuteAction(pHero);
		pHero->GetNextAction()->Clear();
	}
	else
	{		
		if(pHero->IsAutoAttacking())
		{
			if(pHero->GetCurComboNum() < 2)	// �ڵ������� �޺� 2������	12/3�� ȸ�� ��� 3���� 2�� �ٲ�
			{
				if(SKILLMGR->OnSkillCommand(pHero,pHero->GetAutoAttackTarget(),FALSE) == FALSE)
					pHero->DisableAutoAttack();
			}
			else
			{
				OBJECTSTATEMGR->StartObjectState(pHero,eObjectState_SkillDelay);
				OBJECTSTATEMGR->EndObjectState(pHero,eObjectState_SkillDelay,GetComboDelayTime(pHero->GetWeaponEquipType()));
				pHero->SetCurComboNum(0);
			}
		}
	}
}
void CSkillManager::OnExcuteSkillNACKed(SKILLOBJECT_INFO* pInfo)
{
	//ASSERT(0);
}

// SkillObject ��� �� ����
void CSkillManager::DoCreateSkillObject(CSkillObject* pSObj,SKILLOBJECT_INFO* pSOInfo,CTargetList* pTList)
{
	pSObj->Init(pSOInfo,pTList);
	
	CSkillObject* pPreObj = m_SkillObjectTable.GetData(pSObj->GetID());
	//ASSERT(pPreObj == NULL);	
	if(pPreObj != NULL)
	{
		ReleaseSkillObject(pPreObj);
	}

	m_SkillObjectTable.Add(pSObj,pSObj->GetID());
	OBJECTMGR->AddSkillObject(pSObj);
}

CSkillObject* CSkillManager::CreateSkillObject(MSG_SKILLOBJECT_ADD* pSkillObjectAddInfo)
{
	CObject* pOperator = OBJECTMGR->GetObject(pSkillObjectAddInfo->SkillObjectInfo.Operator);

	////////////////////////////////////////////////////////////////////
	/// 06. 08. 2�� ���� - �̿���
	/// �Ϻ� ��ų ���� ������ �� Ʋ�� ����
	/// �̸� ��ų ������ �����;� �Ѵ�
	WORD SkillIdx = pSkillObjectAddInfo->SkillObjectInfo.SkillIdx;
	CSkillInfo* pSkillInfo = GetSkillInfo(SkillIdx);
	CSkillObject* pSObj = pSkillInfo->GetSkillObject();
	////////////////////////////////////////////////////////////////////

	if(pOperator == NULL)
	{
		OBJECTACTIONMGR->ApplyTargetList(pOperator,&pSkillObjectAddInfo->TargetList,eDamageKind_Front);
		return NULL;
	}
	if(pOperator->GetObjectKind() & eObjectKind_Monster)
	{
		VECTOR3 MainTargetPos;
		GetMainTargetPos(&pSkillObjectAddInfo->SkillObjectInfo.MainTarget,&MainTargetPos,NULL);
		/////////////////////////////////////////////////////////////////////////////////////
		/// 06. 08. 2�� ���� - �̿���
		/// ���� SetLookatPos �Լ� ������ ���ڿ� �߰� ȸ������ �߰��ߴ�.
		MOVEMGR->SetLookatPos(pOperator,&MainTargetPos,0,gCurTime, pSkillInfo->GetAddDegree());
		/////////////////////////////////////////////////////////////////////////////////////
	}		
	else
		MOVEMGR->SetAngle(pOperator,DIRTODEG(pSkillObjectAddInfo->SkillObjectInfo.Direction),0);


	DoCreateSkillObject(pSObj,&pSkillObjectAddInfo->SkillObjectInfo,
						&pSkillObjectAddInfo->TargetList);

	return pSObj;
}


CSkillObject* CSkillManager::CreateTempSkillObject(CSkillInfo* pSkillInfo,CHero* pHero,CActionTarget* pTarget)
{
	///////////////////////////////////////////////////////////////////////////
	// 06. 04. ��ų ���� ���� - �̿���
	// �ӽð�ü�� �����ִٸ� ��ų�� �������� �ʴ´�
	CSkillObject* pTempObj = GetSkillObject(TEMP_SKILLOBJECT_ID);
	if(pTempObj)
	{
//		ASSERT(0);
		return NULL;
	}
	///////////////////////////////////////////////////////////////////////////

	CSkillObject* pSObj = pSkillInfo->GetSkillObject();
	
	SKILLOBJECT_INFO sinfo;
	sinfo.SkillIdx = pSkillInfo->GetSkillIndex();
	sinfo.SkillObjectIdx = TEMP_SKILLOBJECT_ID;
	sinfo.Direction = DEGTODIR(pHero->GetAngleDeg());
	pTarget->GetMainTarget(&sinfo.MainTarget);
	sinfo.Operator = pHero->GetID();
	VECTOR3* pHeroPos = &pHero->GetCurPosition();
	VECTOR3* pTargetPos = pTarget->GetTargetPosition();
	if(pTargetPos == NULL)
		return NULL;
	sinfo.Pos = *(pSkillInfo->GetTargetAreaPivotPos(pHeroPos,pTargetPos));
	sinfo.StartTime = gCurTime;

	CTargetList tlist;
	pTarget->GetTargetArray(&tlist);

	DoCreateSkillObject(pSObj,&sinfo,&tlist);

	// for debug
	
	return pSObj;
}

void CSkillManager::ChangeTempObjToRealObj(MSG_SKILLOBJECT_ADD* pSkillObjectAddInfo)
{
	CSkillObject* pTempObj = GetSkillObject(TEMP_SKILLOBJECT_ID);
//	//ASSERT(pTempObj);

	if(pTempObj == NULL)
	{
		return;
	}

	m_SkillObjectTable.Remove(pTempObj->GetID());
	OBJECTMGR->AddGarbageObject(pTempObj);
	
	DoCreateSkillObject(pTempObj,&pSkillObjectAddInfo->SkillObjectInfo,
							&pSkillObjectAddInfo->TargetList);

	// debug��
	m_nSkillUseCount--;
}

void CSkillManager::ReleaseSkillObject(CSkillObject* pSkillObject)
{
	CSkillInfo* pSInfo = GetSkillInfo(pSkillObject->GetSkillIdx());
	if(pSInfo == NULL)
	{
		char temp[256];
		sprintf(temp,"skillkind:%d  SkillName:%s  oper:%d",
			pSkillObject->GetSkillIdx(),
			pSkillObject->GetSkillInfo()->GetSkillName(),
			pSkillObject->GetOperator() ? pSkillObject->GetOperator()->GetID() : 0);
		ASSERTMSG(0,temp);
		return;
	}
	//ASSERT(pSInfo);

	m_SkillObjectTable.Remove(pSkillObject->GetID());
	OBJECTMGR->AddGarbageObject(pSkillObject);
	
	pSkillObject->ReleaseSkillObject();
	pSInfo->ReleaseSkillObject(pSkillObject,0);
	
}
void CSkillManager::OnReceiveSkillObjectAdd(MSG_SKILLOBJECT_ADD* pmsg)
{
	//////////////////////////////////////////////////////////////////
	// 06. 04. ��ų ���� ���� - �̿���
	// �ڱⰡ �� ��ų�̶� 
	// ��ų ���� �����ÿ��� ChangeTempObjToRealObj�� ȣ��������
	// �׵� �׸��� �̵����� ������ Add�� �Ǵ� ��쿡��
	// CreateSkillObject�� ȣ���ؼ� �����ؾ��Ѵ�.
    //
	//if(pmsg->SkillObjectInfo.Operator == HEROID)
	if(pmsg->SkillObjectInfo.Operator == HEROID && pmsg->bCreate)
		ChangeTempObjToRealObj(pmsg);
	else
		CreateSkillObject(pmsg);
}
void CSkillManager::OnReceiveSkillObjectRemove(MSG_DWORD* pmsg)
{
	CSkillObject* pSObj = GetSkillObject(pmsg->dwData);
	if(pSObj == NULL)
	{
		//ASSERTMSG(0,"SkillObject Remove Failed");
		return;
	}
	ReleaseSkillObject(pSObj);
}

void CSkillManager::OnReceiveSkillStartNack()
{
	HERO->SetCurComboNum(0);
	CSkillObject* pSObj = GetSkillObject(TEMP_SKILLOBJECT_ID);
	//ASSERT(pSObj);
	if(pSObj == NULL)
		return;

	if( HERO == pSObj->GetOperator() )
	{
		m_nSkillUseCount--;
	}

	pSObj->SetMissed();
	ReleaseSkillObject(pSObj);

}

void CSkillManager::OnReceiveSkillSingleResult(MSG_SKILL_SINGLE_RESULT* pmsg)
{
	CSkillObject* pSObj = GetSkillObject(pmsg->SkillObjectID);
	if(pSObj == NULL)
	{
		OBJECTACTIONMGR->ApplyTargetList(NULL,&pmsg->TargetList,eDamageKind_ContinueDamage);
		return;
	}

	pSObj->OnReceiveSkillSingleResult(pmsg);
}

void CSkillManager::OnReceiveSkillObjectTargetListAdd(DWORD SkillObjectID,DWORD AddedObjectID,BYTE bTargetKind)
{
	CSkillObject* pSObj = GetSkillObject(SkillObjectID);
	//ASSERT(pSObj);
	if(pSObj == NULL)
	{
		//ASSERTMSG(0,"SkillObject Target Add Failed(No SObj)");
		return;
	}
	CObject* pObject = OBJECTMGR->GetObject(AddedObjectID);
	if(pObject == NULL)
	{
		//ASSERTMSG(0,"SkillObject Target Add Failed(No Target)");
		return;
	}

	pSObj->AddTargetObject(pObject,bTargetKind);
}
void CSkillManager::OnReceiveSkillObjectTargetListRemove(DWORD SkillObjectID,DWORD RemovedObjectID)
{
	CSkillObject* pSObj = GetSkillObject(SkillObjectID);
	//ASSERT(pSObj);
	if(pSObj == NULL)
	{		
		//ASSERTMSG(0,"SkillObject Target Remove Failed");
		return;
	}

	pSObj->RemoveTargetObject(RemovedObjectID);
}

void CSkillManager::OnReceiveSkillStartEffect(DWORD SkillOperator,DWORD SkillIdx)
{
	CObject* pOperator = OBJECTMGR->GetObject(SkillOperator);
	if(pOperator == NULL)
	{
		//ASSERTMSG(0,"SkillObject StartEffect Failed(No Operator)");
		return;
	}
	
	CSkillInfo* pSkillInfo = GetSkillInfo((WORD)SkillIdx);
	if(pSkillInfo == NULL)
	{		
		//ASSERTMSG(0,"SkillObject StartEffect Failed(No SkillInfo)");
		return;
	}

	SkillStartEffect(pOperator,pSkillInfo);
}

void CSkillManager::OnReceiveSkillObjectOperate(MSG_SKILL_OPERATE* pmsg)
{
	CSkillObject* pSObj = GetSkillObject(pmsg->SkillObjectID);
	//ASSERT(pSObj);
	if(pSObj == NULL)
	{		
		//ASSERTMSG(0,"SkillObject Operate Failed(No SObj)");
		return;
	}

	CObject* pReqObj = OBJECTMGR->GetObject(pmsg->RequestorID);
	pSObj->Operate(pReqObj,&pmsg->MainTarget,&pmsg->TargetList);

}

void CSkillManager::NetworkMsgParse(BYTE Protocol,void* pMsg)
{
	printf("[%d] %d // %d\n", ((MSGBASE*)pMsg)->CheckSum, Protocol, ((MSGBASE*)pMsg)->dwObjectID);

	switch(Protocol)
	{
	case MP_SKILL_SKILLOBJECT_ADD:
		{
			MSG_SKILLOBJECT_ADD* pmsg = (MSG_SKILLOBJECT_ADD*)pMsg;
			printf("%d\n", pmsg->SkillObjectInfo.SkillIdx);
			OnReceiveSkillObjectAdd(pmsg);
		}
		break;
	case MP_SKILL_SKILLOBJECT_REMOVE:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;
			OnReceiveSkillObjectRemove(pmsg);
		}
		break;
	case MP_SKILL_START_NACK:
		{
			MSG_BYTE* pmsg = (MSG_BYTE*)pMsg;
			OnReceiveSkillStartNack();
		}
		break;
	case MP_SKILL_SKILL_SINGLE_RESULT:
		{
			MSG_SKILL_SINGLE_RESULT* pmsg = (MSG_SKILL_SINGLE_RESULT*)pMsg;
			OnReceiveSkillSingleResult(pmsg);
		}
		break;

		// TargetList Update
	case MP_SKILL_ADDOBJECT_TO_SKILLOBJECTAREA_ACK:
		{
			MSG_DWORD2BYTE* pmsg = (MSG_DWORD2BYTE*)pMsg;
			OnReceiveSkillObjectTargetListAdd(pmsg->dwData1,pmsg->dwData2,pmsg->bData);
		}
		break;
	case MP_SKILL_REMOVEOBJECT_FROM_SKILLOBJECTAREA_ACK:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			OnReceiveSkillObjectTargetListRemove(pmsg->dwData1,pmsg->dwData2);
		}
		break;

	case MP_SKILL_STARTEFFECT:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			OnReceiveSkillStartEffect(pmsg->dwData1,pmsg->dwData2);
		}
		break;

	case MP_SKILL_OPERATE_ACK:
		{
			MSG_SKILL_OPERATE* pmsg = (MSG_SKILL_OPERATE*)pMsg;
			OnReceiveSkillObjectOperate(pmsg);
		}
		break;
		
	case MP_SKILL_DELAY_NOTIFY:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;

			SKILLDELAYMGR->ContinueSkillDelay( pmsg->dwData1, pmsg->dwData2 );
		}
		break;
	case MP_SKILL_JOB_NACK:
		{
			CHATMGR->AddMsg(CTC_SYSMSG, "��ų ����.");
		}
		break;
	}
}

CSkillArea* CSkillManager::GetSkillArea(CObject* pObject, CActionTarget* pTarget, CSkillInfo* pSkillInfo)
{
	CSkillArea* pArea = GetSkillArea(pObject->GetDirectionIndex(),pSkillInfo->GetSkillAreaIndex());
	
	// Area�� �߽���ǥ���� ���õǾ��� �´�.
	VECTOR3* pPos;
	pPos = pSkillInfo->GetTargetAreaPivotPos(&pObject->GetCurPosition(),pTarget->GetTargetPosition());
	pArea->SetCenterPos(pPos);

	return pArea;	
}

CSkillArea* CSkillManager::GetSkillArea(DIRINDEX dir,WORD SkillAreaIndex)
{
	CSkillArea* pArea = m_SkillAreaMgr.GetSkillArea(SkillAreaIndex,dir);
	
	return pArea;
}

void CSkillManager::ReleaseSkillArea(CSkillArea* pSkillArea)
{
	m_SkillAreaMgr.ReleaseSkillArea(pSkillArea);
}

void CSkillManager::UpdateSkillObjectTargetList(CObject* pObject)
{
	CSkillObject* pSObj;
	DWORD rtCode;
	m_SkillObjectTable.SetPositionHead();
	while(pSObj = m_SkillObjectTable.GetData())
	{
		rtCode = pSObj->Update();		// �ڱⰡ ���� �ִ� ��ų�鿡 ���� ������ ������Ʈ
		pSObj->UpdateTargetList(pObject);	// ���ΰ� ������Ʈ
#ifdef _TESTCLIENT_
		if(rtCode == SO_DESTROYOBJECT)
		{
			m_SkillObjectTable.Remove(pSObj->GetID());
			ReleaseSkillObject(pSObj);
		}
#endif
	}
}

void CSkillManager::RemoveTarget(CObject* pObject,BYTE bTargetKind)
{
	CSkillObject* pSObj;
	m_SkillObjectTable.SetPositionHead();
	while(pSObj = m_SkillObjectTable.GetData())
	{
		pSObj->RemoveTarget(pObject,bTargetKind);
	}
}

void CSkillManager::SkillStartEffect(CObject* pObject,CSkillInfo* pSkillInfo)
{
	TARGETSET set;
	set.pTarget = pObject;
	DWORD flag = 0;
	if(pObject->GetID() == HEROID)
		flag |= EFFECT_FLAG_HEROATTACK;
	EFFECTMGR->StartEffectProcess(pSkillInfo->GetSkillInfo()->EffectStart,pObject,
								&set,1,pObject->GetID(),flag);
}

BOOL CSkillManager::HeroSkillStartEffect(CHero* pHero,CActionTarget* pTarget,CSkillInfo* pSkillInfo)
{
	SkillStartEffect(pHero,pSkillInfo);
	OBJECTSTATEMGR->StartObjectState(pHero,eObjectState_SkillStart);
	OBJECTSTATEMGR->EndObjectState(pHero,eObjectState_SkillStart,pSkillInfo->GetSkillInfo()->EffectStartTime);
	CAction act;
	act.InitSkillActionRealExecute(pSkillInfo,pTarget);
	pHero->SetSkillStartAction(&act);

	// ������ �޼��� ������.
	MSG_DWORD2 msg;
	msg.Category = MP_SKILL;
	msg.Protocol = MP_SKILL_STARTEFFECT;
	msg.dwData1 = pHero->GetID();
	msg.dwData2 = pSkillInfo->GetSkillIndex();
	NETWORK->Send(&msg,sizeof(msg));

	return TRUE;
}

void CSkillManager::MakeSAT()
{
	FILE* fp = fopen("SAT.txt","w");
	fprintf(fp,"%d\n",m_SkillInfoTable.GetDataNum());
	
	CEngineObject man,woman;
	man.Init("man.chx",NULL,eEngineObjectType_Character);
	woman.Init("woman.chx",NULL,eEngineObjectType_Character);
	CSkillInfo* pSkillInfo;
	m_SkillInfoTable.SetPositionHead();
	while(pSkillInfo = m_SkillInfoTable.GetData())
	{
		DWORD StateEndTimeMan = 0;
		DWORD StateEndTimeWoman = 0;
		BOOL bBinding = pSkillInfo->GetSkillInfo()->BindOperator != 0;
		if(!bBinding)
		{
			WORD effectuse = pSkillInfo->GetSkillInfo()->EffectUse;
			StateEndTimeMan = EFFECTMGR->GetOperatorAnimatioEndTime(effectuse,eEffectForMan,&man);
			StateEndTimeWoman = EFFECTMGR->GetOperatorAnimatioEndTime(effectuse,eEffectForWoman,&woman);
			if(StateEndTimeMan == 0)	StateEndTimeMan = 500;
			if(StateEndTimeWoman == 0)	StateEndTimeWoman = 500;			
		}

		fprintf(fp,"%d\t%d\t%d\n",pSkillInfo->GetSkillIndex(),StateEndTimeMan,StateEndTimeWoman);
	}

	man.Release();
	woman.Release();

	fclose(fp);
}

SKILL_CHANGE_INFO * CSkillManager::GetSkillChangeInfo(WORD wMugongIdx)
{
	PTRLISTSEARCHSTART(m_SkillChangeList, SKILL_CHANGE_INFO *, pInfo)
		if(pInfo->wMugongIdx == wMugongIdx)
			return pInfo;
	PTRLISTSEARCHEND
	return NULL;
}

BOOL CSkillManager::IsDeadlyMugong(WORD wMugongIdx)
{
	PTRLISTSEARCHSTART(m_SkillChangeList, SKILL_CHANGE_INFO *, pInfo)
		if(pInfo->wTargetMugongIdx == wMugongIdx)
			return TRUE;
	PTRLISTSEARCHEND
	return FALSE;
}

///////////////////////////////////////////////////////////////////
// 06. 04. ��ų ���� ���� - �̿���
// �ӽý�ų��ü�� ������ �����ִ� �Լ�
void CSkillManager::DeleteTempSkill()
{
	CSkillObject* pSObj = GetSkillObject(TEMP_SKILLOBJECT_ID);

	ReleaseSkillObject(pSObj);
}
///////////////////////////////////////////////////////////////////

////////// 2007. 6. 28. CBH - ������� Ȯ�� ���� �Լ� �߰� ////////////////////
BOOL CSkillManager::LoadJobSkillProbability()
{
	CMHFile file;
	if(!file.Init("Resource/jobskill.bin","rb"))
	{
		return FALSE;
	}

	JOB_SKILL_PROBABILITY_INFO* pJobSkillInfo;
	while(!file.IsEOF())
	{
		pJobSkillInfo = new JOB_SKILL_PROBABILITY_INFO;
		memset( pJobSkillInfo, 0, sizeof(JOB_SKILL_PROBABILITY_INFO) );

		pJobSkillInfo->wSkillLevel = file.GetWord();
		file.GetWord(pJobSkillInfo->ProbabilityArray, MAX_JOBMOB_NUM);

		ASSERT(!m_JobSkillProbabilityTable.GetData(pJobSkillInfo->wSkillLevel));

		m_JobSkillProbabilityTable.Add(pJobSkillInfo, pJobSkillInfo->wSkillLevel);
		pJobSkillInfo = NULL;
	}
	file.Release();

	return TRUE;
}

BOOL CSkillManager::IsJobSkill(CHero* pHero,CActionTarget* pTarget, CSkillInfo* pSkillInfo)
{	
	// Ÿ���� ������� ������Ʈ�� �Ϲ� ��ų ���´�	
	// ������� �������� ������Ʈ�� ������ ������ ���´�. (�޼��� ó��)
	// Ÿ���� �Ϲ� ���̰� ������ų ������ ���� ���ϰ� ���´�.
	CObject* pObject = OBJECTMGR->GetObject(pTarget->GetTargetID());

	if(pObject == NULL)
	{
		return FALSE;
	}
	
	WORD wSkillKind = pSkillInfo->GetSkillKind();
	//Ÿ���� �÷��̾� �϶� �����϶� ó��
	if( pObject->GetObjectKind() == eObjectKind_Player )		
	{
		if(CheckSkillKind(wSkillKind) == TRUE)
		{
			CHATMGR->AddMsg(CTC_SYSMSG, "�ͬ��ޯ�A���������� .");
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}

	//�������� ������ �κ� ó��
	if(pObject->GetObjectKind() == eObjectKind_MapObject)
	{
		return TRUE;
	}

	WORD wObjectKind = pObject->GetObjectKind();	

	if(CheckSkillKind(wSkillKind) == TRUE)	
	{
		pHero->DisableAutoAttack(); //���� ��ų�� ���� ������ �ڵ����� ����� ����.

		int nSkillLevel = pHero->GetMugongLevel(pSkillInfo->GetSkillIndex());

		if( GetObjectKindGroup(wObjectKind) == eOBJECTKINDGROUP_NONE )
		{
			CHATMGR->AddMsg(CTC_SYSMSG, "�ͬ��ޯ�A���������� ");						
			return FALSE;
		}

		//Ÿ��ź ž�½� ������ų �ߵ� �Ұ� ó��
		if(pHero->InTitan() == TRUE)
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1657) );
			return FALSE;
		}

		//��ų������ ���� �������� ��
		BOOL bJobSkillSuccess = FALSE;
		switch(wSkillKind)
		{
		case SKILLKIND_MINING:	// ä��
			{			
				if( wObjectKind == eObjectKind_Mining )
				{
					bJobSkillSuccess = TRUE;
				}				
			}
			break;
		case SKILLKIND_COLLECTION:	// ä��
			{
				if( wObjectKind == eObjectKind_Collection )
				{
					bJobSkillSuccess = TRUE;
				}				
			}
			break;
		case SKILLKIND_HUNT:	// ���
			{
				if( wObjectKind == eObjectKind_Hunt )
				{
					bJobSkillSuccess = TRUE;
				}				
			}
			break;
		}

		if(bJobSkillSuccess == FALSE)
		{
			CHATMGR->AddMsg(CTC_SYSMSG, "���O�������ޯ� ");								
			return FALSE;
		}

		CMonster* pMonster = (CMonster*)pObject;
		if(pMonster == NULL)	//����ó��
		{				
			return FALSE;
		}

		if(nSkillLevel < pMonster->GetMonsterLevel())
		{
			CHATMGR->AddMsg(CTC_SYSMSG, "�ޯ൥�ŹL�C ");
			return FALSE;
		}		
	}
	else if( GetObjectKindGroup(wObjectKind) == eOBJECTKINDGROUP_JOB )
	{
		CHATMGR->AddMsg(CTC_SYSMSG, "�ؼФ���ϥ� ");
		pHero->DisableAutoAttack(); //���� ��ų�� ���� ������ �ڵ����� ����� ����.		
		return FALSE;
	}	
	
	return TRUE;
}

BOOL CSkillManager::CheckSkillKind(WORD wSkillKind)
{
	if((wSkillKind == SKILLKIND_MINING) || (wSkillKind == SKILLKIND_COLLECTION) || (wSkillKind == SKILLKIND_HUNT))	
	{
		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////

///// 2007. 10. 15. CBH - Ÿ��ź ������ ���� üũ(SkillInfo���� �̵�) /////////////
BOOL CSkillManager::CheckTitanWeapon(CHero* pHero, SKILLINFO* SkillInfo)
{
	// magi82 - Titan(070912) Ÿ��ź ����������Ʈ
	// Ÿ��ź�� ž�����϶�..
	if( pHero->InTitan() == TRUE )
	{
		// Ÿ��ź �����̸� Ÿ��ź ���⸦ üũ�Ѵ�.
		if( SkillInfo->SkillKind == SKILLKIND_TITAN )
		{
			if( SkillInfo->WeaponKind != 0 )
			{
				if( pHero->GetTitanWeaponEquipType() != SkillInfo->WeaponKind )
				{
					CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(151) );
					pHero->DisableAutoAttack();
					return FALSE;
				}

				if( pHero->GetWeaponEquipType() != pHero->GetTitanWeaponEquipType() )
				{
					CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1644));
					pHero->DisableAutoAttack();
					return FALSE;
				}
			}
		}
		else	// Ÿ��ź ������ �ƴϸ� ����
		{
			if(SKILLMGR->CheckSkillKind(SkillInfo->SkillKind) == TRUE)
			{
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1657) );
			}
			else
			{
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1653) );
			}			
			return FALSE;
		}
	}
	else	// Ÿ��ź�� ž�������� ������..
	{
		// Ÿ��ź �����̸� ����
		if( SkillInfo->SkillKind == SKILLKIND_TITAN )
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1668) );
			return FALSE;
		}
		else	// Ÿ��ź ������ �ƴϸ� ĳ���� ���⸦ üũ�Ѵ�.
		{
			if( SkillInfo->WeaponKind != 0 )
			{
				if( pHero->GetWeaponEquipType() != SkillInfo->WeaponKind )
				{
					CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(151) );
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////

#endif
