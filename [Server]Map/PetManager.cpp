#include "StdAfx.h"
#include "ServerSystem.h"
#include ".\petmanager.h"
#include "PackedData.h"
#include "Pet.h"
#include "Player.h"
#include "ObjectFactory.h"
#include "CharMove.h"
#include "UserTable.h"
#include "MapDBMsgParser.h"
#include "ItemSlot.h"
#include "ItemManager.h"
#include "CheckRoutine.h"
#include "../[CC]Header/GameResourceManager.h"
#include "../[CC]Header/CommonCalcFunc.h"

#include "CharMove.h"
#include "Battle.h"


//CIndexGenerator	CPetManager::m_PetIDGenerator.Init(10000,PET_ID_START);
DWORD CPetManager::m_dwPetObjectID = PET_ID_START;

CPetManager::CPetManager(void)
{
	//m_PetTable.Initialize(5);
	m_PetInfoList.Initialize(5);
	m_pCurSummonPet		= NULL;
	m_dwSkillRechargeCheckTime	= 0;
	m_dwSkillRechargeAmount	= 0;
	m_bSkillGuageFull	= FALSE;

	m_bPetStaminaZero	= FALSE;

	m_dwReleaseDelayTime = 0;
	m_dwResummonDelayTime = 0;

	m_BuffFlag = ePB_None;

	m_wPetKind = 0;

	m_iFriendshipReduceAmount = 0;

	m_dwEventPetSummonRemainTime = 0;

	m_dwPetValidDistPosCheckTime = gCurTime;
}

CPetManager::~CPetManager(void)
{
}

void CPetManager::Init(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	
	m_pCurSummonPet = NULL;

	m_dwSkillRechargeCheckTime	= 0;
	m_dwSkillRechargeAmount	= 0;
	m_bSkillGuageFull	= FALSE;

	m_bPetStaminaZero	= FALSE;

	m_dwReleaseDelayTime = 0;
	m_dwResummonDelayTime = 0;

	m_BuffFlag = ePB_None;

	m_dwPetValidDistPosCheckTime = gCurTime;

	m_wPetKind = 0;

	m_iFriendshipReduceAmount = 0;

	m_dwEventPetSummonRemainTime = 0;
	m_dwEventPetCheckTime = 0;

	memset(&m_PetEquipOption, 0, sizeof(PETEQUIP_ITEMOPTION));
}

void CPetManager::Release()
{	//���� pPet �� ��ȯ�̳� ������ ������ Null ó��!
// 	m_PetTable.SetPositionHead();
// 	CPet* pPet = NULL;
// 	while( pPet = m_PetTable.GetData() )
// 	{
// 		g_pObjectFactory->ReleaseObject((CObject*)pPet);
// 		//delete pPet;
// 		pPet = NULL;
// 	}
// 	m_PetTable.RemoveAll();

	m_PetInfoList.SetPositionHead();
	PET_TOTALINFO* pInfo = NULL;
	while( pInfo = m_PetInfoList.GetData() )
	{
		delete pInfo;
		pInfo = NULL;
	}
	m_PetInfoList.RemoveAll();

	if(m_pCurSummonPet)
	{
		RemovePet();
	}
}

void CPetManager::ReleaseCurPetMove()
{//grid �� m_CharacterHashTable ����.
	if(m_pCurSummonPet)
		CCharMove::ReleaseMove(m_pCurSummonPet);
}

/*
void CPetManager::AddPet( CPet* pPet ,DWORD	ItemDBIdx )
{
	m_PetTable.Add(pPet,ItemDBIdx);
}*/

// void CPetManager::RemovePet( DWORD ItemDBIdx )
// {
// 	ASSERT(m_PetTable.GetData(ItemDBIdx));
// 	PET_TOTALINFO* pInfo = m_PetInfoList.GetData(ItemDBIdx);
// 	SAFE_DELETE( pInfo );
// 	m_PetTable.Remove(ItemDBIdx);
// }

void CPetManager::DeletePet( DWORD ItemDBIdx )
{//�� ������
	PET_TOTALINFO* pInfo = m_PetInfoList.GetData(ItemDBIdx);
	if(!pInfo)
	{
		ASSERT(0);
		return;
	}

	//DB
	PetDeleteToDB(pInfo->PetDBIdx);

	SAFE_DELETE(pInfo);
	m_PetInfoList.Remove(ItemDBIdx);

	if(m_pCurSummonPet)
	{
		if(m_pCurSummonPet->GetPetSummonItemDBIdx() == ItemDBIdx)
		{
			RemovePet();
		}
	}
}

void CPetManager::AddPetTotalInfo( PET_TOTALINFO* pPetInfo, int flagSendMsgTo /*= eServerOnly*/)
{//����& ���
	if(!pPetInfo)
	{
		return;
	}
	if(m_PetInfoList.GetData(pPetInfo->PetSummonItemDBIdx))
	{
		//ASSERT(0);
		return;
	}

	PET_TOTALINFO* pNewInfo = new PET_TOTALINFO;
	memcpy(pNewInfo, pPetInfo, sizeof(PET_TOTALINFO));
	m_PetInfoList.Add(pNewInfo, pNewInfo->PetSummonItemDBIdx);

	if(eServerNClient == flagSendMsgTo)
	{
		SEND_ADDPET_FROMITEM msg;
		memcpy(&msg.Info, pPetInfo, sizeof(PET_TOTALINFO));
		msg.Category = MP_PET;
		msg.Protocol = MP_PET_ADDNEW_FROMITEM;
		msg.dwObjectID = m_pPlayer->GetID();

		m_pPlayer->SendMsg(&msg, sizeof(SEND_ADDPET_FROMITEM));
	}
}

void CPetManager::RemovePetTotalInfo( DWORD dwSummonItemDBIdx )
{
	PET_TOTALINFO* pInfo = m_PetInfoList.GetData(dwSummonItemDBIdx);

	ASSERT(pInfo);
	SAFE_DELETE(pInfo);

	m_PetInfoList.Remove(dwSummonItemDBIdx);
}

/*
CPet* CPetManager::GetPet(DWORD ItemDBIdx)
{
	return m_PetTable.GetData(ItemDBIdx);
}*/

BOOL CPetManager::UpGradePet( DWORD dwSummonItemDBIdx, BOOL bCheckProb /*= TRUE*/ )
{
	PET_TOTALINFO* pInfo = GetPetTotalInfo(dwSummonItemDBIdx);
	if(!pInfo)	return eUpgradeFailforEtc;

	BASE_PET_LIST* pList = GAMERESRCMNGR->GetPetListInfo(pInfo->PetKind);
	if(!pList)	return eUpgradeFailforEtc;

	BOOL rt = eUpgradeSucess;
	
	if(m_pCurSummonPet)
	{
		if(m_pCurSummonPet->GetPetSummonItemDBIdx() == dwSummonItemDBIdx)
			return eUpgradeFailforSamePetSummoned;
	}

	//�����۸��� ����.�����Ǽ���->��ȹ�� ����.
	if(bCheckProb)
	{
		BYTE Rnd = random(0,100);

		switch(pInfo->PetGrade)
		{
		case 1:
			if( Rnd > GAMERESRCMNGR->GetPetRule()->GradeUpProb1to2 )	//80
				rt = eUpgradeFailforProb;
			break;
		case 2:
			if( Rnd > GAMERESRCMNGR->GetPetRule()->GradeUpProb2to3 )	//80
				rt = eUpgradeFailforProb;
			break;
		case 3:
			return eUpgradeFailfor3rdUp;
		default:
			break;
		}
	}
	
	//Ȯ�����н� ģ�е� 10% ����
	if(rt == eUpgradeFailforProb)
	{
		int Petfriendly = GAMERESRCMNGR->GetPetRule()->FriendShipPerUpgradeFail;

		if( pInfo->PetFriendly - Petfriendly < 0 )
		{
			pInfo->PetFriendly = 0;
		}
		else
		{
			pInfo->PetFriendly += Petfriendly;
		}

		MSG_DWORD2 msg;
		msg.Category = MP_PET;
		msg.Protocol = MP_PET_UPDATE_FRIENDSHIP;
		msg.dwObjectID = m_pPlayer->GetID();
		msg.dwData1 = pInfo->PetSummonItemDBIdx;
		msg.dwData2 = pInfo->PetFriendly;

		m_pPlayer->SendMsg(&msg, sizeof(msg));

		LogPet(m_pPlayer->GetID(),m_pPlayer->GetUserID(),ePetLog_UpGradeFailed, pInfo->PetDBIdx, pInfo->PetSummonItemDBIdx,
			pInfo->PetGrade,pInfo->PetStamina,pInfo->PetFriendly,pInfo->bAlive);
	}
	else if(rt == eUpgradeSucess)
	{
		if( pInfo->PetGrade == PET_MAX_LEVEL )	return eUpgradeFailforEtc;
		else
			++pInfo->PetGrade;
		pInfo->PetFriendly = GAMERESRCMNGR->GetPetRule()->DefaultFriendship;
		pInfo->PetStamina = pList->StaminaMax[pInfo->PetGrade-1];

		LogPet(m_pPlayer->GetID(),m_pPlayer->GetUserID(),ePetLog_GradeUp, pInfo->PetDBIdx, pInfo->PetSummonItemDBIdx,
			pInfo->PetGrade,pInfo->PetStamina,pInfo->PetFriendly,pInfo->bAlive);
	}

	PetUpdateToDB(m_pPlayer->GetUserID(), pInfo);

	return rt;
}

