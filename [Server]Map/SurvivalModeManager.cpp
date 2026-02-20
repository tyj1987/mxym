#include "stdafx.h"
#include "ServerSystem.h"
#include "UserTable.h"
#include "Player.h"
#include "Network.h"
#include "ObjectStateManager.h"
#include "MapDBMsgParser.h"
#include "QuestManager.h"
#include "PackedData.h"

#include ".\survivalmodemanager.h"

GLOBALTON(CSurvivalModeManager);

CSurvivalModeManager::CSurvivalModeManager(void)
{
	m_SVModeUserTable.Initialize(50);
	m_SVItemUsingCounter.Initialize(50);

	Init();
}

CSurvivalModeManager::~CSurvivalModeManager(void)
{
	Release();
}

void CSurvivalModeManager::Init()
{
	m_wModeState = eSVVMode_None;
	m_dwStateRemainTime = 0;
	m_nUserAlive = 0;

	m_dwUsingCountLimit = 5;	//⺻ 5
}

void CSurvivalModeManager::Release()
{
	m_SVModeUserTable.RemoveAll();
	m_SVItemUsingCounter.RemoveAll();

	DWORD* pCounter = NULL;
	m_SVItemUsingCounter.SetPositionHead();
	while( pCounter = m_SVItemUsingCounter.GetData() )
	{
		delete pCounter;
	}
	m_SVModeAliveUserList.RemoveAll();
}

void CSurvivalModeManager::Process()
{

	switch(m_wModeState)
	{
	case eSVVMode_None:
		{
			//GM   Ready  ȭ
		}
		break;
	case eSVVMode_Ready:
		{
			//10 īƮ  Fight  ȭ
			if( CheckRemainTime() )
			{
				ChangeStateTo(eSVVMode_Fight);
			}
		}
		break;
	case eSVVMode_Fight:
		{
			//1 Ƴ  End  ȭ
		}
		break;
	case eSVVMode_End:
		{
			//10 īƮ  Ready  ȭ
			if( CheckRemainTime() )
			{
				ChangeStateTo(eSVVMode_None);
			}
		}
		break;
	default:
		break;
	}
}

void CSurvivalModeManager::NetworkMsgParse( DWORD dwConnectionIndex, BYTE Protocol, void* pMsg )
{
	MSGBASE* pTempMsg = (MSGBASE*)pMsg;
	CPlayer* pSender = (CPlayer*)g_pUserTable->FindUser( pTempMsg->dwObjectID );
	if(pSender)
	{
		if( pSender->GetUserLevel() > eUSERLEVEL_GM && Protocol != MP_SURVIVAL_LEAVE_SYN )
		{
			char buf[64];
			sprintf(buf, "User Lvl Chk! Character_idx : %d", pTempMsg->dwObjectID);
			ASSERTMSG(0, buf);
			return;
		}
	}

	switch( Protocol )
	{
	case MP_SURVIVAL_READY_SYN:
		{
			if( m_wModeState != eSVVMode_None )
				SendNackMsg(pSender, MP_SURVIVAL_READY_NACK, 1);
			else
				ChangeStateTo(eSVVMode_Ready);
		}
		break;
	case MP_SURVIVAL_STOP_SYN:
		{
			if( m_wModeState != eSVVMode_Fight )
				SendNackMsg(pSender, MP_SURVIVAL_STOP_NACK, 1);
			else
				ChangeStateTo(eSVVMode_None);
		}
		break;
	case MP_SURVIVAL_MAPOFF_SYN:
		{
			//   ǵ.
			ReturnToMap();
		}
		break;
	case MP_SURVIVAL_LEAVE_SYN:
		{
			MSG_DWORD3* pmsg = (MSG_DWORD3*)pMsg;

			CObject* pObject = g_pUserTable->FindUser(pmsg->dwObjectID);
			if( pObject != NULL )
			{
				if( pObject->GetObjectKind() == eObjectKind_Player )
				{
					MSG_DWORD msg;
					SetProtocol( &msg, MP_SURVIVAL, MP_USERCONN_GAMEIN_NACK );
					msg.dwData = pmsg->dwObjectID;
					g_Network.Send2Server( dwConnectionIndex, (char*)&msg, sizeof(msg) );
					return;
				}
			}

			CPlayer* pPlayer = g_pServerSystem->AddPlayer( pmsg->dwObjectID, dwConnectionIndex, pmsg->dwData1, pmsg->dwData3 );
			if(!pPlayer) return;

			pPlayer->SetUserLevel( pmsg->dwData2 );
			CharacterNumSendAndCharacterInfo(pmsg->dwObjectID, MP_USERCONN_GAMEIN_SYN);
			CharacterMugongInfo(pmsg->dwObjectID, MP_USERCONN_GAMEIN_SYN);
			CharacterItemOptionInfo(pmsg->dwObjectID, MP_USERCONN_GAMEIN_SYN);
			//SW050920 Rare
			CharacterItemRareOptionInfo(pmsg->dwObjectID, MP_USERCONN_GAMEIN_SYN);
			CharacterAbilityInfo(pmsg->dwObjectID, MP_USERCONN_GAMEIN_SYN);

			QuestTotalInfo(pmsg->dwObjectID);

			QUESTMGR->CreateQuestForPlayer( pPlayer );
			// ׻ Ʈ  о ...
//			QuestMainQuestLoad(pmsg->dwObjectID);
			QuestSubQuestLoad(pmsg->dwObjectID);
//			QuestItemload(pmsg->dwObjectID);			
		}
		break;
	case MP_SURVIVAL_ITEMUSINGCOUNT_SET:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;
			SetUsingCountLimit(pmsg->dwData);
		}
		break;
	default:
		break;
	}
}

