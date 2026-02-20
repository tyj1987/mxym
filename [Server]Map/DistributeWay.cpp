// DistributeWay.cpp: implementation of the CDistributeWay class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerSystem.h"
#include "DistributeWay.h"
#include "Party.h"
#include "ItemDrop.h"
#include "ItemManager.h"
#include "GuildManager.h"
#include "..\[CC]Header\GameResourceManager.h"
#include "MapItemDrop.h"	// �� ������ ��� �߰� by Stiner(2008/05/30)-MapItemDrop
#include "FortWarManager.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDistributeWay::CDistributeWay()
{

}

CDistributeWay::~CDistributeWay()
{

}

void CDistributeWay::SendToPersonalExp(CPlayer* pReceivePlayer, DWORD Exp)
{
	// ���� �߰� - RaMa 04.10.18
	// Exp *= gExpRate;
	// ���� - RaMa 05.05.11	
	Exp = (DWORD)(Exp*gEventRate[eEvent_ExpRate]);
	
	//060719 ��������Ʈ
	DWORD AddExpFromGuildPlustime = 0;
	if( pReceivePlayer->GetGuildIdx() )
	{
		AddExpFromGuildPlustime = Exp;
		if( FALSE == GUILDMGR->GetGuildPlustimeRt( pReceivePlayer->GetGuildIdx(), eGPT_Exp, &AddExpFromGuildPlustime ) )
		{
			AddExpFromGuildPlustime = 0;
		}
	}

	// RaMa - 04.11.24   -> ShopItemOption �߰�
	if( pReceivePlayer->GetShopItemStats()->AddExp )
	{
//		Exp *= (pReceivePlayer->GetShopItemStats()->AddExp*0.010f+1);
		DWORD temp = Exp * pReceivePlayer->GetShopItemStats()->AddExp;
		Exp = Exp + temp / 100 + ( temp > temp / 100 * 100 ? 1 : 0 );//�ø�
	}
	if( pReceivePlayer->GetShopItemStats()->PlustimeExp )
	{
		float tmp = (100+pReceivePlayer->GetShopItemStats()->PlustimeExp)*0.01f;
		Exp = (DWORD)(Exp*tmp);
	}

	Exp += AddExpFromGuildPlustime;

	pReceivePlayer->AddPlayerExpPoint(Exp);
}

void CDistributeWay::SendToPersonalAbil(CPlayer* pReceivePlayer, LEVELTYPE MonsterLevel)
{
	if(pReceivePlayer->GetState() == eObjectState_Die)
		return;
	DWORD AbilityExp;
	AbilityExp = CalcObtainAbilityExp(pReceivePlayer->GetMaxLevel(),MonsterLevel);

//	AbilityExp = CalcObtainAbilityExp(pReceivePlayer->GetLevel(),MonsterLevel);
	if(AbilityExp)
	{
		// �������� ���
		if( pReceivePlayer->GetStage() != eStage_Normal )
			AbilityExp += 10;

		// Ư��ġ �߰�. - RaMa 04.10.18
		// AbilityExp *= gAbilRate;
		AbilityExp = (DWORD)(AbilityExp*gEventRate[eEvent_AbilRate]);

		//SW060719 ��������Ʈ
		DWORD AddAbilExpFromGuildPlustime = 0;
		if(pReceivePlayer->GetGuildIdx())
		{
			AddAbilExpFromGuildPlustime = AbilityExp;
			if( FALSE == GUILDMGR->GetGuildPlustimeRt( pReceivePlayer->GetGuildIdx(), eGPT_SuRyun, &AddAbilExpFromGuildPlustime ) )
			{
				AddAbilExpFromGuildPlustime = 0;
			}
		}

		// ShopItemOption �߰� - RaMa 05.05.19
		if( pReceivePlayer->GetShopItemStats()->AddAbility )
		{
//			AbilityExp *= ((float)(pReceivePlayer->GetShopItemStats()->AddAbility)*0.010f+1.0f);

			DWORD temp = AbilityExp * pReceivePlayer->GetShopItemStats()->AddAbility;
			AbilityExp = AbilityExp + temp / 100 + ( temp > temp / 100 * 100 ? 1 : 0 );//�ø�
		}
		if( pReceivePlayer->GetShopItemStats()->PlustimeAbil )
		{
			float tmp = (100+pReceivePlayer->GetShopItemStats()->PlustimeAbil)*0.01f;
			AbilityExp = (DWORD)(AbilityExp*tmp);
		}

		AbilityExp += AddAbilExpFromGuildPlustime;

		pReceivePlayer->AddAbilityExp(AbilityExp);
	}
}