BOOL CPetManager::RevivalPet( DWORD dwSummonItemDBIdx, int iGrade )
{
	PET_TOTALINFO* pInfo = GetPetTotalInfo(dwSummonItemDBIdx);
	if(!pInfo)	return FALSE;

	BASE_PET_LIST* pList = GAMERESRCMNGR->GetPetListInfo(pInfo->PetKind);
	if(!pList)	return FALSE;

	if(iGrade)
	{
	if(pInfo->PetGrade != iGrade)
		return FALSE;

	}
	ASSERT(FALSE==pInfo->bAlive);

	pInfo->bAlive = TRUE;
	if(iGrade)
		pInfo->PetFriendly = GAMERESRCMNGR->GetPetRule()->RevivalFriendship;
	else
		pInfo->PetFriendly = GAMERESRCMNGR->GetPetRule()->RevivalFriendship_Shop;

	pInfo->PetStamina = pList->StaminaMax[pInfo->PetGrade-1];

	PetUpdateToDB(m_pPlayer->GetUserID(), pInfo);

	LogPet(m_pPlayer->GetID(),m_pPlayer->GetUserID(),ePetLog_PetRevival, pInfo->PetDBIdx, pInfo->PetSummonItemDBIdx,
		pInfo->PetGrade,pInfo->PetStamina,pInfo->PetFriendly,pInfo->bAlive);

	return TRUE;
}

void CPetManager::RemovePet()
{
	if(!m_pCurSummonPet)	return;

	UpdateCurPetInfo();

	g_pServerSystem->RemovePet(m_pCurSummonPet->GetID());

	m_pCurSummonPet = NULL;

	m_BuffFlag = ePB_None;

	m_dwEventPetSummonRemainTime = 0;
	m_dwEventPetCheckTime = 0;
}

void CPetManager::AddPetInfoList( PET_TOTALINFO* pPetInfo )
{//���
	if(!pPetInfo)
	{
		return;
	}

	if(m_PetInfoList.GetData(pPetInfo->PetSummonItemDBIdx))
	{
		ASSERT(0);
		return;
	}

	m_PetInfoList.Add(pPetInfo, pPetInfo->PetSummonItemDBIdx);
}

void CPetManager::RemovePetInfoList( DWORD dwSummonItemDBIdx )
{//����
	PET_TOTALINFO* pInfo = m_PetInfoList.GetData(dwSummonItemDBIdx);

	ASSERT(pInfo);

	m_PetInfoList.Remove(dwSummonItemDBIdx);
}

WORD CPetManager::GetPetInfoList(PET_TOTALINFO* RtInfo)
{//ĳ�� ���ӽ� �� ���� ����
	WORD PetCount = 0;

	m_PetInfoList.SetPositionHead();
	PET_TOTALINFO* pInfo = NULL;
	while(pInfo = m_PetInfoList.GetData())
	{
		memcpy(&RtInfo[PetCount], pInfo, sizeof(PET_TOTALINFO));
		++PetCount;
	}

	return PetCount;
}

PET_TOTALINFO* CPetManager::GetPetTotalInfo( DWORD dwItemDBIdx, int flg /*= eWithNULLOBJ*/ )
{
	PET_TOTALINFO* pInfo = m_PetInfoList.GetData(dwItemDBIdx);
	if(pInfo)
		return pInfo;

	if( flg == eWithNULL )
	{
		return NULL;
	}
	else
	{
		static PET_TOTALINFO NULLINFO;
		memset(&NULLINFO,0,sizeof(PET_TOTALINFO));
		return &NULLINFO;
	}
}

BOOL CPetManager::CheckPetDefaultFriendship(DWORD dwItemDBIdx)
{
	PET_TOTALINFO* pPetInfo = GetPetTotalInfo(dwItemDBIdx);

	if( pPetInfo->PetFriendly < GAMERESRCMNGR->GetPetRule()->DefaultFriendship )
	{
		return FALSE;
	}
	else
		return TRUE;

	//return (pPetInfo->PetFriendly >= PET_DEFAULT_FRIENDLY);
}

void CPetManager::SummonPet( DWORD dwItemDBIdx, BOOL bSummonFromUser /*= TRUE*/ )
{
	/*
	if(m_pCurSummonPet)
	{
		if(m_pCurSummonPet->GetPetSummonItemDBIdx() == dwItemDBIdx)
		{
			//���� �Ѵ�.
			SealPet();
			return;
		}
		else if(m_pCurSummonPet->GetPetKind() == CRISTMAS_EVENTPET)
		{
			SealPet();
		}
		else
		{
			//�̹̼�ȯ���� ����� �����϶�� �޽���//Ŭ���̾�Ʈ������ üũ
			char szBuf[128] = {0,};
			sprintf(szBuf, "AnotherPetHasSummoned.ClientCheckedProcess. PlayerID:%d", m_pPlayer->GetID());
			ASSERTMSG(0, szBuf);

			MSGBASE msg;
			msg.Category = MP_PET;
			msg.Protocol = MP_PET_SUMMON_RESULT;
			msg.dwObjectID = m_pPlayer->GetID();

			m_pPlayer->SendMsg(&msg,sizeof(msg));
			
			return;
		}
	}
	else*/
	if( TRUE == CheckBeforeSummonPet( dwItemDBIdx ) )
	{
		PET_TOTALINFO* pInfo = GetPetTotalInfo(dwItemDBIdx);

		if(!pInfo)
		{
			char szBuf[128] = {0,};
			sprintf(szBuf, "Player Have No PetData with Item! PlayerID :%d\t ItemDBIdx:%d\n"
				,m_pPlayer->GetID(),dwItemDBIdx);
			ASSERTMSG(0,szBuf);
			return;
		}

		if( !pInfo->bAlive )
		{
			return;
		}

		//��������
		if( m_pPlayer->GetUserLevel() > eUSERLEVEL_GM )
		if( FALSE == CheckResummonAvailable() )
			return;

		//�� �ʱ���ġ
		VECTOR3 PetPos = m_pPlayer->GetMoveInfo()->CurPosition;
		int	Basic_distance = random(100,300);
		int temp = rand() % 200 - 100;
		PetPos.x += Basic_distance + temp;
		temp = rand() % 200 - 100;
		PetPos.y += Basic_distance + temp;

		int MapNum = g_pServerSystem->GetMapNum();
		if( g_pServerSystem->GetMap()->CollisionTilePos(PetPos.x,PetPos.z,MapNum) == TRUE )
		{	//������ġ�� ��ȿ���� ������ ����ĳ������ ��ġ�� ����.
			PetPos = m_pPlayer->GetMoveInfo()->CurPosition;
		}

		CPet* pPet = g_pServerSystem->AddPet(m_pPlayer, pInfo->PetDBIdx+m_dwPetObjectID, pInfo, &PetPos);

		//��ų�������� �ʱ�
		m_dwSkillRechargeAmount = 0;

		if(pPet)
		{
			m_pCurSummonPet = pPet;
		}
		else
		{
			ASSERT(0);
			m_pCurSummonPet = NULL;
			return;
		}

		//�� ���� ���� ����
		SetPetBuffInfo();

		//�� ��ȯ���·� �� �̵���
		RefleshPetMaintainBuff();

		//SW060406 �� ���� �ִ뽺�¹̳�����
		m_pCurSummonPet->CalcPetMaxStamina();

		m_pCurSummonPet->SendPetInfoMsg();

		if(TRUE == bSummonFromUser)
		{
			MSG_DWORD msg;
			msg.Category = MP_PET;
			msg.Protocol = MP_PET_SUMMON_ACK;
			msg.dwObjectID = m_pPlayer->GetID();
			msg.dwData = m_pCurSummonPet->GetID();

			PACKEDDATA_OBJ->QuickSend( m_pPlayer, &msg, sizeof(msg) );
		}
	}
}

