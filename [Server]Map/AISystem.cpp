// AISystem.cpp: implementation of the CAISystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerSystem.h"
#include "AISystem.h"

#include "UserTable.h"
#include "Monster.h"
#include "MsgRouter.h"
#include "MHFile.h"
#include "Player.h"
#include "..\[CC]Header\GameResourceManager.h"
#include "RegenPrototype.h"
#include "ChannelSystem.h"
#include "RegenManager.h"
#include "RegenConditionInfo.h"
#include "AIUniqueGroup.h"
// �ʵ庸�� - 05.12 �̿���
#include "FieldBossMonsterManager.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAISystem * g_pAISystem = NULL;

CAISystem::CAISystem()
{
	m_pROUTER = new CMsgRouter;
	m_pROUTER->InitMsgRouter();
	m_AISubordinatedObject.Initialize(500);
	m_MonsterIDGenerator.Init(10000, NEWOBJECT_STARTINDEX);
}

CAISystem::~CAISystem()
{
	if(m_pROUTER)
	{
		m_pROUTER->ReleaseMsgRouter();
		delete m_pROUTER;
		m_pROUTER = NULL;
	}
	m_MonsterIDGenerator.Release();
	// inner �޸𸮴� UserTable����Ʈ���� ������
	RemoveAllList();
}
void CAISystem::RemoveAllList()
{
	m_AISubordinatedObject.RemoveAll();
}
void CAISystem::Process()
{
	CObject * pObj= NULL;
	m_AISubordinatedObject.SetPositionHead();
	while((pObj = m_AISubordinatedObject.GetData()))
	{
		ConstantProcess(pObj);
		PeriodicProcess(pObj);
	}

	m_pROUTER->MsgLoop();

//--�׻� ���ʿ䰡 ����. 1�ʸ��� �ѹ���?
	static DWORD dwRegenCheckTime = 0;

	if( gCurTime - dwRegenCheckTime >= 3000 ) //3��
	{
		GROUPMGR->RegenProcess();
		dwRegenCheckTime = gCurTime;
	}
}
void CAISystem::ConstantProcess(CObject * obj)
{
	//CMonster * pMob = (CMonster *)obj;
	//pMob->GetStateMachine()->Process(eSEVENT_Process, NULL);
	
	//������ ������Ʈ�� ���� ó��
	if(obj->GetObjectKind() == eObjectKind_BossMonster)
		return;
	GSTATEMACHINE.Process( obj, eSEVENT_Process, NULL);
}

void CAISystem::PeriodicProcess(CObject * obj)
{

}

void CAISystem::AddObject(CObject * obj)
{
	
	CMonster * pMob = (CMonster * )obj;
	m_AISubordinatedObject.Add(obj, pMob->GetID());
	
//	CAIGroup * pGroup = GROUPMGR->GetGroup(pMob->GetMonsterGroupNum(), pMob->GetGridID());
//	if(pGroup)
//	{
//		pGroup->Alive(pMob->GetID());
//	}
//	else
//	{
//		//��ȯ ���ʹ� �׷��� ����!
//		return;
//	}

}
CObject * CAISystem::RemoveObject(DWORD dwID)
{
	CObject *outObj = m_AISubordinatedObject.GetData(dwID);
	if(!outObj)
		return NULL;
	m_AISubordinatedObject.Remove(dwID);

	CAIGroup * pGroup = GROUPMGR->GetGroup(((CMonster*)outObj)->GetMonsterGroupNum(), ((CMonster*)outObj)->GetGridID());
	if(pGroup)
	{
		pGroup->Die(outObj->GetID());
		pGroup->RegenCheck();
	}
	else
	{
		//��ȯ ���ʹ� �׷��� ����!
	}
	
	
	return outObj;
}

void CAISystem::SetTransition(CObject * obj, DWORD inputEvent)
{
	//obj->GetStateMachine()->SetState()
}