DWORD CDistributeWay::CalcObtainAbilityExp(LEVELTYPE KillerLevel,LEVELTYPE MonsterLevel)
{
	//���� Ư��ġ ��
	//*. ���� : [������ ����] - [�ڽ� ������] + 5 (0���ϸ� "0")
	//���� ������ �ڽ� �������� 9���� �̻��̸� ���� ������ ĳ���� ����+9�� ���
	if(MonsterLevel+5 < KillerLevel)
		return 0;
	if(KillerLevel + 9 <MonsterLevel)
		MonsterLevel = KillerLevel+ 9;
	return (MonsterLevel - KillerLevel + 5)*10;
}

void CDistributeWay::SendToPersonalMoney(CPlayer* pPlayer, MONEYTYPE Money, WORD MonsterKind)
{
	// ����� �̵����
	DWORD dwMoney = FORTWARMGR->AddProfitMoneyFromMonster( Money );
	if( pPlayer->SetMoney(dwMoney, MONEY_ADDITION, 1, eItemTable_Inventory, eMoneyLog_GetMonster, MonsterKind) != dwMoney )
//	if( pPlayer->SetMoney(Money, MONEY_ADDITION, 1, eItemTable_Inventory, eMoneyLog_GetMonster, MonsterKind) != Money )
	{
		// error msg ������. ���ѷ� �ʰ�
		MSGBASE msg;
		msg.Category = MP_ITEM;
		msg.Protocol = MP_ITEM_MONEY_ERROR;
		msg.dwObjectID = pPlayer->GetID();
		
		pPlayer->SendMsg(&msg, sizeof(msg));
	}
}



void CDistributeWay::ItemChangeAtLv(WORD& DropItemId)
{
	//+�Ӽ��� �پ� �ִٸ� �״�� �ش�.
	if((DropItemId%10) != 0) return;

	//������ �ε��� ���������� �˻��Ѵ�.
	if(DropItemId >= 11000 && DropItemId <= 28759)
	{
		ITEM_INFO* pItemInfo = ITEMMGR->GetItemInfo(DropItemId);

		if(pItemInfo->ItemKind == eEQUIP_ITEM_WEAPON || pItemInfo->ItemKind == eEQUIP_ITEM_DRESS	||
		   pItemInfo->ItemKind == eEQUIP_ITEM_HAT	 || pItemInfo->ItemKind == eEQUIP_ITEM_SHOES)		
		{
			//�����ۿ� Ư���� �Ӽ��� ���� Ȯ���� ����Ѵ�.
			//������ ��� Ư�� �Ӽ��˻縦 ���� �ʴ´�.
			if(pItemInfo->ItemKind != eEQUIP_ITEM_WEAPON)
			{
				
				unsigned int selectRatio = 100000;//Ȯ����

				unsigned char Data[4];
				Data[0]=rand()%256;
				Data[1]=rand()%256;
				Data[2]=rand()%256;
				Data[3]=rand()%256;

				DWORD *dwRandData=(DWORD *)Data;
				DWORD RandRate = ((*dwRandData)%selectRatio);
				DWORD FromPercent = 0;
				DWORD ToPercent = 0;

				int listIdx = GAMERESRCMNGR->CheckAttribItemIndex(DropItemId);

				if(listIdx != -1)
				{
					for( WORD i = 0; i < 5; ++i )
					{
						FromPercent = ToPercent;
						ToPercent += GAMERESRCMNGR->GetItemChangeRateAttribList()[listIdx].attribinfo[i].changeRate;
						if( FromPercent <= RandRate && RandRate < ToPercent )
						{
							WORD idx = GAMERESRCMNGR->GetItemChangeRateAttribList()[listIdx].attribinfo[i].changeIndex;
							if(idx != 0)
							{
								DropItemId = idx; 
								break;
							}
						}
					}
				}

			}

			//+�Ӽ��� ���� �������� ������ Ȯ���� ����Ѵ�.
			if(GAMERESRCMNGR->GetItemChangeRateOfLvList().size() == 0)
				::MessageBox(NULL,"���� �׸��� 0�Դϴ�.","���",MB_OK);

			if(pItemInfo->LimitLevel <= GAMERESRCMNGR->GetItemChangeRateOfLvList().size())
			{
				unsigned int selectRatio = 100000;//Ȯ����
				
				unsigned char Data[4];
				Data[0]=rand()%256;
				Data[1]=rand()%256;
				Data[2]=rand()%256;
				Data[3]=rand()%256;

				DWORD *dwRandData=(DWORD *)Data;
				DWORD RandRate = ((*dwRandData)%selectRatio);
				DWORD FromPercent = 0;
				DWORD ToPercent = 0;

				for( WORD i = 0; i < 10; ++i)
				{
					FromPercent = ToPercent;
					ToPercent += GAMERESRCMNGR->GetItemChangeRateOfLvList()[pItemInfo->LimitLevel-1].Rate[i];
					if( FromPercent <= RandRate && RandRate < ToPercent )
					{
						DropItemId = DropItemId + i;
						break;
					}
				}
			}
		}
	}
}