BOOL CPetManager::CheckBeforeSummonPet(DWORD dwItemDBIdx)
{
	if(m_pCurSummonPet)
	{
		if(m_pCurSummonPet->GetPetSummonItemDBIdx() == dwItemDBIdx)
		{
			//���� �Ѵ�.
			SealPet();
			return FALSE;
		}
		else if(m_pCurSummonPet->GetPetKind() == CRISTMAS_EVENTPET)
		{
			SealPet();
			return TRUE;
		}
		else
		{
			//�̹̼�ȯ���� ����� �����϶�� �޽���//Ŭ���̾�Ʈ������ üũ
			char szBuf[128] = {0,};
			sprintf(szBuf, "AnotherPetHasSummoned.ClientCheckedProcess. PlayerID:%d", m_pPlayer->GetID());
			ASSERTMSG(0, szBuf);

			MSGBASE msg;
			msg.Category = MP_PET;
			msg.Protocol = MP_PET_SUMMON_RESULT;
			msg.dwObjectID = m_pPlayer->GetID();

			m_pPlayer->SendMsg(&msg,sizeof(msg));

			return FALSE;
		}
	}
	else
		return TRUE;
}

void CPetManager::SummonEventPet()
{
	if(m_pCurSummonPet)
	{
		return;
	}

	BASE_PET_LIST*	pPetListInfo;
	pPetListInfo = GAMERESRCMNGR->GetPetListInfo(CRISTMAS_EVENTPET);
	if( NULL == pPetListInfo )
		return;

	PET_TOTALINFO TotalInfo;
	TotalInfo.PetSummonItemDBIdx = 0;			//�������� DBID	//ù���� �϶� ����.
	TotalInfo.PetKind		= CRISTMAS_EVENTPET;	//����
	TotalInfo.PetStamina	= pPetListInfo->StaminaMax[0];	//���׹̳� �ִ�ġ
	TotalInfo.PetFriendly = GAMERESRCMNGR->GetPetRule()->DefaultFriendship;		//�ֱ⺻ģ�е�
	TotalInfo.PetGrade	= 1;	//�ܰ�
	TotalInfo.PetMasterUserID	= m_pPlayer->GetUserID();	//�ʿ�? �ʿ� DB����
	TotalInfo.bAlive		= TRUE;
	TotalInfo.bSummonning	= FALSE;
	TotalInfo.bRest			= FALSE;

	//�� �ʱ���ġ
	VECTOR3 PetPos = m_pPlayer->GetMoveInfo()->CurPosition;
	int	Basic_distance = random(100,300);
	int temp = rand() % 200 - 100;
	PetPos.x += Basic_distance + temp;
	temp = rand() % 200 - 100;
	PetPos.y += Basic_distance + temp;

	int MapNum = g_pServerSystem->GetMapNum();
	if( g_pServerSystem->GetMap()->CollisionTilePos(PetPos.x,PetPos.z,MapNum) == TRUE )
	{	//������ġ�� ��ȿ���� ������ ����ĳ������ ��ġ�� ����.
		PetPos = m_pPlayer->GetMoveInfo()->CurPosition;
	}

	CPet* pPet = g_pServerSystem->AddPet(m_pPlayer, EVENT_PET_ID_START + m_pPlayer->GetID(), &TotalInfo, &PetPos);

	if(pPet)
	{
		m_pCurSummonPet = pPet;
	}
	else
	{
		ASSERT(0);
		m_pCurSummonPet = NULL;
		return;
	}

	//�� ���� ���� ����
	SetPetBuffInfo();

	//�� ��ȯ���·� �� �̵���
	RefleshPetMaintainBuff();

	//SW060406 �� ���� �ִ뽺�¹̳�����
	//m_pCurSummonPet->CalcPetMaxStamina();

	//m_pCurSummonPet->SendPetInfoMsg();

	m_dwEventPetSummonRemainTime = gCurTime + CRISTMAS_EVENTPET_SUMMONNING_TIME;
	m_dwEventPetCheckTime = gCurTime;

	SetCurSummonPetKind(ePK_EventPet);

	MSG_DWORD msg;
	msg.Category = MP_PET;
	msg.Protocol = MP_PET_SUMMON_ACK;
	msg.dwObjectID = m_pPlayer->GetID();
	msg.dwData = m_pCurSummonPet->GetID();

	PACKEDDATA_OBJ->QuickSend( m_pPlayer, &msg, sizeof(msg) );
}

BOOL CPetManager::CheckCurSummonPetKindIs( int kind )
{
	BOOL rt = (m_wPetKind & kind);

	return rt;
}

void CPetManager::CheckEventPetSummonRemainTime()
{
	//30�� ���� üũ
#define EVENTPET_SUMMONNING_CHECK_TIME_MSEC 30000
	if( gCurTime - m_dwEventPetCheckTime < EVENTPET_SUMMONNING_CHECK_TIME_MSEC )
	{
		return;
	}
	else
	{
		m_dwEventPetCheckTime = gCurTime;
	}

	if( m_dwEventPetSummonRemainTime < gCurTime )
	{
		SealPet();
	}
}

void CPetManager::SealPet()
{
	if(!m_pCurSummonPet)	return;

	if( m_pPlayer->GetState() == eObjectState_Die )
		return;

	//EventPet �� DB����.
	if( !CheckCurSummonPetKindIs(ePK_EventPet) )
		UpdateCurPetInfo();

	MSG_DWORD msg;
	msg.Category = MP_PET;
	msg.Protocol = MP_PET_SEAL_ACK;
	msg.dwObjectID = m_pPlayer->GetID();
	msg.dwData = m_pCurSummonPet->GetID();
	PACKEDDATA_OBJ->QuickSend( m_pPlayer, &msg, sizeof(msg) );

	g_pServerSystem->RemovePet(m_pCurSummonPet->GetID());

	//���� ������
	SetResummonCheckTime();

	m_pCurSummonPet = NULL;

	RefleshPetMaintainBuff();

	m_BuffFlag = ePB_None;

	m_dwEventPetSummonRemainTime = 0;
	m_dwEventPetCheckTime = 0;
	SetCurSummonPetKind(ePK_None);

	//m_BuffData[ePB_Kind_Max] �ʱ�ȭ
	memset(m_BuffData, 0, sizeof(BuffData)*ePB_Kind_Max);
}

