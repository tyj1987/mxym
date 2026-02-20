// ItemDrop.cpp: implementation of the CItemDrop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerSystem.h"
#include "ItemDrop.h"
#include "Player.h"
#include "time.h"
#include "ItemManager.h"
#include "Monster.h"
#include "..\[CC]Header\GameResourceManager.h"
#include "MHFile.h"
#include "PartyManager.h"
#include "Distribute_Random.h"
#include "Party.h"
#include "ChannelSystem.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemDrop::CItemDrop()
{
	m_MonsterItemDropTable.Initialize(500);
}

CItemDrop::~CItemDrop()
{
	ClearMonsterDropItemList();
}

void CItemDrop::Init()
{
	srand((unsigned)time(NULL));
	LoadMonsterDropItemList();
}

/*//KES �̰� ����..? �ּ�ó����!
void CItemDrop::ReloadDropItem(MONSTEREX_LIST * pMonsterExList)
{
	for(int n=0;n<eDROPITEMKIND_MAX;++n)
	{
		ASSERT(pMonsterExList->CurDropRate[n] == 0);
		pMonsterExList->CurDropRate[n] = pMonsterExList->DropRate[n];
	}
}
*/

int CItemDrop::DropItemKind(MONSTEREX_LIST * pMonsterExList, CPlayer* pPlayer)
{
	DWORD CurTotalRate = 0;
	DWORD DropRate[eDROPITEMKIND_MAX];

	for(int i=0; i<eDROPITEMKIND_MAX; i++)
	{
		DropRate[i] = pMonsterExList->CurDropRate[i];

		if(i == eMONEY)
		{
			//DropRate[eMONEY] *= gMoneyRate;
			DropRate[eMONEY] = (DWORD)(DropRate[eMONEY]*gEventRate[eEvent_MoneyRate]);
		}
		else if(i >= eITEM1 && i <= eITEM3)
		{
			// DropRate[i] *= gItemRate;
			DropRate[i] = (DWORD)(DropRate[i]*gEventRate[eEvent_ItemRate]);

			// 
			if( pPlayer->GetPartyIdx() )
			{
				CParty* pParty = PARTYMGR->GetParty( pPlayer->GetPartyIdx() );
				if( pParty )
				{
					int count = pParty->GetMemberCountofMap( pPlayer->GetID() );
					if( count && gPartyEvent[ePartyEvent_ItemRate].Rate[count-1] )
						DropRate[i] = (DWORD)(DropRate[i]*gPartyEvent[ePartyEvent_ItemRate].Rate[count-1]);
				}
			}
		}

		CurTotalRate += DropRate[i];	
	}
	int i;
	// RaMa - 06.02.08 ->��ɲ��Ǻ���
	if( pPlayer->GetShopItemStats()->AddItemDrop )
	{
		for(i=eITEM1; i<=eITEM3; ++i)
		{
			CurTotalRate -= DropRate[i];
			//2007. 11. 20. CBH - ��ɲۺ��� ���� ���� ����
			//(1+(pPlayer->GetShopItemStats()->AddItemDrop*0.01f)) �� ���� 1.999999�� �����µ� DWORD��
			//��ȯ�ϸ鼭 �� �Ҽ����� ������. �ذ�å���� 0.001�� �����༭ 2���� ������ �ؼ� 2�谡 �ǰ� �Ѵ�.
			DropRate[i] *= (DWORD)(1+(pPlayer->GetShopItemStats()->AddItemDrop*0.01f) + 0.001f);
			CurTotalRate += DropRate[i];
		}
	}

	//SW080515 ä�κ� ����
	float fDropRate = CHANNELSYSTEM->GetDropRate( pPlayer->GetChannelID());
	if( fDropRate != 1.0f )
	{
		for(i=eITEM1; i<= eITEM3; ++i)
		{
			CurTotalRate -= DropRate[i];
			DropRate[i] = DWORD(fDropRate * DropRate[i]);
			CurTotalRate += DropRate[i];
		}
	}

	// RaMa -04.11.24  ->���������
	if( CurTotalRate > MAX_DROP_ITEM_PERCENT )
	{
		int dwTemp = DropRate[eMONEY]+DropRate[eITEM1]+DropRate[eITEM2]+DropRate[eITEM3];

		if( dwTemp < MAX_DROP_ITEM_PERCENT )
		{
			DropRate[eNOITEM] = MAX_DROP_ITEM_PERCENT - dwTemp;
			CurTotalRate = MAX_DROP_ITEM_PERCENT;
		}
		else
		{
			DropRate[eNOITEM] = 0;
			CurTotalRate = dwTemp;
		}
	}

	//CurTotalRate == 0�̸� �ȵȴ�.
	if( CurTotalRate == 0 )
	{
//		char buf[128];
//		wsprintf( buf, "MonsterIdx:%d", pMonsterExList->MonsterKind );
//		ASSERTMSG(0, buf);
		ASSERT(0);
		return -1;
	}

	DWORD RandRate = random(1, CurTotalRate);
	DWORD CurMinRate=0,CurMaxRate=0;

	for( i=0; i<eDROPITEMKIND_MAX; i++)
	{
		CurMinRate = CurMaxRate;
		CurMaxRate += DropRate[i];

		if(RandRate > CurMinRate && RandRate <= CurMaxRate)
		{
			return i;
		}
	}
	ASSERT(0);

	char buf[128];
	wsprintf( buf, "MonsterIdx:%d, CurTotalRate:%d, RandRate:%d, CurMinRate:%d, CurMaxRate:%d,",
		pMonsterExList->MonsterKind, CurTotalRate, RandRate, CurMinRate, CurMaxRate );
	ASSERTMSG(0, buf);

	return -1;

}
MONEYTYPE CItemDrop::MoneyItemNoItemPercentCalculator(MONSTEREX_LIST * pMonInfo, CPlayer* pPlayer, WORD MonsterKind)
{

//	ASSERT(pMob->GetObjectKind() & eObjectKind_Monster );
	
	// RaMa - 04.11.24   ->ShopItemOption�� ItemDrop���� �߰�

	MONEYTYPE TotalMoney = 0; 

	for( int i = 0 ; i < pMonInfo->MaxDropItemNum ; ++i )
	{
		int eDropItemKind = DropItemKind(pMonInfo, pPlayer);

		switch(eDropItemKind)
		{
		case eNOITEM:
			{

			}
			break;
		case eMONEY:
			{
				//if( g_pServerSystem->GetMapNum() > TITANMAP_START_INDEX )
				if( g_pServerSystem->GetMap()->IsMapKind(eTitanMap) )
				{
					MONSTERDROP_TIMAP* pMonsterDropInfo = GAMERESRCMNGR->GetTitanMapMonsterDropInfo( pMonInfo->Level );
					if( pMonsterDropInfo )
						TotalMoney = random( pMonsterDropInfo->stDropInfo[0].wMoneyMin, pMonsterDropInfo->stDropInfo[0].wMoneyMax );
				}
				else
					TotalMoney += MonsterDropMoney(pMonInfo);
			}
			break;
		case eITEM1:
			{
				MonsterDropItem(MonsterKind, pMonInfo->ItemListNum1, pPlayer);
			}
			break;
		case eITEM2:
			{
				MonsterDropItem(MonsterKind, pMonInfo->ItemListNum2, pPlayer);
			}
			break;
		case eITEM3:
			{
				MonsterDropItem(MonsterKind, pMonInfo->ItemListNum3, pPlayer);
			}
			break;
		default:
			{
				ASSERT(0);
			}
		}
	}

//	return 0;
	return TotalMoney;
}

