// PKManager.cpp: implementation of the CPKManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerSystem.h"
#include "PKManager.h"
#include "UserTable.h"
#include "PackedData.h"
#include "Player.h"
#include "..\[CC]Header\CommonGameFunc.h"
#include "LootingManager.h"
#include "MapDBMsgParser.h"
#include "cConstLinkedList.h"
#include "ItemManager.h"
#include "MugongManager.h"

#include "..\[CC]Header\GameResourceManager.h"
#include "FameManager.h"
#include "GuildFieldWarMgr.h"
#include "EventMapMgr.h"
#include "PartyWarMgr.h"
#include "Battle.h"
#include "QuestMapMgr.h"
#include "MHFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern int g_nServerSetNum;

CPKManager::CPKManager()
{
}

CPKManager::~CPKManager()
{

}

void CPKManager::Init( BOOL bPKAllow )
{
//	m_bPKAllow = bPKAllow;

	for( int i = 0; i < MAX_CHANNEL_NUM; ++i )
		m_bPKAllow[i] = bPKAllow;
}

void CPKManager::InitForChannel()
{
	CMHFile file;
	char value[64] = {0,};

	char filename[256];
	sprintf(filename,"serverset/%d/ChannelInfo.bin",g_nServerSetNum);

	file.Init(filename, "rb");

	if(file.IsInited() == FALSE)
	{
		MessageBox(NULL,"Can't Open ChannelInfo File", NULL, NULL);
		return;
	}

	while(1)
	{
		if(file.IsEOF())
			break;

		strcpy( value, strupr(file.GetString()) );

		if( strcmp( value,"*PKCHANNEL" ) == 0 )
		{
			int channel = file.GetInt();
			if( channel < 1 )
				channel = 1;
			m_bPKAllow[channel-1] = TRUE;
		}
	}
	file.Release();
}