//void CPetManager::ExchangePet( DWORD dwItemDBIdx, CPlayer* pNewMaster )
//SW060403 �����۸� ������ �ŷ��� ģ�е� �϶� �ȵǰ�..
void CPetManager::ExchangePet( DWORD dwItemDBIdx, CPlayer* pNewMaster, BOOL bReduceFriendship/* = TRUE*/ )
{
	PET_TOTALINFO* pPetInfo = GetPetTotalInfo(dwItemDBIdx);

	if(NULL == pPetInfo)	//�����۸� �� ���� �� //�ѹ��� �������� ���� ����
	{
		return;
	}

	//���� ID ����
	pPetInfo->PetMasterUserID = pNewMaster->GetUserID();

	//ģ�е� 30% �̻󿡼� �ŷ��ȴ�.
	ASSERT((int)pPetInfo->PetFriendly - GAMERESRCMNGR->GetPetRule()->DefaultFriendship >= 0);

	if( TRUE == bReduceFriendship )
	{
		int Petfriendly = GAMERESRCMNGR->GetPetRule()->FriendShipPerTrade;
		pPetInfo->PetFriendly += Petfriendly;
	}

	//DB ���� ����
	PetUpdateToDB(pNewMaster->GetUserID(), pPetInfo);

	LogPet(pNewMaster->GetID(),pNewMaster->GetUserID(),ePetLog_Exchange, pPetInfo->PetDBIdx, pPetInfo->PetSummonItemDBIdx,
		pPetInfo->PetGrade,pPetInfo->PetStamina,pPetInfo->PetFriendly,pPetInfo->bAlive);

	//���� �� ���� �߰�
	pNewMaster->GetPetManager()->AddPetInfoList(pPetInfo);

	//�� ���ο��� Ŭ���̾�Ʈ�� ���� ���� ������Ʈ ���ش�.(ģ�е�)
	MSG_DWORD2 msg;
	msg.Category = MP_PET;
	msg.Protocol = MP_PET_UPDATE_FRIENDSHIP;
	msg.dwObjectID = pNewMaster->GetID();
	msg.dwData1 = pPetInfo->PetSummonItemDBIdx;
	msg.dwData2 = pPetInfo->PetFriendly;
	pNewMaster->SendMsg(&msg, sizeof(msg));

	//m_pCurSummonPet = NULL;	//!!���� �� ���� �ڵ�. -_-..

	//�Ǹ� �� ���� ����
	RemovePetInfoList(dwItemDBIdx);

	return;
}