void CSurvivalModeManager::SendMsgToAllSVModeUser(MSGBASE* pMsg, int msgsize)
{
	/*CObject* pObj = NULL;
	m_SVModeUserTable.SetPositionHead();
	while( pObj = m_SVModeUserTable.GetData() )
	{
		if(pObj->GetObjectKind() == eObjectKind_Player)
		{
			((CPlayer*)pObj)->SendMsg(pMsg,msgsize);
		}
	}*/
	g_pUserTable->SetPositionUserHead();
	while( CObject* pObject = g_pUserTable->GetUserData() )
	{
		if( pObject->GetObjectKind() != eObjectKind_Player ) continue;

		CPlayer* pSVUser = (CPlayer*)pObject;
		pSVUser->SendMsg(pMsg, msgsize);
	}
}

void CSurvivalModeManager::SendAliveUserCount()
{
	MSG_DWORD msg;
	msg.Category = MP_SURVIVAL;
	msg.Protocol = MP_SURVIVAL_ALIVEUSER_COUNT;
	msg.dwData = m_SVModeAliveUserList.GetCount();
	SendMsgToAllSVModeUser(&msg, sizeof(msg));
}

void CSurvivalModeManager::SendNackMsg(CPlayer* pGM, BYTE Protocol, BYTE errstate)
{
	MSG_BYTE nmsg;
	nmsg.Category = MP_SURVIVAL;
	nmsg.Protocol = Protocol;
	nmsg.bData = errstate;
	pGM->SendMsg(&nmsg, sizeof(nmsg));
}

BOOL CSurvivalModeManager::CheckRemainTime()
{// º ȿ ð üũѴ.
	if(m_dwStateRemainTime)
	{
		if(gCurTime > m_dwStateRemainTime)
		{
			m_dwStateRemainTime = 0;
			return TRUE;
		}
		else
			return FALSE;
	}

	return FALSE;
}

void CSurvivalModeManager::SetUsingCountLimit( DWORD limit )
{
	m_dwUsingCountLimit = limit;

	MSG_DWORD msg;
	msg.Category = MP_SURVIVAL;
	msg.Protocol = MP_SURVIVAL_ITEMUSINGCOUNT_LIMIT;
	msg.dwData = limit;

	SendMsgToAllSVModeUser( &msg, sizeof(msg) );
}

BOOL CSurvivalModeManager::AddItemUsingCount( CPlayer* pPlayer )
{
	DWORD* pCounter = NULL;
	pCounter = m_SVItemUsingCounter.GetData( pPlayer->GetID() );
	if( pCounter )
	{
		if( *pCounter >= m_dwUsingCountLimit )
		{
			return FALSE;
		}
		else
		{
			(*pCounter)++;
			return TRUE;
		}
	}
	return FALSE;
}

