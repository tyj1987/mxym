#include "stdafx.h"
#include "ServerSystem.h"
#include "FieldBossMonsterManager.h"
#include "FieldBossMonster.h"
#include "FieldSubMonster.h"
#include "AIGroupManager.h"
#include "AISystem.h"
#include "RegenManager.h"
#include "RegenPrototype.h"
#include "MHFile.h"
#include "..\[cc]BattleSystem\BattleSystem_Server.h"
#include "UserTable.h"
#include "ObjectFactory.h"
#include "AttackManager.h"
#include "PackedData.h"
#include "CharMove.h"
#include "MHError.h"
#include "GridSystem.h"
#include "GridTable.h"
#include "TileManager.h"

GLOBALTON(CFieldBossMonsterManager);

CFieldBossMonsterManager::CFieldBossMonsterManager(void)
{
	m_pFieldGroupPool = new CMemoryPoolTempl<FIELDGROUP>;
	m_pFieldGroupPool->Init( 10, 2, "FieldGroupPool" );
	m_FieldGroupTable.Initialize(10);

	m_pFieldMonsterInfoPool = new CMemoryPoolTempl<FIELDMONSTERINFO>;
	m_pFieldMonsterInfoPool->Init( 40, 10, "FieldMonsterInfoPool" );
	m_FieldMonsterInfoTable.Initialize(40);

	m_pFieldBossDropItemPool = new CMemoryPoolTempl<FIELDBOSSDROPITEMLIST>;
	m_pFieldBossDropItemPool->Init( 20, 10, "FieldBossDropItemPool" );
	m_FieldBossDropItemTable.Initialize(20);
}

CFieldBossMonsterManager::~CFieldBossMonsterManager(void)
{
	m_FieldGroupTable.SetPositionHead();
	m_FieldMonsterInfoTable.SetPositionHead();
	m_FieldBossDropItemTable.SetPositionHead();

	FIELDGROUP* pGroup = NULL;
	
	while( pGroup = m_FieldGroupTable.GetData() )
	{
		m_pFieldGroupPool->Free( pGroup );
	}
	m_FieldGroupTable.RemoveAll();
	SAFE_DELETE( m_pFieldGroupPool );

	FIELDMONSTERINFO* pMonsterInfo = NULL;
	
	while( pMonsterInfo = m_FieldMonsterInfoTable.GetData() )
	{
		m_pFieldMonsterInfoPool->Free( pMonsterInfo );
	}
	m_FieldMonsterInfoTable.RemoveAll();
	SAFE_DELETE( m_pFieldMonsterInfoPool );

	FIELDBOSSDROPITEMLIST* pItemInfo = NULL;
	
	while( pItemInfo = m_FieldBossDropItemTable.GetData() )
	{
		m_pFieldBossDropItemPool->Free( pItemInfo );
	}
	m_FieldBossDropItemTable.RemoveAll();
	SAFE_DELETE( m_pFieldBossDropItemPool );

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ʵ庸�� ���� ���
	//if(m_LogFile)
	//	fclose(m_LogFile);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
}