void CDistributeWay::SendToPersonalItem(CPlayer* pPlayer, WORD DropItemId, DWORD DropItemRatio, MONSTEREX_LIST * pMonInfo, WORD MonsterKind)
{
	if(ChkGetItemLevel(pPlayer->GetLevel(), pMonInfo->Level) == FALSE)
		return;
	if(DropItemId == 0)
	{
		MONEYTYPE money = ITEMDROP_OBJ->MoneyItemNoItemPercentCalculator(pMonInfo, pPlayer, MonsterKind);
		// �� ������ ��� ��� by Stiner(2008/05/30)-MapItemDrop
		MAPITEMDROP_OBJ->CalculateDropRate( pPlayer, MonsterKind );

		if(0 == money) return;
		SendToPersonalMoney(pPlayer, money, MonsterKind);
		
		//SW060515 ��ɷ��߰� -�ι�ȹ��
		BOOL bGetTwice = FALSE;
		pPlayer->GetPetManager()->GetPetBuffResultRt(ePB_Item_DoubleChance, &bGetTwice);
		if(bGetTwice)
		{
			SendToPersonalMoney(pPlayer, money, MonsterKind);
		}
	}
	else
	{
		//�����Լ� --; ��� �������
		if( DropItemRatio )
		{
		if( ( rand()%100 ) % ( 100 / DropItemRatio ) == 0 )
			//����Ʈ���� Ȯ���� ���� �������� �������� ������� �ε����� ��ü�Ѵ�.
			//ItemChangeAtLv(DropItemId);

			

			ITEMMGR->MonsterObtainItem(pPlayer,DropItemId,MonsterKind);

			// �� ������ ��� ��� by Stiner(2008/05/30)-MapItemDrop
			MAPITEMDROP_OBJ->CalculateDropRate( pPlayer, MonsterKind );
		}
	}
}

BOOL CDistributeWay::ChkGetItemLevel(LEVELTYPE PlayerLvl, LEVELTYPE MonsterLvl)
{
#ifdef _JAPAN_LOCAL_
	return TRUE;
#else
	//if(g_pServerSystem->GetMapNum() > TITANMAP_START_INDEX)
	if( g_pServerSystem->GetMap()->IsMapKind(eTitanMap) )
	{
		return TRUE;
	}
	else
	{
		if(PlayerLvl > MonsterLvl + 6)
			return FALSE;
		else
			return TRUE;
	}
#endif
}