void CAISystem::SendMsg(WORD msgID, DWORD src, DWORD dest, DWORD delay, DWORD flag)
{
	m_pROUTER->SendMsg(msgID, src, dest, delay, flag);
}

DWORD CAISystem::GeneraterMonsterID()
{
	DWORD id = m_MonsterIDGenerator.GenerateIndex();
	ASSERT(id != 0);
	return id;
}
void CAISystem::ReleaseMonsterID(DWORD id)
{
	BOOL rt = m_MonsterIDGenerator.ReleaseIndex(id);
//	ASSERT(rt);
}
//--------------------------------------------------------------------------------------
// *** Caution ***
// RegenObject �˻� ���
// Group�� ã�� ���� => ObjectID
// GroupObject �˻� ���
// => GroupID, battleID
//--------------------------------------------------------------------------------------
void CAISystem::LoadAIGroupList()
{
	CMHFile file;
	char filename[256];
#ifdef _FILE_BIN_
//	sprintf(filename,"Resource/Monster_%02d.bin",GAMERESRCMNGR->GetLoadMapNum());
	sprintf(filename,"Resource/Server/Monster_%02d.bin",GAMERESRCMNGR->GetLoadMapNum());
	if(!file.Init(filename,"rb"))
		return ;
#else
	sprintf(filename,"Resource/Monster_%02d.txt",GAMERESRCMNGR->GetLoadMapNum());
	if(!file.Init(filename,"rt"))
		return ;
#endif
	
	int ChannelCount = CHANNELSYSTEM->GetChannelCount();
	DWORD BattleID = 0;
	DWORD TARGETGROUPID	= 0;
	DWORD GROUPID = 0;
	DWORD RandomGridID = CHANNELSYSTEM->GetRandomChannelID();
//	DWORD LastGridID = CHANNELSYSTEM->GetLastChannelID();

	CAIGroup * pNewGroup[MAX_CHANNEL_NUM+1];
//	CAIUniqueGroup * pNewUniqueGroup;
	CAIUniqueGroup* pNewUniqueGroup[MAX_CHANNEL_NUM+1];
	CRegenObject * pNewRegenObject[MAX_CHANNEL_NUM+1];
	CRegenPrototype * pRegenPrototype = NULL;

	char buff[256] = {0,};
	while(1)
	{
		if(file.IsEOF())
		{
			break;
		}
		file.GetString(buff);
		if(buff[0] == '}')
		{	
			continue;
		}
		if(buff[0] == '{')
		{
			continue;
		}
		if(buff[0] == '@')
		{
			file.GetLineX(buff, 256);
			continue;
		}
		strcpy(buff, _strupr(buff));	
		
		CMD_ST(buff)
		CMD_CS("$GROUP")		
			GROUPID = file.GetDword();
			for(int i = 0 ; i < ChannelCount ; ++i)
			{
				pNewGroup[i] = new CAIGroup;
				pNewGroup[i]->m_dwGroupID			= GROUPID;
				pNewGroup[i]->m_dwGridID		= CHANNELSYSTEM->GetChannelID(i);
				GROUPMGR->AddGroup(pNewGroup[i]);
			}		
		CMD_CS("#MAXOBJECT")
			file.GetDword();
		CMD_CS("#PROPERTY")
			file.GetWord();
		CMD_CS("#GROUPNAME")
			file.GetString();
		CMD_CS("#ADDCONDITION")
			TARGETGROUPID			= file.GetDword();
			float RATIO					= file.GetFloat();
			DWORD REGENDELAY			= file.GetDword();
			BOOL bRegen					= file.GetBool();
			for(int i = 0 ; i < ChannelCount ; ++i)
			{
				CRegenConditionInfo * pCondInfo = new CRegenConditionInfo;
				pCondInfo->dwStartRegenTick		= 0;
				pCondInfo->dwTargetGroupID		= TARGETGROUPID;
				pCondInfo->fRemainderRatio		= RATIO;
				//pCondInfo->dwRegenDelay			= REGENDELAY;
				//SW080515 ä�κ� ����
				pCondInfo->dwRegenDelay			= REGENDELAY * CHANNELSYSTEM->GetRegenRate(i+1);
				pCondInfo->bRegen				= bRegen;
				ASSERT(GROUPMGR->GetGroup(TARGETGROUPID,CHANNELSYSTEM->GetChannelID(i)));
				pNewGroup[i]->AddConditionInfo(pCondInfo);
			}
		CMD_CS("#ADD")

			pRegenPrototype = new CRegenPrototype;
			BYTE OBJECTKIND			= pRegenPrototype->ObjectKind		= file.GetByte();
			file.GetDword();
			DWORD OBJECTID			= pRegenPrototype->dwObjectID		= GeneraterMonsterID();//pRegenPrototype->dwObjectID		= file.GetDword();
			WORD MONSTERKIND		= pRegenPrototype->wMonsterKind		= file.GetWord();
			float POSX				= pRegenPrototype->vPos.x			= file.GetFloat();
			float POSZ				= pRegenPrototype->vPos.z			= file.GetFloat();

			BOOL ISREGEN			= file.GetBool();

			BASE_MONSTER_LIST* pMonsterInfo = GAMERESRCMNGR->GetMonsterListInfo(MONSTERKIND);
			WORD HELPTYPE			= pRegenPrototype->InitHelpType		= pMonsterInfo->InitHelpType;
			BOOL ISHEARING			= pRegenPrototype->bHearing			= pMonsterInfo->bHearing;
			DWORD HEARINGDISTANCE	= pRegenPrototype->HearingDistance	= pMonsterInfo->HearingDistance;

			REGENMGR->AddPrototype(pRegenPrototype);
	
			for(int i = 0 ; i < ChannelCount ; ++i)
			{
				pNewRegenObject[i] = new CRegenObject;
				pNewRegenObject[i]->InitPrototype(pRegenPrototype);
				pNewRegenObject[i]->m_dwObjectID = GeneraterMonsterID();
				pNewRegenObject[i]->m_dwSubObjectID = OBJECTID;
				pNewRegenObject[i]->m_dwGridID = CHANNELSYSTEM->GetChannelID(i);
				pNewRegenObject[i]->m_dwGroupID = pNewGroup[i]->m_dwGroupID;
				pNewRegenObject[i]->InitHelpType();
				pNewGroup[i]->AddRegenObject(pNewRegenObject[i]);
				
				if(ISREGEN)
				{
					pNewGroup[i]->Die(pNewRegenObject[i]->m_dwObjectID);
				}
			}
			pRegenPrototype = NULL;
		//////////////////////////////////////////////////////////////////////////
		// for boss monster
		CMD_CS("$UNIQUE")		 
//			GROUPID = file.GetDword();
//			pNewUniqueGroup = new CAIUniqueGroup;
//			pNewUniqueGroup->m_dwGroupID		= GROUPID;
//			pNewUniqueGroup->m_dwGridID		= RandomGridID;
//			GROUPMGR->AddGroup(pNewUniqueGroup);
		
			GROUPID = file.GetDword();
			pNewUniqueGroup[RandomGridID] = new CAIUniqueGroup;
			pNewUniqueGroup[RandomGridID]->m_dwGroupID = GROUPID;
			pNewUniqueGroup[RandomGridID]->m_dwGridID = RandomGridID;
			GROUPMGR->AddGroup(pNewUniqueGroup[RandomGridID]);
/*
			if( g_pServerSystem->GetNation() == eNATION_KOREA )
			if( ChannelCount > 1 )
			{
				pNewUniqueGroup[LastGridID] = new CAIUniqueGroup;
				pNewUniqueGroup[LastGridID]->m_dwGroupID = GROUPID;
				pNewUniqueGroup[LastGridID]->m_dwGridID = LastGridID;
				GROUPMGR->AddGroup(pNewUniqueGroup[LastGridID]);
			}
*/
		CMD_CS("#UNIQUEADD")
				
/*			pRegenPrototype = new CRegenPrototype;
			BYTE OBJECTKIND			= pRegenPrototype->ObjectKind		= file.GetByte();
			file.GetDword();
			DWORD OBJECTID			= pRegenPrototype->dwObjectID		= GeneraterMonsterID();//pRegenPrototype->dwObjectID		= file.GetDword();
			WORD MONSTERKIND		= pRegenPrototype->wMonsterKind		= file.GetWord();
			float POSX				= pRegenPrototype->vPos.x			= file.GetFloat();
			float POSZ				= pRegenPrototype->vPos.z			= file.GetFloat();
			BOOL ISREGEN			= file.GetBool();
			
			BASE_MONSTER_LIST* pMonsterInfo = GAMERESRCMNGR->GetMonsterListInfo(MONSTERKIND);
			WORD HELPTYPE			= pRegenPrototype->InitHelpType		= pMonsterInfo->InitHelpType;
			BOOL ISHEARING			= pRegenPrototype->bHearing			= pMonsterInfo->bHearing;
			DWORD HEARINGDISTANCE	= pRegenPrototype->HearingDistance	= pMonsterInfo->HearingDistance;
			
			REGENMGR->AddPrototype(pRegenPrototype);
			
			pNewRegenObject[0] = new CRegenObject;
			pNewRegenObject[0]->InitPrototype(pRegenPrototype);
			pNewRegenObject[0]->m_dwObjectID = GeneraterMonsterID();
			pNewRegenObject[0]->m_dwSubObjectID = OBJECTID;
			pNewRegenObject[0]->m_dwGridID = RandomGridID;
			pNewRegenObject[0]->m_dwGroupID = pNewUniqueGroup->m_dwGroupID;
			pNewRegenObject[0]->InitHelpType();
			pNewUniqueGroup->AddRegenObject(pNewRegenObject[0]);
				
			if(ISREGEN)
			{
				pNewUniqueGroup->Die(pNewRegenObject[0]->m_dwObjectID);
			}
			pRegenPrototype = NULL;
			*/

			pRegenPrototype = new CRegenPrototype;
			BYTE OBJECTKIND			= pRegenPrototype->ObjectKind		= file.GetByte();
			file.GetDword();
			DWORD OBJECTID			= pRegenPrototype->dwObjectID		= GeneraterMonsterID();//pRegenPrototype->dwObjectID		= file.GetDword();
			WORD MONSTERKIND		= pRegenPrototype->wMonsterKind		= file.GetWord();

			float POSX				= pRegenPrototype->vPos.x			= file.GetFloat();
			float POSZ				= pRegenPrototype->vPos.z			= file.GetFloat();

			// �ʵ庸���� ���
			if( OBJECTKIND == eObjectKind_FieldBossMonster )
			{
				// �ʵ庸�� �Ŵ����� ���� ��ġ�� 1�� �̻� ��ϵǾ� �ִٸ�
				if( FIELDBOSSMONMGR->GetRegenPositionCount() > 0 )
				{
					// ��ϵ� ���� ��ġ���� �������� �����Ѵ�
					VECTOR3 pos = FIELDBOSSMONMGR->GetRegenPosition();
					pRegenPrototype->vPos.x = pos.x;
					pRegenPrototype->vPos.z = pos.z;
				}
			}

			BOOL ISREGEN			= file.GetBool();
			
			BASE_MONSTER_LIST* pMonsterInfo = GAMERESRCMNGR->GetMonsterListInfo(MONSTERKIND);
			WORD HELPTYPE			= pRegenPrototype->InitHelpType		= pMonsterInfo->InitHelpType;
			BOOL ISHEARING			= pRegenPrototype->bHearing			= pMonsterInfo->bHearing;
			DWORD HEARINGDISTANCE	= pRegenPrototype->HearingDistance	= pMonsterInfo->HearingDistance;
			
			REGENMGR->AddPrototype(pRegenPrototype);
			
			pNewRegenObject[RandomGridID] = new CRegenObject;
			pNewRegenObject[RandomGridID]->InitPrototype(pRegenPrototype);
			pNewRegenObject[RandomGridID]->m_dwObjectID = GeneraterMonsterID();
			pNewRegenObject[RandomGridID]->m_dwSubObjectID = OBJECTID;
			pNewRegenObject[RandomGridID]->m_dwGridID = RandomGridID;
			pNewRegenObject[RandomGridID]->m_dwGroupID = pNewUniqueGroup[RandomGridID]->m_dwGroupID;
			pNewRegenObject[RandomGridID]->InitHelpType();
			pNewUniqueGroup[RandomGridID]->AddRegenObject(pNewRegenObject[RandomGridID]);
			if(ISREGEN)
			{
				pNewUniqueGroup[RandomGridID]->Die(pNewRegenObject[RandomGridID]->m_dwObjectID);
			}
/*
			if( g_pServerSystem->GetNation() == eNATION_KOREA )
			if( ChannelCount > 1 )
			{
				pNewRegenObject[LastGridID] = new CRegenObject;
				pNewRegenObject[LastGridID]->InitPrototype(pRegenPrototype);
				pNewRegenObject[LastGridID]->m_dwObjectID = GeneraterMonsterID();
				pNewRegenObject[LastGridID]->m_dwSubObjectID = OBJECTID;
				pNewRegenObject[LastGridID]->m_dwGridID = LastGridID;
				pNewRegenObject[LastGridID]->m_dwGroupID = pNewUniqueGroup[LastGridID]->m_dwGroupID;
				pNewRegenObject[LastGridID]->InitHelpType();
				pNewUniqueGroup[LastGridID]->AddRegenObject(pNewRegenObject[LastGridID]);
				if(ISREGEN)
				{
					pNewUniqueGroup[LastGridID]->Die(pNewRegenObject[LastGridID]->m_dwObjectID);
				}
			}
*/
			pRegenPrototype = NULL;
	
		// 
		// �ʵ庸�� - 05.12 �̿���
		// �ʵ庸�� ���� ��ġ �߰� ��
		CMD_CS("#FILEDBOSSREGENPOSITION")
			float x = file.GetFloat();
			float z = file.GetFloat();
			FIELDBOSSMONMGR->AddRegenPosition(x, z);
		
		CMD_CS("#UNIQUEREGENDELAY")
			DWORD delayTime = file.GetDword() * 60000;
			CAIUniqueGroup * pNewGroup = (CAIUniqueGroup*)GROUPMGR->GetGroup( GROUPID, RandomGridID );
			if(pNewGroup)
			{
				pNewGroup->GetRegenInfo()->SetRegenDelayTime(delayTime);
			}

		CMD_CS("#UNIQUERANDOMREGENDELAY2")
			
			DWORD minTime = file.GetDword();
			DWORD maxTime = file.GetDword();
			DWORD delayTime = random(minTime, maxTime) * 60000;
			CAIUniqueGroup * pNewGroup = (CAIUniqueGroup*)GROUPMGR->GetGroup( GROUPID, RandomGridID );
			if(pNewGroup)
			{
				pNewGroup->GetRegenInfo()->SetRegenDelayTime(delayTime);
			}

		CMD_CS("#RANDOMREGENDELAY")
			DWORD minTime = file.GetDword();
			DWORD maxTime = file.GetDword();

			for(int i = 0 ; i < ChannelCount ; ++i)
			{
				DWORD GridID = CHANNELSYSTEM->GetChannelID(i);

				DWORD delayTime = random(minTime, maxTime) * 60000;

				CAIGroup * pNewGroup = GROUPMGR->GetGroup( GROUPID, GridID );
				if(pNewGroup)
				{
					pNewGroup->GetRegenInfo()->SetRegenDelayTime(delayTime);
				}
			}
		CMD_CS("#UNIQUERANDOMREGENDELAY")
			/*
			DWORD minTime = file.GetDword();
			DWORD maxTime = file.GetDword();
			DWORD delayTime = random(minTime, maxTime);
			CAIUniqueGroup * pNewGroup = (CAIUniqueGroup*)GROUPMGR->GetGroup( GROUPID, RandomGridID );
			if(pNewGroup)
			{
				pNewGroup->GetRegenInfo()->SetRegenDelayTime(delayTime);
			}*/

			DWORD minTime = file.GetDword();
			DWORD maxTime = file.GetDword();
			DWORD delayTime = random(0, minTime) * 60000;
			CAIUniqueGroup * pNewGroup = (CAIUniqueGroup*)GROUPMGR->GetGroup( GROUPID, RandomGridID );
			if(pNewGroup)
			{
				pNewGroup->GetRegenInfo()->SetRegenDelayTime(delayTime);
			}
/*
			if( g_pServerSystem->GetNation() == eNATION_KOREA )
			if( ChannelCount > 1 )
			{
				pNewGroup = (CAIUniqueGroup*)GROUPMGR->GetGroup( GROUPID, LastGridID );
				if( pNewGroup )
				{
					pNewGroup->GetRegenInfo()->SetRegenDelayTime(delayTime);
				}
			}
*/			
		CMD_CS("#UNIQUEADDCONDITION")
/*			TARGETGROUPID			= file.GetDword();
			float RATIO					= file.GetFloat();
			DWORD REGENDELAY			= file.GetDword();
			BOOL bRegen					= file.GetBool();
			CRegenConditionInfo * pCondInfo = new CRegenConditionInfo;
			pCondInfo->dwStartRegenTick		= 0;
			pCondInfo->dwTargetGroupID		= TARGETGROUPID;
			pCondInfo->fRemainderRatio		= RATIO;
			pCondInfo->dwRegenDelay			= REGENDELAY;
			pCondInfo->bRegen				= bRegen;
			ASSERT(GROUPMGR->GetGroup(TARGETGROUPID,RandomGridID));
			pNewUniqueGroup->AddConditionInfo(pCondInfo);
*/			
			TARGETGROUPID			= file.GetDword();
			float RATIO					= file.GetFloat();
			DWORD REGENDELAY			= file.GetDword();
			BOOL bRegen					= file.GetBool();
			CRegenConditionInfo * pCondInfo = new CRegenConditionInfo;
			pCondInfo->dwStartRegenTick		= 0;
			pCondInfo->dwTargetGroupID		= TARGETGROUPID;
			pCondInfo->fRemainderRatio		= RATIO;
			pCondInfo->dwRegenDelay			= REGENDELAY;
			pCondInfo->bRegen				= bRegen;
			ASSERT(GROUPMGR->GetGroup(TARGETGROUPID,RandomGridID));
			pNewUniqueGroup[RandomGridID]->AddConditionInfo(pCondInfo);
/*
			if( g_pServerSystem->GetNation() == eNATION_KOREA )
			if( ChannelCount > 1 )
			{
				CRegenConditionInfo* pCondInfo1 = new CRegenConditionInfo;
				pCondInfo1->dwStartRegenTick = 0;
				pCondInfo1->dwTargetGroupID = TARGETGROUPID;
				pCondInfo1->fRemainderRatio	= RATIO;
				pCondInfo1->dwRegenDelay = REGENDELAY;
				pCondInfo1->bRegen = bRegen;
				ASSERT(GROUPMGR->GetGroup(TARGETGROUPID,LastGridID));
				pNewUniqueGroup[LastGridID]->AddConditionInfo(pCondInfo1);
			}
*/
		CMD_EN
	}
	file.Release();
}