// �ʵ庸�� ���� ���� �������� �о���δ�
void CFieldBossMonsterManager::Init()
{
	CMHFile file;

	// �ʵ� ���� �� ���� �߰� ���� ����Ʈ
#ifdef _FILE_BIN_
//	if(!file.Init("Resource/FieldBossList.bin","rb"))
	if(!file.Init("Resource/Server/FieldBossList.bin","rb"))
		return ;
#else
	if(!file.Init("Resource/FieldBossList.txt","rt"))
		return ;
#endif

	while(1)
	{
		if(file.IsEOF())
		{
			break;
		}

		FIELDMONSTERINFO* pMonsterInfo = m_pFieldMonsterInfoPool->Alloc();

		pMonsterInfo->m_MonsterKind	= file.GetWord();
		pMonsterInfo->m_ObjectKind	= file.GetWord();
		pMonsterInfo->m_SubKind		= file.GetWord();
		pMonsterInfo->m_SubCount	= file.GetWord();

		pMonsterInfo->m_dwRegenTimeMin	= file.GetDword();
		pMonsterInfo->m_dwRegenTimeMax	= file.GetDword();
		pMonsterInfo->m_dwDistructTime	= file.GetDword() * 60000;

		pMonsterInfo->m_dwRecoverStartTime	= file.GetDword() * 1000;
		pMonsterInfo->m_dwRecoverDelayTime	= file.GetDword() * 1000;
		pMonsterInfo->m_fLifeRate	= file.GetFloat() / 100;
		pMonsterInfo->m_fShieldRate	= file.GetFloat() / 100;

		m_FieldMonsterInfoTable.Add(pMonsterInfo, pMonsterInfo->m_MonsterKind);
	}

	file.Release();
	// �ʵ� ���� ��� ������ ���� ����Ʈ
#ifdef _FILE_BIN_
//	if(!file.Init("Resource/FieldBossDropItemList.bin","rb"))
	if(!file.Init("Resource/Server/FieldBossDropItemList.bin","rb"))
		return ;
#else
	if(!file.Init("Resource/FieldBossDropItemList.txt","rt"))
		return ;
#endif

	while(1)
	{
		if(file.IsEOF())
		{
			break;
		}

		FIELDBOSSDROPITEMLIST* pItemList = m_pFieldBossDropItemPool->Alloc();

		pItemList->m_wMonsterKind = file.GetWord();

		for(WORD group = 0; group < MAX_FIELDBOSS_DROPITEM_GROUP_NUM; group++)
		{
			file.GetWord();
			pItemList->m_ItemList[ group ].m_wCount = file.GetWord();
			pItemList->m_ItemList[ group ].m_wDropRate = file.GetWord();
			pItemList->m_ItemList[ group ].m_wDamageRate = file.GetWord();

			for(WORD count = 0; count < MAX_FIELDBOSS_DROPITEM_NUM; count++)
			{
				pItemList->m_ItemList[ group ].m_wItemIndex[ count ] = file.GetWord();
				pItemList->m_ItemList[ group ].m_wItemCount[ count ] = file.GetWord();
			}
		}

		m_FieldBossDropItemTable.Add(pItemList, pItemList->m_wMonsterKind);
	}

	file.Release();

#ifndef _HK_LOCAL_	//hk block
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ʵ庸�� ���� ���
	SYSTEMTIME ti;
	GetLocalTime( &ti );

	sprintf(m_LogFile,"./Log/FieldBoss_%02d_%02d%02d%02d.txt", g_pServerSystem->GetMapNum(), ti.wYear, ti.wMonth, ti.wDay);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
}