void CSurvivalModeManager::ChangeStateTo( WORD nextState )
{//  ȭ ش ó
	if(m_wModeState == nextState)
		return;
	
	m_wModeState = nextState;

	switch(nextState)
	{
	case eSVVMode_None:
		{
			m_dwStateRemainTime = 0;

			//  츮 ʱȭ.
			ReadyToSurvivalMode();

			SendAliveUserCount();

			MSGBASE msg;
			msg.Category = MP_SURVIVAL;
			msg.Protocol = MP_SURVIVAL_NONE_ACK;
			
			SendMsgToAllSVModeUser(&msg, sizeof(msg));
		}
		break;
	case eSVVMode_Ready:
		{
			m_dwStateRemainTime = gCurTime + eSVVMD_TIME_READY;

			//m_nUserAlive = m_SVModeUserTable.GetDataNum();

			MSGBASE msg;
			msg.Category = MP_SURVIVAL;
			msg.Protocol = MP_SURVIVAL_READY_ACK;

			SendMsgToAllSVModeUser(&msg, sizeof(msg));
		}
		break;
	case eSVVMode_Fight:
		{
			m_dwStateRemainTime = 0;
			MSGBASE msg;
			msg.Category = MP_SURVIVAL;
			msg.Protocol = MP_SURVIVAL_FIGHT_ACK;

			SendMsgToAllSVModeUser(&msg, sizeof(msg));
		}
		break;
	case eSVVMode_End:
		{
			m_dwStateRemainTime = gCurTime + eSVVMD_TIME_END;

			//ִ Ѹ ã  ְ
			PTRLISTPOS pos = m_SVModeAliveUserList.GetHeadPosition();

			CPlayer* pTheWinner = (CPlayer*)m_SVModeAliveUserList.GetAt(pos);

			if(pTheWinner)
			{
				//ӽų ״° . -_-;
				pTheWinner->SetLife(pTheWinner->GetMaxLife());

				////Ʈ ó -> MSG_SVVMODE_ENDINFO ϳ ó.
				//MSG_DWORD Msg;
				//Msg.Category = MP_SURVIVAL;
				//Msg.Protocol = MP_SURVIVAL_WINNER_EFFECT;
				//Msg.dwObjectID = pTheWinner->GetID();
				//Msg.dwData = pTheWinner->GetID();

				//PACKEDDATA_OBJ->QuickSendExceptObjectSelf(pTheWinner, &Msg, sizeof(MSG_DWORD));

				MSG_SVVMODE_ENDINFO msg;
				msg.Category = MP_SURVIVAL;
				msg.Protocol = MP_SURVIVAL_END_ACK;
				msg.WinnerID = pTheWinner->GetID();
				//ID ó ϴ  ٸ  ׸   ֱ !
				SafeStrCpy( msg.WinnerName, pTheWinner->GetObjectName(), MAX_NAME_LENGTH+1);

				SendMsgToAllSVModeUser(&msg, sizeof(msg));
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}
}

void CSurvivalModeManager::ReadyToSurvivalMode()
{//   츮 غ ¿ .
	m_SVModeAliveUserList.RemoveAll();

	DWORD* pCounter = NULL;

	CObject* pObj = NULL;
	m_SVModeUserTable.SetPositionHead();
	while( pObj = m_SVModeUserTable.GetData() )
	{
		if(pObj->GetObjectKind() == eObjectKind_Player)
		{
			//츮
			CPlayer* pPlayer = ((CPlayer*)pObj);

			if( OBJECTSTATEMGR_OBJ->GetObjectState(pObj) == eObjectState_Die )
				pPlayer->RevivePresentSpot();

			pPlayer->SetLife(pPlayer->GetMaxLife());
			pPlayer->SetNaeRyuk(pPlayer->GetMaxNaeRyuk());
			pPlayer->SetShield(pPlayer->GetMaxShield());

			m_SVModeAliveUserList.AddTail(pObj);

			//SW061129 ȫ߰û۾ - 밹
			pCounter = m_SVItemUsingCounter.GetData(pObj->GetID());
			if(pCounter)
				*pCounter = 0;
		}
	}
}

void CSurvivalModeManager::ReturnToMap()
{
	MSG_DWORD msg;
	msg.Category = MP_SURVIVAL;
	msg.Protocol = MP_SURVIVAL_RETURNTOMAP;

	CObject* pObj = NULL;
	m_SVModeUserTable.SetPositionHead();
	while( pObj = m_SVModeUserTable.GetData() )
	{
		if(pObj->GetObjectKind() == eObjectKind_Player )
		{
			CPlayer* pPlayer = ((CPlayer*)pObj);

			msg.dwData = pPlayer->GetReturnMapNum();
			pPlayer->SendMsg(&msg, sizeof(msg));

			g_pServerSystem->RemovePlayer( pPlayer->GetID() );
		}
	}
}

void CSurvivalModeManager::AddSVModeUser( CObject* pObject )
{
	if( pObject->GetObjectKind() == eObjectKind_Player )
	if( ((CPlayer*)pObject)->GetUserLevel() <= eUSERLEVEL_GM )
		return;

	if( m_SVModeUserTable.GetData( pObject->GetID() ) )
		return;

	CObject* pObj = m_SVModeUserTable.GetData( pObject->GetID() );
	if( pObj )
	{
		char buf[128] = {0,};
		sprintf(buf, "User Duplication! Name: %s ID: %d", pObj->GetObjectName(), pObj->GetID() );
		ASSERTMSG(0, buf);
		return;
	}
	m_SVModeUserTable.Add( pObject, pObject->GetID() );

	AddAliveUser(pObject);

	//SW061129 ȫ߰û۾ - 밹
	DWORD* pCounter = new DWORD;
	*pCounter = 0;
	m_SVItemUsingCounter.Add( pCounter, pObject->GetID() );

	MSG_DWORD msg;
	msg.Category = MP_SURVIVAL;
	msg.Protocol = MP_SURVIVAL_ITEMUSINGCOUNT_LIMIT;
	msg.dwData = m_dwUsingCountLimit;
	((CPlayer*)pObject)->SendMsg(&msg, sizeof(msg));

	//if( pObject->GetLife() )	//DB̶ 0.	//Inited ķ ?
		//AddAliveUserCount(TRUE);
}

void CSurvivalModeManager::RemoveSVModeUser( CObject* pObject )
{
	if( pObject->GetObjectKind() == eObjectKind_Player )
	if( ((CPlayer*)pObject)->GetUserLevel() <= eUSERLEVEL_GM )
		return;

	CObject* pObj = m_SVModeUserTable.GetData( pObject->GetID() );

	if(pObj)
	{
		if(pObj->GetObjectKind() != eObjectKind_Player )
		{
			char buf[128] = {0,};
			sprintf(buf, "SVVMODE_ObjectTable ERROR!! OBJ_ID: %d OBJ_NAME: %s", pObj->GetID(), pObj->GetObjectName() );
			ASSERTMSG(0, buf);
		}

		m_SVModeUserTable.Remove( pObject->GetID() );

		//if( pObj->GetLife() )	//̰ ..
		RemoveAliveUser(pObj);

		//SW061129 ȫ߰û۾ - 밹
		DWORD* pCounter = m_SVItemUsingCounter.GetData( pObj->GetID() );
		if(pCounter)
			delete pCounter;
		m_SVItemUsingCounter.Remove( pObj->GetID() );

	}
	else
		return;

}

void CSurvivalModeManager::AddAliveUser( CObject* pObject )
{	// Ʈ
	m_SVModeAliveUserList.AddTail(pObject);

	SendAliveUserCount();
}

void CSurvivalModeManager::RemoveAliveUser( CObject* pObject )
{
	m_SVModeAliveUserList.Remove(pObject);

	int AliveCount = m_SVModeAliveUserList.GetCount();

	SendAliveUserCount();

#define THE_ONE	1
	if( THE_ONE == AliveCount && eSVVMode_Fight == GetCurModeState())
	{
		ChangeStateTo(eSVVMode_End);
	}
}

//int CSurvivalModeManager::AddAliveUserCount( BOOL bPlus )
//{
//	int before = m_nUserAlive;
//
//	if(bPlus)	//+
//		m_nUserAlive++;
//	else		//-
//		m_nUserAlive--;
//
//	//  īƮ  
//	MSG_DWORD msg;
//	msg.Category = MP_SURVIVAL;
//	msg.Protocol = MP_SURVIVAL_ALIVEUSER_COUNT;
//	msg.dwData = m_nUserAlive;
//	SendMsgToAllSVModeUser(&msg, sizeof(msg));
//		
//	return m_nUserAlive;
//
//	//if( GetCurModeState() == eSVVMode_Fight )
//	if( before > 1 && m_nUserAlive == 1 )
//	{
//		ChangeStateTo(eSVVMode_End);
//	}
//}
