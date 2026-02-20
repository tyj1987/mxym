// HelpRequestManager.cpp: implementation of the CHelpRequestManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerSystem.h"
#include "HelpRequestManager.h"
#include "Monster.h"
#include "Player.h"
#include "AISystem.h"
#include "RegenPrototype.h"
#include "AIManager.h"
#include "cMonsterSpeechManager.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelpRequestManager::CHelpRequestManager()
{

}

CHelpRequestManager::~CHelpRequestManager()
{

}

// �������� ���� ���� ȣ��
void CHelpRequestManager::MonsterRequestProcess(CMonster * pMonster)
{
	CRegenObject * pORInfo = GROUPMGR->GetRegenObject(pMonster->GetMonsterGroupNum(), pMonster->GetGridID(), pMonster->GetID());
	if(!pORInfo) return;/*��ȯ�� ����*/
	switch(pORInfo->GetCurHelpType())
	{
	case HELPREQUEST_NONE: { } break;
	case HELPREQUEST_ONETIMEIFHP50: 
		{
			if( pMonster->GetLife() < pMonster->GetMaxLife()/2 )
			{
				// ���� ��û
				//g_pAISystem->SendMsg(eMK_HelpShout, pMonster->GetID(), pMonster->GetID(),0,0);
				g_pAISystem->SendMsg(eMK_HelpRequest, pMonster->GetID(), 0, 0, 0);
				// HelpType �ʱ�ȭ
				pORInfo->SetCurHelpType(HELPREQUEST_NONE);
			}
		}
		break;
	case HELPREQUEST_ALWAYSIFHP30: 
		{
			if( pMonster->GetLife() < pMonster->GetMaxLife()*0.3 )
			{
				// ���� ��û
				//g_pAISystem->SendMsg(eMK_HelpShout, pMonster->GetID(), pMonster->GetID(),0,0);
				g_pAISystem->SendMsg(eMK_HelpRequest, pMonster->GetID(), 0, 0, 0);
			}
		}
		break;
	case HELPREQUEST_DIE: 
		{ 
			// taiyo ok?
			if(pMonster->GetLife() == 0)
			{
				// ���� ��û
				//g_pAISystem->SendMsg(eMK_HelpShout, pMonster->GetID(), pMonster->GetID(),0,0);
				g_pAISystem->SendMsg(eMK_HelpRequest, pMonster->GetID(), 0, 0, 0);
				
			}
		} 
		break;
	case HELPREQUEST_ALWAYS: 
		{
			// �׻� ���� ��û
			//g_pAISystem->SendMsg(eMK_HelpShout, pMonster->GetID(), pMonster->GetID(),0,0);
			g_pAISystem->SendMsg(eMK_HelpRequest, pMonster->GetID(), 0, 0, 0);
		} 
		break;
	}
}

//SW050901
void CHelpRequestManager::SetHelperMonster(CMonster* pAsker, CMonster * pHelper, CPlayer * pTargeter)
{
	if( pAsker == NULL && pHelper == NULL )
		return;

	BYTE objectKind = pHelper->GetObjectKind();
	if( (objectKind != eObjectKind_Monster) || (objectKind != eObjectKind_TitanMonster) )
		return;

	if( pHelper->GetGridID() != ((CObject*)pTargeter)->GetGridID() )
		return;

	if ( pHelper->m_stateParam.stateOld == eMA_PERSUIT ) return;

	if( pHelper->m_stateParam.stateCur == eMA_REST || pHelper->m_stateParam.stateCur == eMA_RUNAWAY )
		return;

	if( CAIManager::IsTargetChange(pTargeter, pHelper) )
	{
//		pHelper->SetLastAttackPlayer( pTargeter );
//		pHelper->SetTObject( pTargeter );
//		GSTATEMACHINE.SetState(pHelper, eMA_ATTACK);

		//SW060703 ����� �߰�
		//2008. 6. 9. CBH - ���� ������ ���� �ٴ� ���� ������ ���� ������ �Ϲݸ�����
		//���� �� ����ֿ� ���� �޴� ���� ����
		if( FALSE == g_pServerSystem->GetMap()->IsMapKind(eBossMap) )
		{
			if( (objectKind == eObjectKind_Monster) || (objectKind == eObjectKind_TitanMonster) )	//�Ϲݸ���
			{
				BOOL bNoForeAtk = FALSE;
				pTargeter->GetPetManager()->GetPetBuffResultRt( ePB_NoForeAtkMonster, & bNoForeAtk );
				if( bNoForeAtk )
				{
					return;
				}
			}
		}

		//���� ����
		pHelper->SetLastAttackPlayer( pTargeter );
		GSTATEMACHINE.SetState(pHelper, eMA_ATTACK);
		pHelper->SetTObject( pTargeter );

		MonSpeechInfo* pTemp = MON_SPEECHMGR->GetCurStateSpeechIndex( pAsker->GetMonsterKind(), eMon_Speech_Help );
		if( pTemp )
			pAsker->AddSpeech( pTemp->SpeechType, pTemp->SpeechIndex );

	}
}