void CFieldBossMonsterManager::Process()
{
	FIELDGROUP* pGroup = NULL;
	
	m_FieldGroupTable.SetPositionHead();

	// ������ ���� �׷� ������ �����´�
	while( pGroup = m_FieldGroupTable.GetData() )
	{
		if(pGroup)
		{
			BOOL bAllDead = TRUE;

			// ������ �ִ°�?
			if( pGroup->m_pBoss )
			{
				bAllDead = FALSE;

				// �������ΰ�?
				if( !pGroup->m_pBoss->IsBattle() )
				{
					// �Ҹ�ð��� �Ǿ��°�?
					if( pGroup->m_pBoss->IsDistruct() )
					{
						// ���ϰ� �����ִٸ� ���� �����Ѵ�
						if( pGroup->m_SubList.GetCount() )
						{
							PTRLISTPOS pos = pGroup->m_SubList.GetHeadPosition();
							CFieldSubMonster* pSub;

							while(pos)
							{
								pSub = (CFieldSubMonster*)pGroup->m_SubList.GetNext(pos);

								if( pSub )
								{
									// �������� �༮�� �н�...
									if( pSub->IsBattle() )
										continue;

									// ���� ����Ʈ���� ����
									pGroup->m_SubList.Remove((void*)pSub);

									// �������� ����
									g_pServerSystem->RemoveMonster( pSub->GetID() );
								}
							}
						}

						
						//g_pServerSystem->RemoveMonster( pGroup->m_pBoss->GetID() );
/*
						// �ʵ庸�� �Ҹ� Ȯ�� �αױ��
						g_Console.LOG(4, "FieldBossMonster Delete : GROUPID: %d, ID: %d, NAME: %s, Time: %d",
							pGroup->m_pBoss->GetMonsterGroupNum(), 
							pGroup->m_pBoss->GetGridID(), 
							pGroup->m_pBoss->GetObjectName(),
							gCurTime / 1000);
*/
						//MHERROR->OutputFile("Debug.txt", MHERROR->GetStringArg("FieldBoss Delete = GridID: %d, gCurTime: %d", pGroup->m_pBoss->GetGridID(), gCurTime));
						//////////////////////////////////////////////////////////////////////////////////////////////////////
						// �ʵ庸�� ���� ���
#ifndef _HK_LOCAL_	//hk block
						VECTOR3 Pos;
						pGroup->m_pBoss->GetPosition(&Pos);
						FILE* file = fopen(m_LogFile, "a+");
						fprintf(file, "%s\t%d\t%u\t%d\t%u\t%f\t%f\t%u\n",
							"�����Ҹ�", pGroup->m_pBoss->GetMonsterKind(), pGroup->m_pBoss->GetID(), pGroup->m_pBoss->GetGridID(), gCurTime,
							Pos.x, Pos.z, pGroup->m_pBoss->GetLife());
						fclose(file);
						//////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

						// ���� ����
						g_pServerSystem->RemoveBossMonster( pGroup->m_pBoss->GetID(), TRUE );
						//BATTLESYSTEM->DeleteObjectFromBattle(pGroup->m_pBoss);
						//g_pUserTable->RemoveUser(pGroup->m_pBoss->GetID());
						//g_pObjectFactory->ReleaseObject(pGroup->m_pBoss);

						pGroup->m_pBoss = NULL;
					}
					else
						pGroup->m_pBoss->Recover();
				}
				
			}
			
			// ���ϰ� �ִ°�?
			if( pGroup->m_SubList.GetCount() )
			{
				bAllDead = FALSE;

				PTRLISTPOS pos = pGroup->m_SubList.GetHeadPosition();
				CFieldSubMonster* pSub;

				while(pos)
				{
					pSub = (CFieldSubMonster*)pGroup->m_SubList.GetNext(pos);

					if( pSub )
					{
						// ������ �ִ°�?
						if( pGroup->m_pBoss )
						{
							// �Ҹ�ð� ����
							pSub->ResetDistructTime();
						}

						// �������ΰ�?
						if( !pSub->IsBattle() )
						{
							// �Ҹ�ð��� �Ǿ��°�?
							if( pSub->IsDistruct() )
							{
								//////////////////////////////////////////////////////////////////////////////////////////////////////
								// �ʵ庸�� ���� ���
								//VECTOR3 Pos;
								//pSub->GetPosition(&Pos);
								//fprintf(m_LogFile, "%s\t%d\t%u\t%d\t%u\t%f\t%f\t%u\n",
								//	"���ϼҸ�", pSub->GetMonsterKind(), pSub->GetID(), pSub->GetGridID(), gCurTime,
								//	Pos.x, Pos.z, pSub->GetLife());
								//////////////////////////////////////////////////////////////////////////////////////////////////////

								// ���� ����Ʈ���� ����
								pGroup->m_SubList.Remove((void*)pSub);

								// �������� ����
								g_pAISystem->ReleaseMonsterID( pSub->GetID() );
								g_pAISystem->ReleaseMonsterID( pSub->GetSubID() );
								g_pServerSystem->RemoveMonster( pSub->GetID() );
							}
							else
								pSub->Recover();
						}
					}
				}
			}

			// ��� �׾��°�?
			if( bAllDead )
			{
				CAIGroup* pAIGroup = GROUPMGR->GetGroup(pGroup->m_GroupID, pGroup->m_GridID);

				if(pAIGroup)
				{
					// ���� �����̸� ������ش�
					DWORD delaytime = pGroup->m_dwRegenTimeMax - pGroup->m_dwRegenTimeMin;

					if( delaytime )
						delaytime = rand() % (delaytime) * 60000;

					delaytime += pGroup->m_dwRegenTimeMin * 60000;
					
					pAIGroup->GetRegenInfo()->SetRegenDelayTime(delaytime);

					/////////////////////////////////////////////////////////////////////////////////
					// 06. 07. �ʵ庸�� ���� - �̿���
					// CRegenManager::RegenObject���� �����ϴ� ���� ��ġ �������� ����
					/*
					// ���� ��ġ ���� ����
					VECTOR3	regen_pos = GetRegenPosition();
					VECTOR3* target_pos = pAIGroup->GetRegenObject(pGroup->m_BossObjectID)->GetPos();
					
					target_pos->x = regen_pos.x;
					target_pos->z = regen_pos.z;
					*/
					/////////////////////////////////////////////////////////////////////////////////

					// ���� ä���� CAIUniqueGroup::RegenCheck() ���� �������� �������ش�

					// ���� ������ ���
					g_pAISystem->RemoveObject(pGroup->m_BossObjectID);
					//pAIGroup->Die(pGroup->m_BossObjectID);
					//pAIGroup->RegenCheck();
/*
//					g_Console.LOG(4, "FieldBossMonster Regen Count Start : GROUPID: %d, ID: %d, CurTime: %d, RegenTime: %d",
//								  pGroup->m_GroupID, 
//								  pGroup->m_BossObjectID,
//								  gCurTime / 1000,
//								  delaytime / 60000);
*/
					//MHERROR->OutputFile("Debug.txt", MHERROR->GetStringArg("FieldBoss Regen Count Start = gCurTime: %d", gCurTime));
				

					m_FieldGroupTable.Remove(pGroup->m_BossObjectID);

					pGroup->m_BossObjectID = 0;
					pGroup->m_dwRegenTimeMax = 0;
					pGroup->m_dwRegenTimeMin = 0;
					pGroup->m_GridID = 0;
					pGroup->m_GroupID = 0;
					pGroup->m_pBoss = NULL;

					m_pFieldGroupPool->Free(pGroup);
				}
				else
				{
					g_pAISystem->RemoveObject(pGroup->m_BossObjectID);

					m_FieldGroupTable.Remove(pGroup->m_BossObjectID);

					pGroup->m_BossObjectID = 0;
					pGroup->m_dwRegenTimeMax = 0;
					pGroup->m_dwRegenTimeMin = 0;
					pGroup->m_GridID = 0;
					pGroup->m_GroupID = 0;
					pGroup->m_pBoss = NULL;

					m_pFieldGroupPool->Free(pGroup);
				}
			}
		}
	}
}