void CPKManager::SetPKAllowWithMsg( BOOL bAllow )
{
//	if( m_bPKAllow == bAllow ) return;
//	m_bPKAllow = bAllow;
	if( m_bPKAllow[0] == bAllow ) return;
	for( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        m_bPKAllow[i] = bAllow;
	
	MSG_BYTE msg;
	msg.Category	= MP_PK;
	msg.Protocol	= MP_PK_PKALLOW_CHANGED;
	
	CObject* pObject = NULL;
	g_pUserTable->SetPositionUserHead();
	while( pObject = g_pUserTable->GetUserData() )
	{
		if( pObject->GetObjectKind() != eObjectKind_Player ) continue;

		if( bAllow == FALSE )
		{
			if( ((CPlayer*)pObject)->IsPKMode() )
				((CPlayer*)pObject)->PKModeOffForce();
			//�����ϴ��͵� ĵ��?
		}

		msg.bData		= bAllow;
						
		((CPlayer*)pObject)->SendMsg( &msg, sizeof( msg ) );
	}
}

void CPKManager::DiePanelty( CPlayer* pDiePlayer, CObject* pAttacker )
{
	// questmap
	if( QUESTMAPMGR->IsQuestMap() )
		return;
	// eventmap
	if( EVENTMAPMGR->IsEventMap() )
		return;
	// guildfieldwar
//	if( pDiePlayer->IsDieFromGFW() )
//		return;
	// PK����̸� �����ʵ������� ������ �ƴϴ�
	if( pDiePlayer->IsPKMode() )
	{
		pDiePlayer->SetDieForGFW( FALSE );
	}
	// partywar
	if( PARTYWARMGR->IsEnemy( pDiePlayer, (CPlayer*)pAttacker ) )
		return;
	//�񹫽ÿ� �״� �г�Ƽ�� ����. �񹫷� �����ϰ� �׾���.
	if( pDiePlayer->m_bNeedRevive == FALSE )
		return;
	// RaMa - 05.05.20 �������ÿ� �׾ �г�Ƽ ����
	if( pDiePlayer->GetBattle()->GetBattleKind() == eBATTLE_KIND_GTOURNAMENT )
		return;

	// RaMa �������� ������ �г�Ƽ ����
	if( pDiePlayer->IsActionPanelty() == FALSE )
		return;

	//�Ǹ�ġ ������ ������ �޴� �г�Ƽ.	
	PKPlayerDiePanelty( pDiePlayer );

	if( pAttacker->GetObjectKind() != eObjectKind_Player )
		return;

	//PK�ϸ� �޴� �Ǹ�ġ
	PKPlayerKillPanelty( pDiePlayer, (CPlayer*)pAttacker );

	//ô����ϸ� �޴� �г�Ƽ.
	PKPlayerKiiledByWantedPanelty( pDiePlayer, (CPlayer*)pAttacker );
}


void CPKManager::PKPlayerKillPanelty( CPlayer* pDiePlayer, CPlayer* pAttacker )
{
	if( pAttacker->IsPKMode() )
	{
		if( !pDiePlayer->IsPKMode() )
		{
			DWORD dwBadFame = pAttacker->GetBadFame();
			DWORD dwAddBadFame = 0;
			if( dwBadFame < 50 )
				dwAddBadFame = 2;
			else if( dwBadFame < 4000 )
				dwAddBadFame = 5;
			else if( dwBadFame < 20000 )
				dwAddBadFame = 10;
			else if( dwBadFame < 80000 )
				dwAddBadFame = 30;
			else if( dwBadFame < 400000 )
				dwAddBadFame = 50;
			else if( dwBadFame < 1600000 )
				dwAddBadFame = 80;
			else if( dwBadFame < 8000000 )
				dwAddBadFame = 100;
			else if( dwBadFame < 32000000 )
				dwAddBadFame = 200;
			else if( dwBadFame < 100000000 )
				dwAddBadFame = 300;
			else if( dwBadFame < 500000000 )
				dwAddBadFame = 500;
			else
				dwAddBadFame = 600;
			
			if( dwBadFame + dwAddBadFame < 1000000000 )
				pAttacker->SetBadFame( dwBadFame + dwAddBadFame );
			else
				pAttacker->SetBadFame( 1000000000 );	//10��
			
			BadFameCharacterUpdate( pAttacker->GetID(), pAttacker->GetBadFame() );
			FAMEMGR->SendBadFameMsg( pAttacker, pAttacker->GetBadFame() );
		}

		//�߰�
		pAttacker->AddPKContinueTime( 60000 );	//��� ���ӽð� ����
	}
}

void CPKManager::PKPlayerKiiledByWantedPanelty( CPlayer* pDiePlayer, CPlayer* pAttacker )
{
	//die <--PKPlayer
	//attack <--has wanted player

	if( pAttacker->IsWantedOwner( pDiePlayer->GetWantedIdx() ) )	//ô����� ������뿡�� �׾���.
	{
//��ȭ : �Ǹ�ġ ���� ========
		DWORD dwBadFame = pDiePlayer->GetBadFame();

		if( dwBadFame != 0 )
		{
			DWORD dwMinusBadFame = 0;
			
			if( dwBadFame < 50 )
				dwMinusBadFame = 2;
			else if( dwBadFame < 4000 )
				dwMinusBadFame = 5;
			else if( dwBadFame < 20000 )
				dwMinusBadFame = 10;
			else if( dwBadFame < 80000 )
				dwMinusBadFame = 30;
			else if( dwBadFame < 400000 )
				dwMinusBadFame = 50;
			else if( dwBadFame < 1600000 )
				dwMinusBadFame = 80;
			else if( dwBadFame < 8000000 )
				dwMinusBadFame = 100;
			else if( dwBadFame < 32000000 )
				dwMinusBadFame = 200;
			else if( dwBadFame < 100000000 )
				dwMinusBadFame = 300;
			else if( dwBadFame < 500000000 )
				dwMinusBadFame = 500;
			else
				dwMinusBadFame = 600;
			
			if( dwBadFame > dwMinusBadFame )
				pDiePlayer->SetBadFame( dwBadFame - dwMinusBadFame );
			else
				pDiePlayer->SetBadFame( 0 );	//0
			BadFameCharacterUpdate( pDiePlayer->GetID(), pDiePlayer->GetBadFame() );
			FAMEMGR->SendBadFameMsg( pDiePlayer, pDiePlayer->GetBadFame() );
		}
//==============

		//���� //������ ��������.	//����... %�� 3% �߰��Ǿ� ������ �׾����ÿ� ���Ƽ�� �޴´�.
/*
		int nRatio = 0;	//�����ı� Ȯ��
		
		CItemSlot* pWear	= pDiePlayer->GetSlot( eItemTable_Weared );
		CItemSlot* pInven	= pDiePlayer->GetSlot( eItemTable_Inventory );
		if( pWear->GetItemCount() + pInven->GetItemCount() < 8 )
		{
			if( dwBadFame < 50 )
				nRatio = 0;
			else if( dwBadFame < 4000 )
				nRatio = 20;
			else if( dwBadFame < 20000 )
				nRatio = 25;
			else if( dwBadFame < 80000 )
				nRatio = 30;
			else if( dwBadFame < 400000 )
				nRatio = 40;
			else if( dwBadFame < 1600000 )
				nRatio = 50;
			else if( dwBadFame < 8000000 )
				nRatio = 60;
			else if( dwBadFame < 32000000 )
				nRatio = 70;
			else if( dwBadFame < 100000000 )
				nRatio = 80;
			else if( dwBadFame < 500000000 )
				nRatio = 90;
			else
				nRatio = 100;
		}
		
		if( nRatio != 0 )
		{
			int nSeed = rand()%100;
			if( nSeed < nRatio )
			{
				MUGONG_TOTALINFO TotalInfo;
				pDiePlayer->GetMugongTotalInfo( &TotalInfo );
				cConstLinkedList<const MUGONGBASE*> ListMugong;
				for( int i = 0 ; i < SLOT_MUGONGTOTAL_NUM ; ++i )
				{
					if( TotalInfo.mugong[i].dwDBIdx != 0 )
					{
						ListMugong.AddTail( &TotalInfo.mugong[i] );
					}
				}
				
				if( ListMugong.GetCount() == 0 ) 
				{
					//���� ������ ����. 
					return;
				}
				
				int nIndex = rand()%ListMugong.GetCount();
				const MUGONGBASE* pMugong = ListMugong.GetAt(nIndex);
				
				if( pMugong )
				{
				if( MUGONGMNGR->RemMugong( pDiePlayer, pMugong->wIconIdx, pMugong->Position ) )
				}
				{
					MSG_MUGONG_REM_ACK ToMsg;
					ToMsg.Category		= MP_PK;
					ToMsg.Protocol		= MP_PK_DESTROY_MUGONG;
					ToMsg.dwObjectID	= pDiePlayer->GetID();
					ToMsg.wMugongIdx	= pMugong->wIconIdx;
					ToMsg.TargetPos		= pMugong->Position;
						
					pDiePlayer->SendMsg(&ToMsg, sizeof(ToMsg));
				}
					
				ListMugong.DeleteAll();
			}
		}
		*/
	}
}

void CPKManager::PKPlayerDiePanelty( CPlayer* pPlayer )
{
	//�׾����� �⺻������ �� �������� �г�Ƽ
	PKModeExitPanelty( pPlayer );
	//���������� �ı�
	DWORD dwBadFame = pPlayer->GetBadFame();

	int nMinDestroyItemNum	= 0;
	int nMaxDestroyItemNum	= 0;

#ifdef _CHINA_LOCAL_
	if( dwBadFame < 30 ) return;
#endif
//	if( g_pServerSystem->GetNation() == eNATION_CHINA )
//	{
//		if( dwBadFame < 30 ) return;
//	}

	if( dwBadFame == 0 )
	{
		nMinDestroyItemNum = 0;
		nMaxDestroyItemNum = 0;
	}
	else if( dwBadFame < 50)
	{
		nMinDestroyItemNum = 0;
		nMinDestroyItemNum = 0;
	}
	else if( dwBadFame < 4000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 1;
//		nDestroyRatio = 50;
	}
	else if( dwBadFame < 20000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 2;
//		nDestroyRatio = 60;
	}
	else if( dwBadFame < 80000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 2;
//		nDestroyRatio = 70;
	}
	else if( dwBadFame < 400000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 2;
//		nDestroyRatio = 80;
	}
	else if( dwBadFame < 1600000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 3;
//		nDestroyRatio = 90;
	}
	else if( dwBadFame < 8000000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 3;
//		nDestroyRatio = 100;
	}
	else if( dwBadFame < 32000000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 4;
//		nDestroyRatio = 100;
	}
	else if( dwBadFame < 100000000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 4;
//		nDestroyRatio = 100;
	}
	else if( dwBadFame < 500000000 )
	{
		nMinDestroyItemNum = 1;
		nMaxDestroyItemNum = 4;
//		nDestroyRatio = 100;
	}
	else
	{
		nMinDestroyItemNum = 2;
		nMaxDestroyItemNum = 4;
//		nDestroyRatio = 100;
	}

	int nDestroyRatio	= dwBadFame ? (int)(0.3070f * sqrt((float)dwBadFame) + 2.6930f) : 0;	//����, ����, Ư�� ��� �ش�
	
	BOOL bWantedPlayer = FALSE;
	if( pPlayer->GetWantedIdx() != 0 )	//������̸� 3%����
	{
		nDestroyRatio += 3;
		bWantedPlayer = TRUE;
	}

	if( nDestroyRatio == 0 ) return;	//�ı�Ȯ���� ������ ����.

	if( nDestroyRatio > 100 ) nDestroyRatio = 100;	

	int nDestoryItemNum = random( nMinDestroyItemNum, nMaxDestroyItemNum );

	int nSeed = rand()%100;

	if( nDestoryItemNum != 0 )
	{
		if( nSeed < nDestroyRatio )
		{
			CWearSlot* pWear			= (CWearSlot*)pPlayer->GetSlot( eItemTable_Weared );
			if( pWear->GetItemCount() )
			{
				//---���������� ����ۼ�
				cConstLinkedList<const ITEMBASE*> ListWearItem;
				for( int i = TP_WEAR_START ; i < TP_WEAR_END ; ++i )
				{
					//��ȯ��ҿ� ������ҵ��� ���� ��Ų�Ŀ�!
					if( !pWear->IsEmpty( i ) )
						if( !pWear->IsLock( i ) )
						{
							ListWearItem.AddTail( pWear->GetItemInfoAbs(i) );
						}
				}
				
				//---���������� ���� ����
				while( ListWearItem.GetCount() )
				{
					int nIndex		= rand()%ListWearItem.GetCount();
					ITEMBASE ItemInfo = *ListWearItem.GetAt( nIndex );
					
					ListWearItem.DeleteAt( nIndex );
					
					//CItemSlot* pDSlot = pDiePlayer->GetSlot( pItemInfo->Position );			
					//			if( EI_TRUE != pDSlot->DeleteItemAbs( pDiePlayer, m_LootingItemArray[nArrayNum].dwData, &ItemBase ) )
					//				return FALSE;
					MSG_ITEM_DESTROY msg;
					msg.Category		= MP_PK;
					msg.Protocol		= MP_PK_DESTROY_ITEM;
					msg.wAbsPosition	= ItemInfo.Position;

					WORD wType = bWantedPlayer && nSeed < 3 ? eLog_ItemDestroyByWanted : eLog_ItemDestroyByBadFame;

					if( wType == eLog_ItemDestroyByWanted )
						msg.cbReason		= 1;		//������̹Ƿ� �ı�
					else
						msg.cbReason		= 0;		//���� �Ǹ����� �ı�

					if( EI_TRUE == ITEMMGR->DiscardItem( pPlayer, ItemInfo.Position, ItemInfo.wIconIdx,
						ItemInfo.Durability ) )
					{
						pPlayer->SendMsg( &msg, sizeof(msg) );
						// Log
						LogItemMoney(pPlayer->GetID(), pPlayer->GetObjectName(), 0, "",
							wType, pPlayer->GetMoney(), 0, 0, 
							ItemInfo.wIconIdx, ItemInfo.dwDBIdx, ItemInfo.Position, 0, 
							ItemInfo.Durability, pPlayer->GetPlayerExpPoint());
					}					
					
					if( --nDestoryItemNum <= 0 ) break;
				}
				ListWearItem.DeleteAll();
			}
		}
	}

	//�����ı�	(������ ���ԵǾ� �ִ°ǰ�?)

#ifdef _CHINA_LOCAL_
	if( dwBadFame < 60 ) return;
#endif
//	if( g_pServerSystem->GetNation() == eNATION_CHINA )
//	{
//		if( dwBadFame < 60 ) return;
//	}

	nSeed = rand()%100;
	if( nSeed < nDestroyRatio )
	{
		MUGONG_TOTALINFO TotalInfo;
		pPlayer->GetMugongTotalInfo( &TotalInfo );

		cConstLinkedList<const MUGONGBASE*> ListMugong;
		for( int i = 0 ; i < SLOT_MUGONGTOTAL_NUM ; ++i )
		{
			if( TotalInfo.mugong[i].dwDBIdx != 0 )
			{
				ListMugong.AddTail( &TotalInfo.mugong[i] );
			}
		}
				
		if( ListMugong.GetCount() != 0 ) 
		{
			int nIndex = rand()%ListMugong.GetCount();
			const MUGONGBASE* pMugong = ListMugong.GetAt(nIndex);

			WORD logType = bWantedPlayer && nSeed < 3 ? eLog_MugongLevelDownByWanted : eLog_MugongLevelDownByBadFame;
			
			if( pMugong )
				pPlayer->MugongLevelDown(pMugong->wIconIdx, logType);
					
			ListMugong.DeleteAll();
		}
	}
/*
	//Ư���ı� (�⺻Ư������ ����)
	nSeed = rand()%100;
	if( nSeed < nDestroyRatio )
	{
		MUGONG_TOTALINFO TotalInfo;
		pPlayer->GetMugongTotalInfo( &TotalInfo );

		cConstLinkedList<const MUGONGBASE*> ListMugong;
		for( int i = 0 ; i < SLOT_MUGONGTOTAL_NUM ; ++i )
		{
			if( TotalInfo.mugong[i].dwDBIdx != 0 )
			{
				ListMugong.AddTail( &TotalInfo.mugong[i] );
			}
		}
				
		if( ListMugong.GetCount() != 0 ) 
		{
			int nIndex = rand()%ListMugong.GetCount();
			const MUGONGBASE* pMugong = ListMugong.GetAt(nIndex);
			
			if( pMugong )
			{
			if( MUGONGMNGR->RemMugong( pPlayer, pMugong->wIconIdx, pMugong->Position ) )
			}
			{
				MSG_MUGONG_REM_ACK ToMsg;
				ToMsg.Category		= MP_PK;
				ToMsg.Protocol		= MP_PK_DESTROY_MUGONG;
				ToMsg.dwObjectID	= pPlayer->GetID();
				ToMsg.wMugongIdx	= pMugong->wIconIdx;
				ToMsg.TargetPos		= pMugong->Position;
				pPlayer->SendMsg(&ToMsg, sizeof(ToMsg));
			}
					
			ListMugong.DeleteAll();
		}
	}
*/
}

void CPKManager::PKModeExitPanelty( CPlayer* pPlayer, BOOL bPKModePanelty ) //���� ���� Ȥ�� �׾����� 
{
	DWORD dwBadFame = pPlayer->GetBadFame();

	if( dwBadFame < 50 && !bPKModePanelty )
	{
		return;
	}

	EXPTYPE dwExp		= 0;
	MONEYTYPE dwMoney	= 0;
	EXPTYPE pointForLevel	= GAMERESRCMNGR->GetMaxExpPoint( pPlayer->GetLevel() );	

	// 06.08.29. RaMa. ȫ�ῡ�� ����
#ifdef _HK_LOCAL_
#ifndef _TW_LOCAL_

	if( dwBadFame < 50 )
	{
		dwExp	= pointForLevel / 100;
		dwMoney = pPlayer->GetMoney() * 2 / 100;
	}
	else if( dwBadFame < 100 )
	{
		dwExp	= pointForLevel * 2 / 100;
		dwMoney = pPlayer->GetMoney() * 4 / 100;
	}
	else if( dwBadFame < 500 )
	{
		dwExp	= pointForLevel * 3 / 100;
		dwMoney = pPlayer->GetMoney() * 6 / 100;
	}
	else if( dwBadFame < 1000 )
	{
		dwExp	= pointForLevel * 10 / 100;
		dwMoney = pPlayer->GetMoney() * 13 / 100;
	}
	else if( dwBadFame < 2000 )
	{
		dwExp	= pointForLevel * 17 / 100;
		dwMoney = pPlayer->GetMoney() * 21 / 100;
	}
	else if( dwBadFame < 5000 )
	{
		dwExp	= pointForLevel * 24 / 100;
		dwMoney = pPlayer->GetMoney() * 29 / 100;
	}
	else if( dwBadFame < 10000 )
	{
		dwExp	= pointForLevel * 31 / 100;
		dwMoney = pPlayer->GetMoney() * 36 / 100;
	}
	else if( dwBadFame < 20000 )
	{
		dwExp	= pointForLevel * 38 / 100;
		dwMoney = pPlayer->GetMoney() * 44 / 100;
	}
	else if( dwBadFame < 50000 )
	{
		dwExp	= pointForLevel * 45 / 100;
		dwMoney = pPlayer->GetMoney() * 52 / 100;
	}
	else if( dwBadFame < 100000 )
	{
		dwExp	= pointForLevel * 52 / 100;
		dwMoney = pPlayer->GetMoney() * 59 / 100;
	}
	else if( dwBadFame < 200000 )
	{
		dwExp	= pointForLevel * 59 / 100;
		dwMoney = pPlayer->GetMoney() * 67 / 100;
	}
	else if( dwBadFame < 500000 )
	{
		dwExp	= pointForLevel * 66 / 100;
		dwMoney = pPlayer->GetMoney() * 74 / 100;
	}
	else if( dwBadFame < 1000000 )
	{
		dwExp	= pointForLevel * 73 / 100;
		dwMoney = pPlayer->GetMoney() * 82 / 100;
	}
	else	//10�����
	{
		dwExp	= pointForLevel * 80 / 100;
		dwMoney = pPlayer->GetMoney() * 90 / 100;
	}

#else

	if( dwBadFame < 2000 )
	{
		dwExp	= pointForLevel / 100;
		dwMoney = pPlayer->GetMoney() * 2 / 100;
	}
	else if( dwBadFame < 5000 )
	{
		dwExp	= pointForLevel * 2 / 100;
		dwMoney = pPlayer->GetMoney() * 4 / 100;
	}
	else if( dwBadFame < 20000 )
	{
		dwExp	= pointForLevel * 3 / 100;
		dwMoney = pPlayer->GetMoney() * 6 / 100;
	}
	else if( dwBadFame < 50000 )
	{
		dwExp	= pointForLevel * 10 / 100;
		dwMoney = pPlayer->GetMoney() * 13 / 100;
	}
	else if( dwBadFame < 200000 )
	{
		dwExp	= pointForLevel * 17 / 100;
		dwMoney = pPlayer->GetMoney() * 21 / 100;
	}
	else if( dwBadFame < 500000 )
	{
		dwExp	= pointForLevel * 24 / 100;
		dwMoney = pPlayer->GetMoney() * 29 / 100;
	}
	else if( dwBadFame < 2000000 )
	{
		dwExp	= pointForLevel * 31 / 100;
		dwMoney = pPlayer->GetMoney() * 36 / 100;
	}
	else if( dwBadFame < 5000000 )
	{
		dwExp	= pointForLevel * 38 / 100;
		dwMoney = pPlayer->GetMoney() * 44 / 100;
	}
	else if( dwBadFame < 20000000 )
	{
		dwExp	= pointForLevel * 45 / 100;
		dwMoney = pPlayer->GetMoney() * 52 / 100;
	}
	else if( dwBadFame < 100000000 )
	{
		dwExp	= pointForLevel * 52 / 100;
		dwMoney = pPlayer->GetMoney() * 59 / 100;
	}
	else if( dwBadFame < 200000000 )
	{
		dwExp	= pointForLevel * 59 / 100;
		dwMoney = pPlayer->GetMoney() * 67 / 100;
	}
	else if( dwBadFame < 400000000 )
	{
		dwExp	= pointForLevel * 66 / 100;
		dwMoney = pPlayer->GetMoney() * 74 / 100;
	}
	else if( dwBadFame < 800000000 )
	{
		dwExp	= pointForLevel * 73 / 100;
		dwMoney = pPlayer->GetMoney() * 82 / 100;
	}
	else	//10�����
	{
		dwExp	= pointForLevel * 80 / 100;
		dwMoney = pPlayer->GetMoney() * 90 / 100;
	}

#endif // _TW_LOCAL_
#else
	
	if( dwBadFame < 2000 )
	{
		dwExp	= pointForLevel / 100;
		dwMoney = pPlayer->GetMoney() * 2 / 100;
	}
	else if( dwBadFame < 5000 )
	{
		dwExp	= pointForLevel * 2 / 100;
		dwMoney = pPlayer->GetMoney() * 4 / 100;
	}
	else if( dwBadFame < 20000 )
	{
		dwExp	= pointForLevel * 3 / 100;
		dwMoney = pPlayer->GetMoney() * 6 / 100;
	}
	else if( dwBadFame < 50000 )
	{
		dwExp	= pointForLevel * 10 / 100;
		dwMoney = pPlayer->GetMoney() * 13 / 100;
	}
	else if( dwBadFame < 200000 )
	{
		dwExp	= pointForLevel * 17 / 100;
		dwMoney = pPlayer->GetMoney() * 21 / 100;
	}
	else if( dwBadFame < 500000 )
	{
		dwExp	= pointForLevel * 24 / 100;
		dwMoney = pPlayer->GetMoney() * 29 / 100;
	}
	else if( dwBadFame < 2000000 )
	{
		dwExp	= pointForLevel * 31 / 100;
		dwMoney = pPlayer->GetMoney() * 36 / 100;
	}
	else if( dwBadFame < 5000000 )
	{
		dwExp	= pointForLevel * 38 / 100;
		dwMoney = pPlayer->GetMoney() * 44 / 100;
	}
	else if( dwBadFame < 20000000 )
	{
		dwExp	= pointForLevel * 45 / 100;
		dwMoney = pPlayer->GetMoney() * 52 / 100;
	}
	else if( dwBadFame < 100000000 )
	{
		dwExp	= pointForLevel * 52 / 100;
		dwMoney = pPlayer->GetMoney() * 59 / 100;
	}
	else if( dwBadFame < 200000000 )
	{
		dwExp	= pointForLevel * 59 / 100;
		dwMoney = pPlayer->GetMoney() * 67 / 100;
	}
	else if( dwBadFame < 400000000 )
	{
		dwExp	= pointForLevel * 66 / 100;
		dwMoney = pPlayer->GetMoney() * 74 / 100;
	}
	else if( dwBadFame < 800000000 )
	{
		dwExp	= pointForLevel * 73 / 100;
		dwMoney = pPlayer->GetMoney() * 82 / 100;
	}
	else	//10�����
	{
		dwExp	= pointForLevel * 80 / 100;
		dwMoney = pPlayer->GetMoney() * 90 / 100;
	}

#endif // _HK_LOCAL_

	if( dwMoney )
	{
		pPlayer->SetMoney( dwMoney, MONEY_SUBTRACTION, MF_LOST, eItemTable_Inventory, eMoneyLog_LosePKModeExitPanelty, 0 );
	}

	if( dwExp && pPlayer->GetLevel() >= 5 )		//5���� �̻��϶��� �Ǹ�ġ �г�Ƽ�� �ִ�.
	{
//		if( pPlayer->GetPlayerExpPoint() < dwExp )
//			dwExp = pPlayer->GetPlayerExpPoint();
//		if( dwExp )

		pPlayer->ReduceExpPoint( dwExp , eExpLog_LosebyBadFame );
	}
}

void CPKManager::NetworkMsgParse( BYTE Protocol, void* pMsg )
{
	switch( Protocol )
	{
	case MP_PK_PKON_SYN:
		{
			MSGBASE* pmsg = (MSGBASE*)pMsg;

			CPlayer* pPlayer	= (CPlayer*)g_pUserTable->FindUser(pmsg->dwObjectID);
			if( !pPlayer ) break;

			DWORD dwChannel = pPlayer->GetChannelID();
			
			if( !IsPKAllow(dwChannel) )	//�ӽ�
			{
				MSG_BYTE msg;				//error code?
				msg.Category	= MP_PK;
				msg.Protocol	= MP_PK_PKON_NACK;
				msg.bData		= ePKCODE_NOTALLAW;
				pPlayer->SendMsg( &msg, sizeof(msg) );
				break;
			}

			int rt;
			if( ( rt = pPlayer->PKModeOn() ) == ePKCODE_OK )
			{
				MSG_DWORD msg;
				msg.Category	= MP_PK;
				msg.Protocol	= MP_PK_PKON_ACK;
				msg.dwData		= pmsg->dwObjectID;
				msg.dwObjectID	= pPlayer->GetID();

				PACKEDDATA_OBJ->QuickSend( pPlayer, &msg, sizeof(msg) );
			}
			else
			{
				MSG_BYTE msg;
				msg.Category	= MP_PK;
				msg.Protocol	= MP_PK_PKON_NACK;
				msg.bData		= rt;
				pPlayer->SendMsg( &msg, sizeof(msg) );
			}
		}
		break;

	case MP_PK_PKOFF_SYN:
		{
			MSGBASE* pmsg = (MSGBASE*)pMsg;

			CPlayer* pPlayer	= (CPlayer*)g_pUserTable->FindUser(pmsg->dwObjectID);
			if( !pPlayer ) break;

			if( pPlayer->PKModeOff() )
			{
				MSG_DWORD msg;
				msg.Category	= MP_PK;
				msg.Protocol	= MP_PK_PKOFF_ACK;
				msg.dwData		= pmsg->dwObjectID;
				msg.dwObjectID	= pPlayer->GetID();

				PACKEDDATA_OBJ->QuickSend( pPlayer, &msg, sizeof(msg) );
			}
			else
			{
				MSGBASE msg;
				msg.Category	= MP_PK;
				msg.Protocol	= MP_PK_PKOFF_NACK;
				pPlayer->SendMsg( &msg, sizeof(msg) );
			}
		}
		break;
		
	case MP_PK_LOOTING_SELECT_SYN:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CLootingRoom* pRoom = LOOTINGMGR->GetLootingRoom( pmsg->dwData1 );
			if( pRoom )
			{
				LOOTINGMGR->Loot( pmsg->dwData1, pmsg->dwData2 );
				
			}
			else
			{
				CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser(pmsg->dwObjectID);
				if( pPlayer )
				{
					MSG_DWORD2 msg;
					msg.Category	= MP_PK;
					msg.Protocol	= MP_PK_LOOTING_ERROR;
					msg.dwData1		= pmsg->dwData1;
					msg.dwData2		= eLOOTINGERROR_NO_LOOTINGROOM;
					pPlayer->SendMsg( &msg, sizeof(msg) );
				}
			}
		}
		break;
	
	case MP_PK_LOOTING_ENDLOOTING:
		{
			MSG_DWORD*	pmsg = (MSG_DWORD*)pMsg;
			LOOTINGMGR->CloseLootingRoom( pmsg->dwData, FALSE );
		}
	}	
}