BOOL CPetManager::IsCurPetSummonItem( DWORD dwItemDBIdx )
{
	if(!m_pCurSummonPet)	return FALSE;

	const PET_TOTALINFO* pInfo = m_pCurSummonPet->GetPetTotalInfo();
	if( pInfo->PetSummonItemDBIdx == dwItemDBIdx )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CPetManager::FeedUpPet( DWORD dwFeedAmount )
{
	BYTE rt = ePFR_Sucess;

	if(!m_pCurSummonPet)
	{
		rt = ePFR_Unsummoned;
		return;
	}

	if(m_pCurSummonPet->IsPetStaminaFull())
	{
		rt = ePFR_StaminaFull;
		return;
	}

	if(m_pCurSummonPet && rt == ePFR_Sucess)
	{
		DWORD AddAmount = dwFeedAmount;

		if(m_PetEquipOption.iPetStaminaRecoverateIncrease)
		{
			AddAmount += AddAmount * m_PetEquipOption.iPetStaminaRecoverateIncrease / 100;
		}

		if(m_PetEquipOption.iPetStaminaRecoverateAmount)
		{
			AddAmount += m_PetEquipOption.iPetStaminaRecoverateAmount;
		}

		m_pCurSummonPet->AddStamina(AddAmount, TRUE);
	}

// 	if( rt != ePFR_Sucess)
// 	{
// 		MSG_BYTE msg;
// 		msg.Category = MP_PET;
// 		msg.Protocol = MP_PET_FEED_FAILED;
// 		msg.dwObjectID = m_pPlayer->GetID();
// 		msg.bData = rt;
// 
// 		m_pPlayer->SendMsg(&msg, sizeof(msg));
// 	}
}

//CPet* CPetManager::GetPetFromSummonItemDBIdx(DWORD dwItemDBIdx)
//{
//	m_PetTable.GetPositionHead();
//	CPet* pPet;
//	while(pPet = (CPet*)m_PetTable.GetData())
//	{
//		if( dwItemDBIdx == pPet->m_PetTotalInfo.PetSummonItemID )
//			return pPet;
//	}
//
//	return NULL;
//}

/*
void CPetManager::SummonPet(DWORD dwItemDBIdx)
{
	//���� ��ȯ�� ó��
	if(m_pCurSummonPet)
	{
		SealPet();
	}

	CPet* pPet = GetPet(dwItemDBIdx);
	ASSERTMSG(pPet,"SummonPet is Not Exist");
	if(!pPet)	return;

	m_pCurSummonPet = pPet;
	m_pCurSummonPet->SetPetSeal(FALSE);
	m_pPlayer->SetCurPet(pPet);

	//!!!�� �ʱ� ��ġ���� ����Ѵ�. �׸��忡 �����ش�.
	VECTOR3 PetPos = m_pPlayer->GetMoveInfo()->CurPosition;
	int	Basic_distance = random(100,300);
	int temp = rand() % 200 - 100;
	PetPos.x += Basic_distance + temp;
	temp = rand() % 200 - 100;
	PetPos.y += Basic_distance + temp;

	int MapNum = g_pServerSystem->GetMapNum();
	if( g_pServerSystem->GetMap()->CollisionTilePos(PetPos.x,PetPos.z,MapNum) == TRUE )
	{	//������ġ�� ��ȿ���� ������ ����ĳ������ ��ġ�� ����.
		PetPos = m_pPlayer->GetMoveInfo()->CurPosition;
	}
	//////
	m_pCurSummonPet->SetInited();	//!CCharMove::InitMove(.. ���� ��ġ
	//////
	CCharMove::SetPosition(m_pCurSummonPet, &PetPos);

	CCharMove::InitMove(m_pCurSummonPet, &PetPos);

	// �� �������̺� ���
	g_pUserTable->AddUser(m_pCurSummonPet, m_pCurSummonPet->GetID());
}*/

/*
void CPetManager::SealPet()
{
	m_pCurSummonPet->SetPetSeal(TRUE);
	CCharMove::ReleaseMove(m_pCurSummonPet);
	//////
	m_pCurSummonPet->SetNotInited();
	//////
	// �� �������̺� ����
	g_pUserTable->RemoveUser(m_pCurSummonPet->GetID());

	MSGBASE msg;
	msg.Category = MP_PET;
	msg.Protocol = MP_PET_SEAL_ACK;
	msg.dwObjectID = m_pPlayer->GetID();
	m_pPlayer->SendMsg(&msg, sizeof(msg));

	m_pCurSummonPet = NULL;
}*/

void CPetManager::PetProcess()
{
	//���� �̵� ��� ���. Ȯ��/ó��.
	//ģ�е� ���
	//���׹̳� ���
	//��ų������ ���
	//

	if(m_pCurSummonPet)
	{
		CheckPosValidDistWithMaster();

		m_pCurSummonPet->Process();
		CCharMove::MoveProcess(m_pCurSummonPet);
		//CalcPetSkillRecharge();
		CheckStaminaZero();
		CheckDelayRelease();

		//SW061211 ũ���������̺�Ʈ
		if( CheckCurSummonPetKindIs(ePK_EventPet) )
		{
			CheckEventPetSummonRemainTime();
		}
	}
}

void CPetManager::CheckPosValidDistWithMaster()
{
	if(!m_pCurSummonPet)	return;
#define PET_VALID_DISTANCE_CHECK_TIME_MSEC	3000

	if( gCurTime - m_dwPetValidDistPosCheckTime < PET_VALID_DISTANCE_CHECK_TIME_MSEC )	return;
	/*
	BASEMOVE_INFO* pPetMove = m_pCurSummonPet->GetMoveInfo();
	BASEMOVE_INFO* pPlayerMove = m_pPlayer->GetMoveInfo();

	m_vPetPos = pPetMove->CurPosition;
	m_vPlayerPos = pPlayerMove->CurPosition;
	*/

	VECTOR3* pPetCurPos = CCharMove::GetPosition(m_pCurSummonPet);
	VECTOR3* pPlayerCurPos = CCharMove::GetPosition(m_pPlayer);

	float Dist = CalcDistanceXZ(pPetCurPos,pPlayerCurPos);
	if(Dist > 1600)
	{
		//warp
		CCharMove::SetPosition(m_pCurSummonPet,pPlayerCurPos);

		CCharMove::EndMove(m_pCurSummonPet,gCurTime,pPlayerCurPos);

		MOVE_POS	Msg;
		Msg.Category = MP_MOVE;
		Msg.Protocol = MP_MOVE_PET_WARP_ACK;
		Msg.dwObjectID = m_pPlayer->GetID();
		Msg.dwMoverID = m_pCurSummonPet->GetID();
		Msg.cpos.Compress(pPlayerCurPos);
		PACKEDDATA_OBJ->QuickSend(m_pPlayer,&Msg,sizeof(Msg));
	}

	m_dwPetValidDistPosCheckTime = gCurTime;
}

void CPetManager::CheckStaminaZero()
{
	if(!m_pCurSummonPet)	return;

	//SW080411 pet log �߰�
	/*
	if( FALSE == m_bPetStaminaZero && TRUE == m_pCurSummonPet->IsPetStaminaZero() )
	{
		if(m_pCurSummonPet->IsPetRest())
			return;
		else
		{
			m_pCurSummonPet->SetPetRest(TRUE);
			MSGBASE msg;
			msg.Category = MP_PET;
			msg.dwObjectID = m_pPlayer->GetID();
			msg.Protocol = MP_PET_REST_ON_ACK;
			m_pPlayer->SendMsg(&msg, sizeof(msg));
		}
	}*/
	const PET_TOTALINFO* pPetInfo = m_pCurSummonPet->GetPetTotalInfo();
	if( FALSE == m_bPetStaminaZero && TRUE == m_pCurSummonPet->IsPetStaminaZero() )
	{
		if(m_pCurSummonPet->IsPetRest())
			return;
		else
		{
			m_pCurSummonPet->SetPetRest(TRUE);
			MSGBASE msg;
			msg.Category = MP_PET;
			msg.dwObjectID = m_pPlayer->GetID();
			msg.Protocol = MP_PET_REST_ON_ACK;
			m_pPlayer->SendMsg(&msg, sizeof(msg));
		}
		
		LogPet(m_pPlayer->GetID(),m_pPlayer->GetUserID(),ePetLog_StaminaZero, pPetInfo->PetDBIdx, pPetInfo->PetSummonItemDBIdx,
			pPetInfo->PetGrade,pPetInfo->PetStamina,pPetInfo->PetFriendly,pPetInfo->bAlive);
	}
	else if( TRUE == m_bPetStaminaZero && FALSE == m_pCurSummonPet->IsPetStaminaZero() )
	{
		LogPet(m_pPlayer->GetID(),m_pPlayer->GetUserID(),ePetLog_StaminaFromZero, pPetInfo->PetDBIdx, pPetInfo->PetSummonItemDBIdx,
			pPetInfo->PetGrade,pPetInfo->PetStamina,pPetInfo->PetFriendly,pPetInfo->bAlive);
	}

	m_bPetStaminaZero = m_pCurSummonPet->IsPetStaminaZero();
}

void CPetManager::CalcPetSkillRecharge()
{
	//!!!�޽� ���¸� Ȧ��.
	if(m_pCurSummonPet->IsPetRest())
		return;

	if(gCurTime - m_dwSkillRechargeCheckTime < PET_SKILLCHARGE_CHECKTIME)	return;

	const BASE_PET_LIST* pPetBaseInfo = m_pCurSummonPet->GetPetBaseInfo();

	//���¹̳ʰ� ������ ������ ä���ְ�
	if( FALSE == m_pCurSummonPet->IsPetStaminaZero() )
	{
		m_dwSkillRechargeAmount += pPetBaseInfo->SkillRecharge;
	}
	else
	{
		m_dwSkillRechargeAmount = 0;
	}

	if( m_dwSkillRechargeAmount >= PET_MAX_SKILL_CHARGE )
	{
		m_dwSkillRechargeAmount = PET_MAX_SKILL_CHARGE;
		m_bSkillGuageFull = TRUE;
	}

	m_dwSkillRechargeCheckTime = gCurTime;
}

void CPetManager::ReleaseMoveWithDelay(DWORD delayTime)
{
	m_dwReleaseDelayTime = gCurTime + delayTime;
}

void CPetManager::CheckDelayRelease()	//�� ������. ���ν� �ƴ�.
{
	if(m_dwReleaseDelayTime)
	{
		if(gCurTime < m_dwReleaseDelayTime)
			return;
		else
		{
			RemovePet();
			m_dwReleaseDelayTime = 0;
		}
	}
}

void CPetManager::NetworkMsgParse( BYTE Protocol, void* pMsg )
{
	/*
	if(!m_pCurSummonPet)
	{
		char buf[256] = {0,};
		sprintf(buf, "PlayerID(%d),that have not SummonPet!", m_pPlayer->GetID());
		ASSERTMSG(0, buf);
		return;
	}*/

	MSG_DWORD msg;
	msg.Category = MP_PET;
	msg.dwObjectID = m_pPlayer->GetID();

	switch( Protocol )
	{
	case MP_PET_REST_ON:
		{
			if( m_pCurSummonPet && FALSE == m_pCurSummonPet->IsPetRest() )
			{
				m_pCurSummonPet->SetPetRest(TRUE);
				
				RefleshPetMaintainBuff();

				msg.Protocol = MP_PET_REST_ON_ACK;
			}
			else
			{
				msg.Protocol = MP_PET_REST_NACK;
			}
		}
		break;
	case MP_PET_REST_OFF:
		{
			if( m_pCurSummonPet && TRUE == m_pCurSummonPet->IsPetRest() )
			{
				if(m_pCurSummonPet->IsPetStaminaZero())
					msg.Protocol = MP_PET_REST_ON_NACK;
				else
				{
					m_pCurSummonPet->SetPetRest(FALSE);

					RefleshPetMaintainBuff();

					msg.Protocol = MP_PET_REST_OFF_ACK;
				}
			}
			else
			{
				msg.Protocol = MP_PET_REST_NACK;
			}
		}
		break;
	case MP_PET_USE_SKILL_SYN:
		{
			if(FALSE == IsSkillGuageFull())
			{
				msg.Protocol = MP_PET_USE_SKILL_NACK;
				break;
			}

			if( m_pCurSummonPet && m_pCurSummonPet->UsePetSkill())
			{
				//��ų ������ �ʱ�ȭ & ������� �ʱ�ȭ
				m_dwSkillRechargeAmount = 0;
				m_bSkillGuageFull = FALSE;

				msg.Protocol = MP_PET_USE_SKILL_ACK;
			}
			else
			{
				msg.Protocol = MP_PET_USE_SKILL_NACK;
			}
		}
		break;
	case MP_PET_SEAL_SYN:
		{
			//if(FALSE == m_pCurSummonPet->IsPetSealed()) //��ȯ ���̸�
			if(m_pCurSummonPet)
			{
				SealPet();
				//msg.Protocol = MP_PET_SEAL_ACK;
			}
			else
			{
				msg.Protocol = MP_PET_SEAL_NACK;
			}
		}
		break;
	case MP_PET_UPGRADE_SYN:
		{
			MSG_DWORD3* pmsg = (MSG_DWORD3*)pMsg;

			//ItemDBIdx �� ������ �ش� ������ ã�� Ȯ�� �Ǻ��� ó��
			BOOL rt = UpGradePet(pmsg->dwData1);

			if(!CHKRT->ItemOf(m_pPlayer, (POSTYPE)pmsg->dwData2, (WORD)pmsg->dwData3,0,0,CB_EXIST|CB_ICONIDX))
				rt = eUpgradeFailforEtc;

			switch( rt )
			{
			case eUpgradeSucess:
				{
					msg.Protocol = MP_PET_UPGRADE_ACK;
				}
				break;
			//case eUpgradeFailfor3rdUp:
			case eUpgradeFailforProb:
				{
					msg.Protocol = MP_PET_UPGRADE_NACK;
					//�� ģ�е� ����	//UpGradePet(..
				}
				break;
			case eUpgradeFailforEtc:
				{
					char szBuf[128] = {0,};
					sprintf(szBuf, "PetDataErrorAtPetUpgrade! PlayerID:%d\tItemDBIdx:%d\n", m_pPlayer->GetID(), pmsg->dwData1);
					ASSERTMSG(0, szBuf);
					return;
				}
			case eUpgradeFailfor3rdUp:
				{
					msg.Protocol = MP_PET_UPGRADE_NACK;
					char szBuf[128] = {0,};
					sprintf(szBuf, "PetProcessErrorAtPetUpgrade! Attempt 3rd Upgrade PlayerID:%d\tItemDBIdx:%d\n", m_pPlayer->GetID(), pmsg->dwData1);
					ASSERTMSG(0, szBuf);
				}
				break;
			case eUpgradeFailforSamePetSummoned:
				{
					msg.Protocol = MP_PET_UPGRADE_NACK;
					char szBuf[128] = {0,};
					sprintf(szBuf, "PetProcessErrorAtPetUpgrade! Attempt SamePet PlayerID:%d\tItemDBIdx:%d\n", m_pPlayer->GetID(), pmsg->dwData1);
					ASSERTMSG(0, szBuf);
				}
				break;
			default:
				break;
			}

			//�� ���� ������ �����
			if( eUpgradeSucess == rt || eUpgradeFailforProb == rt )
			{
				if( EI_TRUE != ITEMMGR->DiscardItem( m_pPlayer, (POSTYPE)pmsg->dwData2, (WORD)pmsg->dwData3, 1 ) )
				{
					ASSERTMSG(0,"DeleteItemFailedError! PetUpgradeItem");
					return;
				}

				//Ŭ���̾�Ʈ ��� ���� ������
				MSG_ITEM_USE_ACK Msg;
				Msg.Category = MP_ITEM;
				Msg.Protocol = MP_ITEM_USE_ACK;
				Msg.TargetPos = (POSTYPE)pmsg->dwData2;
				Msg.wItemIdx = (WORD)pmsg->dwData3;
				//msg.dwObjectID = m_pPlayer->GetID();
				m_pPlayer->SendMsg(&Msg,sizeof(Msg));
			}
		}
		break;
	case MP_PET_REVIVAL_SYN:
		{
			MSG_DWORD4* pmsg = (MSG_DWORD4*)pMsg;
			
			if(!CHKRT->ItemOf(m_pPlayer, (POSTYPE)pmsg->dwData2, (WORD)pmsg->dwData3,0,0,CB_EXIST|CB_ICONIDX))
			{
				ASSERTMSG(0,"TargetItemNotExist! PetRevivalItem");
				return;
			}

			//pmsg->dwData4 0�� �����۸���Ȱ, 1,2,3�� ���ӳ� �ܰ躰
			if( RevivalPet(pmsg->dwData1,pmsg->dwData4) )
			{
				msg.Protocol = MP_PET_REVIVAL_ACK;
				msg.dwData = pmsg->dwData4;
				if( EI_TRUE != ITEMMGR->DiscardItem( m_pPlayer, (POSTYPE)pmsg->dwData2, (WORD)pmsg->dwData3, 1 ) )
				{
					ASSERTMSG(0,"DeleteItemFailedError! PetRevivalItem");
					return;
				}

				//Ŭ���̾�Ʈ ��� ���� ������
				MSG_ITEM_USE_ACK Msg;
				Msg.Category = MP_ITEM;
				Msg.Protocol = MP_ITEM_USE_ACK;
				Msg.TargetPos = (POSTYPE)pmsg->dwData2;
				Msg.wItemIdx = (WORD)pmsg->dwData3;
				//msg.dwObjectID = m_pPlayer->GetID();
				m_pPlayer->SendMsg(&Msg,sizeof(Msg));
			}
			else
			{
				char szBuf[128] = {0,};
				sprintf(szBuf, "PetDataErrorAtPetRevival! PlayerID:%d\tItemDBIdx:%d\n", m_pPlayer->GetID(), pmsg->dwData1);
				ASSERTMSG(0, szBuf);
				msg.Protocol = MP_PET_REVIVAL_NACK;
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	m_pPlayer->SendMsg(&msg, sizeof(msg));
}

void CPetManager::UpdateCurPetInfo()
{
	if( !m_pCurSummonPet ) return;

	//�� ���� ����
	const PET_TOTALINFO* pCurPetInfo = m_pCurSummonPet->GetPetTotalInfo();
	//DWORD dwItemDBIdx = m_pCurSummonPet->GetPetSummonItemDBIdx();
	DWORD dwItemDBIdx = pCurPetInfo->PetSummonItemDBIdx;
	PET_TOTALINFO* TInfo = GetPetTotalInfo(dwItemDBIdx);
	if(!TInfo)
	{
		ASSERT(0);
		return;
	}
	
	*TInfo = *pCurPetInfo;
	//memcpy(TInfo, pCurPetInfo, sizeof(PET_TOTALINFO));

	PetUpdateToDB(m_pPlayer->GetUserID(), TInfo);
}

void CPetManager::UpdateLogoutToDB()
{
	//!!!���̺� �� ������ DB ������Ʈ.
}

void CPetManager::SetSommonPetStamina( BYTE bFlag )
{
	//��ȯ���� ���¹̳� ���� bFlag 1(MAX)�� ��� SInfo �� �ִ�ġ�� ����.
	//0(MIN)�� ��� 100���� ����.

	if( !m_pCurSummonPet )	return;

	DWORD stamina = 0;

	if(1 == bFlag)
	{
		const BASE_PET_LIST* pList = m_pCurSummonPet->GetPetBaseInfo();
		
		stamina = pList->StaminaMax[m_pCurSummonPet->GetPetCurGrade()-1];
	}
	else if(0 == bFlag)
	{
		stamina = 100;		//for cheat
	}
	else
		return;

	m_pCurSummonPet->SetPetStamina(stamina);

	UpdateCurPetInfo();
}

int CPetManager::SetSommonPetFriendship( DWORD dwFriendship )
{
	//��ȯ���� ģ�е� ���� bFlag 1(MAX)�� ��� 100%(10000)�� ����.
	//0(MIN)�� ��� 1%(100)�� ����

	if( !m_pCurSummonPet )	return 1;

	DWORD max_friendly = GAMERESRCMNGR->GetPetRule()->MaxFriendship;

	if( dwFriendship > max_friendly )
		return 2;
	else
	{
		m_pCurSummonPet->SetPetFriendShip(dwFriendship);
		return 0;
	}

	UpdateCurPetInfo();
}

void CPetManager::SetSommonPetSkillReady()
{
	//���� ��ų������ ���� �ִ�ġ�� �����ϰ� ��ų�غ����(�ߵ�����)�� ����.
	m_dwSkillRechargeAmount = PET_MAX_SKILL_CHARGE;
	m_bSkillGuageFull = TRUE;
	//��ų�κ��� Ŭ���̾�Ʈ�κ��� üũ�Ҷ��� ���δ�. ���� Ǯ���� ���� �����ִ� ���� ������ SEND MSG!

	MSGBASE msg;
	msg.Category = MP_CHEAT;
	msg.Protocol = MP_CHEAT_PET_SKILLREADY_ACK;
	msg.dwObjectID = m_pPlayer->GetID();

	m_pPlayer->SendMsg(&msg, sizeof(msg));
}

void CPetManager::SetPetBuffInfo()
{
	if(!m_pCurSummonPet)	return;

	const BASE_PET_LIST* pBaseInfo = m_pCurSummonPet->GetPetBaseInfo();

	m_BuffFlag = 0;

	for( int i = 0; i < 3; ++i )	//3 : Pet Max BuffNum.
	{
		WORD BuffIdx = pBaseInfo->BuffList[m_pCurSummonPet->GetPetCurGrade()-1][i];

		PET_BUFF_LIST* pList = GAMERESRCMNGR->GetPetBuffInfo(BuffIdx);
		if(!pList)	continue;

		WORD BuffKind = pList->BuffKind;
		m_BuffFlag |= (1<<BuffKind);
		m_BuffData[BuffKind].Prob = pList->BuffSuccessProb;
		m_BuffData[BuffKind].BuffValueData = pList->BuffValueData;
		m_BuffData[BuffKind].BuffAdditionalData = pList->BuffAdditionalData;
		/*
		switch(pList->BuffKind)
		{
		case 1://������Up
			m_BuffFlag |= (1<<ePB_DemageUp);
			m_BuffData[ePB_DemageUp].BuffValueData = pList->BuffValueData;
			m_BuffData[ePB_DemageUp].BuffAdditionalData = pList->BuffAdditionalData;
			break;
		case 2://ȸ��
			m_BuffFlag |= (1<<ePB_Dodge);
			m_BuffData[ePB_Dodge].BuffValueData = pList->BuffValueData;
			m_BuffData[ePB_Dodge].BuffAdditionalData = pList->BuffAdditionalData;
			break;
		default:
			ASSERT(0)
			break;
		}*/
	}
}

void CPetManager::GetPetBuffResultRt( int BuffKind, void* Data )
{
	if(!m_pCurSummonPet)	return;

	if(m_pCurSummonPet->IsPetRest())	return;

	if(m_pCurSummonPet->IsPetStaminaZero())	return;

	if( !(m_BuffFlag & (1<<BuffKind)) )	return;
	
	DWORD RandV = random(0,100);

	switch(BuffKind)
	{
	case ePB_Demage_Percent:
		{
			//Ȯ�����
			if(RandV > m_BuffData[ePB_Demage_Percent].Prob)	return;

			double* pDamage = (double*)Data;
			double Damg = *pDamage;
			Damg = double(Damg + Damg*(m_BuffData[ePB_Demage_Percent].BuffValueData * 0.01));
			*pDamage = Damg;
		}
		break;
	case ePB_Dodge:
		{
			if(RandV > m_BuffData[ePB_Dodge].Prob)	return;
			BOOL* pDodge = (BOOL*)Data;
			*pDodge = TRUE;
		}
		break;
	case ePB_MasterAllStatUp:	//����(ȣ��)�� �� Ȱ��/�޽� ��.
		{
			//if(RandV > m_BuffData[ePB_MasterStatUp].Prob)	return;
			WORD PlusStat = (WORD)m_BuffData[ePB_MasterAllStatUp].BuffValueData;
			player_calc_stats* pMasterAdditionalStats = (player_calc_stats*)Data;

			pMasterAdditionalStats->GenGol += PlusStat;
			pMasterAdditionalStats->MinChub += PlusStat;
			pMasterAdditionalStats->CheRyuk += PlusStat;
			pMasterAdditionalStats->SimMaek += PlusStat;

		}
		break;
	case ePB_Item_DoubleChance:
		{
			if(RandV > m_BuffData[ePB_Item_DoubleChance].Prob)	return;
			BOOL* pDouble = (BOOL*)Data;
			*pDouble = TRUE;
		}
		break;
	case ePB_NoForeAtkMonster:
		{
			//if(RandV > m_BuffData[ePB_NoForeAtkMonster].Prob)	return; //100% at AllGrade
			BOOL* pNoForeAtk = (BOOL*)Data;
			*pNoForeAtk = TRUE;
		}
		break;
	case ePB_ReduceCriticalDmg:
		{
			//Ȯ�����
			//if(RandV > m_BuffData[ePB_ReduceCriticalDmg].Prob)	return;

			double* pDamage = (double*)Data;
			double Damg = *pDamage;
			Damg = Damg * (100 - m_BuffData[ePB_ReduceCriticalDmg].BuffValueData) * 0.01f;
			*pDamage = Damg;			
		}
		break;
	case ePB_MasterAllStatRound:
		{
			//WORD Roundnum = (WORD)m_BuffData[ePB_MasterAllStatRound].BuffValueData;
			//GetCharStats() ouf of sense!
			//player_calc_stats* pMasterStats = m_pPlayer->GetCharStats();
			player_calc_stats* pMasterAddStats = (player_calc_stats*)Data;

			pMasterAddStats->GenGol += GetAddStatFromRound(m_pPlayer->GetGenGol());
			pMasterAddStats->MinChub += GetAddStatFromRound(m_pPlayer->GetMinChub());
			pMasterAddStats->CheRyuk += GetAddStatFromRound(m_pPlayer->GetCheRyuk());
			pMasterAddStats->SimMaek += GetAddStatFromRound(m_pPlayer->GetSimMek());
		}
		break;
	case ePB_Item_RareProbUp:
		{
			float* pAddprob = (float*)Data;

			float addprob = 1.0f - (float)m_BuffData[ePB_Item_RareProbUp].BuffValueData * 0.01f;
			*pAddprob = addprob;
		}
		break;
	case ePB_MussangTimeIncrease:
		{
			DWORD* pAddtime = (DWORD*)Data;

			DWORD addtime = (DWORD)m_BuffData[ePB_MussangTimeIncrease].BuffValueData * 1000;
			*pAddtime = addtime;
		}
		break;
	default:
		break;
	}
}

void CPetManager::RefleshPetMaintainBuff()
{//���� ��ȯ ����� ������ ���ӹ�����ų�� ������ ����/ó���Ѵ�.
	//SW060324 �� �����߰�
	if( (m_BuffFlag & (1<<ePB_MasterAllStatUp)) ||
		(m_BuffFlag & (1<<ePB_MasterAllStatRound)) )
		m_pPlayer->CalcState();
}

void CPetManager::SetPetEquipOption( DWORD ItemIdx, BOOL bAddOpt )
{
	//������ �ε������� Ȯ�� (�޸Ŵ�������?)
	ITEM_INFO* pItemInfo = ITEMMGR->GetItemInfo((WORD)ItemIdx);

	if(!pItemInfo)
	{
		ASSERT(0);
		return;
	}

	int symbol = 1;
	if(FALSE == bAddOpt)
		symbol = -1;
	
	//�� ���¹̳� �Һ� ���ҷ�(%)
	if(pItemInfo->LimitGenGol)	
	{
		/*
		//�ӽ�..5��2�� ��ġ ���Խ�Ű��. 5��9�� �� �ҽ�
		if( bAddOpt == TRUE && m_PetEquipOption.iPetStaminaReductionDecrease )
		{
			char buf[128];
			sprintf(buf, "������ �ߺ������ ItemIdx: 55801 CharcterID: %d", m_pPlayer->GetID() );
			ASSERTMSG(0, buf);
			return;
		}*/
		m_PetEquipOption.iPetStaminaReductionDecrease += pItemInfo->LimitGenGol * symbol;
		if(m_PetEquipOption.iPetStaminaReductionDecrease < 0)
			m_PetEquipOption.iPetStaminaReductionDecrease = 0;
	}
	//�� ���¹̳� �ִ�ġ ������(val)
	if(pItemInfo->LimitMinChub)
	{
		/*
		if( bAddOpt == TRUE && m_PetEquipOption.iPetStaminaMaxIncreaseAmount )
		{
			char buf[128];
			sprintf(buf, "������ �ߺ������ ItemIdx: 55800 CharcterID: %d", m_pPlayer->GetID() );
			ASSERTMSG(0, buf);
			return;
		}*/
		m_PetEquipOption.iPetStaminaMaxIncreaseAmount += pItemInfo->LimitMinChub * symbol;
		if(m_PetEquipOption.iPetStaminaMaxIncreaseAmount < 0)
			m_PetEquipOption.iPetStaminaMaxIncreaseAmount = 0;

		if(m_pCurSummonPet)
		{
			/*
			const BASE_PET_LIST* pBaseInfo = m_pCurSummonPet->GetPetBaseInfo();
			if(pBaseInfo)
				m_pCurSummonPet->SetPetMaxStamina(pBaseInfo->StaminaMax[m_pCurSummonPet->GetPetCurGrade()-1]
				+ m_PetEquipOption.iPetStaminaMaxIncreaseAmount);*/
			m_pCurSummonPet->CalcPetMaxStamina();
		}
	}
	//�� ���¹̳� ȸ�� ������(%)
	if(pItemInfo->LimitCheRyuk)
	{
		m_PetEquipOption.iPetStaminaRecoverateIncrease += pItemInfo->LimitCheRyuk * symbol;
		if(m_PetEquipOption.iPetStaminaRecoverateIncrease < 0)
			m_PetEquipOption.iPetStaminaRecoverateIncrease = 0;
	}
	//�� ���¹̳� ȸ�� �߰���(val)
	if(pItemInfo->LimitSimMek)
	{
		m_PetEquipOption.iPetStaminaRecoverateAmount += pItemInfo->LimitSimMek * symbol;
		if(m_PetEquipOption.iPetStaminaRecoverateAmount < 0)
			m_PetEquipOption.iPetStaminaRecoverateAmount = 0;
	}
	//�� ģ�е� �߰� ȹ�淮(%)
	if(pItemInfo->ItemGrade)
	{
		m_PetEquipOption.iPetFriendshipIncrease += pItemInfo->ItemGrade * symbol;
		if(m_PetEquipOption.iPetFriendshipIncrease < 0)
			m_PetEquipOption.iPetFriendshipIncrease = 0;
	}
	//���� ���� �� �� ģ�е� ��ȣ��(%)
	if(pItemInfo->RangeType)	//50 -> 50%
	{
		m_PetEquipOption.fPetFriendshipProtectionRate += (float)pItemInfo->RangeType * 0.01f * symbol;
		if(m_PetEquipOption.fPetFriendshipProtectionRate < 0)
			m_PetEquipOption.fPetFriendshipProtectionRate = 0.f;
	}
}

void CPetManager::SetPetSummonning( int flag )	//���̵��� �� ��ȯ���� DB����
{
	if(!m_pCurSummonPet)	return;

	//�� ���� ����
	if(ePSS_ReleaseSummon == flag)
	{
		m_pCurSummonPet->SetPetSummonning(FALSE);
	}
	else	//ePSS_SaveSummon
	{
		m_pCurSummonPet->SetPetSummonning(TRUE);
		//�� �޽Ļ��´� ���� �� ���� �״�� ����Ǹ� �ȴ�.
	}

	//PetUpdateToDB(m_pPlayer->GetID(), m_pCurSummonPet->GetPetTotalInfo());	///RemovePet()����
}

void CPetManager::CheckSummonningPet()
{
	PET_TOTALINFO* pInfo = NULL;


	m_PetInfoList.SetPositionHead();

	while(pInfo = m_PetInfoList.GetData())
	{
		if( TRUE == pInfo->bSummonning )
		{
			SummonPet(pInfo->PetSummonItemDBIdx,FALSE);	//FALSE => ������ȯ�̾ƴϴ�.
			//���̵��ü�ȯ�� ���� ����
			SetPetSummonning(ePSS_ReleaseSummon);

			return;
		}
	}
}

void CPetManager::SetResummonCheckTime()
{
	m_dwResummonDelayTime = gCurTime;
}

BOOL CPetManager::CheckResummonAvailable()
{
	return ( gCurTime-m_dwResummonDelayTime > PET_RESUMMON_VALID_TIME );
}

void CPetManager::SendPetInfo( PET_TOTALINFO* pPetInfo )
{
	if(!pPetInfo)
	{
		return;
	}
	if(m_PetInfoList.GetData(pPetInfo->PetSummonItemDBIdx))
	{
		//ASSERT(0);
		return;
	}

	SEND_ADDPET_FROMITEM msg;
	memcpy(&msg.Info, pPetInfo, sizeof(PET_TOTALINFO));
	msg.Category = MP_PET;
	msg.Protocol = MP_PET_ADDINFO;
	msg.dwObjectID = m_pPlayer->GetID();

	m_pPlayer->SendMsg(&msg, sizeof(SEND_ADDPET_FROMITEM));
}

void CPetManager::SetReduceAmountPetFriendship( CObject* pAttacker )
{// ������ ������ DoDie(.. ���ߵ� ���� ����. ��Ȱ�� Revival##(.. ����.

	//��ȯ�� ������ ����.
	if(!m_pCurSummonPet)	return;

	m_iFriendshipReduceAmount = GAMERESRCMNGR->GetPetRule()->FriendShipPerMasterDie;

	float applyRate = 1.0f;
	//�� �Ǻ�(�� ���ݺ��� �г�Ƽ �ٸ�)
	//WORD wMapNum = g_pServerSystem->GetMapNum();

	//if( wMapNum == Tournament ||
	//	wMapNum == QuestRoom ||//QUESTMAPNUM1
	//	wMapNum == guul || //QUESTMAPNUM2
	//	wMapNum == 95
	//	)
	DWORD dwCheckBit = eTournament | eQuestRoom;
	if( (TRUE == g_pServerSystem->GetMap()->IsMapKind(dwCheckBit)) || (g_pServerSystem->GetMapNum() == 95) )
	{
		applyRate = 0;	// ģ�е� �ȱ���.
	}

	//������ �Ǻ�(������ �Ӽ����� �г�Ƽ �ٸ�)
	//SW070109 ȫ��/�븸 �� ����	//Player::DoDie(.. ���� �Űܿ�.
	if(pAttacker->GetObjectKind() & eObjectKind_Monster )
	{
		//applyRate *= 1.0f;
		if( m_pPlayer->GetBattle()->GetBattleKind() == eBATTLE_KIND_SURYUN )
		{
			applyRate = 0;
		}
	}
#ifdef _HK_LOCAL_
	else if( pAttacker->GetObjectKind() == eObjectKind_Player )
	{
		CPlayer* pAttackPlayer = (CPlayer*)pAttacker;
		CBattle* pBattle = pAttacker->GetBattle();
		if(pBattle->GetBattleKind() == eBATTLE_KIND_NONE || pAttacker->GetBattleID() != m_pPlayer->GetBattleID() )
		{
			if( pAttackPlayer->IsPKMode() == TRUE )
			{
				if( FALSE == m_pPlayer->IsPKMode() || wMapNum == nakyang_siege )
				{
					applyRate *= 0.5f;
				}

				if( (TRUE == PARTYWARMGR->IsEnemy(m_pPlayer, pAttackPlayer) && FALSE == pAttackPlayer->IsPKMode())		//������ ����̸� ��� ���°� �ƴϸ�
					|| TRUE == GUILDFIELDWARMGR->IsEnemy(m_pPlayer, pAttackPlayer) && FALSE == pAttackPlayer->IsPKMode() )	//������ ����̸� ��� ���°� �ƴϸ�
				{
					//Do nothing
					applyRate = 0;
				}
				/*else
				{
					pPet->AddFriendship(Petfriendly, TRUE);

					const PET_TOTALINFO* pInfo = pPet->GetPetTotalInfo();
					LogPet(GetID(),GetUserID(),ePetLog_MasterDie, pInfo->PetDBIdx, pInfo->PetSummonItemDBIdx,
						pInfo->PetGrade,pInfo->PetStamina,pInfo->PetFriendly,pInfo->bAlive);
				}*/
			}
		}
	}
#else
	else if( pAttacker->GetObjectKind() == eObjectKind_Player )
	{
		applyRate = 0;
	}
#endif

	if( m_PetEquipOption.fPetFriendshipProtectionRate > 1.f )
	{
		 m_PetEquipOption.fPetFriendshipProtectionRate = 1;
		 ASSERT(0);
	}
	if( m_PetEquipOption.fPetFriendshipProtectionRate > 0 )
	{
		applyRate *= (1 - m_PetEquipOption.fPetFriendshipProtectionRate);
	}

	m_iFriendshipReduceAmount = int(m_iFriendshipReduceAmount * applyRate);
}

void CPetManager::ReducePetFriendshipWithMastersRevive()
{
	if(!m_pCurSummonPet)	return;

	if( 0 == m_iFriendshipReduceAmount )	return;

	m_pCurSummonPet->AddFriendship(m_iFriendshipReduceAmount, TRUE);

	const PET_TOTALINFO* pInfo = m_pCurSummonPet->GetPetTotalInfo();
	LogPet(m_pPlayer->GetID(),m_pPlayer->GetUserID(),ePetLog_MasterDie, pInfo->PetDBIdx, pInfo->PetSummonItemDBIdx,
		pInfo->PetGrade,pInfo->PetStamina,pInfo->PetFriendly,pInfo->bAlive);
}