void CFieldBossMonsterManager::AddRegenPosition(float x, float z)
{
	if( m_RegnePosCount >= MAX_REGENPOS_NUM )
		return;
	
	m_RegenPos[ m_RegnePosCount ].x = x;
	m_RegenPos[ m_RegnePosCount ].z = z;

	m_RegnePosCount++;
}

VECTOR3 CFieldBossMonsterManager::GetRegenPosition(int index)
{
	if(index < 0)
	{
		if( m_RegnePosCount != 0)
			index = rand() % m_RegnePosCount;
		else
		{
			VECTOR3 temp = {0.0f, 0.0f, 0.0f};
			return temp;
		}
	}

	return m_RegenPos[ index ];
}

void CFieldBossMonsterManager::BossDead(CFieldBossMonster* pBoss)
{
	FIELDGROUP* pGroup = NULL;
	
	DWORD ID = pBoss->GetID();

	// ������ ���� �׷� ������ �����´�
	pGroup = m_FieldGroupTable.GetData(ID);

	if(pGroup)
	{
		// ���� ������ ��ġ�ϸ� �����Ѵ�
		if( pGroup->m_pBoss == pBoss )
		{
			// �ʵ庸�� ���� Ȯ�� �αױ��
			//g_Console.LOG(4, "FieldBossMonster Dead : GROUPID: %d, ID: %d, NAME: %s, Time: %d",
			//			  pGroup->m_pBoss->GetMonsterGroupNum(), 
			//			  pGroup->m_pBoss->GetGridID(), 
			//			  pGroup->m_pBoss->GetObjectName(),
			//			  gCurTime / 1000);

			//MHERROR->OutputFile("Debug.txt", MHERROR->GetStringArg("FieldBoss Dead = GridID: %d, gCurTime: %d", pGroup->m_pBoss->GetGridID(), gCurTime));
#ifndef _HK_LOCAL_	//hk block
			//////////////////////////////////////////////////////////////////////////////////////////////////////
			// �ʵ庸�� ���� ���
			VECTOR3 Pos;
			pBoss->GetPosition(&Pos);
			FILE* file = fopen(m_LogFile, "a+");
			fprintf(file, "%s\t%d\t%u\t%d\t%u\t%f\t%f\t%u\n",
				"��������", pBoss->GetMonsterKind(), pBoss->GetID(), pBoss->GetGridID(), gCurTime,
				Pos.x, Pos.z, 0);
			fclose(file);
			//////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


			// �׸���� �ʿ��� ����
			CGridTable* pGridTable = g_pServerSystem->GetGridSystem()->GetGridTable(pGroup->m_pBoss);
			pGridTable->RemoveObject(pGroup->m_pBoss);
			g_pServerSystem->GetMap()->GetTileManager()->RemoveObject(pGroup->m_pBoss);

			BATTLESYSTEM->DeleteObjectFromBattle(pGroup->m_pBoss);
//			g_pUserTable->RemoveUser(pGroup->m_pBoss->GetID());
//			g_pObjectFactory->ReleaseObject(pGroup->m_pBoss);
			
			pGroup->m_pBoss = NULL;
		}
	}
}