void CDistributeWay::CalcAbilandSend( LEVELTYPE MonsterLevel, PARTY_RECEIVE_MEMBER* pMemberInfo, LEVELTYPE MaxLevel)
{
	DWORD tempoint = 0;
	DWORD AbilPoint = 0;
	DWORD CalAbilPoint = 0;
	BYTE bStage = 0;	
	int nMemberNum = pMemberInfo->count;
	if( nMemberNum == 0 )	return;
	
	if( MonsterLevel+5 < MaxLevel )	return;

	if( MaxLevel+9 >= MonsterLevel )
	{
		tempoint = (MonsterLevel - MaxLevel + 5)*10;
	}
	else if( MaxLevel+10 == MonsterLevel )
	{
		if( nMemberNum < 3 )
			tempoint = 140;
		else
			tempoint = 150;
	}
	else if( MaxLevel+11 == MonsterLevel )
	{
		if( nMemberNum < 3 )
			tempoint = 140;
		else if( nMemberNum < 5 )
			tempoint = 150;
		else
			tempoint = 160;
	}
	else if( MaxLevel+12 <= MonsterLevel )
	{
		if( nMemberNum < 3 )
			tempoint = 140;
		else if( nMemberNum < 5 )
			tempoint = 150;
		else if( nMemberNum < 7 )
			tempoint = 160;
		else
			tempoint = 170;
	}

	AbilPoint = (DWORD)(( tempoint * (0.9 + (0.1*nMemberNum)) ) / nMemberNum);
	
	if( AbilPoint == 0 ) return;

	CalAbilPoint = AbilPoint;
	
	//SW060719 ��������Ʈ
	DWORD AddAbilExpFromGuildPlustime = 0;
	CPlayer* pPlayer = NULL;

	for(int i=0; i<nMemberNum; ++i)
	{
		bStage = ((CPlayer*)pMemberInfo->pPlayer[i])->GetStage();
		if( bStage != eStage_Normal )
			CalAbilPoint += 10;

		// Ư��ġ �߰�. - RaMa 04.10.18
		CalAbilPoint = (DWORD)(CalAbilPoint*gEventRate[eEvent_AbilRate]);
		
		pPlayer = ((CPlayer*)pMemberInfo->pPlayer[i]);
		if(pPlayer->GetGuildIdx())
		{
			AddAbilExpFromGuildPlustime = CalAbilPoint;
			if( FALSE == GUILDMGR->GetGuildPlustimeRt( pPlayer->GetGuildIdx(), eGPT_SuRyun, &AddAbilExpFromGuildPlustime ) )
			{
				AddAbilExpFromGuildPlustime = 0;
			}
		}
		// ShopItemOption �߰� - RaMa 05.05.19
		if( ((CPlayer*)pMemberInfo->pPlayer[i])->GetShopItemStats()->AddAbility )
		{
			DWORD temp = CalAbilPoint * ((CPlayer*)pMemberInfo->pPlayer[i])->GetShopItemStats()->AddAbility;
			CalAbilPoint = CalAbilPoint + temp / 100 + ( temp > temp / 100 * 100 ? 1 : 0 ); //�ø�
		}
		if( ((CPlayer*)pMemberInfo->pPlayer[i])->GetShopItemStats()->PlustimeAbil )
		{
			float tmp = (100+((CPlayer*)pMemberInfo->pPlayer[i])->GetShopItemStats()->PlustimeAbil)*0.01f;
			CalAbilPoint = (DWORD)(CalAbilPoint*tmp);
		}
		
		CalAbilPoint += AddAbilExpFromGuildPlustime;

		((CPlayer*)pMemberInfo->pPlayer[i])->AddAbilityExp(CalAbilPoint);
		CalAbilPoint = AbilPoint;
	}
}
/*
void CDistributeWay::CalcAbilandSend(LEVELTYPE MonsterLevel, PARTY_RECEIVE_MEMBER* pMemberInfo)
{
	DWORD tempoint = 0;
	DWORD AbilPoint = 0;
	LEVELTYPE playerlvl =0;
	float levelavg = 0;
	LEVELTYPE maxlevel = 0;  
	LEVELTYPE curlevel = 0;
	LEVELTYPE curmonlvl = MonsterLevel;
	BYTE bStage = 0;	
	
	for(int i=0; i<pMemberInfo->count; ++i)
	{
		curlevel = ((CPlayer*)pMemberInfo->pPlayer[i])->GetMaxLevel();
		if(maxlevel < curlevel)
			maxlevel = curlevel;
		levelavg += curlevel;			
	}
	levelavg /= (float)pMemberInfo->count;

	for(i=0; i<pMemberInfo->count; ++i)
	{
		if(((CPlayer*)pMemberInfo->pPlayer[i]) == 0)
			return;
		playerlvl = ((CPlayer*)pMemberInfo->pPlayer[i])->GetMaxLevel();
		if(MonsterLevel+5 < playerlvl)
			continue;
		if(playerlvl + 9 < MonsterLevel)
			curmonlvl = playerlvl+ 9;
		else
			curmonlvl = MonsterLevel;
		tempoint = curmonlvl - playerlvl + 5;

		if(pMemberInfo->count != 1)
		{
			AbilPoint = tempoint * ( playerlvl * (10 + pMemberInfo->count) / 9.f / (float)levelavg )
				/ (float)pMemberInfo->count;
		}
		else
			AbilPoint = tempoint;
		if(tempoint == 0)
			AbilPoint = 1;

		bStage = ((CPlayer*)pMemberInfo->pPlayer[i])->GetStage();
		if( bStage != eStage_Normal )
			AbilPoint += 1;

		// Ư��ġ �߰�. - RaMa 04.10.18
		// AbilPoint *= gAbilRate;
		AbilPoint *= gEventRate[eEvent_AbilRate];
		
		// ShopItemOption �߰� - RaMa 05.05.19
		if( ((CPlayer*)pMemberInfo->pPlayer[i])->GetShopItemStats()->AddAbility )
		{
//			AbilPoint *= (((CPlayer*)pMemberInfo->pPlayer[i])->GetShopItemStats()->AddAbility*0.01f+1);
			DWORD temp = AbilPoint * ((CPlayer*)pMemberInfo->pPlayer[i])->GetShopItemStats()->AddAbility;
			AbilPoint = AbilPoint + temp / 100 + ( temp > temp / 100 * 100 ? 1 : 0 );//�ø�
		}
	
		((CPlayer*)pMemberInfo->pPlayer[i])->AddAbilityExp(AbilPoint);
	}
}
*/

//060822 - 3�� �̻��� ������ �̾Ƴ��� ���� ���� Ŭ���� by wonju
//---------------------------------------------------------------------------------

DWORD cRand_Big::RandomVal()
{
	DWORD randValue;

	randValue = ((DWORD(rand() << 16)) + DWORD(rand()));
	
	return randValue;
}

//--------------------------------------------------------------------------------