MONEYTYPE CItemDrop::MonsterDropMoney(MONSTEREX_LIST * pMonsterExList)
{
	MONEYTYPE MinMoney = pMonsterExList->MinDropMoney;
	MONEYTYPE MaxMoney = pMonsterExList->MaxDropMoney;
//	ASSERT(MaxMoney >= MinMoney);

	MONEYTYPE ResultMoney = 0;
/*
	if(MinMoney == MaxMoney)
	{
		ResultMoney = MinMoney;
	}
	else
	{
		ResultMoney = (rand() % (MaxMoney - MinMoney)) + MinMoney;
	}
*/	
	if( MaxMoney > MinMoney )
	{
		ResultMoney = random( MinMoney, MaxMoney );
	}
	else
	{
		ResultMoney = MinMoney;
	}

	// RaMa -04.11.24   ->�� ��� �׼�
	// ResultMoney *= gGetMoney;
	ResultMoney = (MONEYTYPE)(ResultMoney*gEventRate[eEvent_GetMoney]);

	return ResultMoney;
}

void CItemDrop::MonsterDropItem(WORD MonsterKind, WORD wDropItemIdx, CPlayer* pPlayer)
{
	MONSTER_DROP_ITEM * pDropItem = GetMonsterItemDrop(wDropItemIdx);
	ASSERT(pDropItem);
	if(pDropItem == NULL)
		return;
	
	if( pDropItem->dwCurTotalRate == 0 )
	{
		// yh �������� ������� �ʴ� ���(TotalRate �� 0)�� FALSE ����
		if(ReloadMonsterDropItem(pDropItem) == FALSE)
			return;
	}

	DWORD RandRate = random( 1, pDropItem->dwCurTotalRate );
	
	DWORD FromPercent	= 0;
	DWORD ToPercent		= 0;

	for(WORD i = 0 ; i < pDropItem->wTotalNum ; ++i)
	{
		FromPercent = ToPercent;
		ToPercent += pDropItem->dwCurMonsterDropItemPercent[i];
//		if(FromPercent <= RandRate && RandRate < ToPercent)
		if(FromPercent < RandRate && RandRate <= ToPercent)
		{
//			ASSERT(pDropItem->CurMonsterDropItemPercent[i]!=0);
//			ASSERT(pDropItem->wCurTotalRate!=0);
			--pDropItem->dwCurMonsterDropItemPercent[i];
			--pDropItem->dwCurTotalRate;


			if(pDropItem->MonsterDropItemPercent[i].wItemIdx)
			{
				WORD DropItemId = pDropItem->MonsterDropItemPercent[i].wItemIdx;

					DISTRIBUTERAND->ItemChangeAtLv(DropItemId);

				
				ITEMMGR->MonsterObtainItem(pPlayer, DropItemId, MonsterKind);
			}
			break;
		}
	}

}