void CFieldBossMonsterManager::SubDead(CFieldSubMonster* pSub)	
{
	FIELDGROUP* pGroup = NULL;
	
	DWORD ID = pSub->GetBossID();

	pGroup = m_FieldGroupTable.GetData(ID);

	if(pGroup)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// �ʵ庸�� ���� ���
		//VECTOR3 Pos;
		//pSub->GetPosition(&Pos);
		//fprintf(m_LogFile, "%s\t%d\t%u\t%d\t%u\t%f\t%f\t%u\n",
		//	"��������", pSub->GetMonsterKind(), pSub->GetID(), pSub->GetGridID(), gCurTime,
		//	Pos.x, Pos.z, 0);
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		// ���� ������ �����
		pGroup->m_SubList.Remove((void*)pSub);
		g_pAISystem->ReleaseMonsterID( pSub->GetID() );
		g_pAISystem->ReleaseMonsterID( pSub->GetSubID() );
//		g_pServerSystem->RemoveMonster( pSub->GetID() );
	}
}

void CFieldBossMonsterManager::AddFieldBossMonster( CFieldBossMonster* pBoss )
{
#ifndef _HK_LOCAL_	//hk block
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	VECTOR3 Pos;
	pBoss->GetPosition(&Pos);
	FILE* file = fopen(m_LogFile, "a+");
	fprintf(file, "%s\t%d\t%u\t%d\t%u\t%f\t%f\t%u\n",
		"��������", pBoss->GetMonsterKind(), pBoss->GetID(), pBoss->GetGridID(), gCurTime,
		Pos.x, Pos.z, 0);
	fclose(file);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

	// ���� ������ �߰� ������ �����´�
	FIELDMONSTERINFO* pBossInfo = m_FieldMonsterInfoTable.GetData( pBoss->GetMonsterKind() );
	if( pBossInfo == NULL )
	{
		ASSERT(0);
		return;
	}
	FIELDMONSTERINFO* pSubInfo = m_FieldMonsterInfoTable.GetData( pBossInfo->m_SubKind );
	if( pSubInfo == NULL )
	{
		ASSERT(0);
		return;
	}
	
	pBossInfo->m_dwCountTime = pBossInfo->m_dwDistructTime;
	
	// �߰� ���� ����
	pBoss->SetInfo( pBossInfo );	

	// �׷������� �����Ѵ�
	FIELDGROUP* pGroup = m_pFieldGroupPool->Alloc();

	// 061026 RaMa - �޸𸮸� �Ҵ� �� �޾Ҵ�.
	if( !pGroup )
	{
#ifndef _HK_LOCAL_	//hk block
		FILE* file = fopen(m_LogFile, "a+");
		fprintf(file, "%s\t%d\t%u\t%d\t%u\t\n",
			"�׷�޸��Ҵ������", pBoss->GetMonsterKind(), pBoss->GetID(), pBoss->GetGridID(), gCurTime);
		fclose(file);
#endif
		return;
	}

	// �׷� ������ ID�� ���� ������ ID�� ����Ѵ�
	pGroup->m_GroupID = pBoss->GetMonsterGroupNum();
	pGroup->m_GridID = pBoss->GetGridID();
	pGroup->m_pBoss = pBoss;
	pGroup->m_BossObjectID = pBoss->GetID();

	pGroup->m_dwRegenTimeMin = pBossInfo->m_dwRegenTimeMin;
	pGroup->m_dwRegenTimeMax = pBossInfo->m_dwRegenTimeMax;	

	if(pSubInfo)
	{
		pSubInfo->m_dwCountTime = pSubInfo->m_dwDistructTime;

		// ���� ���� �⺻����
		DWORD dwGridID = pBoss->GetGridID();			// ������ ���� ä��
		WORD wObjectKind = eObjectKind_FieldSubMonster;	// �������� ����
		WORD wMonsterKind = pBossInfo->m_SubKind;		// ���͹�ȣ ����
		DWORD dwGroupID = (WORD)RECALL_GROUP_ID;				// �׷�ID ����
		if(pGroup->m_GroupID == (WORD)EVENTRECALL_GROUP_ID)
			dwGroupID = (WORD)EVENTRECALL_GROUP_ID;
		WORD DropItemID = 0;							// ��������� �⺻��
		DWORD dwDropRatio = 100;						// ����� �⺻��
		BOOL bRandomPos = FALSE;						
		BOOL bEventRegen = FALSE;		

		for( WORD count = 0; count < pBossInfo->m_SubCount; count++ )
		{
			CFieldSubMonster* pSub = NULL;

			DWORD dwID = g_pAISystem->GeneraterMonsterID();		// ID ����
			DWORD dwSubID = g_pAISystem->GeneraterMonsterID();	// SubID ����

			// ���� ��ġ ����
			VECTOR3 BossPos;
			VECTOR3 TargetPos;

			pBoss->GetPosition(&BossPos);

			// ���� ��ġ�κ��� +- 10 ���� �������� ����
			TargetPos.x = BossPos.x + ( (rand() % 2) ? -1 : 1 ) * (rand() % 10) * 100;
			TargetPos.z = BossPos.z + ( (rand() % 2) ? -1 : 1 ) * (rand() % 10) * 100;

			// ���� ���� ����
			pSub = (CFieldSubMonster*)REGENMGR->RegenObject( dwID, dwSubID, dwGridID, wObjectKind, wMonsterKind, &TargetPos, dwGroupID, DropItemID, dwDropRatio, bRandomPos, bEventRegen);

			if( pSub )
			{
				//////////////////////////////////////////////////////////////////////////////////////////////////////
				// �ʵ庸�� ���� ���
				//VECTOR3 Pos;
				//pSub->GetPosition(&Pos);
				//fprintf(m_LogFile, "%s\t%d\t%u\t%d\t%u\t%f\t%f\t%u\n",
				//	"���ϸ���", pSub->GetMonsterKind(), pSub->GetID(), pSub->GetGridID(), gCurTime,
				//	Pos.x, Pos.z, 0);
				//////////////////////////////////////////////////////////////////////////////////////////////////////

				// �߰� ���� ����
				pSub->SetInfo( pSubInfo );
				pSub->SetBossID( pGroup->m_BossObjectID );
				//���� ������ �ε��� ����
				pSub->SetRegenPosIndex(pBoss->GetRegenPosIndex());

				// �׷� ������ ���� ���� �߰�
				pGroup->m_SubList.AddTail( pSub );
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////////////////////////////////
				// �ʵ庸�� ���� ���
				//fprintf(m_LogFile, "%s\t%d\t%u\t%d\t%u\t%f\t%f\t%u\n",
				//	"���ϸ�������", wMonsterKind, 0, dwGridID, gCurTime, TargetPos.x, TargetPos.z, 0);
				//////////////////////////////////////////////////////////////////////////////////////////////////////
			}
		}
	}
	m_FieldGroupTable.Add( pGroup, pGroup->m_BossObjectID );
}

FIELDBOSSDROPITEMLIST* CFieldBossMonsterManager::GetFieldBossDropItemList( WORD MonsterKind )
{
	FIELDBOSSDROPITEMLIST* pItemInfo = m_FieldBossDropItemTable.GetData(MonsterKind);

	return pItemInfo;
}
/*
CFieldBossMonster* CFieldBossMonsterManager::GetFieldBossFromChannel(DWORD dwChannelIndex)
{
	DWORD dwIndex = 0;
	FIELDGROUP* pFieldGroup = NULL;

	m_FieldGroupTable.SetPositionHead();

	while (pFieldGroup = m_FieldGroupTable.GetData())
	{
		if( (pFieldGroup->m_pBoss->GetObjectKind() == eObjectKind_FieldBossMonster) && (dwIndex == dwChannelIndex) )
			return pFieldGroup->m_pBoss;

		dwIndex++;
	}
	

	return NULL;
}
*/