BOOL CItemDrop::ReloadMonsterDropItem(MONSTER_DROP_ITEM* pDropItem)
{
	WORD HaveItem = pDropItem->wTotalNum;

	pDropItem->dwCurTotalRate = 0;
	for(WORD i = 0; i < HaveItem; ++i)
	{
		pDropItem->dwCurMonsterDropItemPercent[i] = pDropItem->MonsterDropItemPercent[i].dwDropPercent;
		pDropItem->dwCurTotalRate += pDropItem->dwCurMonsterDropItemPercent[i];
	}
	if(pDropItem->dwCurTotalRate == 0)
		return FALSE;

	return TRUE;
}


void CItemDrop::ClearMonsterDropItemList()
{
	// reset data
	m_MonsterItemDropTable.SetPositionHead();
	MONSTER_DROP_ITEM* pDropItem;
	while(pDropItem = m_MonsterItemDropTable.GetData())
	{
		delete pDropItem;
	}
	m_MonsterItemDropTable.RemoveAll();
}

BOOL CItemDrop::LoadMonsterDropItemList()
{
	ClearMonsterDropItemList();

	char filename[64];
	CMHFile file;
#ifdef _FILE_BIN_
//	sprintf(filename,"./Resource/MonsterDropItemList.bin");	
	sprintf(filename,"./Resource/Server/MonsterDropItemList.bin");
	if(!file.Init(filename, "rb"))
		return FALSE;
#else
	sprintf(filename,"./Resource/MonsterDropItemList.txt");	
	if(!file.Init(filename, "rt"))
		return FALSE;
#endif

	while(1)
	{
		if(file.IsEOF())
			break;
		
		MONSTER_DROP_ITEM* pDropItem = new MONSTER_DROP_ITEM;
		memset(pDropItem, 0, sizeof(MONSTER_DROP_ITEM));
		pDropItem->wItemDropIndex = file.GetWord();// �б⸸ �Ѵ�.

		WORD totalnum = 0;
		for(int idx=0; idx<MAX_DROPITEM_NUM; ++idx)
		{
#ifdef _DEBUG
			//file.GetString(pDropItem->MonsterDropItemPercent[idx].Name);
			SafeStrCpy( pDropItem->MonsterDropItemPercent[idx].Name, file.GetString(), MAX_ITEMNAME_LENGTH+1 );
			
			pDropItem->MonsterDropItemPercent[idx].wItemIdx = file.GetWord();		
			pDropItem->MonsterDropItemPercent[idx].dwDropPercent = file.GetDword();
			if(pDropItem->MonsterDropItemPercent[idx].dwDropPercent)
//			if(pDropItem->MonsterDropItemPercent[idx].wItemIdx)
				++totalnum;
#else
			char szTempName[256];
			file.GetString(szTempName);
			pDropItem->MonsterDropItemPercent[idx].wItemIdx = file.GetWord();		
			pDropItem->MonsterDropItemPercent[idx].dwDropPercent = file.GetDword();
			if(pDropItem->MonsterDropItemPercent[idx].dwDropPercent)
//			if(pDropItem->MonsterDropItemPercent[idx].wItemIdx)
				++totalnum;
#endif
		}
		
		pDropItem->wTotalNum = totalnum;
		pDropItem->dwCurTotalRate = 0;

		ASSERT(!m_MonsterItemDropTable.GetData(pDropItem->wItemDropIndex));
		m_MonsterItemDropTable.Add(pDropItem,pDropItem->wItemDropIndex);

		ReloadMonsterDropItem(pDropItem);
	}	
	
	file.Release();
	
	return TRUE;
}

MONSTER_DROP_ITEM* CItemDrop::GetMonsterItemDrop(WORD wItemDropIdx)
{
	return m_MonsterItemDropTable.GetData(wItemDropIdx);
}

//////////////////////////////////////////////////////////////////////////
// test
// old code 
/*


void CItemDrop::TestMonsterDropItem(WORD wDropItemListIdx)
{
	int Kind = wDropItemListIdx;
	
	MONSTER_ITEM_DROP* pDropItem = GetMonsterItemDrop(Kind);
	int HaveItem = pDropItem->m_MonsterDropTotalNum;	
	int MaxDropNum = 0;
	int DropNum = 0;
	int DropPercent = 0;
	int CurPercent = 0;
	int TempPercent = 0;
	
	int VarMaxPercent = pDropItem->m_MonsterMaxVarPercent; // �и�
	if(!VarMaxPercent)
		return;

	DropNum = 1;
	for(int i = 0; i < DropNum; i++)
	{
		CurPercent = 0;
		TempPercent = 0;
		DropPercent = rand() % VarMaxPercent; // ����߸� Ȯ��
		if(!DpPercent)
			DropPercent = 1;
		for(int j=0; j<HaveItem; j++)
		{
			CurPercent = pDropItem->m_MonsterDropItemVarPercent[j].DropPercent;
			if(!CurPercent)	// �ۼ�Ʈ�� 0�̸� Zeroüũ�� Reload
			{
				BOOL flag = ZeroCheckPercent(Kind);
				if(flag)
				{
					ReloadDropItemPercent(Kind); // ��� 0�̸� ������
					
					FILE* fp = fopen("itemdroptest.txt","a+");
					fprintf(fp,"ReloadDropItemPercent\n",pDropItem->m_MonsterDropItemVarPercent[j].Name,
						pDropItem->m_MonsterDropItemVarPercent[j].ItemIdx);
					
					
					fclose(fp);
				}
			}
			else if(TempPercent<DropPercent && TempPercent+CurPercent>=DropPercent)
			{
				// Ȯ���� �������Ƿ� ���
				MONSTER_ITEM tmp;
				WORD HuntItemNum = pDropItem->m_MonsterDropItemVarPercent[j].ItemIdx;
				tmp = pDropItem->m_MonsterDropItemVarPercent[j];
				if(HuntItemNum == 0) 
					return;
				ASSERT(HuntItemNum != 52685);
				FILE* fp = fopen("itemdroptest.txt","a+");
				fprintf(fp,"item : %s (%d)\n",pDropItem->m_MonsterDropItemVarPercent[j].Name,
											pDropItem->m_MonsterDropItemVarPercent[j].ItemIdx);
				

				fclose(fp);
				
				CalculItemPercent(Kind, HaveItem, j);
			}
			TempPercent = TempPercent+CurPercent;
		}
		
	}
}
*/
//////////////////////////////////////////////////////////////////////////


/*
void CItemDrop::CalculItemPercent(WORD Kind, int HaveItem, int idx)
{
	MONSTER_ITEM_DROP* pDropItem = GetMonsterItemDrop(Kind);

	int VarPercent = 0;
	int tmpPercent = pDropItem->m_MonsterDropItemVarPercent[idx].DropPercent;
	int VarMaxPercent = pDropItem->m_MonsterMaxVarPercent; // �и�
	if(tmpPercent)
	{
		for(int i = 0; i < HaveItem; i++)
		{
			VarPercent = pDropItem->m_MonsterDropItemVarPercent[i].DropPercent;
			if(!VarPercent)
			{
				pDropItem->m_MonsterDropItemVarPercent[i].DropPercent = VarPercent;
				continue;
			}
			if(!VarMaxPercent)
				ReloadDropItemPercent(Kind); // 0�̸� ������
			if(i == idx)
			{
				pDropItem->m_MonsterDropItemVarPercent[i].DropPercent = VarPercent-1;
				pDropItem->m_MonsterMaxVarPercent = VarMaxPercent-1;
			}
		}
		// ��� �ۼ�Ʈ�� 0���� �˻�
		BOOL flag = ZeroCheckPercent(Kind);
		if(flag)
			ReloadDropItemPercent(Kind); // ��� 0�̸� ������
	}
	else
	{
		// ���⿡ �ɸ��� �������
		BOOL flag = ZeroCheckPercent(Kind);
		if(flag)
			ReloadDropItemPercent(Kind); // ��� 0�̸� ������
	}
}

BOOL CItemDrop::ZeroCheckPercent(WORD Kind)
{
	MONSTER_ITEM_DROP* pDropItem = GetMonsterItemDrop(Kind);

	int tmpPercent[MAX_DROPITEM_NUM];
	int AddPercent = 0;
	for(int i = 0; i < MAX_DROPITEM_NUM; i++)
	{
		tmpPercent[i] = pDropItem->m_MonsterDropItemVarPercent[i].DropPercent;
		AddPercent = AddPercent + tmpPercent[i];
	}

	if(AddPercent) // �ۼ�Ʈ ������ ���� 0�̸� ���� 0��
		return FALSE;  // Reload �϶�� ����
	else
		return TRUE;
}
*/
