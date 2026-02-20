// ItemManager.cpp: implementation of the CItemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemManager.h"
#include "DealDialog.h"
#include "ExchangeDialog.h"
#include "MixDialog.h"

#include "./Interface/cIcon.h"
#include "./Interface/cImage.h"

#include "GameIn.h"
#include "WindowIDEnum.h"
#include "./Interface/cWindowManager.h"
#include "./Interface/cScriptManager.h"
#include "cDivideBox.h"
#include "ObjectManager.h"
#include "DebugDlg.h"

#include "AppearanceManager.h"
#include "GameDef.h"
#include "QuickManager.h"
#include "QuickItem.h"
#include "ExchangeItem.h"
#include "SkillManager_Client.h"

#include "ChatManager.h"
#include "./Audio/MHAudioManager.h"
#include "mhFile.h"


#include "GuildWarehouseDialog.h"
#include "InventoryExDialog.h"
#include "PyogukDialog.h"
#include "QuickDialog.h"
#include "ChatDialog.h"
#include "GameEventManager.h"
#include "UpgradeDlg.h"
#include "ReinforceDlg.h"
#include "MHMap.h"
#include "DissolutionDialog.h"
#include "MugongManager.h"
#include "ItemShopDialog.h"
#include "StatusIconDlg.h"
#include "MoveDialog.h"
#include "CharacterDialog.h"
#include "cMsgBox.h"
#include "ObjectStateManager.h"
#include "MoveManager.h"
#include "EventMapInfo.h"

#include "ChaseDialog.h"
#include "NameChangeDialog.h"
#include "ReviveDialog.h"
#include "CharChangeDlg.h"

#include "GMNotifyManager.h"
#include "SealDialog.h"
#include "SiegeWarMgr.h"
#include "ChangeJobDialog.h"
#include "ReinforceResetDlg.h"
#include "RareCreateDialog.h"

#include "ReinforceManager.h"
#include "StatsCalcManager.h"

#include "TitanPartsMakeDlg.h"
#include "TitanMixDlg.h"
#include "TitanUpgradeDlg.h"
#include "TitanBreakDlg.h"
#include "TitanRepairDlg.h"
#include "TitanInventoryDlg.h"
#include "TitanRegisterDlg.h"
#include "TitanDissolutionDlg.h"
#include "TitanGuageDlg.h"
#include "PetManager.h"
#include "PetInventoryDlg.h"
#include "UniqueItemCurseCancellationDlg.h"
#include "UniqueItemMixDlg.h"
#include "SkinSelectDialog.h"
#include "CostumeSkinSelectDialog.h"
#include "StreetStallManager.h"
#include "FortWarDialog.h"
#include "FortWarManager.h"

#include "TitanManager.h"
#ifdef _TL_LOCAL_
#include "./Interface/cFont.h"
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
GLOBALTON(CItemManager)
CItemManager::CItemManager()
{
	m_ItemHash.Initialize(128);
	m_ItemInfoList.Initialize(MAX_ITEM_NUM);
	m_UnfitItemInfoListForHide.Initialize(100);
	m_ItemToolTipTextList.Initialize(64);		//confirm
	m_ItemOptionList.Initialize(MAX_ITEM_OPTION_NUM);
	m_ItemRareOptionList.Initialize(MAX_ITEM_OPTION_NUM);
	m_UsedItemList.Initialize(32);
	m_IconIndexCreator.Init(MAX_ITEMICON_NUM, IG_ITEM_STARTINDEX);
	m_bAddPrice = FALSE;
	m_RareItemInfoTable.Initialize(64);
	memset(&m_TempDeleteItem, 0, sizeof(ITEMBASE));
	m_SetItemOptionList.Initialize(MAX_SETITEM_KIND_NUM); //2007. 6. 8. CBH - ��Ʈ������ ����Ʈ �ʱ�ȭ �߰�

	m_nItemUseCount = 0;
}

CItemManager::~CItemManager()
{
	//m_GroundHash.R
	Release();
	m_IconIndexCreator.Release();

	ITEM_INFO * pInfo = NULL;
	m_ItemInfoList.SetPositionHead();
	while(pInfo = m_ItemInfoList.GetData())
		delete pInfo;
	m_ItemInfoList.RemoveAll();
	m_UnfitItemInfoListForHide.RemoveAll();	

	TOOLTIP_TEXT* pToolTipText = NULL;
	TOOLTIP_TEXT* pTemp = NULL;

	m_ItemToolTipTextList.SetPositionHead();
	while( pToolTipText = m_ItemToolTipTextList.GetData() )
	{
		do
		{
			pTemp = pToolTipText->pNext;
			if( pToolTipText->strToolTip )
				delete[] pToolTipText->strToolTip;
			delete pToolTipText;
			pToolTipText = pTemp;

		} while( pToolTipText );
	}
	m_ItemToolTipTextList.RemoveAll();

	SHOPITEMBASE* pShopItemInfo = NULL;
	m_UsedItemList.SetPositionHead();
	while(pShopItemInfo = m_UsedItemList.GetData())
		delete pShopItemInfo;
	m_UsedItemList.RemoveAll();

	sRareItemInfo* pList = NULL;
	m_RareItemInfoTable.SetPositionHead();
	while( pList = m_RareItemInfoTable.GetData() )
	{
		delete pList;
		pList = NULL;
	}
	m_RareItemInfoTable.RemoveAll();

	////////// 2007. 6. 8. CBH - ��Ʈ������ ����Ʈ ���� //////////
	SET_ITEM_OPTION* pSetItemOption = NULL;
	m_SetItemOptionList.SetPositionHead();
	while(pSetItemOption = m_SetItemOptionList.GetData())
		delete pSetItemOption;
	m_SetItemOptionList.RemoveAll();
	//////////////////////////////////////////////////////////////
}


cImage * CItemManager::GetIconImage(WORD ItemIdx, cImage * pImageGet)
{
	//char buff[128];
	//wsprintf(buff, "image/item/%d.tif",GetItemInfo(ItemIdx)->Image2DNum);
	//cImage* pImage = WINDOWMGR->GetImageOfFile(buff, 40);
	//return pImage;
	SCRIPTMGR->GetImage( GetItemInfo(ItemIdx)->Image2DNum, pImageGet, PFT_ITEMPATH );	
	return pImageGet;
}

CItem* CItemManager::MakeNewItem(ITEMBASE* pBaseInfo, char* strKind)
{
	ITEM_INFO* pItemInfo = GetItemInfo(pBaseInfo->wIconIdx);

	
	if(pItemInfo == NULL)
	{
		char temp[256];
		sprintf(temp,"NoItemInfo %d,  %d,  %s",HEROID,pBaseInfo->wIconIdx,strKind);
		ASSERTMSG(pItemInfo, temp);
		return NULL;
	}
	
	if(pBaseInfo->dwDBIdx == 0)
		return NULL;

	CItem* pItem = NULL;
	if(!(pItem = GetItem(pBaseInfo->dwDBIdx)))
	{
		pItem = new CItem(pBaseInfo);
		m_ItemHash.Add(pItem,pItem->GetDBIdx());						// HASH���� ����AIAU ��O��a
		cImage tmpIconImage;
		pItem->Init(0,0,DEFAULT_ICONSIZE,DEFAULT_ICONSIZE,
					GetIconImage(pItem->GetItemIdx(), &tmpIconImage),
					m_IconIndexCreator.GenerateIndex());
		pItem->SetIconType(eIconType_AllItem);
		pItem->SetData(pItem->GetItemIdx());
		pItem->SetLinkPosition( 0 );

		WINDOWMGR->AddWindow(pItem);
		//////////////////////////////////////////////////////////////////////////
		// tooltip ���� �κШ�
//		if(pItem->GetDurability() != 0 && !IsDupItem(pItem->GetItemIdx()))
//			SetToolTipIcon(pItem, GetItemOption(pItem->GetDurability()));
		//!!! NULL Ȯ�� SW050920 Rare

		// magi82 - Source ����ȭ(������ �����Լ� ������ �ּ�ȭ)
		//if( pItem->GetDurability() != 0 && !IsDupItem((pItem->GetItemIdx())) )
		//{
		//	SetToolTipIcon(pItem, GetItemOption(pItem->GetDurability()), GetItemRareOption(pItem->GetRareness()) );
		//}
		//else
		//	SetToolTipIcon( pItem, NULL, GetItemRareOption(pItem->GetRareness()) );
		//////////////////////////////////////////////////////////////////////////
	}
	else
	{
		char buf[64];
		sprintf(buf, "�ߺ��� DB�ε����� ���! DBIndex: %d", pBaseInfo->dwDBIdx);
		ASSERTMSG(0, buf);
	}

	return pItem;
}

void CItemManager::SetToolTipIcon(cIcon * pIcon, ITEM_OPTION_INFO * pOptionInfo, ITEM_RARE_OPTION_INFO* pRareOptionInfo, DWORD dwItemDBIdx )
{
//	char ToolTipText[256];
	char buf[256] = { 0, };
	WORD index = (WORD)pIcon->GetData();
	eITEM_KINDBIT bits = GetItemKind(index);

	cImage imgToolTip;
	SCRIPTMGR->GetImage( 63, &imgToolTip, PFT_HARDPATH );
	// ""�� ������ �ʱ�ȭ�� �Ѵ�.
	pIcon->SetToolTip( "", RGBA_MAKE(255, 255, 255, 255), &imgToolTip, TTCLR_ITEM_CANEQUIP );

	if(bits & eEQUIP_ITEM)
	{
		ITEM_INFO * pInfo = GetItemInfo(index);
		if(pInfo)
		SetEquipItemToolTip( pIcon, pInfo, pOptionInfo, pRareOptionInfo );
	}
	else if(bits & eMUGONG_ITEM)
	{
		ITEM_INFO * pInfo = GetItemInfo(index);
		if(pInfo)
		SetMugongItemToolTip( pIcon, pInfo );
	}
	else if(bits & eYOUNGYAK_ITEM)
	{
		ITEM_INFO * pInfo = GetItemInfo(index);
		if(pInfo)
		SetYoungyakItemToolTip( pIcon, pInfo );
	}
	else if(bits & eEXTRA_ITEM || bits & eEXTRA_ITEM_UPGRADE)
	{
		ITEM_INFO * pInfo = GetItemInfo(index);
		if(pInfo)
		SetExtraItemToolTip( pIcon, pInfo );
	}
	else if( bits & eQUEST_ITEM )
	{
		ITEM_INFO * pInfo = GetItemInfo(index);
		if( pInfo )
		{
			char line[128];
			wsprintf( line, "[%s]", pInfo->ItemName );
			pIcon->AddToolTipLine( line );
			pIcon->AddToolTipLine( "" );

			if( FALSE == IsPetSummonItem(index) )
				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(260), TTTC_QUESTITEM );

			if( IsPetSummonItem(index) )
			{
				SetPetSummonItemToolTip(pIcon,pInfo,dwItemDBIdx);
				/*
				//char line[128];
				DWORD	ItemDBIdx = 0;
				if(dwItemDBIdx)
				{
					ItemDBIdx = dwItemDBIdx;
				}
				else
				{
					const ITEMBASE* pInfo = ((CItem*)pIcon)->GetItemBaseInfo();
					ItemDBIdx = pInfo->dwDBIdx;
				}
				
				PET_TOTALINFO* pInfo = pInfo = PETMGR->GetPetInfo(ItemDBIdx);
				if( pInfo )
				{
					if( !pInfo->bAlive )
					{
						DWORD RedFilter = RGBA_MAKE(255,10,10,255);
						SetIconColorFilter(pIcon,RedFilter);
					}
					wsprintf( line, CHATMGR->GetChatMsg(1244), pInfo->PetGrade );
					pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
					wsprintf( line, CHATMGR->GetChatMsg(1245), (PETMGR->GetPetFriendship(ItemDBIdx))/1000 );
				}
				else
				{
					wsprintf( line, CHATMGR->GetChatMsg(1244), PET_DEFAULT_GRADE );
					pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
					wsprintf( line, CHATMGR->GetChatMsg(1245), PET_DEFAULT_FRIENDLY/1000 );
				}
				pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
				*/
			}

			AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
		}
	}
	else if( bits & eCHANGE_ITEM )
	{
		ITEM_INFO * pInfo = GetItemInfo(index);
		if( pInfo )
		{
			char line[128];
			wsprintf( line, "[%s]", pInfo->ItemName );			
			pIcon->AddToolTipLine( line );
			pIcon->AddToolTipLine( "" );
			if( bits == eCHANGE_ITEM_LOCK )
			{
				sprintf(line, CHATMGR->GetChatMsg(238), pInfo->ItemType);
				if( HERO->GetCharacterTotalInfo()->Level < pInfo->ItemType )
					pIcon->AddToolTipLine( line, TTTC_LIMIT );
				else
					pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
			}
			AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
		}
	}
	else if( bits & eSHOP_ITEM )
	{
		ITEM_INFO* pInfo = GetItemInfo(index);
		if(pInfo)
		{
			char line[128];
			wsprintf( line, "[%s]", pInfo->ItemName );
			DWORD dwColor = TTTC_DEFAULT;
			// 2008. 2. 27. CBH - �������� �ֹ��� ������ �̸� ���� ���� ó��
			if( pInfo->ItemKind == eSHOP_ITEM_INCANTATION && pInfo->LimitJob )
			{				
				if( pInfo->LimitGender == 0 && HERO->GetLevel() > 50 )
				{
					dwColor = TTTC_LIMIT;
				}
				else if( pInfo->LimitGender == 1 && (HERO->GetLevel() <51 || HERO->GetLevel() > 70 ) )
				{
					dwColor = TTTC_LIMIT;
				}
				else if( pInfo->LimitGender == 2 && (HERO->GetLevel() <71 || HERO->GetLevel() > 90 ) )
				{
					dwColor = TTTC_LIMIT;
				}
			}
			else if( HERO->GetLevel() < pInfo->NaeRyukRecoverRate )	//������ �������� ĳ���� ������ ������ ������
			{
				dwColor = TTTC_LIMIT;
			}
			pIcon->AddToolTipLine( line, dwColor );

			//2007. 9. 21. CBH - ���� �ִ� �� ������ ���� �ѷ��ֱ�
			if( pInfo->NaeRyukRecoverRate != 0 )
			{
				pIcon->AddToolTipLine("");
				sprintf(line, CHATMGR->GetChatMsg(238), (int)pInfo->NaeRyukRecoverRate);
				if( HERO->GetCharacterTotalInfo()->Level < pInfo->NaeRyukRecoverRate )
					pIcon->AddToolTipLine( line, TTTC_LIMIT );
				else
					pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
			}
//-------
			
			if( pIcon->GetType() == WT_EXCHANGEITEM || pIcon->GetType() == WT_STALLITEM || pInfo->ItemKind == eSHOP_ITEM_PET )
			{
				CExchangeItem* pExItem = (CExchangeItem*)pIcon;
				if( pExItem->GetItemParam() & ITEM_PARAM_SEAL )
				{
					if( IsPetSummonItem(index) )
					{
						SetPetSummonItemToolTip(pIcon,pInfo,dwItemDBIdx);
					}

					// ���ΰ��� ������ 6000��
					AddItemDescriptionToolTip( pIcon, 6000 );
					AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
				}
				else
				{
					//SW051129 Pet
					const ITEMBASE* pBaseInfo = ((CItem*)pIcon)->GetItemBaseInfo();
					if( IsPetSummonItem(index) )
					{
						SetPetSummonItemToolTip(pIcon,pInfo,dwItemDBIdx);
						/*
						DWORD	ItemDBIdx = 0;
						if(dwItemDBIdx)
						{
							ItemDBIdx = dwItemDBIdx;
						}
						else
						{
							ItemDBIdx = pBaseInfo->dwDBIdx;
						}

						PET_TOTALINFO* pPetInfo = PETMGR->GetPetInfo(ItemDBIdx);
						if( pPetInfo )
						{
							if( !pPetInfo->bAlive )
							{
								DWORD RedFilter = RGBA_MAKE(255,10,10,255);
								SetIconColorFilter(pIcon,RedFilter);
							}
							wsprintf( line, CHATMGR->GetChatMsg(1244), pPetInfo->PetGrade );
							pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
							wsprintf( line, CHATMGR->GetChatMsg(1245), pPetInfo->PetFriendly/1000 );
							pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
						}
						else	//���� ȹ���� �� �������̸�
						{
							wsprintf( line, CHATMGR->GetChatMsg(1244), 1 );	// �⺻ 1�ܰ�
							pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
							wsprintf( line, CHATMGR->GetChatMsg(1245), PET_DEFAULT_FRIENDLY/1000 );
							pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
						}*/
					}

					AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
				}
				return;
			}

			if( pIcon->GetType() != WT_ITEM && pIcon->GetType() != WT_VIRTUALITEM )
				return;

//--------
			CItem* pItem = (CItem*)pIcon;
			if( pInfo->ItemKind == eSHOP_ITEM_MAKEUP || pInfo->ItemKind == eSHOP_ITEM_DECORATION )
			{
				if( !(pItem->GetItemParam() & ITEM_PARAM_SEAL) )
				{
					SHOPITEMBASE* pShopItemBase = m_UsedItemList.GetData( pItem->GetItemIdx() );
					if( pShopItemBase ) 
						AddUsedAvatarItemToolTip( pShopItemBase );
					return;
				}				
			}
			if( pInfo->ItemKind == eSHOP_ITEM_EQUIP )
			{
				if( !(pItem->GetItemParam() & ITEM_PARAM_SEAL) )
				{
					SHOPITEMBASE* pShopItemBase = m_UsedItemList.GetData( pItem->GetItemIdx() );
					if( pShopItemBase )
						AddUsedShopEquipItemToolTip( pShopItemBase );
					return;
				}
			}
			if( pInfo->ItemKind == eSHOP_ITEM_PET_EQUIP )
			{
				if( !(pItem->GetItemParam() & ITEM_PARAM_SEAL) )
				{
					SHOPITEMBASE* pShopItemBase = m_UsedItemList.GetData( pItem->GetItemIdx() );
					if( pShopItemBase )
						AddUsedPetEquipItemToolTip( pShopItemBase );
					return;
				}
			}
			if( pIcon->GetType()==WT_ITEM && (pItem->GetItemParam() & ITEM_PARAM_SEAL) )
			{
				// ���ΰ��� ������ 6000��
				AddItemDescriptionToolTip( pIcon, 6000 );
				// 060911 KKR ����, ���Ⱓ �ѷ��ֱ�
 				AddShopItemToolTip( (cIcon*)pItem, pInfo );
				PrintShopItemUseTime( pItem, pInfo );
				
			}
			else
			{
				AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
				// 060911 KKR ����, ���Ⱓ �ѷ��ֱ�
				AddShopItemToolTip( (cIcon*)pItem, pInfo );                
							
				//if( !(pInfo->ItemKind == eSHOP_ITEM_SUNDRIES) )
				if( pInfo->ItemType == 11 )
				{
	//				if( !(pInfo->ItemKind == eSHOP_ITEM_SUNDRIES) )
					{
						SHOPITEMBASE* pShopItemBase = m_UsedItemList.GetData( pItem->GetItemIdx() );
						if( pShopItemBase )
						{
							// ����ð� ���Ǵ°�
							if( pInfo->SellPrice == eShopItemUseParam_Realtime )
							{
								// ����ð�
								pItem->AddToolTipLine("");
								pItem->AddToolTipLine( CHATMGR->GetChatMsg(766), TTTC_DEFAULT );

								stTIME time;
								char buf[128] = { 0, };
								time.value = pShopItemBase->Remaintime;
								wsprintf( buf, CHATMGR->GetChatMsg(767), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute() );
								pItem->AddToolTipLine( buf, TTTC_QUESTITEM );
							}
							// �Ⱓ��
							else 
							{
								// ���Ⱓ ǥ��.
								pItem->AddToolTipLine("");
								pItem->AddToolTipLine( CHATMGR->GetChatMsg(1442), TTTC_DEFAULT );
								pItem->AddToolTipLine( CHATMGR->GetChatMsg(1444), TTTC_QUESTITEM );
							
							}

						}

					}
				}
				else{
					PrintShopItemUseTime( pItem, pInfo );
				}

			}			
			
		}
	}
	else if( bits & eTITAN_EQUIPITEM || bits == eTITAN_ITEM_RECIPE || bits == eTITAN_ITEM_PAPER || bits == eTITAN_ITEM_PARTS )
	{
		ITEM_INFO * pInfo = GetItemInfo(index);
		if(!pInfo)
			return;

		char line[128];
		wsprintf( line, "[%s]", pInfo->ItemName );
		pIcon->AddToolTipLine( line );
		pIcon->AddToolTipLine( "" );

		// magi82 - Titan(070420)
		if(bits & eTITAN_EQUIPITEM)
		{
			DWORD dwDBIdx = 0;
			if( pIcon->GetType() == WT_EXCHANGEITEM || pIcon->GetType() == WT_STALLITEM )
			{
				CExchangeItem* pItem = (CExchangeItem*)pIcon;
				dwDBIdx = pItem->GetDBId();
			}
			else
			{
				CItem* pItem = (CItem*)pIcon;
				dwDBIdx = pItem->GetDBIdx();
			}

			SetTitanEquipItemToolTip(pIcon, pInfo);

			TITAN_ENDURANCE_ITEMINFO* pEnduranceInfo = NULL;
			if(dwItemDBIdx > 0)
			{
				pEnduranceInfo = TITANMGR->GetTitanEnduranceInfo(dwItemDBIdx);
			}
			else
			{
				if(dwDBIdx)
				{
					pEnduranceInfo = TITANMGR->GetTitanEnduranceInfo(dwDBIdx);
				}
			}

			if(pEnduranceInfo)
			{
				float fEnduranceRate = (float)pEnduranceInfo->Endurance * 100 / TITAN_EQUIPITEM_ENDURANCE_MAX;
				if(fEnduranceRate > 0.0f && fEnduranceRate < 1.0f)
				{
					fEnduranceRate = 1.0f;
				}

				ZeroMemory(&line, sizeof(line));
				wsprintf( line, CHATMGR->GetChatMsg(1516), (DWORD)fEnduranceRate);
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
			}
		}
		if(bits == eTITAN_ITEM_PAPER )
		{
			DWORD dwDBIdx = 0;
			if( pIcon->GetType() == WT_EXCHANGEITEM || pIcon->GetType() == WT_STALLITEM )
			{
				CExchangeItem* pItem = (CExchangeItem*)pIcon;
				dwDBIdx = pItem->GetDBId();
			}
			else
			{
				CItem* pItem = (CItem*)pIcon;
				dwDBIdx = pItem->GetDBIdx();
			}

			ZeroMemory(&line, sizeof(line));
			wsprintf( line, CHATMGR->GetChatMsg(1519), pInfo->Plus_Value );
			pIcon->AddToolTipLine( line );

			if( dwDBIdx != 0 && dwDBIdx == TITANMGR->GetRegistedTitanItemDBIdx() )
			{
				ZeroMemory(&line, sizeof(line));
				wsprintf( line, CHATMGR->GetChatMsg(1520) );
				pIcon->AddToolTipLine( line );
			}
		}

		AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
	}
	else
	{
		ASSERT(0);
	}
}

void CItemManager::SetExtraItemToolTip( cIcon* pIcon, ITEM_INFO* pInfo )
{
	char line[128];
	wsprintf( line, "[%s]", pInfo->ItemName );
	pIcon->AddToolTipLine( line );
	pIcon->AddToolTipLine( "" );

	switch( pInfo->ItemType )
	{
	case eEXTRA_ITEM_JEWEL:				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(251), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_MATERIAL:			pIcon->AddToolTipLine( CHATMGR->GetChatMsg(252), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_METAL:				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(253), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_BOOK:				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(254), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_HERB:				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(255), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_ETC:				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(256), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_UPGRADE_ATTACK:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(257), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_UPGRADE_DEFENSE:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(258), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_COMP:				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(259), TTTC_EXTRAITEM );	break;
	case eEXTRA_ITEM_QUEST:				pIcon->AddToolTipLine( CHATMGR->GetChatMsg(260), TTTC_EXTRAITEM );	break;
	}

	AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
}

void CItemManager::SetYoungyakItemToolTip( cIcon* pIcon, ITEM_INFO* pInfo )
{
	char line[128];
	wsprintf( line, "[%s]", pInfo->ItemName );
	pIcon->AddToolTipLine( line );
	pIcon->AddToolTipLine( "" );

	if( pInfo->GenGol != 0)
	{
		wsprintf(line, "%s +%d", CHATMGR->GetChatMsg(261), pInfo->GenGol);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( pInfo->MinChub != 0)
	{
		wsprintf(line, "%s +%d", CHATMGR->GetChatMsg(262), pInfo->MinChub);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( pInfo->CheRyuk != 0)
	{
		wsprintf(line, "%s +%d", CHATMGR->GetChatMsg(263), pInfo->CheRyuk);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( pInfo->SimMek != 0)
	{
		wsprintf(line, "%s +%d", CHATMGR->GetChatMsg(264), pInfo->SimMek);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrRegist.GetElement_Val(ATTR_FIRE)) !=0 )
	{
		wsprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(265), (int)(pInfo->AttrRegist.GetElement_Val(ATTR_FIRE)));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrRegist.GetElement_Val(ATTR_WATER)) !=0 )
	{
		wsprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(266), (int)(pInfo->AttrRegist.GetElement_Val(ATTR_WATER)));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrRegist.GetElement_Val(ATTR_TREE)) !=0 )
	{
		wsprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(267), (int)(pInfo->AttrRegist.GetElement_Val(ATTR_TREE)));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrRegist.GetElement_Val(ATTR_IRON)) !=0 )
	{
		wsprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(268), (int)(pInfo->AttrRegist.GetElement_Val(ATTR_IRON)));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrRegist.GetElement_Val(ATTR_EARTH)) !=0 )
	{
		wsprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(269), (int)(pInfo->AttrRegist.GetElement_Val(ATTR_EARTH)));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( pInfo->LifeRecover !=0 )
	{
		if(pInfo->ItemKind == eYOUNGYAK_ITEM_PET)
		{
			wsprintf(line, "%s %d", CHATMGR->GetChatMsg(1276), pInfo->LifeRecover);
		}
		else
		{
			wsprintf(line, "%s %d", CHATMGR->GetChatMsg(270), pInfo->LifeRecover);
		}
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->LifeRecoverRate*100) !=0 )
	{
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(270), (int)(pInfo->LifeRecoverRate*100));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( pInfo->NaeRyukRecover !=0 )
	{
		wsprintf(line, "%s %d", CHATMGR->GetChatMsg(271), pInfo->NaeRyukRecover);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->NaeRyukRecoverRate*100) !=0 )
	{
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(271), (int)(pInfo->NaeRyukRecoverRate*100));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}

#ifdef TAIWAN_LOCAL
//========================= �ް� �ϵ��ڵ�
	if( pInfo->ItemIdx == 53031 || pInfo->ItemIdx == 53094 )
	{
		wsprintf(line, "%s %d%%", "����ָ�", 100);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else if( pInfo->ItemIdx == 53032 || pInfo->ItemIdx == 53095 )
	{
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(270), 100);	//������ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(271), 100);	//����ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		wsprintf(line, "%s %d%%", "����ָ�", 100);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
//==========================
#else
//========================= �ް� �ϵ��ڵ�
	if( pInfo->ItemIdx == 53102 )
	{
		wsprintf(line, "%s %d", CHATMGR->GetChatMsg(270), 1000);	//������ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		wsprintf(line, "%s %d", CHATMGR->GetChatMsg(271), 1000);	//����ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		wsprintf(line, "%s %d", CHATMGR->GetChatMsg(181), 1000);	//ȣ�Ű���ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else if( pInfo->ItemIdx == 53031 || pInfo->ItemIdx == 53094 )
	{
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(181), 100);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else if( pInfo->ItemIdx == 53216 || pInfo->ItemIdx == 53222 )
	{
#ifdef _HK_LOCAL_
		wsprintf(line, "%s %d%", CHATMGR->GetChatMsg(181), 1000);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
#else
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(181), 100);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
#endif
	}
	else if( pInfo->ItemIdx == 53032 || pInfo->ItemIdx == 53095 
		|| pInfo->ItemIdx == 53103 || pInfo->ItemIdx == 53217 || pInfo->ItemIdx == 53223)
	{
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(270), 100);	//������ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(271), 100);	//����ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		wsprintf(line, "%s %d%%", CHATMGR->GetChatMsg(181), 100);	//ȣ�Ű���ȸ��
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}

//==========================

#endif
	
	AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
}

void CItemManager::SetMugongItemToolTip( cIcon* pIcon, ITEM_INFO* pInfo )
{
	char line[128];
	wsprintf( line, "[%s]", pInfo->ItemName );
	pIcon->AddToolTipLine( line );
	pIcon->AddToolTipLine( "" );

	switch( pInfo->ItemKind )
	{
	case 1025: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(274), TTTC_MUGONGKIND ); break;
	case 1026: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(275), TTTC_MUGONGKIND ); break;
	case 1027: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(276), TTTC_MUGONGKIND ); break;
	case 1028: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(277), TTTC_MUGONGKIND ); break;
	case 1029: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(278), TTTC_MUGONGKIND ); break;
	case 1030: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(283), TTTC_MUGONGKIND ); break;
	case 1031: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(284), TTTC_MUGONGKIND ); break;
	case 1032: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(285), TTTC_MUGONGKIND ); break;
	case 1033: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(286), TTTC_MUGONGKIND ); break;
	case 1034: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(287), TTTC_MUGONGKIND ); break;
	case 1035: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(288), TTTC_MUGONGKIND ); break;
	case 1036: pIcon->AddToolTipLine( CHATMGR->GetChatMsg(289), TTTC_MUGONGKIND ); break;
	case eMUGONG_ITEM_SIMBUB:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(243), TTTC_MUGONGKIND ); break;
	case eMUGONG_ITEM_JINBUB:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(244), TTTC_MUGONGKIND ); break;
	case eMUGONG_ITEM_KYUNGGONG:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(245), TTTC_MUGONGKIND ); break;
	case 1040:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1362), TTTC_MUGONGKIND ); break;
    case eMUGONG_ITEM_JOB:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1669), TTTC_MUGONGKIND ); break;		//2007. 10. 28. CBH - ������� ����
	}
#ifdef _JAPAN_LOCAL_
	if( pInfo->wItemAttr > 0 && pInfo->wItemAttr < 6 )
	{
		wsprintf(line, "%s %s", CHATMGR->GetChatMsg(1194), CHATMGR->GetChatMsg(1186+pInfo->wItemAttr) );
		if( HERO->GetMainCharAttr() == pInfo->wItemAttr || HERO->GetSubCharAttr() == pInfo->wItemAttr )
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
	}
	else if( pInfo->wItemAttr > 10 && pInfo->wItemAttr < 16 )
	{
		wsprintf(line, "%s %s", CHATMGR->GetChatMsg(1195), CHATMGR->GetChatMsg(1186+pInfo->wItemAttr-10) );
		if( HERO->GetMainCharAttr() == pInfo->wItemAttr-10 )
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
	}

	if( pInfo->wAcquireSkillIdx1 )
	{
		CSkillInfo* psinfo = SKILLMGR->GetSkillInfo( pInfo->wAcquireSkillIdx1 );
		if( psinfo )
		{
			char temp[64] = {0,};
			sprintf( temp, psinfo->GetSkillName() );
			if( pInfo->wAcquireSkillIdx1 != pInfo->wAcquireSkillIdx2 )
			{
				int len = strlen(temp);
				if( len > 2 ) temp[len-2] = 0;
			}
			wsprintf(line, "%s %s", temp, CHATMGR->GetChatMsg(1196));
			if( CheckItemAquireSkill( pInfo->wAcquireSkillIdx1, pInfo->wAcquireSkillIdx2 ) )
				pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
			else
				pIcon->AddToolTipLine( line, TTTC_LIMIT );
		}
	}
#endif
	// stage limit
	SetItemToolTipForStage( pIcon, pInfo );

	switch( pInfo->LimitGender )
	{
	case 1:
		pIcon->AddToolTipLine(CHATMGR->GetChatMsg(236), TTTC_LIMIT );
		break;
	case 2:
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(237), TTTC_LIMIT );
		break;
	}

	if( pInfo->LimitLevel != 0 )
	{
		wsprintf(line, CHATMGR->GetChatMsg(238), pInfo->LimitLevel);
		if( HERO->GetCharacterTotalInfo()->Level < pInfo->LimitLevel )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitGenGol != 0 )
	{
		wsprintf(line, CHATMGR->GetChatMsg(239), pInfo->LimitGenGol);
		if( HERO->GetHeroTotalInfo()->wGenGol < pInfo->LimitGenGol )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitMinChub != 0 )
	{
		wsprintf(line, CHATMGR->GetChatMsg(240), pInfo->LimitMinChub);
		if( HERO->GetHeroTotalInfo()->wMinChub < pInfo->LimitMinChub )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitCheRyuk != 0 )
	{
		wsprintf(line, CHATMGR->GetChatMsg(241), pInfo->LimitCheRyuk);
		if( HERO->GetHeroTotalInfo()->wCheRyuk < pInfo->LimitCheRyuk )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitSimMek != 0 )
	{
		wsprintf(line, CHATMGR->GetChatMsg(242), pInfo->LimitSimMek);
		if( HERO->GetHeroTotalInfo()->wSimMek < pInfo->LimitSimMek )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}

	////////////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ������ȯ
	if(pInfo->ItemKind == 1040)
	{
		SKILLOPTION* pOption = SKILLMGR->GetSkillOptionByItemIndex(pInfo->ItemIdx);

		if(pOption)
		{
			if(pOption->Range > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1316), pOption->Range);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->Range < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1320), -pOption->Range);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->ReduceNaeRyuk > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1317), (int)(pOption->ReduceNaeRyuk * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->ReduceNaeRyuk < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1321), (int)(-pOption->ReduceNaeRyuk * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->PhyAtk > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1318), (int)(pOption->PhyAtk * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->PhyAtk < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1322), (int)(-pOption->PhyAtk * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			
			if(pOption->BaseAtk > 0)
			{
			char ToolTipText[32];
			wsprintf(ToolTipText, CHATMGR->GetChatMsg(1740), (int)(pOption->BaseAtk * 100));
			pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			
			if(pOption->AttAtk > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1319), (int)(pOption->AttAtk * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->AttAtk < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1323), (int)(-pOption->AttAtk * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->Life > 0)	
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1340), pOption->Life);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->Life < 0)	
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1341), -pOption->Life);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->NaeRyuk > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1342), pOption->NaeRyuk);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->NaeRyuk < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1343), -pOption->NaeRyuk);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->Shield > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1344), pOption->Shield);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->Shield < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1345), -pOption->Shield);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->PhyDef > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1348), (int)(pOption->PhyDef * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->PhyDef < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1349), (int)(-pOption->PhyDef * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->AttDef > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1324), (int)(pOption->AttDef * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->AttDef < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1325), (int)(-pOption->AttDef * 100 + 0.5));
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}

			if(pOption->Duration > 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1346), pOption->Duration);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
			if(pOption->Duration < 0)
			{
				char ToolTipText[32];
				wsprintf(ToolTipText, CHATMGR->GetChatMsg(1347), pOption->Duration);
				pIcon->AddToolTipLine( ToolTipText, TTTC_TRANSMUGONGOPTION );
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////

	// ������ ���
//	if(pInfo->ItemKind>=1024 && pInfo->ItemKind<=eMUGONG_ITEM_KYUNGGONG)	
//		AddItemDescriptionToolTip( pIcon, SKILLMGR->GetSkillTooltipInfo(pInfo->MugongNum));	
//	else
//		AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
	AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
}

void CItemManager::SetEquipItemToolTip( cIcon* pIcon, ITEM_INFO* pInfo, ITEM_OPTION_INFO * pOptionInfo, ITEM_RARE_OPTION_INFO* pRareOptionInfo )
{
	char line[128];
	if(pOptionInfo)
		sprintf( line, "[%s*]", pInfo->ItemName );
	else
		sprintf( line, "[%s]", pInfo->ItemName );
	
	int nLen = strlen( pInfo->ItemName );

	//SW050920 ����
	if(pRareOptionInfo && pRareOptionInfo->dwItemDBIdx)
		pIcon->AddToolTipLine( line, TTTC_RAREITEM );
	else if(pInfo->wSetItemKind != 0)
		pIcon->AddToolTipLine( line, TTTC_SETITEM_NAME);
	else if( *(pInfo->ItemName + nLen - 2) == '+' || *(pInfo->ItemName + nLen - 3) == '+' )	// +10�̻� ������ �߰� �۾� by Stiner(2008/06/10)-10+Item
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTRITEM );
	else if(pInfo->ItemKind == eEQUIP_ITEM_UNIQUE)	// magi82 - UniqueItem(070628)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else
		pIcon->AddToolTipLine( line );
	pIcon->AddToolTipLine( "" );

	float attrvalue = 0;
	float attroptvalue = 0;
	//SW050920 ����
	float attrRareOptValue = 0;
	// stage limit
	SetItemToolTipForStage( pIcon, pInfo );

	switch( pInfo->LimitGender )
	{
	case 1:
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(236), TTTC_LIMIT );
		break;
	case 2:
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(237), TTTC_LIMIT );
		break;
	}

	if( pInfo->LimitLevel != 0 )
	{
		sprintf(line, CHATMGR->GetChatMsg(238), pInfo->LimitLevel);
		if( HERO->GetCharacterTotalInfo()->Level < pInfo->LimitLevel )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitGenGol != 0 )
	{
		sprintf(line, CHATMGR->GetChatMsg(239), pInfo->LimitGenGol);
		if( HERO->GetHeroTotalInfo()->wGenGol < pInfo->LimitGenGol )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitMinChub != 0 )
	{
		sprintf(line, CHATMGR->GetChatMsg(240), pInfo->LimitMinChub);
		if( HERO->GetHeroTotalInfo()->wMinChub < pInfo->LimitMinChub )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitCheRyuk != 0 )
	{
		sprintf(line, CHATMGR->GetChatMsg(241), pInfo->LimitCheRyuk);
		if( HERO->GetHeroTotalInfo()->wCheRyuk < pInfo->LimitCheRyuk )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}
	if( pInfo->LimitSimMek != 0 )
	{
		sprintf(line, CHATMGR->GetChatMsg(242), pInfo->LimitSimMek);
		if( HERO->GetHeroTotalInfo()->wSimMek < pInfo->LimitSimMek )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );
	}

	/*SW050920
	if( pInfo->GenGol != 0)
	{
		if(pOptionInfo && pOptionInfo->GenGol != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(382), pInfo->GenGol, pOptionInfo->GenGol);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(382), pInfo->GenGol);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(pOptionInfo && pOptionInfo->GenGol != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(382), pOptionInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare �ٰ�
	DWORD RareState;
	if( pRareOptionInfo && pRareOptionInfo->GenGol )
		RareState = pRareOptionInfo->GenGol;
	else
		RareState = 0;

	if( pInfo->GenGol != 0)
	{
		if(pOptionInfo && pOptionInfo->GenGol != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(382), pInfo->GenGol + RareState, pOptionInfo->GenGol);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(382), pInfo->GenGol + RareState);
		if(pRareOptionInfo && pRareOptionInfo->GenGol)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(RareState && pOptionInfo && pOptionInfo->GenGol != 0)
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(382), RareState, pOptionInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->GenGol != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(382), pOptionInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(pRareOptionInfo && pRareOptionInfo->GenGol != 0)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(382), pRareOptionInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

	/*SW050920
	if( pInfo->MinChub != 0)
	{
		if(pOptionInfo && pOptionInfo->MinChub != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(383), pInfo->MinChub, pOptionInfo->MinChub);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(383), pInfo->MinChub);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(pOptionInfo && pOptionInfo->MinChub != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(383), pOptionInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare ��ø
	if( pRareOptionInfo && pRareOptionInfo->MinChub )
		RareState = pRareOptionInfo->MinChub;
	else
		RareState = 0;
	if( pInfo->MinChub != 0)
	{
		if(pOptionInfo && pOptionInfo->MinChub != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(383), pInfo->MinChub + RareState, pOptionInfo->MinChub);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(383), pInfo->MinChub + RareState);

		if(pRareOptionInfo && pRareOptionInfo->MinChub)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(RareState && pOptionInfo && pOptionInfo->MinChub != 0)
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(383), RareState, pOptionInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->MinChub != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(383), pOptionInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(pRareOptionInfo && pRareOptionInfo->MinChub != 0)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(383), pRareOptionInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

	/*SW050920
	if( pInfo->CheRyuk != 0)
	{
		if(pOptionInfo && pOptionInfo->CheRyuk != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(384), pInfo->CheRyuk, pOptionInfo->CheRyuk);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(384), pInfo->CheRyuk);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(pOptionInfo && pOptionInfo->CheRyuk != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(384), pOptionInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare ü��
	if( pRareOptionInfo && pRareOptionInfo->CheRyuk )
		RareState = pRareOptionInfo->CheRyuk;
	else
		RareState = 0;
	if( pInfo->CheRyuk != 0)
	{
		if(pOptionInfo && pOptionInfo->CheRyuk != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(384), pInfo->CheRyuk + RareState, pOptionInfo->CheRyuk);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(384), pInfo->CheRyuk + RareState);

		if(pRareOptionInfo && pRareOptionInfo->CheRyuk)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(RareState && pOptionInfo && pOptionInfo->CheRyuk != 0)
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(384), RareState, pOptionInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->CheRyuk != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(384), pOptionInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(pRareOptionInfo && pRareOptionInfo->CheRyuk != 0)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(384), pRareOptionInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

	/*SW050920
	if( pInfo->SimMek != 0)
	{
		if(pOptionInfo && pOptionInfo->SimMek != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(385), pInfo->SimMek, pOptionInfo->SimMek);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(385), pInfo->SimMek);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(pOptionInfo && pOptionInfo->SimMek != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(385), pOptionInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare �ɸ�
	if( pRareOptionInfo && pRareOptionInfo->SimMek )
		RareState = pRareOptionInfo->SimMek;
	else
		RareState = 0;
	if( pInfo->SimMek != 0)
	{
		if(pOptionInfo && pOptionInfo->SimMek != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(385), pInfo->SimMek + RareState, pOptionInfo->SimMek);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(385), pInfo->SimMek + RareState);

		if(pRareOptionInfo && pRareOptionInfo->SimMek)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(RareState && pOptionInfo && pOptionInfo->SimMek != 0)
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(385), RareState, pOptionInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->SimMek != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(385), pOptionInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(pRareOptionInfo && pRareOptionInfo->SimMek != 0)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(385), pRareOptionInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

	/*SW050920
	if( pInfo->Life != 0 )
	{
		if(pOptionInfo && pOptionInfo->Life != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(386), pInfo->Life, pOptionInfo->Life);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(386), pInfo->Life );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(pOptionInfo && pOptionInfo->Life != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(386), pOptionInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare ������
	if( pRareOptionInfo && pRareOptionInfo->Life )
		RareState = pRareOptionInfo->Life;
	else
		RareState = 0;
	if( pInfo->Life != 0)
	{
		if(pOptionInfo && pOptionInfo->Life != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(386), pInfo->Life + RareState, pOptionInfo->Life);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(386), pInfo->Life + RareState);

		if(pRareOptionInfo && pRareOptionInfo->Life)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(RareState && pOptionInfo && pOptionInfo->Life != 0)
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(386), RareState, pOptionInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->Life != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(386), pOptionInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(pRareOptionInfo && pRareOptionInfo->Life != 0)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(386), pRareOptionInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

	/*SW050920 Rare
	if( pInfo->NaeRyuk != 0 )
	{
		if(pOptionInfo && pOptionInfo->NaeRyuk != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(387), pInfo->NaeRyuk, pOptionInfo->NaeRyuk);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(387), pInfo->NaeRyuk );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(pOptionInfo && pOptionInfo->NaeRyuk != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(387), pOptionInfo->NaeRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare ����
	if( pRareOptionInfo && pRareOptionInfo->NaeRyuk )
		RareState = pRareOptionInfo->NaeRyuk;
	else
		RareState = 0;
	if( pInfo->NaeRyuk != 0)
	{
		if(pOptionInfo && pOptionInfo->NaeRyuk != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(387), pInfo->NaeRyuk + RareState, pOptionInfo->NaeRyuk);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(387), pInfo->NaeRyuk + RareState);

		if(pRareOptionInfo && pRareOptionInfo->NaeRyuk)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(RareState && pOptionInfo && pOptionInfo->NaeRyuk != 0)
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(387), RareState, pOptionInfo->NaeRyuk);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->NaeRyuk != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(387), pOptionInfo->NaeRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(pRareOptionInfo && pRareOptionInfo->NaeRyuk != 0)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(387), pRareOptionInfo->NaeRyuk);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

	/*SW050920
	if( pInfo->Shield != 0 )
	{
		if(pOptionInfo && pOptionInfo->Shield != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(388), pInfo->Shield, pOptionInfo->Shield);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(388), pInfo->Shield );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );		
	}
	else
	{
		if(pOptionInfo && pOptionInfo->Shield != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(388), pOptionInfo->Shield);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare ȣ�Ű���
	if( pRareOptionInfo && pRareOptionInfo->Shield )
		RareState = pRareOptionInfo->Shield;
	else
		RareState = 0;
	if( pInfo->Shield != 0)
	{
		if(pOptionInfo && pOptionInfo->Shield != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(388), pInfo->Shield + RareState, pOptionInfo->Shield);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(388), pInfo->Shield + RareState);

		if(pRareOptionInfo && pRareOptionInfo->Shield)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(RareState && pOptionInfo && pOptionInfo->Shield != 0)
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(388), RareState, pOptionInfo->Shield);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->Shield != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(388), pOptionInfo->Shield);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(pRareOptionInfo && pRareOptionInfo->Shield != 0)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(388), pRareOptionInfo->Shield);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

#ifdef _JAPAN_LOCAL_
	//265, 268, 269, 267, 266
	int Arr_ReviseAttr[5] = { 1, 4, 5, 3, 2 };	//ȭ �� �� �� �� ����
	for(int i = ATTR_FIRE; i <= ATTR_MAX; ++i )
	{
		attrvalue = pInfo->AttrRegist.GetElement_Val(Arr_ReviseAttr[i-1]);
		if(pOptionInfo)
			attroptvalue = pOptionInfo->AttrRegist.GetElement_Val(Arr_ReviseAttr[i-1]);

		if(pRareOptionInfo)
		{
			attrRareOptValue = pRareOptionInfo->AttrRegist.GetElement_Val(Arr_ReviseAttr[i-1]);
		}
		else attrRareOptValue = 0;

		if( (int)(attrvalue) != 0 )
		{
			if( (int)(attroptvalue) != 0 )
			{
				sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(265+i-1), (int)(attrvalue + attrRareOptValue), (int)(attroptvalue));
			}
			else
			{
				sprintf(line, "%s +%d", CHATMGR->GetChatMsg(265+i-1), (int)(attrvalue+ attrRareOptValue) );
			}
			if(pRareOptionInfo && attrRareOptValue)
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else
		{
			if( (int)(attroptvalue) != 0 )
			{
				if( attrRareOptValue )
				{
					sprintf( line, "%s +%d (+%d)", CHATMGR->GetChatMsg(265+i-1), (int)(attrRareOptValue), (int)(attroptvalue) );
				}
				else
				{
					sprintf( line, "%s (+%d)", CHATMGR->GetChatMsg(265+i-1), (int)(attroptvalue) );
				}
			}
			else
			{
				if(attroptvalue)	continue;	//�Ҽ��� ���ϴ� ǥ������ �ʴ´�.

				if( attrRareOptValue )	//����������̴�.
					sprintf( line, "%s +%d", CHATMGR->GetChatMsg(265+i-1), (int)(attrRareOptValue) );
			}
			if(attrRareOptValue)
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else if(attroptvalue)
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );

		}
	}
#else

	//SW050920 �����߰� �Ӽ�����
	for(int i = ATTR_FIRE; i <= ATTR_MAX; ++i )
	{
		attrvalue = 100 * pInfo->AttrRegist.GetElement_Val(i);
		if(pOptionInfo)
			attroptvalue = 100 * pOptionInfo->AttrRegist.GetElement_Val(i);
		if(pRareOptionInfo)
		{
			//attrRareOptAttackValue = 100 * pRareOptionInfo->AttrAttack.GetElement_Val(i);
			attrRareOptValue = 100 * pRareOptionInfo->AttrRegist.GetElement_Val(i);
		}
		else attrRareOptValue = 0;
		
		if( (int)(attrvalue) != 0 )	//�⺻�ɼǿ� �Ӽ������� �ִ�.
		{
			if( (int)(attroptvalue) != 0 )	//��ȭ�������̴�.
				sprintf( line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(265+i-1), (int)(attrvalue + attrRareOptValue), (int)(attroptvalue));
			else
				sprintf( line, "%s +%d%%", CHATMGR->GetChatMsg(265+i-1), (int)(attrvalue + attrRareOptValue) );

		if(pRareOptionInfo && attrRareOptValue)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		
		}
		else	//�⺻�ɼǿ� �Ӽ������� ����.
		{
			if( (int)(attroptvalue) != 0 )	//��ȭ�������̴�.
			{
				if( attrRareOptValue )	//����������̴�.
					sprintf( line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(265+i-1), (int)(attrRareOptValue), (int)(attroptvalue) );
				else
					sprintf( line, "%s (+%d%%)", CHATMGR->GetChatMsg(265+i-1), (int)(attroptvalue) );
			}
			else
			{
				if( attrRareOptValue )	//����������̴�.
					sprintf( line, "%s +%d%%", CHATMGR->GetChatMsg(265+i-1), (int)(attrRareOptValue) );
			}
			if(attrRareOptValue)
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else if(attroptvalue)
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

/*SW050920
	attrvalue = 100 * pInfo->AttrRegist.GetElement_Val(ATTR_FIRE);
	if(pOptionInfo)
		attroptvalue = 100 * pOptionInfo->AttrRegist.GetElement_Val(ATTR_FIRE);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(265), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(265), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(265), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = 100 * pInfo->AttrRegist.GetElement_Val(ATTR_WATER);
	if(pOptionInfo)
		attroptvalue = 100 * pOptionInfo->AttrRegist.GetElement_Val(ATTR_WATER);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(266), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(266), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(266), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	attrvalue = 100 * pInfo->AttrRegist.GetElement_Val(ATTR_TREE);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrRegist.GetElement_Val(ATTR_TREE);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(267), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(267), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(267), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = 100 * pInfo->AttrRegist.GetElement_Val(ATTR_IRON);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrRegist.GetElement_Val(ATTR_IRON);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(268), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(268), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(268), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = 100 * pInfo->AttrRegist.GetElement_Val(ATTR_EARTH);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrRegist.GetElement_Val(ATTR_EARTH);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(269), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(269), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(269), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/

#endif

	switch( pInfo->RangeType )
	{
	case 0:
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(272) );
		break;
	case 1:
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(273) );
		break;
	}

//SW050920 �������ݷ�
	WORD RareOptPhyAttack;
	if( pRareOptionInfo && pRareOptionInfo->PhyAttack )
		RareOptPhyAttack = pRareOptionInfo->PhyAttack;
	else
		RareOptPhyAttack = 0;
	
	if( pInfo->MeleeAttackMin || pInfo->MeleeAttackMax )
	{
		if( pInfo->ItemKind != eEQUIP_ITEM_ARMLET )	//���� ���� �������� ���
		{
			if( pInfo->MeleeAttackMin == pInfo->MeleeAttackMax )
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )	//���� ���� ��ȭ ������
					sprintf( line, "%s %d (+%d)", CHATMGR->GetChatMsg(389), (pInfo->MeleeAttackMin + RareOptPhyAttack), pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d", CHATMGR->GetChatMsg(389), (pInfo->MeleeAttackMin + RareOptPhyAttack) );
			}
			else
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )
					sprintf( line, "%s %d ~ %d (+%d)", CHATMGR->GetChatMsg(389), (pInfo->MeleeAttackMin + RareOptPhyAttack), (pInfo->MeleeAttackMax + RareOptPhyAttack), pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d ~ %d", CHATMGR->GetChatMsg(389), (pInfo->MeleeAttackMin + RareOptPhyAttack), (pInfo->MeleeAttackMax + RareOptPhyAttack) );
			}

			if( pRareOptionInfo && pRareOptionInfo->PhyAttack )
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else
				pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
		else //���� ���� �ɼ��� ������ �ٴ´�. �̰� ó�� ����.
		{
			if( pInfo->MeleeAttackMin == pInfo->MeleeAttackMax )
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )
					sprintf( line, "%s %d%% (+%d)", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d%%", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin );
			}
			else
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )
					sprintf( line, "%s %d%% ~ %d%% (+%d)", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pInfo->MeleeAttackMax, pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d%% ~ %d%%", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pInfo->MeleeAttackMax );
			}
			pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
	}
/*SW050920
	if(pInfo->MeleeAttackMin != 0  || pInfo->MeleeAttackMax != 0 )
	{
		if( pInfo->ItemKind != eEQUIP_ITEM_ARMLET )
		{
			if( pInfo->MeleeAttackMin == pInfo->MeleeAttackMax )
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s %d ( +%d )", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s %d", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin);
			}
			else
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s %d ~ %d ( +%d )", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pInfo->MeleeAttackMax, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s %d ~ %d", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pInfo->MeleeAttackMax);
			}

			pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
		else
		{
			if( pInfo->MeleeAttackMin == pInfo->MeleeAttackMax )
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s +%d%% ( +%d )", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin);
			}
			else
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s +%d%% ~ +%d%% ( +%d )", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pInfo->MeleeAttackMax, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s +%d%% ~ +%d%%", CHATMGR->GetChatMsg(389), pInfo->MeleeAttackMin, pInfo->MeleeAttackMax);
			}
			
			pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
	}*/

	if(pOptionInfo && pOptionInfo->CriticalPercent != 0)
	{
		sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(390), pOptionInfo->CriticalPercent );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	
//SW050920 ���Ÿ� ����
//�ߺ�
//	if( !pRareOptionInfo )
//		RareOptPhyAttack = 0;
//	else
//		RareOptPhyAttack = pRareOptionInfo->PhyAttack;
	
	if( pInfo->RangeAttackMin || pInfo->RangeAttackMax )
	{
		if( pInfo->ItemKind != eEQUIP_ITEM_ARMLET)	//���� ���� �������� ���
		{
			if( pInfo->RangeAttackMin == pInfo->RangeAttackMax )
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )	//���� ���� ��ȭ ������
					sprintf( line, "%s %d (+%d)", CHATMGR->GetChatMsg(391), (pInfo->RangeAttackMin + RareOptPhyAttack), pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d", CHATMGR->GetChatMsg(391), (pInfo->RangeAttackMin + RareOptPhyAttack) );
			}
			else
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )
					sprintf( line, "%s %d ~ %d (+%d)", CHATMGR->GetChatMsg(391), (pInfo->RangeAttackMin + RareOptPhyAttack), (pInfo->RangeAttackMax + RareOptPhyAttack), pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d ~ %d", CHATMGR->GetChatMsg(391), (pInfo->RangeAttackMin + RareOptPhyAttack), (pInfo->RangeAttackMax + RareOptPhyAttack) );
			}

			if( pRareOptionInfo && pRareOptionInfo->PhyAttack )
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else
				pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
		else //���� ���� �ɼ��� ������ �ٴ´�. �̰� ó�� ����.
		{
			if( pInfo->RangeAttackMin == pInfo->RangeAttackMax )
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )
					sprintf( line, "%s %d%% (+%d)", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMax, pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d%%", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMax );
			}
			else
			{
				if( pOptionInfo && pOptionInfo->PhyAttack )
					sprintf( line, "%s %d%% ~ %d%% (+%d)", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pInfo->RangeAttackMax, pOptionInfo->PhyAttack );
				else
					sprintf( line, "%s %d%% ~ %d%%", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pInfo->RangeAttackMax );
			}
			pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
	}
/*SW050920
	if( pInfo->RangeAttackMin  != 0  || pInfo->RangeAttackMax != 0 )
	{
		if( pInfo->ItemKind != eEQUIP_ITEM_ARMLET )
		{
			if( pInfo->RangeAttackMin == pInfo->RangeAttackMax )
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s %d", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin);
			}
			else
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s %d ~ %d ( +%d )", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pInfo->RangeAttackMax, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s %d ~ %d", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pInfo->RangeAttackMax);
			}

			pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
		else
		{
			if( pInfo->RangeAttackMin == pInfo->RangeAttackMax )
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s +%d%% ( +%d )", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin);
			}
			else
			{
				if(pOptionInfo && pOptionInfo->PhyAttack != 0)
					sprintf(line, "%s +%d%% ~ +%d%% ( +%d )", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pInfo->RangeAttackMax, pOptionInfo->PhyAttack);
				else
					sprintf(line, "%s +%d%% ~ +%d%%", CHATMGR->GetChatMsg(391), pInfo->RangeAttackMin, pInfo->RangeAttackMax);
			}

			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
//	else if( pOptionInfo && pOptionInfo->PhyAttack != 0 )
//	{
//		sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(391), pOptionInfo->PhyAttack);
//		pIcon->AddToolTipLine( line, TTTC_NUMBER );
//	}

	/*SW050920
	if( pInfo->MeleeAttackMin == 0  && pInfo->MeleeAttackMax == 0 && pInfo->RangeAttackMin  == 0  && pInfo->RangeAttackMax == 0)
	if( pOptionInfo && pOptionInfo->PhyAttack != 0 )
	{
		sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(681), pOptionInfo->PhyAttack);
		pIcon->AddToolTipLine( line, TTTC_NUMBER );
	}*/
	//SW050920 Rare
	if( pInfo->MeleeAttackMin == 0  && pInfo->MeleeAttackMax == 0 && pInfo->RangeAttackMin  == 0  && pInfo->RangeAttackMax == 0)
	{
		if( pRareOptionInfo && pRareOptionInfo->PhyAttack && pOptionInfo && pOptionInfo->PhyAttack )
		{
			sprintf(line, "%s +%d (+&d)", CHATMGR->GetChatMsg(681), pRareOptionInfo->PhyAttack, pOptionInfo->PhyAttack );
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if( pOptionInfo && pOptionInfo->PhyAttack != 0 )
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(681), pOptionInfo->PhyAttack);
			pIcon->AddToolTipLine( line, TTTC_NUMBER );
		}
		else if( pRareOptionInfo && pRareOptionInfo->PhyAttack )
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(681), pRareOptionInfo->PhyAttack );
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}

#ifdef _JAPAN_LOCAL_
	for(int i = ATTR_FIRE; i <= ATTR_MAX; ++i )
	{
		attrvalue = pInfo->AttrAttack.GetElement_Val(Arr_ReviseAttr[i-1]);
		if(pOptionInfo)
			attroptvalue = pOptionInfo->AttrAttack.GetElement_Val(Arr_ReviseAttr[i-1]);

		if(pRareOptionInfo)
		{
			attrRareOptValue = pRareOptionInfo->AttrAttack.GetElement_Val(Arr_ReviseAttr[i-1]);
		}
		else attrRareOptValue = 0;

		if( (int)(attrvalue) != 0 )
		{
			if( (int)(attroptvalue) != 0 )
			{
				sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(392+i-1), (int)(attrvalue + attrRareOptValue), (int)(attroptvalue));
			}
			else
			{
				sprintf(line, "%s +%d", CHATMGR->GetChatMsg(392+i-1), (int)(attrvalue+ attrRareOptValue) );
			}
			if(pRareOptionInfo && attrRareOptValue)
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else
		{
			if( (int)(attroptvalue) != 0 )
			{
				if( attrRareOptValue )
				{
					sprintf( line, "%s +%d (+%d)", CHATMGR->GetChatMsg(392+i-1), (int)(attrRareOptValue), (int)(attroptvalue) );
				}
				else
				{
					sprintf( line, "%s (+%d)", CHATMGR->GetChatMsg(392+i-1), (int)(attroptvalue) );
				}
			}
			else
			{
				if(attroptvalue)	continue;	//�Ҽ��� ���ϴ� ǥ������ �ʴ´�.

				if( attrRareOptValue )	//����������̴�.
					sprintf( line, "%s +%d", CHATMGR->GetChatMsg(392+i-1), (int)(attrRareOptValue) );
			}
			if(attrRareOptValue)
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else if(attroptvalue)
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );

		}
	}
	/*
	attrvalue = pInfo->AttrAttack.GetElement_Val(ATTR_FIRE);
	if(pOptionInfo)
	attroptvalue = pOptionInfo->AttrAttack.GetElement_Val(ATTR_FIRE);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(392), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(392), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(392), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = pInfo->AttrAttack.GetElement_Val(ATTR_WATER);
	if(pOptionInfo)
	attroptvalue = pOptionInfo->AttrAttack.GetElement_Val(ATTR_WATER);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(393), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(393), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(393), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	attrvalue = pInfo->AttrAttack.GetElement_Val(ATTR_TREE);
	if(pOptionInfo)
	attroptvalue = pOptionInfo->AttrAttack.GetElement_Val(ATTR_TREE);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(394), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(394), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(394), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = pInfo->AttrAttack.GetElement_Val(ATTR_IRON);
	if(pOptionInfo)
	attroptvalue = pOptionInfo->AttrAttack.GetElement_Val(ATTR_IRON);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(395), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(395), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(395), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = pInfo->AttrAttack.GetElement_Val(ATTR_EARTH);
	if(pOptionInfo)
	attroptvalue = pOptionInfo->AttrAttack.GetElement_Val(ATTR_EARTH);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(396), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(396), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(396), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
#else

/*SW050920
	attrvalue = 100 * pInfo->AttrAttack.GetElement_Val(ATTR_FIRE);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrAttack.GetElement_Val(ATTR_FIRE);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(392), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(392), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(392), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = 100 * pInfo->AttrAttack.GetElement_Val(ATTR_WATER);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrAttack.GetElement_Val(ATTR_WATER);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(393), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(393), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(393), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	attrvalue = 100 * pInfo->AttrAttack.GetElement_Val(ATTR_TREE);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrAttack.GetElement_Val(ATTR_TREE);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(394), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(394), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(394), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = 100 * pInfo->AttrAttack.GetElement_Val(ATTR_IRON);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrAttack.GetElement_Val(ATTR_IRON);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(395), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(395), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(395), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	attrvalue = 100 * pInfo->AttrAttack.GetElement_Val(ATTR_EARTH);
	if(pOptionInfo)
	attroptvalue = 100 * pOptionInfo->AttrAttack.GetElement_Val(ATTR_EARTH);
	if( (int)(attrvalue) != 0 )
	{
		if( (int)(attroptvalue) != 0 )
			sprintf(line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(396), (int)(attrvalue), (int)(attroptvalue));
		else
			sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(396), (int)(attrvalue) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( (int)(attroptvalue) != 0 )
		{
			sprintf(line, "%s (+%d%%)", CHATMGR->GetChatMsg(396), (int)(attroptvalue) );
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/

	//SW050920 �����߰� �Ӽ�����
	for(int j = ATTR_FIRE; j <= ATTR_MAX; ++j )
	{
		attrvalue = 100 * pInfo->AttrAttack.GetElement_Val(j);
		if(pOptionInfo)
			attroptvalue = 100 * pOptionInfo->AttrAttack.GetElement_Val(j);
		if(pRareOptionInfo)
		{
			//attrRareOptAttackValue = 100 * pRareOptionInfo->AttrAttack.GetElement_Val(i);
			attrRareOptValue = 100 * pRareOptionInfo->AttrAttack.GetElement_Val(j);
		}
		else attrRareOptValue = 0;
		
		if( (int)(attrvalue) != 0 )	//�⺻�ɼǿ� �Ӽ������� �ִ�.
		{
			if( (int)(attroptvalue) != 0 )	//��ȭ�������̴�.
				sprintf( line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(392+j-1), (int)(attrvalue + attrRareOptValue), (int)(attroptvalue));
			else
				sprintf( line, "%s +%d%%", CHATMGR->GetChatMsg(392+j-1), (int)(attrvalue + attrRareOptValue) );
			
			if(pRareOptionInfo && attrRareOptValue)
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else	//�⺻�ɼǿ� �Ӽ������� ����.
		{
			if( (int)(attroptvalue) != 0 )	//��ȭ�������̴�.
			{
				if( attrRareOptValue )	//����������̴�.
					sprintf( line, "%s +%d%% (+%d%%)", CHATMGR->GetChatMsg(392+j-1), (int)(attrRareOptValue), (int)(attroptvalue) );
				else
					sprintf( line, "%s (+%d%%)", CHATMGR->GetChatMsg(392+j-1), (int)(attroptvalue) );
			}
			else
			{
				if( attrRareOptValue )	//����������̴�.
					sprintf( line, "%s +%d%%", CHATMGR->GetChatMsg(392+j-1), (int)(attrRareOptValue) );
			}
			if(attrRareOptValue)
				pIcon->AddToolTipLine( line, TTTC_RAREITEM );
			else if(attroptvalue)
				pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	/*if( (int)(pInfo->AttrAttack.GetElement_Val(ATTR_FIRE) * 100.0f) != 0 )
	{
		sprintf(line, "ȭ�Ӽ����ݷ� +%d%%", (int)(pInfo->AttrAttack.GetElement_Val(ATTR_FIRE) * 100.0f) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrAttack.GetElement_Val(ATTR_WATER) * 100.0f) != 0 )
	{
		sprintf(line, "���Ӽ����ݷ� +%d%%", (int)(pInfo->AttrAttack.GetElement_Val(ATTR_WATER) * 100.0f) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrAttack.GetElement_Val(ATTR_TREE) * 100.0f) != 0 )
	{
		sprintf(line, "��Ӽ����ݷ� +%d%%", (int)(pInfo->AttrAttack.GetElement_Val(ATTR_TREE) * 100.0f) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	if( (int)(pInfo->AttrAttack.GetElement_Val(ATTR_IRON) * 100.0f) != 0 )
	{
		sprintf(line, "�ݼӼ����ݷ� +%d%%", (int)(pInfo->AttrAttack.GetElement_Val(ATTR_IRON) * 100.0f) );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}*/

#endif

	/*SW050920
	if( pInfo->PhyDef != 0 )
	{
		if(pOptionInfo && pOptionInfo->PhyDefense != 0)
			sprintf(line, "%s +%d ( +%d )", CHATMGR->GetChatMsg(397), pInfo->PhyDef, pOptionInfo->PhyDefense);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(397), pInfo->PhyDef);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if(pOptionInfo && pOptionInfo->PhyDefense != 0)
		{
			sprintf(line, "%s ( +%d )", CHATMGR->GetChatMsg(397), pOptionInfo->PhyDefense);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}*/
	//SW050920 Rare ��������
	WORD RareOptPhyDef;
	if( pRareOptionInfo && pRareOptionInfo->PhyDefense )
		RareOptPhyDef = pRareOptionInfo->PhyDefense;
	else
		RareOptPhyDef = 0;
	if( pInfo->PhyDef != 0 )	//�⺻ ������ ������ �ִ�.
	{
		if(pOptionInfo && pOptionInfo->PhyDefense != 0)
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(397), pInfo->PhyDef + RareOptPhyDef, pOptionInfo->PhyDefense);
		else
			sprintf(line, "%s +%d", CHATMGR->GetChatMsg(397), pInfo->PhyDef + RareOptPhyDef);
		
		if(pRareOptionInfo && RareOptPhyDef)
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		else
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	else
	{
		if( RareOptPhyDef && pOptionInfo && pOptionInfo->PhyDefense )
		{
			sprintf(line, "%s +%d (+%d)", CHATMGR->GetChatMsg(397), RareOptPhyDef, pOptionInfo->PhyDefense);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
		else if(pOptionInfo && pOptionInfo->PhyDefense != 0)
		{
			sprintf(line, "%s (+%d)", CHATMGR->GetChatMsg(397), pOptionInfo->PhyDefense);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		else if(RareOptPhyDef)
		{
			sprintf(line, "%s +%d ", CHATMGR->GetChatMsg(397), RareOptPhyDef);
			pIcon->AddToolTipLine( line, TTTC_RAREITEM );
		}
	}
	
	if( pInfo->Plus_Value != 0 )
	{
		sprintf(line, CHATMGR->GetChatMsg(398), pInfo->ItemName, pInfo->Plus_Value);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}

	if( pInfo->AllPlus_Value != 0 && pInfo->AllPlus_Kind != 0 )
	{
		char buf[32] = {0,};
		switch( pInfo->AllPlus_Kind )
		{
		case 1: strcpy(buf, CHATMGR->GetChatMsg(406) ); break;
		case 2: strcpy(buf, CHATMGR->GetChatMsg(407) ); break;
		case 3: strcpy(buf, CHATMGR->GetChatMsg(408) ); break;
		case 4: strcpy(buf, CHATMGR->GetChatMsg(409) ); break;
		case 5: strcpy(buf, CHATMGR->GetChatMsg(410) ); break;
		case 6: strcpy(buf, CHATMGR->GetChatMsg(411) ); break;
		case 7: strcpy(buf, CHATMGR->GetChatMsg(412) ); break;
		case 8: strcpy(buf, CHATMGR->GetChatMsg(413) ); break;
		case 9: strcpy(buf, CHATMGR->GetChatMsg(414) ); break;
		case 10: strcpy(buf, CHATMGR->GetChatMsg(415) ); break;
		case 11: strcpy(buf, CHATMGR->GetChatMsg(416) ); break;
		}
		sprintf(line, CHATMGR->GetChatMsg(417), buf, pInfo->AllPlus_Value );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}

	// 060911 KKR �����ѷ��ֱ�
	if(pInfo->ItemKind & eSHOP_ITEM)
	{
		AddShopItemToolTip( (cIcon*)pIcon, pInfo );
	}

	if(pInfo->wSetItemKind != 0)
	{
		AddSetItemToolTip( (cIcon*)pIcon, pInfo); // 2007. 6. 8. CBH - ��Ʈ������ ����ó�� �߰�
	}

	// magi82 - UniqueItem(070628)
	if(pInfo->ItemKind == eEQUIP_ITEM_UNIQUE)
	{
		AddUniqueItemToolTip( (cIcon*)pIcon, pInfo);
	}

	AddItemDescriptionToolTip( pIcon, pInfo->ItemTooltipIdx );
}

void CItemManager::SetItemToolTipForStage( cIcon * pIcon, ITEM_INFO * pInfo )
{
	BYTE bStage = HERO->GetStage();
	switch( bStage )
	{
	case eStage_Normal:
		{
			switch( pInfo->LimitJob )
			{
			case eItemStage_ChangeStage:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(888), TTTC_LIMIT );	break;
			case eItemStage_ChangeStage2:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1354), TTTC_LIMIT);	break;
			case eItemStage_OverHwa:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(894), TTTC_LIMIT );	break;
			case eItemStage_Hwa:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(895), TTTC_LIMIT );	break;
			case eItemStage_Hyun:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(896), TTTC_LIMIT );	break;
			case eItemStage_OverGeuk:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(897), TTTC_LIMIT );	break;
			case eItemStage_Geuk:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(898), TTTC_LIMIT );	break;
			case eItemStage_Tal:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(899), TTTC_LIMIT );	break;
			}
		}
		break;
	case eStage_Hwa:
		{
			switch( pInfo->LimitJob )
			{
			case eItemStage_ChangeStage:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(888), TTTC_FREELIMIT );	break;
			case eItemStage_ChangeStage2:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1354), TTTC_LIMIT);	break;
			case eItemStage_OverHwa:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(894), TTTC_FREELIMIT );	break;
			case eItemStage_Hwa:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(895), TTTC_FREELIMIT );	break;
			case eItemStage_Hyun:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(896), TTTC_LIMIT );	break;
			case eItemStage_OverGeuk:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(897), TTTC_LIMIT );	break;
			case eItemStage_Geuk:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(898), TTTC_LIMIT );	break;
			case eItemStage_Tal:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(899), TTTC_LIMIT );	break;
			}
		}
		break;
	case eStage_Hyun:
		{
			switch( pInfo->LimitJob )
			{
			case eItemStage_ChangeStage:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(888), TTTC_FREELIMIT );	break;
			case eItemStage_ChangeStage2:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1354), TTTC_FREELIMIT);	break;
			case eItemStage_OverHwa:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(894), TTTC_FREELIMIT );	break;
			case eItemStage_Hwa:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(895), TTTC_FREELIMIT );	break;
			case eItemStage_Hyun:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(896), TTTC_FREELIMIT );	break;
			case eItemStage_OverGeuk:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(897), TTTC_LIMIT );	break;
			case eItemStage_Geuk:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(898), TTTC_LIMIT );	break;
			case eItemStage_Tal:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(899), TTTC_LIMIT );	break;
			}
		}
		break;
	case eStage_Geuk:
		{
			switch( pInfo->LimitJob )
			{
			case eItemStage_ChangeStage:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(888), TTTC_FREELIMIT );	break;
			case eItemStage_ChangeStage2:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1354), TTTC_LIMIT);	break;
			case eItemStage_OverHwa:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(894), TTTC_LIMIT );	break;
			case eItemStage_Hwa:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(895), TTTC_LIMIT );	break;
			case eItemStage_Hyun:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(896), TTTC_LIMIT );	break;
			case eItemStage_OverGeuk:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(897), TTTC_FREELIMIT );	break;
			case eItemStage_Geuk:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(898), TTTC_FREELIMIT );	break;
			case eItemStage_Tal:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(899), TTTC_LIMIT );	break;
			}
		}
		break;
	case eStage_Tal:
		{
			switch( pInfo->LimitJob )
			{
			case eItemStage_ChangeStage:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(888), TTTC_FREELIMIT );	break;
			case eItemStage_ChangeStage2:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1354), TTTC_FREELIMIT);	break;
			case eItemStage_OverHwa:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(894), TTTC_LIMIT );	break;
			case eItemStage_Hwa:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(895), TTTC_LIMIT );	break;
			case eItemStage_Hyun:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(896), TTTC_LIMIT );	break;
			case eItemStage_OverGeuk:	pIcon->AddToolTipLine( CHATMGR->GetChatMsg(897), TTTC_FREELIMIT );	break;
			case eItemStage_Geuk:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(898), TTTC_FREELIMIT );	break;
			case eItemStage_Tal:		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(899), TTTC_FREELIMIT );	break;
			}
		}
		break;
	}
}

void CItemManager::SetPetSummonItemToolTip( cIcon * pIcon, ITEM_INFO * pInfo, DWORD dwItemDBIdx /*= 0*/ )
{
	char line[128];

	DWORD	ItemDBIdx = 0;

	const ITEMBASE* pBaseInfo = ((CItem*)pIcon)->GetItemBaseInfo();

	if(dwItemDBIdx)
	{
		ItemDBIdx = dwItemDBIdx;
	}
	else
	{
		//const ITEMBASE* pBaseInfo = ((CItem*)pIcon)->GetItemBaseInfo();
		ItemDBIdx = pBaseInfo->dwDBIdx;
	}

	PET_TOTALINFO* pPetInfo = PETMGR->GetPetInfo(ItemDBIdx);
	BASE_PET_LIST* pList = NULL;
	if(pPetInfo)
		pList = GAMERESRCMNGR->GetPetListInfo(pPetInfo->PetKind);
	PET_BUFF_LIST* pBuffList = NULL;

	if( pPetInfo )
	{
		//������ ǥ��
		if( !pPetInfo->bAlive )
		{
			DWORD RedFilter = RGBA_MAKE(255,10,10,255);
			SetIconColorFilter(pIcon,RedFilter);
		}
		//�ܰ�
		wsprintf( line, CHATMGR->GetChatMsg(1244), pPetInfo->PetGrade );
		pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
		//��밡�ɷ���
//		wsprintf( line, CHATMGR->GetChatMsg(1281), pInfo->LimitLevel );
//		pIcon->AddToolTipLine( line, TTTC_QUESTITEM	);
		
		if(0 < pPetInfo->PetFriendly && pPetInfo->PetFriendly <= 100000)
		{
			wsprintf( line, CHATMGR->GetChatMsg(1245), 1 );
		}
		else
		{
			wsprintf( line, CHATMGR->GetChatMsg(1245), pPetInfo->PetFriendly/100000 );
		}
	}
	else
	{
		wsprintf( line, CHATMGR->GetChatMsg(1244), PET_DEFAULT_GRADE );
		pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
		DWORD friendly = GAMERESRCMNGR->GetPetRule()->DefaultFriendship;
		wsprintf( line, CHATMGR->GetChatMsg(1245), friendly/100000);
	}
	pIcon->AddToolTipLine( line, TTTC_QUESTITEM );

	//����ĭ ����ǥ�� 06/02/25
	//wsprintf( line, CHATMGR->GetChatMsg(1265), pPetInfo->PetGrade );
	/*
	if(pList)
	{
		wsprintf( line, CHATMGR->GetChatMsg(1265), PETMGR->GetPetEquipItemMax(ItemDBIdx) );
		pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
	}*/

	if(pList)
	{
		wsprintf( line, CHATMGR->GetChatMsg(1266), pList->InventoryTapNum[pPetInfo->PetGrade-1]);
		pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
	}
	else
		return;

//#define MAX_PET_BUFF_NUM	3
	if(pList->BuffList[0])
	{
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1267), TTTC_QUESTITEM );

		char BuffName[32] = {0,};
		for( int i = 0; i < MAX_PET_BUFF_NUM; ++i )
		{
			WORD BuffIdx = pList->BuffList[pPetInfo->PetGrade-1][i];

			if(0 == BuffIdx) continue;

			pBuffList = GAMERESRCMNGR->GetPetBuffInfo(BuffIdx);

			if(pBuffList && pBuffList->BuffValueData)
			{
				//char* pBuffName = NULL;
				if( pBuffList->BuffKind == ePB_Demage_Percent )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1270), pBuffList->BuffValueData );
				}
				else if( pBuffList->BuffKind == ePB_Dodge )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1271), pBuffList->BuffSuccessProb );
				}
				else if( pBuffList->BuffKind == ePB_MasterAllStatUp )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1286), pBuffList->BuffValueData );
				}
				else if( pBuffList->BuffKind == ePB_Item_DoubleChance )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1305), pBuffList->BuffSuccessProb );
				}
				else if( pBuffList->BuffKind == ePB_NoForeAtkMonster )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1350) );
				}
				else if( pBuffList->BuffKind == ePB_ReduceCriticalDmg )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1388), pBuffList->BuffValueData );
				}
				else if( pBuffList->BuffKind == ePB_MasterAllStatRound )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1389) );
				}
				else if( pBuffList->BuffKind == ePB_Item_RareProbUp )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1476) );
				}
				else if( pBuffList->BuffKind == ePB_MussangTimeIncrease )
				{
					wsprintf( BuffName, CHATMGR->GetChatMsg(1477) );
				}

			}

			/*
			if( 0 == i )
			{
				wsprintf( line, CHATMGR->GetChatMsg(1267), BuffName);
			}
			else
			{
				wsprintf( line, CHATMGR->GetChatMsg(1268), BuffName);
			}*/
			wsprintf( line, CHATMGR->GetChatMsg(1268), BuffName );

			pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
		}
	}

	//��밡�ɷ���
	wsprintf( line, CHATMGR->GetChatMsg(1281), pInfo->LimitLevel );
	pIcon->AddToolTipLine( line, TTTC_QUESTITEM	);
	/*
	if(pList->SkillIdx[pPetInfo->PetGrade-1])
	{
		char* pSName = NULL;
		CSkillInfo* pSkillInfo = SKILLMGR->GetSkillInfo(pList->SkillIdx[pPetInfo->PetGrade-1]);
		if(pSkillInfo)
		{
			pSName = pSkillInfo->GetSkillName();
		}
		wsprintf( line, CHATMGR->GetChatMsg(1269), pSName);

		pIcon->AddToolTipLine( line, TTTC_QUESTITEM );
	}*/

	if( pInfo->ItemKind == eSHOP_ITEM_PET )
	{
		if( pInfo->ItemType == 11 )
		{
			if( !(((CItem*)pIcon)->GetItemBaseInfo()->ItemParam & ITEM_PARAM_SEAL) )
				sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1465));
			else
				sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1464));
			pIcon->AddToolTipLine( "", TTTC_EXTRAATTR	);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR	);
		}
	}
}

void CItemManager::SetTitanEquipItemToolTip(cIcon * pIcon, ITEM_INFO * pInfo)
{
	char line[128];

	CItem* pItem = (CItem*)pIcon;

	//���� ����
	if( pInfo->LimitLevel != 0 )
	{		
		sprintf(line, CHATMGR->GetChatMsg(238), pInfo->LimitLevel);
		if( HERO->GetCharacterTotalInfo()->Level < pInfo->LimitLevel )
			pIcon->AddToolTipLine( line, TTTC_LIMIT );
		else
			pIcon->AddToolTipLine( line, TTTC_FREELIMIT );				
	}
	//Ÿ��ź ž�� ��밡�� ����
	if(HERO->InTitan() == TRUE)
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1655), TTTC_FREELIMIT );
	else
		pIcon->AddToolTipLine( CHATMGR->GetChatMsg(1655), TTTC_LIMIT );
	
	pIcon->AddToolTipLine("");

	//���� ����
	//�ٰŸ� �ּҰ���
	if(pInfo->MeleeAttackMin > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1521), pInfo->MeleeAttackMin);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//�ٰŸ� �ִ����
	if(pInfo->MeleeAttackMax > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1522), pInfo->MeleeAttackMax);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//���Ÿ� �ּҰ���
	if(pInfo->RangeAttackMin > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1523), pInfo->RangeAttackMin);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//���Ÿ� �ִ����
	if(pInfo->RangeAttackMax > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1524), pInfo->RangeAttackMax);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//�ϰ�
	if(pInfo->CriticalPercent > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1525), pInfo->CriticalPercent);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//�����Ÿ�
	if(pInfo->MugongNum > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1526), pInfo->MugongNum);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//����
	if(pInfo->PhyDef > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1527), pInfo->PhyDef);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//EP
	if(pInfo->Life > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1528), pInfo->Life);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	/*
	if(pInfo->NaeRyuk > 0)
	{
		ZeroMemory(&line, sizeof(line));
		wsprintf( line, CHATMGR->GetChatMsg(1529), pInfo->NaeRyuk);
		pIcon->AddToolTipLine( line );
	}
	*/
	//���� ���ݷ�
	if(pInfo->AttrAttack.GetElement_Val(ATTR_FIRE) > 0)
	{
		ZeroMemory(&line, sizeof(line));
		sprintf( line, CHATMGR->GetChatMsg(1667), pInfo->AttrAttack.GetElement_Val(ATTR_FIRE));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//ȭ�Ӽ� ����
	if(pInfo->AttrRegist.GetElement_Val(ATTR_FIRE) > 0)
	{
		ZeroMemory(&line, sizeof(line));
		sprintf( line, CHATMGR->GetChatMsg(1530), pInfo->AttrRegist.GetElement_Val(ATTR_FIRE));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//���Ӽ� ����
	if(pInfo->AttrRegist.GetElement_Val(ATTR_WATER) > 0)
	{
		ZeroMemory(&line, sizeof(line));
		sprintf( line, CHATMGR->GetChatMsg(1531), pInfo->AttrRegist.GetElement_Val(ATTR_WATER));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//��Ӽ� ����
	if(pInfo->AttrRegist.GetElement_Val(ATTR_TREE) > 0)
	{
		ZeroMemory(&line, sizeof(line));
		sprintf( line, CHATMGR->GetChatMsg(1532), pInfo->AttrRegist.GetElement_Val(ATTR_TREE));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//�ݼӼ� ����
	if(pInfo->AttrRegist.GetElement_Val(ATTR_IRON) > 0)
	{
		ZeroMemory(&line, sizeof(line));
		sprintf( line, CHATMGR->GetChatMsg(1533), pInfo->AttrRegist.GetElement_Val(ATTR_IRON));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//��Ӽ� ����
	if(pInfo->AttrRegist.GetElement_Val(ATTR_EARTH) > 0)
	{
		ZeroMemory(&line, sizeof(line));
		sprintf( line, CHATMGR->GetChatMsg(1534), pInfo->AttrRegist.GetElement_Val(ATTR_EARTH));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	//��������
	if(pInfo->WeaponType > 0)
	{		
		switch(pInfo->WeaponType)
		{
		case WP_GUM:	//��
			sprintf(line, CHATMGR->GetChatMsg(1661));
			break;
		case WP_GWUN:	//��
			sprintf(line, CHATMGR->GetChatMsg(1663));
			break;
		case WP_DO:		//��
			sprintf(line, CHATMGR->GetChatMsg(1662));
			break;
		case WP_CHANG:	//â
			sprintf(line, CHATMGR->GetChatMsg(1664));
			break;
		case WP_GUNG:	//��
			sprintf(line, CHATMGR->GetChatMsg(1665));
			break;
		case WP_AMGI:	//�ϱ�
			sprintf(line, CHATMGR->GetChatMsg(1666));
			break;
		}
		pIcon->AddToolTipLine( line );
	}	
}

void CItemManager::AddItemDescriptionToolTip( cWindow* pWindow, WORD wIndex, DWORD dwTxtColor, int nSpaceLine )
{
	TOOLTIP_TEXT* pTooltipText = m_ItemToolTipTextList.GetData( wIndex );

	if( pTooltipText )
	{
		for( int i = 0 ; i < nSpaceLine ; ++i )
			pWindow->AddToolTipLine("");
	}

	while( pTooltipText )
	{
		pWindow->AddToolTipLine( pTooltipText->strToolTip, dwTxtColor );
		pTooltipText = pTooltipText->pNext;
	}
}

void CItemManager::ItemDelete(CItem* pItem)
{
	m_ItemHash.Remove(pItem->GetDBIdx());	
	//ICFreeIndex(&m_IconIndexCreator,pItem->GetID()-START_ITEMTCON_ID);
	m_IconIndexCreator.ReleaseIndex(pItem->GetID());
	
//KES EXCHANGE 031002
	if( pItem->GetLinkItem() )
	{
		GAMEIN->GetExchangeDialog()->DelItem( 0, pItem->GetLinkItem()->GetPosition() );
	}
//

//	WINDOWMGR->DeleteWindowForID(pItem->GetID());	//KES ������������ÿ�. 031128
	WINDOWMGR->AddListDestroyWindow( pItem );
}

void CItemManager::Release()
{
	CItem* pItem;
	m_ItemHash.SetPositionHead();
	while(pItem = m_ItemHash.GetData())
	{
		ItemDelete(pItem);
		//ICFreeIndex(&m_IconIndexCreator,pItem->GetID()-START_ITEMTCON_ID);		
	}
	m_ItemHash.RemoveAll();

	ITEM_OPTION_INFO * pOInfo = NULL;
	m_ItemOptionList.SetPositionHead();
	while(pOInfo = m_ItemOptionList.GetData())
		delete pOInfo;
	m_ItemOptionList.RemoveAll();

	ITEM_RARE_OPTION_INFO* pRareInfo = NULL;
	m_ItemRareOptionList.SetPositionHead();
	while(pRareInfo = m_ItemRareOptionList.GetData())
		delete pRareInfo;
	m_ItemRareOptionList.RemoveAll();

	// ������ �޴������� Delete���ֱ� ������ ���� ���ʿ䰡 ����

	// �����츦 ������ �ʱ� ������ ���� delete�� ���־�� �Ѵ�. KES 040316
	GAMEIN->GetInventoryDialog()->ReleaseInventory();

	m_nItemUseCount = 0;
}

CItem* CItemManager::GetItem(DWORD dwDBIdx)
{
	CItem* pItem;
	pItem = m_ItemHash.GetData(dwDBIdx);
	return pItem;
}

void CItemManager::CombineItem( MSG_ITEM_COMBINE_ACK * pMsg )
{	
	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);
	WORD ToTableIdx = GetTableIdxForAbsPos(pMsg->ToPos);

	CItem * pFromItem = GetItemofTable(FromTableIdx, pMsg->FromPos);
	CItem * pToItem = GetItemofTable(ToTableIdx, pMsg->ToPos);

	if( !pFromItem || !pToItem )
	{
		ASSERTMSG(0, "Item Combine Failed : pToItem=NULL or pFromItem=NULL");
		return;
	}
	if( ( pFromItem->GetItemIdx() != pToItem->GetItemIdx() ) || ( pFromItem->GetItemIdx() != pMsg->wItemIdx) )
//	if( ( pFromItem->GetItemIdx() != pToItem->GetItemIdx() ) || ( pFromItem->GetItemIdx() != pMsg->CombineItemIdx ) )
	{
		ASSERTMSG(0, "Item Combine Failed : FromItemIdx != ToItemIdx");
		return;
	}
	pFromItem->SetDurability( pMsg->FromDur );
	pToItem->SetDurability( pMsg->ToDur );
	
	if( pMsg->FromDur == 0 )
	{
		DeleteItemofTable(FromTableIdx, pFromItem->GetPosition());
	//	DeleteItem( pFromItem->GetPosition() );
	}

	if( pMsg->ToDur == 0 )
	{
		DeleteItemofTable(FromTableIdx, pFromItem->GetPosition());
	//	DeleteItem( pToItem->GetPosition() );
	}
}

void CItemManager::MoveItem( MSG_ITEM_MOVE_ACK * pMsg )
{
 	BYTE tableIdx = GetTableIdxForAbsPos(pMsg->ToPos);
	if(tableIdx == eItemTable_Pyoguk)
		MoveItemToPyoguk(pMsg);
	else if(tableIdx == eItemTable_GuildWarehouse)
		MoveItemToGuild(pMsg);
	else if(tableIdx == eItemTable_Shop)
		MoveItemToShop(pMsg);
	else if(tableIdx == eItemTable_ShopInven)
		MoveItemToShopInven(pMsg);
	else if(tableIdx == eItemTable_PetInven || tableIdx == eItemTable_PetWeared )
	{
		MoveItemToPetInven(pMsg);
	}
	// magi82 - Titan(070207)
	else if(tableIdx == eItemTable_Titan)
		MoveItemToTitanInven(pMsg);
	// magi82 - Titan(070228)
	else if(tableIdx == eItemTable_TitanShopItem)
		MoveItemToTitanShopInven(pMsg);
	else if(tableIdx == eItemTable_Inventory || tableIdx == eItemTable_Weared)
	{
		CInventoryExDialog * pInven = GAMEIN->GetInventoryDialog();
		CPyogukDialog* pPyoguk = GAMEIN->GetPyogukDialog();
		CGuildWarehouseDialog* pGuild = GAMEIN->GetGuildWarehouseDlg();
		CItemShopDialog* pShop = GAMEIN->GetItemShopDialog();
		CPetInventoryDlg* pPetInven = GAMEIN->GetPetInventoryDialog();		
		CTitanInventoryDlg* pTitanInven = GAMEIN->GetTitanInventoryDlg();
		
		CItem * FromItem = NULL;
		CItem * ToItem = NULL;
		
		pInven->DeleteItem(pMsg->ToPos, &ToItem);

		WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);
		if(FromTableIdx == eItemTable_Pyoguk)
		{
			pPyoguk->DeleteItem( pMsg->FromPos, &FromItem );
		}
		else if(FromTableIdx == eItemTable_GuildWarehouse)
		{
			pGuild->DeleteItem( pMsg->FromPos, &FromItem );
		}
		else if(FromTableIdx == eItemTable_Shop)
		{
			// �Ŀ� �������� �κ����� ������ �߰�

			pShop->DeleteItem( pMsg->FromPos, &FromItem );
		}
		else if(FromTableIdx == eItemTable_PetInven)
		{
			pPetInven->DeleteItem( pMsg->FromPos, &FromItem );
		}
		// magi82(070208)
		else if(FromTableIdx == eItemTable_Titan)
		{
            pTitanInven->DeleteItem( pMsg->FromPos, &FromItem );

			HERO->GetTitanAppearInfo()->WearedItemIdx[pMsg->FromPos-TP_TITANWEAR_START] = 0;
			
			// ������� Ÿ��ź ������ ����Ʈ�� ����. // ��ȯ�̳� ��ų ���� üũ.
			TITANMGR->RemoveTitanUsingEquipItemList(FromItem->GetDBIdx());
		}
		else 
			pInven->DeleteItem( pMsg->FromPos , &FromItem );

		if(ToItem)
		{
			ToItem->SetPosition( pMsg->FromPos );

			ITEM_INFO* pinfo = GetItemInfo( ToItem->GetItemBaseInfo()->wIconIdx );
			if( pinfo )
			{
				POSTYPE pos = 0;
				if( (pinfo->ItemKind & eEQUIP_ITEM) && pInven->GetBlankPositionRestrictRef( pos ) )
				{
					if( TP_SHOPINVEN_START <= pMsg->FromPos && pMsg->FromPos < TP_SHOPINVEN_END )
					{
						FromTableIdx = eItemTable_Inventory;                        
						ToItem->SetPosition( pos );						
					}
				}
				else if( (pinfo->ItemKind == eSHOP_ITEM_EQUIP) && pInven->GetShopInven()->GetBlankPositionRestrictRef( pos ) )
				{
					if( TP_INVENTORY_START <= pMsg->FromPos && pMsg->FromPos < TP_INVENTORY_END )
					{
						FromTableIdx = eItemTable_ShopInven;
						ToItem->SetPosition( pos );
					}
				}
			}
			if(FromTableIdx == eItemTable_Pyoguk)
			{
				if(ToItem->GetQuickPosition())
					QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
				pPyoguk->AddItem(ToItem);
			}
			else if(FromTableIdx == eItemTable_GuildWarehouse)
			{
				if(ToItem->GetQuickPosition())
					QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
				pGuild->AddItem(ToItem);
				QUICKMGR->RefreshQickItem();
			}
			else if(FromTableIdx == eItemTable_PetInven)
			{
				if(POSTYPE Pos = ToItem->GetQuickPosition())
					QUICKMGR->RemQuickItem(Pos);
				pPetInven->AddItem(ToItem);
			}
			else if(FromTableIdx == eItemTable_Titan)
			{
				if(POSTYPE Pos = ToItem->GetQuickPosition())
					QUICKMGR->RemQuickItem(Pos);
				pTitanInven->AddItem(ToItem);

				if( HERO->IsTitanPreView() )
				{
					HERO->GetTitanPreViewInfo()->WearedItemIdx[pMsg->FromPos-TP_TITANWEAR_START] = ToItem->GetItemIdx();
				}
			}
			else
				pInven->AddItem(ToItem);
		}
		
		if(FromItem == 0)
		{
			ASSERT(0);
			return;
		}
		FromItem->SetPosition(pMsg->ToPos);
		pInven->AddItem(FromItem); 
		QUICKMGR->RefreshQickItem();

		APPEARANCEMGR->InitAppearance( HERO );
		//2007. 10. 5. CBH - Ÿ��ź ž�½� ��� ��ü���� EP DLG ����
		if(HERO->InTitan() == TRUE)
		{			
			GAMEIN->GetTitanGuageDlg()->SetLife(TITANMGR->GetCurRidingTitan()->GetTitanTotalInfo()->Fuel);
		}

		if(((ToItem != NULL) && (ToItem->GetItemInfo()->wSetItemKind != 0)) || (FromItem->GetItemInfo()->wSetItemKind != 0))
		{
			RefreshAllItem();		
		}		
	}
}

void CItemManager::MoveItemToGuild( MSG_ITEM_MOVE_ACK* pMsg )
{	
	CGuildWarehouseDialog * pGuildDlg	= GAMEIN->GetGuildWarehouseDlg();
	CInventoryExDialog * pInvenDlg = GAMEIN->GetInventoryDialog();
		
	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);
	WORD ToTableIdx = GetTableIdxForAbsPos(pMsg->ToPos);

	CItem * FromItem = NULL;
	CItem * ToItem = NULL;

	pGuildDlg->DeleteItem(pMsg->ToPos , &ToItem);

	if(FromTableIdx == eItemTable_GuildWarehouse)
	{
		pGuildDlg->DeleteItem( pMsg->FromPos, &FromItem);
	}
	else if(FromTableIdx == eItemTable_Inventory || FromTableIdx == eItemTable_Weared)
	{
		pInvenDlg->DeleteItem( pMsg->FromPos, &FromItem);
	}
	else 
		ASSERT(0);
	
	if(ToItem)
	{
		ToItem->SetPosition( pMsg->FromPos );
		if(FromTableIdx == eItemTable_Inventory)
			pInvenDlg->AddItem(ToItem);
		else
			pGuildDlg->AddItem(ToItem);
	}

	ReLinkQuickPosition(FromItem);
	if(FromItem->GetQuickPosition())
		QUICKMGR->RemQuickItem(FromItem->GetQuickPosition());
	
	ASSERT(FromItem);
	FromItem->SetPosition(pMsg->ToPos);
	FromItem->SetQuickPosition(0);
	pGuildDlg->AddItem(FromItem);
	
	QUICKMGR->RefreshQickItem();

	SetDisableDialog(FALSE, eItemTable_Inventory);
	SetDisableDialog(FALSE, eItemTable_Pyoguk);
	SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
}

void CItemManager::MoveItemToPyoguk(MSG_ITEM_MOVE_ACK* pMsg)
{
	CInventoryExDialog * pInven = GAMEIN->GetInventoryDialog();
	CPyogukDialog* pPyoguk = GAMEIN->GetPyogukDialog();
	
	CItem * FromItem = NULL;
	CItem * ToItem = NULL;

	pPyoguk->DeleteItem(pMsg->ToPos, &ToItem);
	
	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);

	if(FromTableIdx == eItemTable_Inventory || FromTableIdx == eItemTable_Weared)
	{
		pInven->DeleteItem( pMsg->FromPos, &FromItem );				
	}
		
	else if(FromTableIdx == eItemTable_Pyoguk)
	{
		pPyoguk->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else
	{
		ASSERT(0);
		return;
	}
		
	if(ToItem)
	{
		ToItem->SetPosition( pMsg->FromPos );
		if(FromTableIdx == eItemTable_Inventory || FromTableIdx == eItemTable_Weared)
			pInven->AddItem(ToItem);
		else
			pPyoguk->AddItem(ToItem);
		if(ToItem->GetQuickPosition())
			QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
	}

	ReLinkQuickPosition(FromItem);
	
	if(FromItem->GetQuickPosition())
		QUICKMGR->RemQuickItem(FromItem->GetQuickPosition());
	ASSERT(FromItem);
	FromItem->SetPosition(pMsg->ToPos);
	FromItem->SetQuickPosition(0);
	pPyoguk->AddItem(FromItem);
	
	QUICKMGR->RefreshQickItem();
}



void CItemManager::MoveItemToShop(MSG_ITEM_MOVE_ACK* pMsg)
{
	CInventoryExDialog * pInven = GAMEIN->GetInventoryDialog();
	CItemShopDialog* pShop = GAMEIN->GetItemShopDialog();
	
	CItem * FromItem = NULL;
	CItem * ToItem = NULL;

	pShop->DeleteItem(pMsg->ToPos, &ToItem);

	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);

	if( FromTableIdx == eItemTable_Shop )
	{
		pShop->DeleteItem( pMsg->FromPos, &FromItem );				
	}
	else if( FromTableIdx == eItemTable_ShopInven )
	{
		pInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
		
	if(ToItem)
	{
		ToItem->SetPosition( pMsg->FromPos );		

		if( FromTableIdx == eItemTable_Shop )		
			pShop->AddItem(ToItem);	
		else if( FromTableIdx == eItemTable_ShopInven )		
			pInven->AddItem(ToItem);
		if(ToItem->GetQuickPosition())
			QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
	}

	ReLinkQuickPosition(FromItem);
	if(FromItem->GetQuickPosition())
		QUICKMGR->RemQuickItem(FromItem->GetQuickPosition());

	ASSERT(FromItem);


	FromItem->SetPosition(pMsg->ToPos);
	FromItem->SetQuickPosition(0);
	pShop->AddItem(FromItem);

	QUICKMGR->RefreshQickItem();
}



void CItemManager::MoveItemToShopInven(MSG_ITEM_MOVE_ACK* pMsg)
{
	CInventoryExDialog * pInven = GAMEIN->GetInventoryDialog();
	CItemShopDialog* pShop = GAMEIN->GetItemShopDialog();	
	CPetInventoryDlg* pPetInven = GAMEIN->GetPetInventoryDialog();
	CTitanInventoryDlg* pTitanShopInven = GAMEIN->GetTitanInventoryDlg();	// magi82 - Titan(070228)

	CItem * FromItem = NULL;
	CItem * ToItem = NULL;

	pInven->DeleteItem(pMsg->ToPos, &ToItem);

	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);

	if(FromTableIdx == eItemTable_Shop)
	{
		pShop->DeleteItem( pMsg->FromPos, &FromItem );
	}	
	else if(FromTableIdx == eItemTable_ShopInven)
	{
		pInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else if(FromTableIdx == eItemTable_Weared )
	{
		pInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else if(FromTableIdx == eItemTable_PetWeared)
	{
		pPetInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	// magi82 - Titan(070228)
	else if(FromTableIdx == eItemTable_TitanShopItem)
	{
		pTitanShopInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	
	if(ToItem)
	{
		// ShopInven�������� �̵��Ѵ�.
		ToItem->SetPosition( pMsg->FromPos );

		if(FromTableIdx == eItemTable_Shop)		
			pShop->AddItem(ToItem);
		else if(FromTableIdx == eItemTable_ShopInven)		
			pInven->AddItem(ToItem);
		if(ToItem->GetQuickPosition())
			QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
	}

	ReLinkQuickPosition(FromItem);
	if(FromItem->GetQuickPosition())
		QUICKMGR->RemQuickItem(FromItem->GetQuickPosition());

	ASSERT(FromItem);

	FromItem->SetPosition(pMsg->ToPos);
	FromItem->SetQuickPosition(0);
	pInven->AddItem(FromItem);	

	QUICKMGR->RefreshQickItem();
}

void CItemManager::MoveItemToPetInven(MSG_ITEM_MOVE_ACK* pMsg)
{
	CInventoryExDialog* pInven = GAMEIN->GetInventoryDialog();
	CPetInventoryDlg*	pPetInven = GAMEIN->GetPetInventoryDialog();

	CItem* FromItem	= NULL;
	CItem* ToItem = NULL;

	pPetInven->DeleteItem(pMsg->ToPos, &ToItem);

	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);

	if(FromTableIdx == eItemTable_Inventory || FromTableIdx == eItemTable_Weared || FromTableIdx == eItemTable_ShopInven )
	{
		pInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else if(FromTableIdx == eItemTable_PetInven || FromTableIdx == eItemTable_PetWeared)
	{
		pPetInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else
	{
		ASSERT(0);
		return;
	}

	if(ToItem)
	{
		ToItem->SetPosition( pMsg->FromPos );
		if(FromTableIdx == eItemTable_Inventory || FromTableIdx == eItemTable_Weared || FromTableIdx == eItemTable_ShopInven)
		{
			pInven->AddItem(ToItem);
		}
		else if(FromTableIdx == eItemTable_PetInven || FromTableIdx == eItemTable_PetWeared)
		{
			pPetInven->AddItem(ToItem);
		}
		if(ToItem->GetQuickPosition())
		{
			ASSERT(0);	// ���κ��� �� ���� �ȵ�. ��ȹ.
			QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
			ToItem->SetQuickPosition(0);
		}
	}

	//ReLinkQuickPosition(FromItem);

	ASSERT(FromItem);
	if(FromItem->GetQuickPosition())
	{
		QUICKMGR->RemQuickItem(FromItem->GetQuickPosition());
	}
	FromItem->SetPosition(pMsg->ToPos);
	FromItem->SetQuickPosition(0);
	pPetInven->AddItem(FromItem);

	QUICKMGR->RefreshQickItem();
}

void CItemManager::MoveItemToTitanInven(MSG_ITEM_MOVE_ACK* pMsg)
{
	CInventoryExDialog* pInven = GAMEIN->GetInventoryDialog();	
	CTitanInventoryDlg*	pTitanInven = GAMEIN->GetTitanInventoryDlg();

	CItem* FromItem	= NULL;
	CItem* ToItem = NULL;

	pTitanInven->DeleteItem(pMsg->ToPos, &ToItem);

	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);

	// magi82 - Titan(070208)
	if( FromTableIdx == eItemTable_Inventory || FromTableIdx == eItemTable_Weared || FromTableIdx == eItemTable_ShopInven )
	{
		pInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else if( FromTableIdx == eItemTable_Titan )
	{
		pTitanInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else
	{
		ASSERT(0);
		return;
	}

	if(ToItem)
	{
		TITANMGR->RemoveTitanUsingEquipItemList(ToItem->GetDBIdx());
		ToItem->SetPosition( pMsg->FromPos );
		if(FromTableIdx == eItemTable_Inventory || FromTableIdx == eItemTable_Weared || FromTableIdx == eItemTable_ShopInven)
		{
			pInven->AddItem(ToItem);
		}
		if(ToItem->GetQuickPosition())
		{
			ASSERT(0);
			QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
			ToItem->SetQuickPosition(0);
		}
	}

	//ReLinkQuickPosition(FromItem);

	ASSERT(FromItem);
	if(FromItem->GetQuickPosition())
	{
		QUICKMGR->RemQuickItem(FromItem->GetQuickPosition());
	}
	FromItem->SetPosition(pMsg->ToPos);
	FromItem->SetQuickPosition(0);
	pTitanInven->AddItem(FromItem);

	HERO->GetTitanAppearInfo()->WearedItemIdx[pMsg->ToPos-TP_TITANWEAR_START] = pMsg->wFromItemIdx;
	APPEARANCEMGR->InitAppearance( HERO );	
	GAMEIN->GetTitanGuageDlg()->SetLife(TITANMGR->GetCurRidingTitan()->GetTitanTotalInfo()->Fuel); //2007. 10. 5. CBH - Ÿ��ź ��� ����� EP DLG ����
	
	// ������� Ÿ��ź ������ ����Ʈ�� �߰�. // ��ȯ�̳� ��ų ���� üũ.
	TITAN_ENDURANCE_ITEMINFO* pInfo = TITANMGR->GetTitanEnduranceInfo(FromItem->GetDBIdx());
	if(pInfo)
		TITANMGR->AddTitanUsingEquipItemList(pInfo);
	else
	{
		ASSERT(0);
	}

	QUICKMGR->RefreshQickItem();
}

void CItemManager::MoveItemToTitanShopInven(MSG_ITEM_MOVE_ACK* pMsg)
{
	CInventoryExDialog * pInven = GAMEIN->GetInventoryDialog();
	CTitanInventoryDlg* pTitanInven = GAMEIN->GetTitanInventoryDlg();

	CItem * FromItem = NULL;
	CItem * ToItem = NULL;

	pTitanInven->DeleteItem(pMsg->ToPos, &ToItem);

	WORD FromTableIdx = GetTableIdxForAbsPos(pMsg->FromPos);

	if( FromTableIdx == eItemTable_TitanShopItem )
	{
		pTitanInven->DeleteItem( pMsg->FromPos, &FromItem );
	}
	else if( FromTableIdx == eItemTable_ShopInven )
	{
		pInven->DeleteItem( pMsg->FromPos, &FromItem );
	}

	if(ToItem)
	{
		ToItem->SetPosition( pMsg->FromPos );		

		if( FromTableIdx == eItemTable_Shop )		
			pTitanInven->AddItem(ToItem);	
		else if( FromTableIdx == eItemTable_ShopInven )		
			pInven->AddItem(ToItem);
		else if( FromTableIdx == eItemTable_TitanShopItem )
			pTitanInven->AddItem(ToItem);

		if(ToItem->GetQuickPosition())
			QUICKMGR->RemQuickItem(ToItem->GetQuickPosition());
	}

	ReLinkQuickPosition(FromItem);
	if(FromItem->GetQuickPosition())
		QUICKMGR->RemQuickItem(FromItem->GetQuickPosition());

	ASSERT(FromItem);


	FromItem->SetPosition(pMsg->ToPos);
	FromItem->SetQuickPosition(0);
	pTitanInven->AddItem(FromItem);

	QUICKMGR->RefreshQickItem();
}

void CItemManager::DeleteItem( POSTYPE absPos, CItem** ppItemOut, ITEM_OPTION_INFO * pItemOptionInfoOut, ITEM_RARE_OPTION_INFO* pItemRareOptionInfoOut, WORD RemainOptionFlag )
{
	BYTE TableIdx = GetTableIdxForAbsPos(absPos);
	if(pItemOptionInfoOut)
	{
		memset(pItemOptionInfoOut, 0, sizeof(ITEM_OPTION_INFO));
	}
	//SW050920 Rare
	if(pItemRareOptionInfoOut)
	{
		memset(pItemRareOptionInfoOut, 0, sizeof(ITEM_RARE_OPTION_INFO));
	}
		
	switch(TableIdx)
	{
	case eItemTable_Inventory:
	case eItemTable_Weared:
	case eItemTable_ShopInven:
		{
			CInventoryExDialog * pInven = GAMEIN->GetInventoryDialog();
			pInven->DeleteItem( absPos, ppItemOut );

		}break;
	case eItemTable_Pyoguk:
		{
			CPyogukDialog* pPyoguk = GAMEIN->GetPyogukDialog();
			pPyoguk->DeleteItem( absPos, ppItemOut );
			
		}break;
	case eItemTable_GuildWarehouse:
		{
			CGuildWarehouseDialog* pGuild = GAMEIN->GetGuildWarehouseDlg();
			pGuild->DeleteItem( absPos, ppItemOut );
		}break;
	case eItemTable_PetInven:
		{
			CPetInventoryDlg* pPetInven = GAMEIN->GetPetInventoryDialog();
			pPetInven->DeleteItem( absPos, ppItemOut );
		}break;
	}

	if((*ppItemOut))
	{
		/*
		if((*ppItemOut)->GetQuickPosition())
		{
			QUICKMGR->RemQuickItem((*ppItemOut)->GetQuickPosition());
		}
		
		if((*ppItemOut)->GetDurability() != 0 && !IsDupItem((*ppItemOut)->GetItemIdx()))
		{
			RemoveItemOption((*ppItemOut)->GetDurability(), pItemOptionInfoOut);
		}
		ItemDelete( (*ppItemOut) );*/

		if(IsOptionItem((*ppItemOut)->GetItemIdx(), (*ppItemOut)->GetDurability()) && !(RemainOptionFlag & eRemainReinforceOption))
		{
			RemoveItemOption((*ppItemOut)->GetDurability(), pItemOptionInfoOut);
		}

		//SW050920 Rare
		if(IsRareOptionItem((*ppItemOut)->GetItemIdx(), (*ppItemOut)->GetRareness()) && !(RemainOptionFlag & eRemainRareOption))
		{
			RemoveItemRareOption((*ppItemOut)->GetRareness(), pItemRareOptionInfoOut);
		}

		if( IsTitanCallItem((*ppItemOut)->GetItemIdx()) )
		{
			TITANMGR->RemoveTitanInfo((*ppItemOut)->GetDBIdx());
		}

		if( IsTitanEquipItem((*ppItemOut)->GetItemIdx()) )
		{
			TITANMGR->RemoveTitanEquipInfo((*ppItemOut)->GetDBIdx());
		}

		else
		{
		/*	if(IsDupItem((*ppItemOut)->GetItemIdx()))
			{
				if((*ppItemOut)->GetQuickPosition())
				{				
					CQuickItem * pQItem = GAMEIN->GetQuickDialog()->GetQuickItem((*ppItemOut)->GetQuickPosition());
					DURTYPE dur = GAMEIN->GetInventoryDialog()->GetTotalItemDurability((*ppItemOut)->GetItemIdx());
					if(dur)
					{
						// ������ Quickposition�ٲ���ٰ� ����
						// �ٸ� Link�� ������ �ش�.	
						// ������ ����
						//pQItem->SetLinkItem(GAMEIN->GetInventoryDialog()->GetItemLike((*ppItemOut)->GetItemIdx()));
						GAMEIN->GetQuickDialog()->FakeAddItem((*ppItemOut)->GetQuickPosition(), GAMEIN->GetInventoryDialog()->GetItemLike((*ppItemOut)->GetItemIdx()));
					}
				}
			}	
*/
//			ReLinkQuickPosition((*ppItemOut));
		}

		if((*ppItemOut)->GetQuickPosition())
		{
			QUICKMGR->RemQuickItem((*ppItemOut)->GetQuickPosition());
		}
		ItemDelete( (*ppItemOut) );


	}
		
}

void CItemManager::DeleteItemofTable(WORD TableIdx, POSTYPE absPos, ITEM_OPTION_INFO * pItemOptionInfoOut, ITEM_RARE_OPTION_INFO* pItemRareOptionInfoOut/* = NULL */ )
{
	CItem * pItemOut = NULL;
	if(pItemOptionInfoOut)
	{
		memset(pItemOptionInfoOut, 0, sizeof(ITEM_OPTION_INFO));
	}
	if(pItemRareOptionInfoOut)
	{
		memset(pItemRareOptionInfoOut, 0, sizeof(ITEM_RARE_OPTION_INFO));
	}

	switch(TableIdx)
	{
	case eItemTable_Inventory:
	case eItemTable_Weared:
	case eItemTable_ShopInven:
		{
			CInventoryExDialog * pDlg = GAMEIN->GetInventoryDialog();
			if( !pDlg->DeleteItem( absPos, &pItemOut ) )
			{
				ASSERTMSG(0, "Item Delete Failed : DeleteItem return FALSE");
				return;
			}

			if(pItemOut)
			{
				if( pItemOut->GetItemInfo()->ItemType == 11 )
				{
					if( !(pItemOut->GetItemParam() & ITEM_PARAM_SEAL) )
						RemoveUsedItemInfo( pItemOut->GetItemIdx(), pItemOut->GetItemBaseInfo()->dwDBIdx );
					if( pItemOut->GetItemInfo()->ItemIdx == eIncantation_MemoryMoveExtend || 
						pItemOut->GetItemInfo()->ItemIdx == eIncantation_MemoryMoveExtend7 ||
						pItemOut->GetItemInfo()->ItemIdx == eIncantation_MemoryMove2 ||
						pItemOut->GetItemInfo()->ItemIdx == eIncantation_MemoryMoveExtend30 )
						HERO->CalcShopItemOption( eIncantation_MemoryMoveExtend, FALSE );
				}
			}
		}
		break;
	case eItemTable_Pyoguk:
		{
			CPyogukDialog * pDlg = GAMEIN->GetPyogukDialog();
			if( !pDlg->DeleteItem( absPos, &pItemOut ) )
			{
				ASSERTMSG(0, "Item Delete Failed : DeleteItem return FALSE");
				return;
			}
		}
		break;
	case eItemTable_GuildWarehouse:
		{
			CGuildWarehouseDialog * pDlg = GAMEIN->GetGuildWarehouseDlg();
			if( !pDlg->DeleteItem( absPos, &pItemOut ) )
			{
				ASSERTMSG(0, "Item Delete Failed : DeleteItem return FALSE");
				return;
			}			
		}
		break;
	case eItemTable_Shop:
		{
			CItemShopDialog* pDlg = GAMEIN->GetItemShopDialog();
			if( !pDlg->DeleteItem( absPos, &pItemOut ) )
			{
				ASSERTMSG(0, "Item Delete Failed : DeleteItem return FALSE");
				return;
			}

			if(pItemOut)
			{
				/*if( pItemOut->GetItemInfo()->ItemKind == eSHOP_ITEM_MAKEUP || 
					pItemOut->GetItemInfo()->ItemKind == eSHOP_ITEM_DECORATION )*/
				if( pItemOut->GetItemInfo()->ItemType == 11 )
				{
					RemoveUsedItemInfo( pItemOut->GetItemInfo()->ItemIdx, pItemOut->GetItemBaseInfo()->dwDBIdx );
				}
			}
		}
		break;
	case eItemTable_PetWeared:
	case eItemTable_PetInven:
		{
			CPetInventoryDlg* pDlg = GAMEIN->GetPetInventoryDialog();
			if( !pDlg->DeleteItem( absPos, &pItemOut ) )
			{
				ASSERTMSG(0, "Item Delete Failed : Pet Inven DeleteItem return FALSE");
				return;
			}
		}
		break;
	case eItemTable_Titan:	// magi82 - Titan(070503)
	case eItemTable_TitanShopItem:	// magi82(26)
		{			
			CTitanInventoryDlg* pDlg = GAMEIN->GetTitanInventoryDlg();
			if( !pDlg->DeleteItem( absPos, &pItemOut ) )
			{
				ASSERTMSG(0, "Item Delete Failed : Titan Inven DeleteItem return FALSE");
				return;
			}

			if( HERO->IsTitanPreView() )
			{
				HERO->GetTitanPreViewInfo()->WearedItemIdx[absPos-TP_TITANWEAR_START] = 0;
				APPEARANCEMGR->InitAppearance( HERO );
			}
		}
		break;
	}
	if(pItemOut)
	{
		if(IsOptionItem(pItemOut->GetItemIdx(), pItemOut->GetDurability()))
		{
			RemoveItemOption(pItemOut->GetDurability(), pItemOptionInfoOut);
		}
		if(IsRareOptionItem(pItemOut->GetItemIdx(), pItemOut->GetRareness()))
		{
			RemoveItemRareOption(pItemOut->GetRareness(), pItemRareOptionInfoOut);
		}
		if(IsPetSummonItem(pItemOut->GetItemIdx()))
		{
			PETMGR->RemovePetInfo(pItemOut->GetDBIdx());
		}
		else
		{
	/*		if(IsDupItem(pItemOut->GetItemIdx()))
			{
				if(pItemOut->GetQuickPosition())
				{				
					CQuickItem * pQItem = GAMEIN->GetQuickDialog()->GetQuickItem(pItemOut->GetQuickPosition());
					DURTYPE dur = GAMEIN->GetInventoryDialog()->GetTotalItemDurability(pItemOut->GetItemIdx());
					if(dur)
					{
						// ������ Quickposition�ٲ���ٰ� ����
						// �ٸ� Link�� ������ �ش�.	
						GAMEIN->GetQuickDialog()->FakeAddItem(pItemOut->GetQuickPosition(), GAMEIN->GetInventoryDialog()->GetItemLike(pItemOut->GetItemIdx()));
					}
				}
			}	
*/			ReLinkQuickPosition(pItemOut);
		}
		// magi82 - Titan(070802)
		if(pItemOut->GetItemKind() == eTITAN_ITEM_PAPER)
		{
			TITANMGR->RemoveTitanInfo(pItemOut->GetDBIdx());
            
			// ���� ���� �������� �����������̰� ������������ DB�ε����� ������� �������� DB�ε����� ���ٸ�
			// ����� Ÿ��ź ������ ������̹Ƿ� ������� �������� DB�ε����� 0���� �ʱ�ȭ�ؾ��Ѵ�
			// ��ϵ� Ÿ��ź ������ ���� �������� ��� �ʿ䰡 ���� �ڵ��̱� ������ Ȥ�� �𸣴� ���ܵ�..
			if(pItemOut->GetDBIdx() != TITANMGR->GetRegistedTitanItemDBIdx())
			{
				TITANMGR->SetRegistedTitanItemDBIdx(0);
			}
		}

		if(pItemOut->GetQuickPosition())
		{
			QUICKMGR->RemQuickItem(pItemOut->GetQuickPosition());
		}
		ItemDelete( pItemOut );

		// magi82 - Titan(070503) Ÿ��ź �κ�â���� �����ؾ���
		if(pItemOut->GetItemKind() & eTITAN_EQUIPITEM)
		{
			// ���� Ÿ��ź ����â�� ������������ �����ٸ�..
			if( TableIdx == eItemTable_Titan )
			{
				TITANMGR->RemoveTitanUsingEquipItemList(pItemOut->GetDBIdx());
			}

			TITANMGR->RemoveTitanEquipInfo(pItemOut->GetDBIdx());
		}		
	}

	if( (pItemOut != NULL) && (pItemOut->GetItemInfo()->wSetItemKind != 0) )
	{
		RefreshAllItem();		
	}
}

void CItemManager::InitItemRareOption(ITEM_RARE_OPTION_INFO* pInfo, WORD num)
{
	for(int i = 0; i < num; ++ i)
	{
		AddItemRareOption(&pInfo[i]);
	}
}
	
void CItemManager::AddItemRareOption(ITEM_RARE_OPTION_INFO* pInfo)
{
	if( m_ItemRareOptionList.GetData(pInfo->dwRareOptionIdx) )
	{
		return;
	}
	ITEM_RARE_OPTION_INFO* pNewInfo = new ITEM_RARE_OPTION_INFO;
	memcpy(pNewInfo, pInfo, sizeof(ITEM_RARE_OPTION_INFO));
	m_ItemRareOptionList.Add(pNewInfo, pNewInfo->dwRareOptionIdx);
}

ITEM_RARE_OPTION_INFO* CItemManager::GetItemRareOption(DWORD dwRareOptionIdx)
{
	ITEM_RARE_OPTION_INFO* pInfo = m_ItemRareOptionList.GetData(dwRareOptionIdx);
	if(pInfo)
		return pInfo;

	return NULL;
	//������ �ְ���..
//	static ITEM_RARE_OPTION_INFO NULLINFO;
//	memset(&NULLINFO,0,sizeof(ITEM_RARE_OPTION_INFO));
//	return &NULLINFO;
}

void CItemManager::RemoveItemRareOption(DWORD dwRareOptionIdx, ITEM_RARE_OPTION_INFO* pItemRareOptionInfoOut)
{
	ITEM_RARE_OPTION_INFO* pNewInfo = m_ItemRareOptionList.GetData(dwRareOptionIdx);

	if( pNewInfo == NULL)
	{
		ASSERT(0);
		return;
	}
	
	if(pItemRareOptionInfoOut)
		*pItemRareOptionInfoOut = *pNewInfo;
	SAFE_DELETE( pNewInfo );
	m_ItemRareOptionList.Remove(dwRareOptionIdx);
}

void CItemManager::RemoveItemOption(DWORD dwOptionIdx, ITEM_OPTION_INFO * pItemOptionInfoOut)
{
//	ASSERT(m_ItemOptionList.GetData(dwOptionIdx));

	ITEM_OPTION_INFO * pNewInfo = m_ItemOptionList.GetData(dwOptionIdx);

	if( pNewInfo == NULL )
	{
		ASSERT(0);		
		return;
	}

	if(pItemOptionInfoOut)
		*pItemOptionInfoOut = *pNewInfo;
	SAFE_DELETE( pNewInfo );
	m_ItemOptionList.Remove(dwOptionIdx);
}

void CItemManager::AddItemOption(ITEM_OPTION_INFO * pInfo)
{
	if( m_ItemOptionList.GetData(pInfo->dwOptionIdx) )
	{
		return;
	}
	ITEM_OPTION_INFO * pNewInfo = new ITEM_OPTION_INFO;
	memcpy(pNewInfo, pInfo, sizeof(ITEM_OPTION_INFO));
	m_ItemOptionList.Add(pNewInfo, pNewInfo->dwOptionIdx);
}
ITEM_OPTION_INFO * CItemManager::GetItemOption(DWORD dwOptionIdx)
{
	ITEM_OPTION_INFO* pInfo = m_ItemOptionList.GetData(dwOptionIdx);
	if(pInfo)
		return pInfo;
	
	// �ӽ� �ڵ�
	static ITEM_OPTION_INFO NULLINFO;
	memset(&NULLINFO,0,sizeof(ITEM_OPTION_INFO));
	return &NULLINFO;
}
void CItemManager::InitItemOption(ITEM_OPTION_INFO * pInfo, WORD num)
{
	for(int i = 0 ; i < num ; ++i)
	{
		AddItemOption(&pInfo[i]);
	}
}
void CItemManager::NetworkMsgParse(BYTE Protocol,void* pMsg)
{
	switch(Protocol)
	{
	case MP_ITEM_TOTALINFO_LOCAL:
		{
			ITEM_TOTALINFO * msg = (ITEM_TOTALINFO *)pMsg;
			int i;

			for(i = 0 ; i < SLOT_INVENTORY_NUM ; i++)
			{
				GAMEIN->GetInventoryDialog()->AddItem(&msg->Inventory[i]);	
				if(msg->Inventory[i].QuickPosition != 0)
				{
					if( EVENTMAP->IsEventMap() )
					{
						ITEM_INFO* pItemInfo = GetItemInfo(msg->Inventory[i].wIconIdx);
						if( pItemInfo )
							if( pItemInfo->ItemKind != eYOUNGYAK_ITEM )
								// 06. 01 �̿��� - ����â ����
								// �ʱ� ���� �Լ� AddQuickItemReal -> SetQuickItemReal
								QUICKMGR->SetQuickItemReal(msg->Inventory[i].QuickPosition,msg->Inventory[i].Position, msg->Inventory[i].wIconIdx);
					}
					else
					{
						// 06. 01 �̿��� - ����â ����
						// �ʱ� ���� �Լ� AddQuickItemReal -> SetQuickItemReal
						QUICKMGR->SetQuickItemReal(msg->Inventory[i].QuickPosition,msg->Inventory[i].Position, msg->Inventory[i].wIconIdx);
					}
				}
			}
			for(i = 0 ; i < SLOT_SHOPINVEN_NUM/2 ; i++)
			{
				GAMEIN->GetInventoryDialog()->AddItem(&msg->ShopInventory[i]);
				if(msg->ShopInventory[i].QuickPosition != 0)
				{
					// 06. 01 �̿��� - ����â ����
					// �ʱ� ���� �Լ� AddQuickItemReal -> SetQuickItemReal
					QUICKMGR->SetQuickItemReal(msg->ShopInventory[i].QuickPosition,msg->ShopInventory[i].Position, msg->ShopInventory[i].wIconIdx);
				}
			}
			for(i = 0 ; i < eWearedItem_Max ; i++)
			{
				GAMEIN->GetInventoryDialog()->AddItem(&msg->WearedItem[i]);
				if(msg->WearedItem[i].QuickPosition != 0)
				{
					// 06. 01 �̿��� - ����â ����
					// �ʱ� ���� �Լ� AddQuickItemReal -> SetQuickItemReal
					QUICKMGR->SetQuickItemReal(msg->WearedItem[i].QuickPosition,msg->WearedItem[i].Position,msg->WearedItem[i].wIconIdx);
				}
			}
			for(i = 0 ; i < SLOT_PETWEAR_NUM ; i++)
			{
				GAMEIN->GetPetInventoryDialog()->AddItem(&msg->PetWearedItem[i]);
				if(msg->PetWearedItem[i].QuickPosition != 0)
				{
					QUICKMGR->SetQuickItemReal(msg->PetWearedItem[i].QuickPosition,msg->PetWearedItem[i].Position,msg->PetWearedItem[i].wIconIdx);
				}
			}
			// magi82 - Titan(070209)
			for(i = 0 ; i < SLOT_TITANWEAR_NUM ; i++)
			{				
				GAMEIN->GetTitanInventoryDlg()->AddItem(&msg->TitanWearedItem[i]);
				if(msg->TitanWearedItem[i].QuickPosition != 0)
				{
					QUICKMGR->SetQuickItemReal(msg->TitanWearedItem[i].QuickPosition,msg->TitanWearedItem[i].Position,msg->TitanWearedItem[i].wIconIdx);
				}

				//DBIdx �κ��� usinglist�� �߰��Ѵ�.
				DWORD itemDBIdx = msg->TitanWearedItem[i].dwDBIdx;
				if(itemDBIdx)
				{
					TITAN_ENDURANCE_ITEMINFO* pInfo = TITANMGR->GetTitanEnduranceInfo(itemDBIdx);
					if(pInfo)
					{
						TITANMGR->AddTitanUsingEquipItemList(pInfo);
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////

			// magi82 - Titan(070302)
			for(i = 0 ; i < SLOT_TITANSHOPITEM_NUM ; i++)
			{
				GAMEIN->GetTitanInventoryDlg()->AddItem(&msg->TitanShopItem[i]);
				if(msg->TitanShopItem[i].QuickPosition != 0)
				{
					QUICKMGR->SetQuickItemReal(msg->TitanShopItem[i].QuickPosition,msg->TitanShopItem[i].Position,msg->TitanShopItem[i].wIconIdx);
				}
			}
			//////////////////////////////////////////////////////////////////////////

			// QuickDur�ٽ� ���
			QUICKMGR->RefreshQickItem();
		}
		break;

	case MP_ITEM_PYOGUK_ITEM_INFO_ACK:
		{
			SEND_PYOGUK_ITEM_INFO * pmsg = (SEND_PYOGUK_ITEM_INFO*)pMsg;
			CAddableInfoIterator iter(&pmsg->AddableInfo);
			BYTE AddInfoKind;
			while((AddInfoKind = iter.GetInfoKind()) != CAddableInfoList::None)
			{
				switch(AddInfoKind)
				{
				case(CAddableInfoList::ItemOption):
					{
						ITEM_OPTION_INFO OptionInfo[SLOT_PYOGUK_NUM];
						iter.GetInfoData(&OptionInfo);
						ITEMMGR->InitItemOption(OptionInfo, pmsg->wOptionCount);
					}
					break;
				case(CAddableInfoList::ItemRareOption):
					{
						ITEM_RARE_OPTION_INFO RareOptionInfo[SLOT_PYOGUK_NUM];
						iter.GetInfoData(&RareOptionInfo);
						ITEMMGR->InitItemRareOption(RareOptionInfo, pmsg->wRareOptionCount);
					}
					break;
				case(CAddableInfoList::PetTotalInfo):
					{
						PET_TOTALINFO PetInfo[SLOT_PYOGUK_NUM];
						iter.GetInfoData(&PetInfo);
						PETMGR->InitPetInfo(PetInfo, pmsg->wPetInfoCount);
					}
					break;
				case(CAddableInfoList::TitanTotalInfo):
					{
						TITAN_TOTALINFO TitanInfo[SLOT_PYOGUK_NUM];
						iter.GetInfoData(&TitanInfo);
						TITANMGR->InitTitanInfo(TitanInfo, pmsg->wTitanItemCount);
					}
					break;
				case(CAddableInfoList::TitanEndrncInfo):
					{
						TITAN_ENDURANCE_ITEMINFO TItanEquipInfo[SLOT_PYOGUK_NUM];
						iter.GetInfoData(&TItanEquipInfo);
						TITANMGR->InitTitanEquipItemEnduranceInfo(TItanEquipInfo, pmsg->wTitanEquipItemCount);
					}
					break;
				}
				iter.ShiftToNextData();
			}

			for(int i=0; i<SLOT_PYOGUK_NUM; ++i)
			{
				if(pmsg->PyogukItem[i].dwDBIdx != 0)
					GAMEIN->GetPyogukDialog()->AddItem(&pmsg->PyogukItem[i]);
			}
			GAMEIN->GetPyogukDialog()->SetPyogukMoney(pmsg->money);		

			GAMEIN->GetPyogukDialog()->SetItemInit(TRUE);
			GAMEIN->GetInventoryDialog()->SetActive(TRUE);
			GAMEIN->GetPyogukDialog()->SetActive(TRUE);
			if(HERO->GetPyogukNum() == 0)
			{
#ifdef _JAPAN_LOCAL_
				if( HERO->GetExtraPyogukSlot() == 0 )
					GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_NoWare);
				else
				{
					GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_PyogukWare4);
					GAMEIN->GetPyogukDialog()->AddPyogukMode(ePyogukMode_PyogukWare4);
				}
#elif defined _HK_LOCAL_
				if( HERO->GetExtraPyogukSlot() == 0 )
					GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_NoWare);
				else
				{
					GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_PyogukWare4);
					GAMEIN->GetPyogukDialog()->AddPyogukMode(ePyogukMode_PyogukWare4);
				}

#elif defined _TL_LOCAL_
				if( HERO->GetExtraPyogukSlot() == 0 )
					GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_NoWare);
				else
				{
					GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_PyogukWare4);
					GAMEIN->GetPyogukDialog()->AddPyogukMode(ePyogukMode_PyogukWare4);
				}
#else

#endif	// _JAPAN_LOCAL_
			}
			else
			{
				GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_PyogukWare1);
				GAMEIN->GetPyogukDialog()->AddPyogukMode(ePyogukMode_PyogukWare1);
			}	

			/*
			if(HERO->GetPyogukNum() == 0)
			{
			#ifdef _JAPAN_LOCAL_
			if( HERO->GetExtraPyogukSlot() == 0 )
			GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_NoWare);
			else
			{
			GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_PyogukWare4);
			GAMEIN->GetPyogukDialog()->AddPyogukMode(ePyogukMode_PyogukWare4);
			}
			#else
			#endif	// _JAPAN_LOCAL_
			}
			else
			{

			}*/	
		}
		break;

	case MP_ITEM_PETINVEN_INFO_ACK:
		{
			SEND_PETINVEN_ITEM_INFO* pmsg = (SEND_PETINVEN_ITEM_INFO*)pMsg;

			CAddableInfoIterator iter(&pmsg->AddableInfo);
			BYTE AddInfoKind;
			while((AddInfoKind = iter.GetInfoKind()) != CAddableInfoList::None)
			{
				switch(AddInfoKind)
				{
				case(CAddableInfoList::ItemOption):
					{
						ITEM_OPTION_INFO OptionInfo[SLOT_PETINVEN_NUM];
						iter.GetInfoData(&OptionInfo);
						ITEMMGR->InitItemOption(OptionInfo, pmsg->wOptionCount);
					}
					break;
				case(CAddableInfoList::ItemRareOption):
					{
						ITEM_RARE_OPTION_INFO RareOptionInfo[SLOT_PETINVEN_NUM];
						iter.GetInfoData(&RareOptionInfo);
						ITEMMGR->InitItemRareOption(RareOptionInfo, pmsg->wRareOptionCount);
					}
					break;
				}

				iter.ShiftToNextData();
			}

			for( int i = 0; i < SLOT_PETINVEN_NUM; ++i )
			{
				if(pmsg->PetInvenItem[i].dwDBIdx != 0)
				{
					GAMEIN->GetPetInventoryDialog()->AddItem(&pmsg->PetInvenItem[i]);
				}
			}

			GAMEIN->GetPetInventoryDialog()->SetItemInfoInit(TRUE);

			//�� �ɷ¿� �°� �� ������ ǥ��
			PETMGR->SetPetValidInvenTab();

			GAMEIN->GetPetInventoryDialog()->SetActive(TRUE);
		}
		break;

	case MP_ITEM_MONSTER_RAREOBTAIN_NOTIFY:
		{
			MSG_ITEM_RAREITEM_GET* pmsg = (MSG_ITEM_RAREITEM_GET*)pMsg;

			//			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;
			AddItemRareOption(&pmsg->RareInfo);

			CItem* pItem = NULL;
			if( pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->RareItemBase.Position) )
			{
				pItem->SetItemBaseInfo(&pmsg->RareItemBase);
			}
			else
			{
				pItem = MakeNewItem(&pmsg->RareItemBase, "MP_ITEM_MONSTER_RAREOBTAIN_NOTIFY" );
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
				ASSERT(rt);
			}

			GAMEEVENTMGR->AddEvent(eGameEvent_ObtainItem);

			//������â�� ������ �����.
			cDialog* pDlg = WINDOWMGR->GetWindowForID( DBOX_DIVIDE_INV );
			if( pDlg )
			{
				((cDivideBox*)pDlg)->ExcuteDBFunc( 0 );	//��ҹ�ư ������
			}

			if( pItem )
			{
				CHATMGR->AddMsg( CTC_GETITEM, CHATMGR->GetChatMsg( 121 ), pItem->GetItemInfo()->ItemName );
				//				EFFECTMGR->StartHeroEffectProcess(eEffect_GetItem);
				ItemDropEffect( pItem->GetItemIdx() );
			}

			QUICKMGR->RefreshQickItem();

			//			pmsg->wTargetItemIdx
		}
		break;

	case MP_ITEM_REINFORCE_SUCCESS_ACK:
		{
			MSG_ITEM_REINFORCE_ACK * pmsg = (MSG_ITEM_REINFORCE_ACK*)pMsg;

			// 1. add option
			AddItemOption(&pmsg->OptionInfo);

			// 2. ��� ����
			CItem * ItemOut;
			for(int i = 1 ; i < MAX_REINFORCEGRID_NUM+1 ; ++i)
			{
				CVirtualItem * pVItem = GAMEIN->GetReinforceDialog()->GetVirtualItem(i);
				if(pVItem)
					DeleteItem(pVItem->GetSrcPosition(), &ItemOut);
			}

			//SW051021 ��ȭâ ��� ���� ���� �ʱ�ȭ.
			REINFORCEMGR->SetRareMaterialTotalGravity(0);
			GAMEIN->GetReinforceDialog()->SetGravityText();

			// 3. ���� Dura ����
			CItem * pTItem = NULL;
			CVirtualItem * pVItem = GAMEIN->GetReinforceDialog()->GetVirtualItem(0);
			if( pVItem )
			{
				pTItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pVItem->GetSrcPosition());
				if( pTItem )
				{
					pTItem->SetDurability(pmsg->OptionInfo.dwOptionIdx);
					SetToolTipIcon(pTItem, &pmsg->OptionInfo, ITEMMGR->GetItemRareOption(pTItem->GetRareness()) );
					GAMEIN->GetReinforceDialog()->AddVirtualItemWrap(0, pTItem);
					pTItem->SetLock(TRUE);
				}
			}
			GAMEIN->GetReinforceDialog()->Release();
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(225) );
			QUICKMGR->RefreshQickItem();
			GAMEIN->GetReinforceDialog()->SetDisable(FALSE);

			if( GAMEIN->GetReinforceDialog()->IsShopItemUse() )
				GAMEIN->GetReinforceDialog()->SetActiveRecursive( FALSE );
			else if( pTItem )
			{
				GAMEIN->GetReinforceDialog()->AddVirtualItemWrap(0, pTItem);
				pTItem->SetLock(TRUE);
			}
		}
		break;
	case MP_ITEM_REINFORCE_FAILED_ACK:
		{
			MSG_ITEM_REINFORCE_SYN * pmsg = (MSG_ITEM_REINFORCE_SYN*)pMsg;

			//SW051021 ��ȭâ ��� ���� ���� �ʱ�ȭ.
			REINFORCEMGR->SetRareMaterialTotalGravity(0);
			GAMEIN->GetReinforceDialog()->SetGravityText();

			CItem * ItemOut;
			for(int i = 1 ; i < MAX_REINFORCEGRID_NUM+1 ; ++i)
			{
				CVirtualItem * pVItem = GAMEIN->GetReinforceDialog()->GetVirtualItem(i);
				if(pVItem)
					DeleteItem(pVItem->GetSrcPosition(), &ItemOut);
			}
			GAMEIN->GetReinforceDialog()->Release(eReinforceViewRelease);
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(226) );
			QUICKMGR->RefreshQickItem();
			GAMEIN->GetReinforceDialog()->SetDisable(FALSE);
		}
		break;
	case MP_ITEM_REINFORCE_NACK:
		{
			//SW051021 ��ȭâ ��� ���� ���� �ʱ�ȭ.
			REINFORCEMGR->SetRareMaterialTotalGravity(0);
			GAMEIN->GetReinforceDialog()->SetGravityText();

			ASSERTMSG(0, "�ο��� �ҷ����ƿ�!-��ȭ��������. Reinforce Error.");
			GAMEIN->GetReinforceDialog()->SetDisable(FALSE);
		}
		break;
	case MP_ITEM_REINFORCE_WITHSHOPITEM_NACK:
		{
			MSG_ITEM_ERROR* pmsg = (MSG_ITEM_ERROR*)pMsg;

			char buf[256];
			memset( buf, 0, sizeof(char)*256 );

			switch( pmsg->ECode )
			{
			case 3:				// ��ȭ�������� �ƴϴ�.
				sprintf( buf, CHATMGR->GetChatMsg(1081) );
				break;
			case 4:				// ������ ������ ���� �ʴ´�.
				sprintf( buf, CHATMGR->GetChatMsg(1082) );
				break;
			case 10:			// ��ġ�� ���Ƽ� ��ȣ�ƴ�.
				{					
					CItem * ItemOut;
					for(int i = 1 ; i < MAX_REINFORCEGRID_NUM+1 ; ++i)
					{
						CVirtualItem * pVItem = GAMEIN->GetReinforceDialog()->GetVirtualItem(i);
						if(pVItem)
							DeleteItem(pVItem->GetSrcPosition(), &ItemOut);
					}

					CVirtualItem * pVItem = GAMEIN->GetReinforceDialog()->GetVirtualItem(0);
					if( pVItem )
					{
						CItem * pTItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pVItem->GetSrcPosition() );
						if( pTItem )
						{							
							GAMEIN->GetReinforceDialog()->AddVirtualItemWrap(0, pTItem);
							pTItem->SetLock(TRUE);
						}
					}
					GAMEIN->GetReinforceDialog()->Release();
					QUICKMGR->RefreshQickItem();
					GAMEIN->GetReinforceDialog()->SetDisable(FALSE);
					sprintf( buf, CHATMGR->GetChatMsg(1083) );
				}
				break;
			case 11:			// ��ȭ�� �� ���� ���������̴�.
				sprintf( buf, CHATMGR->GetChatMsg(1084) );
				break;
			case 12:			// ��ȭ��ȣ�� ���� ����.
				sprintf( buf, CHATMGR->GetChatMsg(1085) );
				break;				
			case 13:			// ���������� ����.
				sprintf( buf, CHATMGR->GetChatMsg(1089) );
				break;
			case 14:			// 2�����̻� ��ȭ�� �������̴�.
				sprintf( buf, CHATMGR->GetChatMsg(1090) );
			default:
				break;
			}

			CHATMGR->AddMsg( CTC_SYSMSG, buf );
			GAMEIN->GetReinforceDialog()->SetDisable( FALSE );
			GAMEIN->GetReinforceDialog()->SetActiveRecursive( FALSE );
		}
		break;
	case MP_ITEM_MOVE_ACK:		
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);	
			MoveItem( (MSG_ITEM_MOVE_ACK *)pMsg );			

			WINDOWMGR->DragWindowNull();
		}
		break;
	case MP_ITEM_COMBINE_ACK:	
		{
			CombineItem( (MSG_ITEM_COMBINE_ACK *)pMsg );	

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			WINDOWMGR->BackDragWindow();
		}
		break;
	case MP_ITEM_DIVIDE_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_ShopInven);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			CItem* pPreItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				pPreItem = NULL;
				DeleteItem(pmsg->GetItem(i)->Position, &pPreItem);

				pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_DIVIDE_ACK");	
				if(!AddItem(pItem))
				{
					ASSERT(0);
				}
				if(pItem->GetQuickPosition() != 0)
				{
					//					CQuickItem *pOldQuick = GAMEIN->GetQuickDialog()->GetQuickItem(pItem->GetQuickPosition());
					//					GAMEIN->GetQuickDialog()->FakeAddItem(pItem->GetQuickPosition(), pItem, pOldQuick );

					WORD QuickPosList[4];
					CQuickManager::AnalyzeQuickPosition(pItem->GetQuickPosition(), QuickPosList);

					for(WORD SheetNum = 0; SheetNum < 4; SheetNum++)
					{
						if(QuickPosList[SheetNum])
							QUICKMGR->AddQuickItemReal(((SheetNum * 10) + (QuickPosList[SheetNum] - 1)),pItem->GetPosition(),pItem->GetItemIdx());
					}
				}
			}

			// QuickDur�ٽ� ���
			QUICKMGR->RefreshQickItem();

			WINDOWMGR->DragWindowNull();
		}
		break;
	case MP_ITEM_MONSTER_OBTAIN_NOTIFY:
		{
			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			//			CItem* pPreItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				if( pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					pItem->SetItemBaseInfo(pmsg->GetItem(i));
					//					DeleteItem(pmsg->GetItem(i)->Position, pPreItem);

				}
				else
				{
					pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_MONSTER_OBTAIN_NOTIFY");	
					BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
					if(!rt)
					{
						ASSERT(0);
					}
				}

				// Game event 
				//				GAMEEVENTMGR->AddEvent(eGameEvent1_ObtainItem, (pItem->GetItemBaseInfo())->wIconIdx );
				GAMEEVENTMGR->AddEvent(eGameEvent_ObtainItem);
			}

			//������â�� ������ �����.
			cDialog* pDlg = WINDOWMGR->GetWindowForID( DBOX_DIVIDE_INV );
			if( pDlg )
			{
				((cDivideBox*)pDlg)->ExcuteDBFunc( 0 );	//��ҹ�ư ������
			}

			if( pItem )
			{
				CHATMGR->AddMsg( CTC_GETITEM, CHATMGR->GetChatMsg( 121 ), pItem->GetItemInfo()->ItemName );
				//				EFFECTMGR->StartHeroEffectProcess(eEffect_GetItem);
				ItemDropEffect( pItem->GetItemIdx() );
			}

			QUICKMGR->RefreshQickItem();

		}
		break;
	case MP_ITEM_MOVE_NACK:
		{
			MSG_WORD* pmsg = (MSG_WORD*)pMsg;
			switch(pmsg->wData)
			{
			case 7:
				break;
			default:
				ASSERT(pmsg->wData == 3);	
			}
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_PetInven);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);

			WINDOWMGR->BackDragWindow();
		}
		break;
	case MP_ITEM_DIVIDE_NACK:
		{
			MSG_WORD* pmsg = (MSG_WORD*)pMsg;
			//	ASSERT(pmsg->wData == 0);
			//	ASSERT(0);
			//GAMEIN->GetInventoryDialog()->GetItemForPos(111)->SetMovable(TRUE);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_ShopInven);
			SetDisableDialog(FALSE, eItemTable_PetInven);
			WINDOWMGR->BackDragWindow();
		}
		break;
	case MP_ITEM_COMBINE_NACK:
		{
			MSG_WORD* pmsg = (MSG_WORD*)pMsg;
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			ASSERT(pmsg->wData == 0);
			ASSERT(0);

			WINDOWMGR->BackDragWindow();
		}
		break;
	case MP_ITEM_DISCARD_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			ASSERT(0);
			GAMEIN->GetInventoryDialog()->SetDisable( FALSE );
			MSG_ITEM_ERROR * pmsg = (MSG_ITEM_ERROR*)pMsg;
			if(pmsg->ECode != 4)
				WINDOWMGR->BackDragWindow();
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(799));
		}
		break;
	case MP_ITEM_DISCARD_ACK:
		{
			MSG_ITEM_DISCARD_ACK* pmsg = (MSG_ITEM_DISCARD_ACK*)pMsg;
			//			MSG_ITEM_REM_ACK * pmsg = (MSG_ITEM_REM_ACK *)pMsg;
			WORD wTableIdx = GetTableIdxForAbsPos(pmsg->TargetPos);

			DeleteItemofTable(wTableIdx, pmsg->TargetPos);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);
			QUICKMGR->RefreshQickItem();
		}
		break;
	case MP_ITEM_USE_NOTIFY:
		{
			MSG_DWORDBYTE* pmsg = (MSG_DWORDBYTE*)pMsg;
			CObject* pObject = OBJECTMGR->GetObject( pmsg->dwData );
			if( pObject == NULL ) break;
			if( pObject->GetObjectKind() != eObjectKind_Player ) break;

			CPlayer* pPlayer = (CPlayer*)pObject;

			switch( pmsg->bData )
			{
			case 1:	//������ ȸ�� ����Ʈ
				{
					if(pPlayer->InTitan())
					{
						OBJECTEFFECTDESC desc(FindEffectNum("t_ba_070.beff"));
						pPlayer->AddObjectEffect( LIFE_RECOVER_EFFECTID, &desc, 1 );
					}
					else
					{
						OBJECTEFFECTDESC desc(FindEffectNum("m_ba_070.beff"));
						pPlayer->AddObjectEffect( LIFE_RECOVER_EFFECTID, &desc, 1 );
					}
				}
				break;
			case 2:	//���� ȸ�� ����Ʈ
				{
					if(pPlayer->InTitan())
					{
						OBJECTEFFECTDESC desc(FindEffectNum("t_ba_071.beff"));
						pPlayer->AddObjectEffect( MANA_RECOVER_EFFECTID, &desc, 1 );
					}
					else
					{
						OBJECTEFFECTDESC desc(FindEffectNum("m_ba_071.beff"));
						pPlayer->AddObjectEffect( MANA_RECOVER_EFFECTID, &desc, 1 );
					}
				}
				break;
			case 3:	//ȣ�Ű��� ȸ�� ����Ʈ
				{
					if(pPlayer->InTitan())
					{
						OBJECTEFFECTDESC desc(FindEffectNum("t_ba_077.beff"));
						pPlayer->AddObjectEffect( SHIELD_RECOVER_EFFECTID, &desc, 1 );
					}
					else
					{
						OBJECTEFFECTDESC desc(FindEffectNum("m_ba_077.beff"));
						pPlayer->AddObjectEffect( SHIELD_RECOVER_EFFECTID, &desc, 1 );
					}
				}
				break;
			case 4:	//�� ȸ�� ����Ʈ
				{
					if(pPlayer->InTitan())
					{
						OBJECTEFFECTDESC desc(FindEffectNum("t_ba_078.beff"));
						pPlayer->AddObjectEffect( ALL_RECOVER_EFFECTID, &desc, 1 );
					}
					else
					{
						OBJECTEFFECTDESC desc(FindEffectNum("m_ba_078.beff"));
						pPlayer->AddObjectEffect( ALL_RECOVER_EFFECTID, &desc, 1 );
					}
				}
				break;
			}
		}
		break;
	case MP_ITEM_USE_ACK:
		{
			MSG_ITEM_USE_ACK* pmsg = (MSG_ITEM_USE_ACK*)pMsg;
			//			MSG_ITEM_USE * pmsg = (MSG_ITEM_USE*)pMsg;
			//pmsg->wTableIdx 
			WORD wTableIdx = GetTableIdxForAbsPos(pmsg->TargetPos);

			m_nItemUseCount--;

			switch(wTableIdx)//			switch(pmsg->wTableIdx)
			{
			case eItemTable_Inventory:
			case eItemTable_Weared:
				{

					CItem * item = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->TargetPos);
					//					CItem * item = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wAbsPosition);
					eITEM_KINDBIT bits = item->GetItemKind();
					if(bits & eYOUNGYAK_ITEM)
					{

						if(item->GetDurability() > 1)
						{
							//���� ó��
							GAMEIN->GetInventoryDialog()->UpdateItemDurabilityAdd(pmsg->TargetPos,-1);
						}
						else
						{
							CItem* pOutItem = NULL;
							DeleteItem( pmsg->TargetPos, &pOutItem);
							if( pOutItem )
								ReLinkQuickPosition( pOutItem );
						}

						// use sound
						//AUDIOMGR->Play(68, HERO);
						if( item )
						{
							if( item->GetItemIdx() == 53032 || item->GetItemIdx() == 53095 ||
								item->GetItemIdx() == 53103 || item->GetItemIdx() == 53102 ||
								item->GetItemIdx() == 53217 || item->GetItemIdx() == 53223 ||
								item->GetItemIdx() == 53232 || item->GetItemIdx() == 53234 )
							{
								// magi82(4) - Titan(071022) Ÿ��ź�϶� ����Ʈ ó��(����ȸ������)
								if( HERO->InTitan() == TRUE )
                                    EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_UseItem_FullYoungYak);
								else
									EFFECTMGR->StartHeroEffectProcess(eEffect_UseItem_FullYoungYak);
							}
							else
							{
								//if( item->GetItemKind() != eYOUNGYAK_ITEM )	return;
								if( !(item->GetItemKind() & eYOUNGYAK_ITEM) )	return;

								if( item->GetItemKind() == eYOUNGYAK_ITEM )	//�� �ƴ� �÷��̾�Ը� �ش�
								{
									if( item->GetItemInfo()->LifeRecover || item->GetItemInfo()->LifeRecoverRate )
									{
										// magi82(4) - Titan(071022) Ÿ��ź�϶� ����Ʈ ó��(������ȸ������)
										if( HERO->InTitan() == TRUE )
											EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_UseItem_LifeYoungYak);
										else
											EFFECTMGR->StartHeroEffectProcess(eEffect_UseItem_LifeYoungYak);
									}
									if( item->GetItemInfo()->NaeRyukRecover || item->GetItemInfo()->NaeRyukRecoverRate )
									{
										// magi82(4) - Titan(071022) Ÿ��ź�϶� ����Ʈ ó��(����ȸ������)
										if( HERO->InTitan() == TRUE )
											EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_UseItem_ManaYoungYak);
										else
											EFFECTMGR->StartHeroEffectProcess(eEffect_UseItem_ManaYoungYak);
									}								
									//========================= �ް� �ϵ��ڵ�
									if( item->GetItemIdx() == 53031 || item->GetItemIdx() == 53094 || item->GetItemIdx() == 53109 )										
									{
										// magi82(4) - Titan(071022) Ÿ��ź�϶� ����Ʈ ó��(ȣ�Ű���ȸ������)
										if( HERO->InTitan() == TRUE )
											EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_UseItem_HosinYoungYak);
										else
											EFFECTMGR->StartHeroEffectProcess(eEffect_UseItem_HosinYoungYak);
									}
									//========================= �߼��̺�Ʈ
									if( item->GetItemIdx() == 53216 || item->GetItemIdx() == 53222 )
									{
										// magi82(4) - Titan(071022) Ÿ��ź�϶� ����Ʈ ó��(ȣ�Ű���ȸ������)
										if( HERO->InTitan() == TRUE )
											EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_UseItem_HosinYoungYak);
										else
											EFFECTMGR->StartHeroEffectProcess(eEffect_UseItem_HosinYoungYak);
									}
								}
								else if( item->GetItemKind() == eYOUNGYAK_ITEM_PET )
								{
									//�� ���� ����Ʈ
									// 									TARGETSET set;
									// 									set.pTarget = HEROPET;
									// 									EFFECTMGR->StartEffectProcess(eEffect_FeedUp,HEROPET,&set,0,HEROPET->GetID());

									CPet* pPet = PETMGR->GetCurSummonPet();
									if(pPet)
									{
										TARGETSET set;
										set.pTarget = pPet;
										EFFECTMGR->StartEffectProcess(eEffect_FeedUp,pPet,&set,0,pPet->GetID());
									}
								}
								else if( item->GetItemKind() == eYOUNGYAK_ITEM_TITAN )
								{
									// magi82(26)
#define TITAN_POTION_DELAY 30000
									DWORD dwTime = TITAN_POTION_DELAY;
									if(TITANMGR->GetTitanShopitemOption()->dwEPReduceRate > 0.0f)
									{
										dwTime = (DWORD)(dwTime * TITANMGR->GetTitanShopitemOption()->dwEPReduceRate);
									}

                                    HERO->GetDelayGroup()->AddTitanPotionDelay( dwTime );
								}

							}

							//==========================

						}
					}
					else if(bits & eCHANGE_ITEM)
					{
						if(item->GetDurability() > 1)
						{
							//�Ʃ���o A������
							GAMEIN->GetInventoryDialog()->UpdateItemDurabilityAdd(pmsg->TargetPos,-1);
						}
						else
						{
							CItem* pOutItem = NULL;
							DeleteItem( pmsg->TargetPos, &pOutItem);
							if( pOutItem )
								ReLinkQuickPosition( pOutItem );
						}
					}
					else if(bits & eEXTRA_ITEM)
					{
						//						ASSERT(0);

						if(item->GetDurability() > 1)
						{
							//���� ó��
							GAMEIN->GetInventoryDialog()->UpdateItemDurabilityAdd(pmsg->TargetPos,-1);
						}
						else
						{
							CItem* pOutItem = NULL;
							DeleteItem( pmsg->TargetPos, &pOutItem);
							if( pOutItem )
								ReLinkQuickPosition( pOutItem );
						}
						/*
						CItem * itemOut;
						GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->TargetPos, &itemOut);
						//						GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->wAbsPosition, &itemOut);
						m_dwStateParam = itemOut->GetItemIdx();
						ItemDelete(itemOut);



						//���� ����
						GAMEIN->GetInventoryDialog()->SetState(eITEMUPGRADE_STATE);

						// �޽��� ���
						//						GAMEIN->GetChatDialog()->AddChatMsg(WHOLE,RGBA_MAKE(255,0,0,255), "��n����������| ��c��eCO��I��U.");
						CHATMGR->AddMsg( CTC_TOWHOLE, CHATMGR->GetChatMsg(173) );
						//TAIYO Ŀ�� ����
						*/

					}
					else if(bits & eMUGONG_ITEM)
					{
						CItem* pOutItem = NULL;
						DeleteItem( pmsg->TargetPos, &pOutItem);

						// use sound
						AUDIOMGR->Play(56, HERO);
					}
					else if( bits & eQUEST_ITEM )
					{
						if( bits == eQUEST_ITEM_START )
						{
							if( item->GetDurability() > 1 )
								GAMEIN->GetInventoryDialog()->UpdateItemDurabilityAdd(pmsg->TargetPos,-1);
							else
							{
								CItem* pOutItem = NULL;
								DeleteItem( pmsg->TargetPos, &pOutItem);
								if( pOutItem )
									ReLinkQuickPosition( pOutItem );
							}
						}
					}
					else if( bits & eTITAN_ITEM )
					{
						//!!!
					}
					else
					{
						ASSERT(0);
					}
				}
				break;
			case eItemTable_StreetStall:
				{
					ASSERT(0);
				}
				break;
			case eItemTable_ShopInven:
				{
					CItem * item = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->TargetPos);
					eITEM_KINDBIT bits = item->GetItemKind();

					if( bits & eSHOP_ITEM )
					{
						//if( bits == eSHOP_ITEM_HERB || bits == eSHOP_ITEM_INCANTATION )
						//{
						if(item->GetDurability() > 1)
						{
							GAMEIN->GetInventoryDialog()->UpdateItemDurabilityAdd(pmsg->TargetPos,-1);
						}
						//}
						else
						{
							CItem* pOutItem = NULL;
							DeleteItem( pmsg->TargetPos, &pOutItem);
							if( pOutItem )
								ReLinkQuickPosition( pOutItem );
						}

						// use sound
						AUDIOMGR->Play(68, HERO);
					}
				}
				break;				
			default:
				{
					ASSERT(0);
				}
			}
			QUICKMGR->RefreshQickItem();
		}
		break;
	case MP_ITEM_MIX_ADDITEM_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);			
			if(pItem)
				GAMEIN->GetMixDialog()->AddRealItem( pItem );

			GAMEIN->GetMixDialog()->DieCheck();
		}
		break;
	case MP_ITEM_MIX_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetMixDialog()->Release( eMixAllRelease, FALSE );
			GAMEIN->GetMixDialog()->DieCheck();

			//error msg����ϱ�
		}
		break;
	case MP_ITEM_MIX_SUCCESS_ACK:
		{
			MSG_ITEM_MIX_ACK * pmsg = (MSG_ITEM_MIX_ACK *)pMsg;
			// �⺻ ������ ����
			// ��� ����
			// ��� ������ ����

			GAMEIN->GetMixDialog()->Release(eMixAllRelease, FALSE);

			CItem * BasicItemOut;
			//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->BasicItemPos, &BasicItemOut);
			DeleteItem(pmsg->BasicItemPos, &BasicItemOut, NULL, NULL, eRemainAllOption);


			CItem * pMatItemOut;
			for(int i = 0 ; i < pmsg->wMaterialNum ; ++i)
			{
				if(IsDupItem(pmsg->Material[i].wItemIdx))
				{
					CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->Material[i].ItemPos);

					if(pItem)
						if(pItem->GetDurability() - pmsg->Material[i].Dur != 0)
						{
							pItem->SetDurability(pItem->GetDurability() - pmsg->Material[i].Dur);
							continue;
						}
				}

				DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//ItemDelete(pMatItemOut);
			}

			ITEMBASE NewItem;
			NewItem = *(BasicItemOut->GetItemBaseInfo());
			NewItem.Position = pmsg->BasicItemPos;
			NewItem.wIconIdx = pmsg->wResultItemIdx;
			NewItem.RareIdx	= pmsg->dwRareIdx;
			NewItem.QuickPosition = 0;

			if( !IsDupItem( NewItem.wIconIdx ) )
				NewItem.Durability = 0;

			//ItemDelete(BasicItemOut);

			ITEM_INFO* piteminfo = ITEMMGR->GetItemInfo( NewItem.wIconIdx );

			GAMEIN->GetMixDialog()->Release(eMixBasicRelease, FALSE);
			if( piteminfo && !(piteminfo->ItemKind & eSHOP_ITEM) )
			{
				GAMEIN->GetInventoryDialog()->AddItem(&NewItem);
				CItem * pResultItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->BasicItemPos);
				GAMEIN->GetMixDialog()->AddVirtualItemWrap(eMixPos_Result, pResultItem);
				pResultItem->SetLock(TRUE);
			}

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(207), piteminfo->ItemName );
			QUICKMGR->RefreshQickItem();

			GAMEIN->GetMixDialog()->DieCheck();
			if( GAMEIN->GetMixDialog()->IsShopItemUse() )
			{
				GAMEIN->GetMixDialog()->Release();
				GAMEIN->GetMixDialog()->SetActiveRecursive( FALSE );
			}
		}
		break;
	case MP_ITEM_MIX_BIGFAILED_ACK:
		{
			// �⺻ ������ ����
			// ��� ����
			MSG_ITEM_MIX_ACK * pmsg = (MSG_ITEM_MIX_ACK *)pMsg;
			GAMEIN->GetMixDialog()->Release();

			CItem * BasicItemOut;
			DeleteItem(pmsg->BasicItemPos, &BasicItemOut);
			//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->BasicItemPos, &BasicItemOut);
			//ItemDelete(BasicItemOut);

			CItem * pMatItemOut;
			for(int i = 0 ; i < pmsg->wMaterialNum ; ++i)
			{
				if(IsDupItem(pmsg->Material[i].wItemIdx))
				{
					CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->Material[i].ItemPos);
					if(pItem->GetDurability() - pmsg->Material[i].Dur != 0)
					{
						pItem->SetDurability(pItem->GetDurability() - pmsg->Material[i].Dur);
						continue;
					}
				}

				DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//ItemDelete(pMatItemOut);
			}
			GAMEIN->GetMixDialog()->Release(eMixBasicRelease);
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(208));
			QUICKMGR->RefreshQickItem();

			GAMEIN->GetMixDialog()->DieCheck();
			if( GAMEIN->GetMixDialog()->IsShopItemUse() )
				GAMEIN->GetMixDialog()->SetActiveRecursive( FALSE );
		}
		break;
	case MP_ITEM_MIX_FAILED_ACK:
		{
			GAMEIN->GetMixDialog()->ResetCurSelCellPos();
			// ��� ����
			MSG_ITEM_MIX_ACK * pmsg = (MSG_ITEM_MIX_ACK *)pMsg;

			CItem * pMatItemOut;
			for(int i = 0 ; i < pmsg->wMaterialNum ; ++i)
			{
				if(IsDupItem(pmsg->Material[i].wItemIdx))
				{
					CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->Material[i].ItemPos);
					if(pItem->GetDurability() - pmsg->Material[i].Dur != 0)
					{
						pItem->SetDurability(pItem->GetDurability() - pmsg->Material[i].Dur);
						continue;
					}
				}
				DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//ItemDelete(pMatItemOut);
			}
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(209));
			QUICKMGR->RefreshQickItem();

			GAMEIN->GetMixDialog()->DieCheck();
			if( GAMEIN->GetMixDialog()->IsShopItemUse() )
			{
				GAMEIN->GetMixDialog()->Release();
				GAMEIN->GetMixDialog()->SetActiveRecursive( FALSE );
			}
		}
		break;
	case MP_ITEM_MIX_MSG:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;

			char buf[256];
			memset( buf, 0, sizeof(char)*256 );

			switch( pmsg->dwData1 )
			{
			case 20:			// ShopItem�� ����.
				sprintf( buf, CHATMGR->GetChatMsg(1086) );
				break;
			case 21:			// ����з� ���� ��ȣ�ƴ�.
				{
					CItem * BasicItemOut;
					DeleteItem( (POSTYPE)pmsg->dwData2, &BasicItemOut );
					sprintf( buf, CHATMGR->GetChatMsg(1088) );
				}
				break;
			case 22:			// ���зκ��� ��ȣ�ƴ�.
				sprintf( buf, CHATMGR->GetChatMsg(1087) );
				break;
			case 23:			// Level���ѿ� �ɸ���.
				sprintf( buf, CHATMGR->GetChatMsg(1082) );
				break;
			}

			CHATMGR->AddMsg( CTC_SYSMSG, buf );
			GAMEIN->GetMixDialog()->Release();
			GAMEIN->GetMixDialog()->DieCheck();
			GAMEIN->GetMixDialog()->SetActiveRecursive( FALSE );
		}
		break;
		// magi82 - Titan(070118) ////////////////////////////////////////////////
	case MP_ITEM_TPM_ADDITEM_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);			
			if(pItem)
				GAMEIN->GetTitanPartsMakeDlg()->AddRealItem( pItem );

			GAMEIN->GetTitanPartsMakeDlg()->DieCheck();
		}
		break;
	case MP_ITEM_TPM_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanPartsMakeDlg()->Release( eTPMAllRelease, FALSE );
			GAMEIN->GetTitanPartsMakeDlg()->DieCheck();

			//error msg����ϱ�
		}
		break;
	case MP_ITEM_TPM_SUCCESS_ACK:
		{
			MSG_ITEM_MIX_ACK * pmsg = (MSG_ITEM_MIX_ACK *)pMsg;
			// �⺻ ������ ����
			// ��� ����
			// ��� ������ ����

			// magi82(15) - Titan(071101) Ÿ��ź ���������� ���� ����
			TITAN_ENDURANCE_ITEMINFO EnduranceInfo;
			ITEM_INFO* pBaseInfo = GetItemInfo(pmsg->wBasicItemIdx);
			if(!pBaseInfo)
				break;

			if( (eITEM_KINDBIT)pBaseInfo->ItemKind & eTITAN_EQUIPITEM )
			{
				ITEMBASE* pItemBase = (ITEMBASE*)GetItemInfoAbsIn(HERO, pmsg->BasicItemPos);
				if( !pItemBase )
					break;

                TITAN_ENDURANCE_ITEMINFO* pEnduranceInfo = TITANMGR->GetTitanEnduranceInfo(pItemBase->dwDBIdx);
				CopyMemory( &EnduranceInfo, pEnduranceInfo, sizeof(TITAN_ENDURANCE_ITEMINFO) );
			}
			//////////////////////////////////////////////////////////////////////////

			GAMEIN->GetTitanPartsMakeDlg()->Release(eTPMAllRelease, FALSE);

			CItem * BasicItemOut;
			//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->BasicItemPos, &BasicItemOut);
			DeleteItem(pmsg->BasicItemPos, &BasicItemOut, NULL, NULL, eRemainAllOption);

			CItem * pMatItemOut;
			for(int i = 0 ; i < pmsg->wMaterialNum ; ++i)
			{
				if(IsDupItem(pmsg->Material[i].wItemIdx))
				{
					CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->Material[i].ItemPos);

					if(pItem)
						if(pItem->GetDurability() - pmsg->Material[i].Dur != 0)
						{
							pItem->SetDurability(pItem->GetDurability() - pmsg->Material[i].Dur);
							continue;
						}
				}

				DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//ItemDelete(pMatItemOut);
			}

			ITEMBASE NewItem;
			NewItem = *(BasicItemOut->GetItemBaseInfo());
			NewItem.Position = pmsg->BasicItemPos;
			NewItem.wIconIdx = pmsg->wResultItemIdx;
			NewItem.RareIdx	= pmsg->dwRareIdx;
			NewItem.QuickPosition = 0;

			if( !IsDupItem( NewItem.wIconIdx ) )
				NewItem.Durability = 0;

			//ItemDelete(BasicItemOut);

			GAMEIN->GetInventoryDialog()->AddItem(&NewItem);
			CItem * pResultItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->BasicItemPos);
			GAMEIN->GetTitanPartsMakeDlg()->AddVirtualItemWrap(eTPMPos_Result, pResultItem);
			pResultItem->SetLock(TRUE);

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(207), pResultItem->GetItemInfo()->ItemName );
			QUICKMGR->RefreshQickItem();

			// magi82(15) - Titan(071101) Ÿ��ź ���������� ���� ����
			if( (eITEM_KINDBIT)pBaseInfo->ItemKind & eTITAN_EQUIPITEM )
			{
                TITANMGR->AddTitanEquipItemEnduranceInfo(&EnduranceInfo);
				RefreshItemToolTip(EnduranceInfo.ItemDBIdx);
			}

			GAMEIN->GetTitanPartsMakeDlg()->DieCheck();
			if( GAMEIN->GetTitanPartsMakeDlg()->IsShopItemUse() )
			{
				GAMEIN->GetTitanPartsMakeDlg()->Release();
				GAMEIN->GetTitanPartsMakeDlg()->SetActiveRecursive( FALSE );
			}
		}
		break;
	case MP_ITEM_TPM_BIGFAILED_ACK:
		{
			// �⺻ ������ ����
			// ��� ����
			MSG_ITEM_MIX_ACK * pmsg = (MSG_ITEM_MIX_ACK *)pMsg;
			GAMEIN->GetTitanPartsMakeDlg()->Release();

			CItem * BasicItemOut;
			DeleteItem(pmsg->BasicItemPos, &BasicItemOut);
			//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->BasicItemPos, &BasicItemOut);
			//ItemDelete(BasicItemOut);

			CItem * pMatItemOut;
			for(int i = 0 ; i < pmsg->wMaterialNum ; ++i)
			{
				if(IsDupItem(pmsg->Material[i].wItemIdx))
				{
					CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->Material[i].ItemPos);
					if(pItem->GetDurability() - pmsg->Material[i].Dur != 0)
					{
						pItem->SetDurability(pItem->GetDurability() - pmsg->Material[i].Dur);
						continue;
					}
				}

				DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//ItemDelete(pMatItemOut);
			}
			GAMEIN->GetTitanPartsMakeDlg()->Release(eTPMBasicRelease);
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(208));
			QUICKMGR->RefreshQickItem();

			GAMEIN->GetTitanPartsMakeDlg()->DieCheck();
			if( GAMEIN->GetTitanPartsMakeDlg()->IsShopItemUse() )
				GAMEIN->GetTitanPartsMakeDlg()->SetActiveRecursive( FALSE );
		}
		break;
	case MP_ITEM_TPM_FAILED_ACK:
		{
			GAMEIN->GetTitanPartsMakeDlg()->ResetCurSelCellPos();
			// ��� ����
			MSG_ITEM_MIX_ACK * pmsg = (MSG_ITEM_MIX_ACK *)pMsg;

			CItem * pMatItemOut;
			for(int i = 0 ; i < pmsg->wMaterialNum ; ++i)
			{
				if(IsDupItem(pmsg->Material[i].wItemIdx))
				{
					CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->Material[i].ItemPos);
					if(pItem->GetDurability() - pmsg->Material[i].Dur != 0)
					{
						pItem->SetDurability(pItem->GetDurability() - pmsg->Material[i].Dur);
						continue;
					}
				}
				DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->Material[i].ItemPos, &pMatItemOut);
				//ItemDelete(pMatItemOut);
			}
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(209));
			QUICKMGR->RefreshQickItem();

			GAMEIN->GetTitanPartsMakeDlg()->DieCheck();
			if( GAMEIN->GetTitanPartsMakeDlg()->IsShopItemUse() )
			{
				GAMEIN->GetTitanPartsMakeDlg()->Release();
				GAMEIN->GetTitanPartsMakeDlg()->SetActiveRecursive( FALSE );
			}
		}
		break;
	case MP_ITEM_TPM_MSG:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;

			char buf[256];
			memset( buf, 0, sizeof(char)*256 );

			switch( pmsg->dwData1 )
			{
			case 20:			// ShopItem�� ����.
				sprintf( buf, CHATMGR->GetChatMsg(1086) );
				break;
			case 21:			// ����з� ���� ��ȣ�ƴ�.
				{
					CItem * BasicItemOut;
					DeleteItem( (POSTYPE)pmsg->dwData2, &BasicItemOut );
					sprintf( buf, CHATMGR->GetChatMsg(1088) );
				}
				break;
			case 22:			// ���зκ��� ��ȣ�ƴ�.
				sprintf( buf, CHATMGR->GetChatMsg(1087) );
				break;
			case 23:			// Level���ѿ� �ɸ���.
				sprintf( buf, CHATMGR->GetChatMsg(1082) );
				break;
			}

			CHATMGR->AddMsg( CTC_SYSMSG, buf );
			GAMEIN->GetTitanPartsMakeDlg()->Release();
			GAMEIN->GetTitanPartsMakeDlg()->DieCheck();
			GAMEIN->GetTitanPartsMakeDlg()->SetActiveRecursive( FALSE );
		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070130) //////////////////////////////////////////////////////////////////////////
	case MP_ITEM_TITANMIX_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Deal);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);

			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			CItem* pPreItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				if( pPreItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					DeleteItem(pmsg->GetItem(i)->Position, &pPreItem);
					ASSERT(pPreItem);
				}
				pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_BUY_ACK");	
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
				if(!rt)
				{
					ASSERT(0);					
				}
			}

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1556) );
		}
		break;
	case MP_ITEM_TITANMIX_ACK_DELETEPARTS:
		{
			MSG_ITEM_DISCARD_ACK* pmsg = (MSG_ITEM_DISCARD_ACK*)pMsg;

			GAMEIN->GetTitanMixDlg()->Release( eTMixAllRelease, FALSE );
			GAMEIN->GetTitanMixDlg()->UpdateData(NULL, FALSE);

			WORD wTableIdx = GetTableIdxForAbsPos(pmsg->TargetPos);

			DeleteItemofTable(wTableIdx, pmsg->TargetPos);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);
		}
		break;
	case MP_ITEM_TITANMIX_ADDITEM_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);			

			GAMEIN->GetTitanMixDlg()->DieCheck();
		}
		break;
	case MP_ITEM_TITANMIX_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanMixDlg()->Release( eTMixAllRelease, FALSE );
			GAMEIN->GetTitanMixDlg()->DieCheck();
		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070201)
	case MP_ITEM_TITANUPGRADE_ADDITEM_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);			

			GAMEIN->GetTitanUpgradeDlg()->DieCheck();
		}
		break;
	case MP_ITEM_TITANUPGRADE_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			if( pItem )
				pItem->SetLock( FALSE );

			switch(pmsg->wData2)
			{
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				{
					CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1581));
				}
				break;
			}

			GAMEIN->GetTitanUpgradeDlg()->Release(FALSE);
			GAMEIN->GetTitanUpgradeDlg()->DieCheck();
		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070202)
	case MP_ITEM_TITANUPGRADE_ACK:
	case MP_ITEM_TITANBREAK_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Deal);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);

			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			CItem* pPreItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				if( pPreItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					DeleteItem(pmsg->GetItem(i)->Position, &pPreItem);
					ASSERT(pPreItem);
				}
				pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_BUY_ACK");	
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
				if(!rt)
				{
					ASSERT(0);					
				}
				else
				{
					if(pmsg->Protocol == MP_ITEM_TITANBREAK_ACK)
					{
						CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1501), pItem->GetItemInfo()->ItemName, pItem->GetDurability());
					}
				}
			}

		}
		break;
	case MP_ITEM_TITANUPGRADE_NACK:
		{
			MSG_BYTE* pmsg = (MSG_BYTE*)pMsg;

			switch(pmsg->bData)
			{
			case 0:
				break;
			case 1:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(205) );
				break;
			case 2:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1502) );
				break;
			case 3:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(117) );
				break;
			}

		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070205)
	case MP_ITEM_TITANBREAK_ACK_DELETEITEM:
		{
			MSG_ITEM_DISCARD_ACK* pmsg = (MSG_ITEM_DISCARD_ACK*)pMsg;

			GAMEIN->GetTitanBreakDlg()->Release();

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			CItem * pItemOut;
			if(IsDupItem(pmsg->wItemIdx))
			{
				CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->TargetPos);

				if( pItem && (pItem->GetDurability() - pmsg->ItemNum != 0) )
				{
					pItem->SetDurability(pItem->GetDurability() - pmsg->ItemNum);
					break;
				}
			}

			DeleteItem(pmsg->TargetPos, &pItemOut);
		}
		break;
	case MP_ITEM_TITANBREAK_ADDITEM_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);			

			GAMEIN->GetTitanBreakDlg()->DieCheck();
		}
		break;
	case MP_ITEM_TITANBREAK_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanBreakDlg()->Release(FALSE);
			GAMEIN->GetTitanBreakDlg()->DieCheck();
		}
		break;
		//////////////////////////////////////////////////////////////////////////
	case MP_ITEM_TITANUPGRADE_ACK_DELETEMATERIAL:
		{
			MSG_ITEM_DISCARD_ACK* pmsg = (MSG_ITEM_DISCARD_ACK*)pMsg;

			GAMEIN->GetTitanUpgradeDlg()->Release();

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			CItem * pMatItemOut;
			if(IsDupItem(pmsg->wItemIdx))
			{
				CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->TargetPos);

				if( pItem && (pItem->GetDurability() - pmsg->ItemNum != 0) )
				{
					pItem->SetDurability(pItem->GetDurability() - pmsg->ItemNum);
					break;
				}
			}

			DeleteItem(pmsg->TargetPos, &pMatItemOut);
		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070321)
	case MP_ITEM_TITAN_REGISTER_ADDITEM_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);			
		}
		break;
	case MP_ITEM_TITAN_REGISTER_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanRegisterDlg()->Release(FALSE);
		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070417)
	case MP_ITEM_TITAN_REGISTER_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos((POSTYPE)pmsg->dwData2);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanRegisterDlg()->SetActive(FALSE);
			GAMEIN->GetTitanRegisterDlg()->DieCheck();

			TITAN_TOTALINFO* pInfo = TITANMGR->GetTitanInfo(pItem->GetDBIdx());
			if(!pInfo)
			{
				ASSERT(0);
				break;
			}
			pInfo->RegisterCharacterIdx = pmsg->dwData1;

			if(pInfo->RegisterCharacterIdx > 0)
			{
				TITANMGR->SetRegistedTitanItemDBIdx(pItem->GetDBIdx());
				STATSMGR->CalcTitanStats(pItem->GetDBIdx());
				ITEMPARAM Param = pItem->GetItemBaseInfo()->ItemParam;
				if(!(Param & ITEM_PARAM_TITAN_REGIST))
				{
					Param |= ITEM_PARAM_TITAN_REGIST;
					pItem->SetItemParam( Param );
				}
			}

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1511));
			RefreshItemToolTip(pItem->GetDBIdx());
		}
		break;
	case MP_ITEM_TITAN_REGISTER_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos((POSTYPE)pmsg->dwData2);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanRegisterDlg()->SetActive(FALSE);
			GAMEIN->GetTitanRegisterDlg()->DieCheck();
		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070322)
	case MP_ITEM_TITAN_DISSOLUTION_ADDITEM_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);			
		}
		break;
	case MP_ITEM_TITAN_DISSOLUTION_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanDissolutionDlg()->Release(FALSE);
		}
		break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070417)
	case MP_ITEM_TITAN_DISSOLUTION_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos((POSTYPE)pmsg->dwData2);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanDissolutionDlg()->SetActive(FALSE);
			GAMEIN->GetTitanDissolutionDlg()->DieCheck();

			TITAN_TOTALINFO* pInfo = TITANMGR->GetTitanInfo(pItem->GetDBIdx());
			if(!pInfo)
			{
				ASSERT(0);
				break;
			}
			pInfo->RegisterCharacterIdx = pmsg->dwData1;

			if(pInfo->RegisterCharacterIdx == 0)
			{
				TITANMGR->SetRegistedTitanItemDBIdx(0);
				ITEMPARAM Param = pItem->GetItemBaseInfo()->ItemParam;
				if(Param & ITEM_PARAM_TITAN_REGIST)
				{
					Param ^= ITEM_PARAM_TITAN_REGIST;
					pItem->SetItemParam( Param );
				}
			}

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1512));
			RefreshItemToolTip(pItem->GetDBIdx());
		}
		break;
	case MP_ITEM_TITAN_DISSOLUTION_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);			
			SetDisableDialog(FALSE, eItemTable_PetInven);

			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos((POSTYPE)pmsg->dwData2);
			if( pItem )
				pItem->SetLock( FALSE );

			GAMEIN->GetTitanDissolutionDlg()->SetActive(FALSE);
			GAMEIN->GetTitanDissolutionDlg()->DieCheck();
		}
		break;
		//////////////////////////////////////////////////////////////////////////
	case MP_ITEM_UPGRADE_SUCCESS_ACK:
		{
			MSG_ITEM_UPGRADE_ACK * pmsg = (MSG_ITEM_UPGRADE_ACK *)pMsg;
			//			CItem * item = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemPos);
			// ���� ������ ���� 
			CItem * itemOut;
			CItem * MaterialItemOut;
			ITEM_OPTION_INFO OptionInfo;
			DeleteItem(pmsg->ItemPos, &itemOut, &OptionInfo);
			DeleteItem(pmsg->MaterialItemPos, &MaterialItemOut);
			//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->ItemPos, &itemOut);

			// �޽��� ���
			int grade = pmsg->wItemIdx - itemOut->GetItemIdx();
			if(grade>0)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(176), grade );
			else if(grade == 0)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(177));
			else
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(178), grade );

			//GAMEIN->GetInventoryDialog()->DeleteItem(pmsg->MaterialItemPos, &MaterialItemOut)
			//ItemDelete(MaterialItemOut);
			if(OptionInfo.dwOptionIdx != 0)
				AddItemOption(&OptionInfo);
			ITEMBASE NewItem;
			NewItem = *(itemOut->GetItemBaseInfo());
			NewItem.Position = pmsg->ItemPos;
			NewItem.wIconIdx = pmsg->wItemIdx;
			NewItem.QuickPosition = 0;

			//ItemDelete(itemOut);

			// ���ο� ������ ����
			GAMEIN->GetInventoryDialog()->AddItem(&NewItem);
			// ���ο� Virtual Item ����
			GAMEIN->GetUpgradeDialog()->Release();
			CItem * pResultItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemPos);
			GAMEIN->GetUpgradeDialog()->AddVirtualItemWrap(eRESULTITEM_POS, pResultItem);
			pResultItem->SetLock(TRUE);
		}
		break;
	case MP_ITEM_UPGRADE_NACK:
		{
			ASSERT(0);
			//			GAMEIN->GetChatDialog()->AddChatMsg(WHOLE,RGBA_MAKE(255,0,0,255), "��޾��� ��� �Ǿ����ϴ�.");
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(176) );
			//GAMEIN->GetInventoryDialog()->SetState(eITEMDEFAULT_STATE);
		}
		break;
	case MP_ITEM_USE_NACK:
		{			
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);

			MSG_ITEM_ERROR * pmsg = (MSG_ITEM_ERROR*)pMsg;
			if(pmsg->ECode == eItemUseErr_PreInsert)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(678));
			else if( pmsg->ECode == eItemUseErr_TitanItemTime )
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(909));

			m_nItemUseCount--;
		}
		break;
	case MP_ITEM_SELL_ACK:
		{
			MSG_ITEM_SELL_ACK * pmsg = ( MSG_ITEM_SELL_ACK * )pMsg;	
			CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->TargetPos);
			if(IsDupItem(pmsg->wSellItemIdx) && (pItem->GetDurability() - pmsg->SellItemNum) > 0)
			{
				// update
				pItem->SetDurability( pItem->GetDurability() - pmsg->SellItemNum );
				//				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(218), pItem->GetItemInfo()->ItemName, pmsg->SellItemNum, AddComma( pItem->GetItemInfo()->SellPrice*pmsg->SellItemNum ) );
				DWORD SellPrice = SWPROFIT->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
				// �����
				SellPrice = FORTWARMGR->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(218), pItem->GetItemInfo()->ItemName, pmsg->SellItemNum, AddComma( SellPrice*pmsg->SellItemNum ) );
			}
			else
			{
				// delete
				//				if( pmsg->SellItemNum == 0 || IsOptionItem(pItem->GetItemIdx(), pItem->GetDurability()) )
				//					CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(217), pItem->GetItemInfo()->ItemName, AddComma( pItem->GetItemInfo()->SellPrice ) );
				//				else
				//					CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(218), pItem->GetItemInfo()->ItemName, pmsg->SellItemNum, AddComma( pItem->GetItemInfo()->SellPrice*pmsg->SellItemNum ) );
				DWORD SellPrice = SWPROFIT->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
				// �����
				SellPrice = FORTWARMGR->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
				if( pmsg->SellItemNum == 0 || IsOptionItem(pItem->GetItemIdx(), pItem->GetDurability()) )
					CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(217), pItem->GetItemInfo()->ItemName, AddComma( SellPrice ) );
				else
					CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(218), pItem->GetItemInfo()->ItemName, pmsg->SellItemNum, AddComma( SellPrice*pmsg->SellItemNum ) );

				DeleteItemofTable(GetTableIdxForAbsPos(pmsg->TargetPos), pmsg->TargetPos);
			}
			GAMEIN->GetInventoryDialog()->SetDisable( FALSE );
			GAMEIN->GetDealDialog()->SetDisable( FALSE );
			QUICKMGR->RefreshQickItem();
			/*
			CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->TargetPos);
			//			CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->wAbsPosition );

			//			if( pmsg->resultDur == 0 )
			{
			if( pmsg->sellCount == 0 )
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(217), pItem->GetItemInfo()->ItemName, pItem->GetItemInfo()->SellPrice );
			else
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(218), pItem->GetItemInfo()->ItemName, pmsg->sellCount, pItem->GetItemInfo()->SellPrice*pmsg->sellCount );

			DeleteItem( pmsg->wAbsPosition );
			}
			else
			{
			CItem * pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->wAbsPosition );
			pItem->SetDurability( pmsg->resultDur );

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(218), pItem->GetItemInfo()->ItemName, pmsg->sellCount, pItem->GetItemInfo()->SellPrice*pmsg->sellCount );
			}

			GAMEIN->GetInventoryDialog()->SetDisable( FALSE );
			GAMEIN->GetDealDialog()->SetDisable( FALSE );
			*/
		}
		break;

	case MP_ITEM_SELL_NACK:
		{
			MSG_ITEM_ERROR* pmsg = (MSG_ITEM_ERROR*)pMsg;

			if( pmsg->ECode == 9 )
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(355) );
			else if(pmsg->ECode == 11)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1552) );


			GAMEIN->GetInventoryDialog()->SetDisable( FALSE );
			GAMEIN->GetDealDialog()->SetDisable( FALSE );
		}
		break;
	case MP_ITEM_BUY_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Deal);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);

			BOOL SetQuickLink = FALSE; //TEST code
			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			CItem* pPreItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				if( pPreItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					DeleteItem(pmsg->GetItem(i)->Position, &pPreItem);
					ASSERT(pPreItem);
				}
				pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_BUY_ACK");	
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
				if(!rt)
				{
					ASSERT(0);					
				}
				if(pItem->GetQuickPosition() != 0)
				{
					if(SetQuickLink == TRUE)
					{
						char buf[256];
						sprintf(buf, "Set QuickPosition again! CharacterIDX : %d, ItemDBIdx : %d", HEROID, pItem->GetItemBaseInfo()->dwDBIdx);
						ASSERTMSG(0, buf);
					}

					//���� �� �������� �ε����� ��â�� ��ϵȰ��� �ִٸ�? ��â���� ������.
					//CQuickItem *pOldQuick = GAMEIN->GetQuickDialog()->GetQuickItem(pItem->GetQuickPosition());
					//GAMEIN->GetQuickDialog()->FakeAddItem(pItem->GetQuickPosition(), pItem, pOldQuick );

					WORD QuickPosList[4];
					CQuickManager::AnalyzeQuickPosition(pItem->GetQuickPosition(), QuickPosList);

					for(WORD SheetNum = 0; SheetNum < 4; SheetNum++)
					{
						if(QuickPosList[SheetNum])
							QUICKMGR->AddQuickItemReal(((SheetNum * 10) + (QuickPosList[SheetNum] - 1)),pItem->GetPosition(),pItem->GetItemIdx());
					}
					SetQuickLink = TRUE;
				}
			}
			MONEYTYPE BuyPrice;
			BuyPrice = pItem->GetItemInfo()->BuyPrice;
			if(MAP->IsVillage() == FALSE)
			{
				BuyPrice = (MONEYTYPE)(BuyPrice * 1.2);
			}
			BuyPrice = SWPROFIT->CalTexRateForBuy( BuyPrice );
			// �����
			BuyPrice = FORTWARMGR->CalTexRateForBuy( BuyPrice );
			if( pmsg->wObtainCount > 1 )
			{
				CHATMGR->AddMsg( CTC_GETITEM, CHATMGR->GetChatMsg(216), pItem->GetItemInfo()->ItemName, pmsg->wObtainCount,
					AddComma( BuyPrice * pmsg->wObtainCount ) );	//confirm
			}
			else
			{
				CHATMGR->AddMsg( CTC_GETITEM, CHATMGR->GetChatMsg(215), pItem->GetItemInfo()->ItemName, AddComma( BuyPrice ) );
			}

			QUICKMGR->RefreshQickItem();
		}
		break;

	case MP_ITEM_BUY_NACK:
		{
			MSG_ITEM_ERROR* pmsg = (MSG_ITEM_ERROR*)pMsg;

			//most of all, inventory full process : taiyo
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_Deal);
			//GAMEIN->GetDealDialog()->SetDisable( FALSE );

			switch(pmsg->ECode)
			{
			case NOT_MONEY:	
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(117) );
				break;
			case NOT_SPACE:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(354) );
				break;
			case NOT_EXIST:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(356) );
				break;
			case NOT_PLAYER:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(357) );
				break;
			case NO_DEMANDITEM:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1639) );
				break;
			case NOT_REMAINITEM:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1632) );
				break;
			default:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(358) );
				break;
			}

			//ASSERT(0);
		}
		break;
	case MP_ITEM_DEALER_ACK:
		{			
			MSG_WORD* pmsg = (MSG_WORD*)pMsg;
			if( HERO->GetState() != eObjectState_Die)
				GAMEIN->GetDealDialog()->ShowDealer(pmsg->wData);
		}
		break;

	case MP_ITEM_DEALER_NACK:
		{

		}
		break;

	case MP_ITEM_APPEARANCE_CHANGE:
		{
			APPEARANCE_INFO* pmsg = (APPEARANCE_INFO*)pMsg;
			CObject* pObject = (CObject*)OBJECTMGR->GetObject(pmsg->PlayerID);
			if( pObject && pObject->GetObjectKind() == eObjectKind_Player )
			{
				CPlayer* pPlayer = (CPlayer*)pObject;
				for(int n=0;n<eWearedItem_Max;++n)
					pPlayer->SetWearedItemIdx(n,pmsg->WearedItem[n]);
				APPEARANCEMGR->AddCharacterPartChange(pObject->GetID());
			}
			else
			{
				ASSERT(0);
			}
		}
		break;
		//SW070127 Ÿ��ź
	case MP_ITEM_TITAN_APPEARANCE_CHANGE:
		{
			SEND_TITAN_APPEARANCEINFO* pmsg = (SEND_TITAN_APPEARANCEINFO*)pMsg;
			CObject* pObject = (CObject*)OBJECTMGR->GetObject(pmsg->OwnerID);
			if( pObject && pObject->GetObjectKind() == eObjectKind_Player )
			{
				CPlayer* pPlayer = (CPlayer*)pObject;
				pPlayer->SetTitanAppearanceInfo(&pmsg->titanAppearanceInfo);
				APPEARANCEMGR->InitAppearance( pPlayer );				
				APPEARANCEMGR->AddCharacterPartChange(pPlayer->GetID());				
				pPlayer->SetBaseMotion();
			}
		}
		break;
	case MP_ITEM_MONEY:
		{
			MSG_MONEY* pmsg = (MSG_MONEY*)pMsg;

			MONEYTYPE OriMoney = HERO->GetMoney();
			MONEYTYPE PrintMoney = 0;

			switch(pmsg->bFlag)
			{
			case 64:
				{
					PrintMoney = pmsg->dwTotalMoney - OriMoney;
					if( PrintMoney )
					{
						CHATMGR->AddMsg( CTC_GETMONEY, CHATMGR->GetChatMsg( 120 ), AddComma( PrintMoney ) );

						// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(�� ȹ��)
						if( HERO->InTitan() == TRUE )
							EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetMoney);
						else
							EFFECTMGR->StartHeroEffectProcess(eEffect_GetMoney);
					}
				}
				break;
			case MF_NOMAL:
				{
				}
				break;
			case MF_OBTAIN: // Obtain Money;
				{
					PrintMoney = pmsg->dwTotalMoney - OriMoney;
					if( PrintMoney )
					{
						CHATMGR->AddMsg( CTC_GETMONEY, CHATMGR->GetChatMsg( 120 ), AddComma( PrintMoney ) );
						// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(�� ȹ��)
						if( HERO->InTitan() == TRUE )
							EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetMoney);
						else
							EFFECTMGR->StartHeroEffectProcess(eEffect_GetMoney);
					}
				}
				break;
			case MF_LOST:
				{
					PrintMoney = OriMoney - pmsg->dwTotalMoney;
					if( PrintMoney )
					{
						CHATMGR->AddMsg( CTC_GETMONEY, CHATMGR->GetChatMsg( 118 ), AddComma( PrintMoney) );
					}
				}
				break;
			default:
				break;
			}

			HERO->SetMoney(pmsg->dwTotalMoney);
		}
		break;
	case MP_ITEM_MONEY_ERROR:
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(115) );
		}
		break;
	case MP_ITEM_OBTAIN_MONEY:
		{
			ASSERT(0);
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;
			/*
			CChatDialog * dlg = GAMEIN->GetChatDialog();
			dlg->AddChatMsg(WHOLE,RGBA_MAKE(0,0,255,255),
			"[%s] ���� %d ������ϴ�.", HERO->GetObjectName(), pmsg->dwData - HERO->GetMoney());// pjslocal [5/29/2003]
			*/

			DWORD OriMoney = HERO->GetMoney();
			DWORD PrintMoney = pmsg->dwData - OriMoney;

			if( PrintMoney )
			{
				CHATMGR->AddMsg( CTC_GETMONEY, CHATMGR->GetChatMsg( 120 ), PrintMoney );
				// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(�� ȹ��)
				if( HERO->InTitan() == TRUE )
					EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetMoney);
				else
					EFFECTMGR->StartHeroEffectProcess(eEffect_GetMoney);
			}

			HERO->SetMoney(pmsg->dwData);
		}
		break;
	case MP_ITEM_DIVIDE_NEWITEM_NOTIFY:
		{
			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;
			CItem* pItem = MakeNewItem(&pmsg->ItemInfo,"MP_ITEM_DIVIDE_NEWITEM_NOTIFY");
			int tbidx = GetTableIdxForAbsPos(pItem->GetItemBaseInfo()->Position);
			if(tbidx == eItemTable_Inventory || tbidx == eItemTable_ShopInven)
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
			else if(tbidx == eItemTable_Pyoguk)
				BOOL rt = GAMEIN->GetPyogukDialog()->AddItem(pItem);
			else if(tbidx == eItemTable_GuildWarehouse)
				BOOL rt = GAMEIN->GetGuildWarehouseDlg()->AddItem(pItem);


			CHATMGR->AddMsg( CTC_GETITEM, CHATMGR->GetChatMsg( 123 ));
		}
		break;

		//////////////////////////////////////////////////////////////////////////
		// ����â��	
	case MP_ITEM_GUILD_MOVE_ACK:
		{
			MoveItemToGuild((MSG_ITEM_MOVE_ACK*)pMsg);
			WINDOWMGR->DragWindowNull();
		}
		break;
	case MP_ITEM_GUILD_MOVE_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			WINDOWMGR->BackDragWindow();
		}
		break;
	case MP_ITEM_GUILD_WAREHOUSE_INFO_ACK:
		{
			SEND_MUNPA_WAREHOUSE_ITEM_INFO* pmsg = (SEND_MUNPA_WAREHOUSE_ITEM_INFO*)pMsg;

			CAddableInfoIterator iter(&pmsg->AddableInfo);
			BYTE AddInfoKind;
			while((AddInfoKind = iter.GetInfoKind()) != CAddableInfoList::None)
			{
				switch(AddInfoKind)
				{
				case(CAddableInfoList::ItemOption):
					{
						ITEM_OPTION_INFO OptionInfo[TABCELL_GUILDWAREHOUSE_NUM];
						iter.GetInfoData(&OptionInfo);
						ITEMMGR->InitItemOption(OptionInfo, pmsg->wOptionCount);
					}
					break;
				case(CAddableInfoList::ItemRareOption):
					{
						ITEM_RARE_OPTION_INFO RareOptionInfo[TABCELL_GUILDWAREHOUSE_NUM];
						iter.GetInfoData(&RareOptionInfo);
						ITEMMGR->InitItemRareOption(RareOptionInfo, pmsg->wRareOptionCount);
					}
					break;
				}
				iter.ShiftToNextData();
			}

			GAMEIN->GetGuildWarehouseDlg()->SetWarehouseInfo(pmsg);
			RefreshAllItem();
		}
		break;
	case MP_ITEM_GUILD_WAREHOUSE_INFO_NACK:
		{
			ASSERTMSG(0, "warehouse_info_nack");
			GAMEIN->GetGuildWarehouseDlg()->SetLock(FALSE);
		}
		break;
	case MP_ITEM_DELETEFROM_GUILDWARE_NOTIFY:
		{
			MSG_ITEM * pItembase = (MSG_ITEM*)pMsg;
			CItem* pItem = NULL;
			if(GAMEIN->GetGuildWarehouseDlg()->DeleteItem(pItembase->ItemInfo.Position, &pItem) == TRUE)
				ITEMMGR->ItemDelete(pItem);
			else
			{
				ASSERT(0);
			}
		}
		break;
	case MP_ITEM_ADDTO_GUILDWARE_NOTIFY:
		{
			MSG_ITEM_WITH_OPTION * pItem = (MSG_ITEM_WITH_OPTION*)pMsg;
			if(pItem->IsOptionItem == TRUE)
			{
				if(IsOptionItem(pItem->ItemInfo.wIconIdx, pItem->ItemInfo.Durability) == TRUE)
				{
					AddItemOption(&pItem->OptionInfo);
				}
			}
			//SW050920 Rare
			if(pItem->IsRareOptionItem == TRUE)
			{
				if(IsRareOptionItem(pItem->ItemInfo.wIconIdx, pItem->ItemInfo.RareIdx) == TRUE)
				{
					AddItemRareOption(&pItem->RareOptionInfo);
				}
			}

			GAMEIN->GetGuildWarehouseDlg()->AddItem(&pItem->ItemInfo);
		}
		break;
		/*
		case MP_ITEM_MUNPA_WAREHOUSE_INFO_ACK:
		{
		SEND_MUNPA_WAREHOUSE_ITEM_INFO* pmsg = (SEND_MUNPA_WAREHOUSE_ITEM_INFO*)pMsg;
		for(int i=0; i < pmsg->wOptionCount; ++i)
		{
		AddItemOption(&pmsg->MunpaWareItemOption[i]);
		}

		GAMEIN->GetMunpaDialog()->SetWarehouseInfo(pmsg);
		RefreshAllItem();
		}
		break;
		case MP_ITEM_MUNPA_MOVE_ACK: 
		{
		MoveItemToMunpa( (MSG_ITEM_MOVE_ACK*)pMsg ); 
		WINDOWMGR->DragWindowNull();
		break;
		}
		case MP_ITEM_MUNPA_MOVE_NACK: 
		{
		ASSERT(0);
		}
		break;
		case MP_ITEM_DELETEFROM_MUNPAWARE_NOTIFY: 
		{
		MSG_ITEM * pItembase = (MSG_ITEM*)pMsg;
		CItem* pItem = NULL;
		if(GAMEIN->GetMunpaDialog()->DeleteItem(pItembase->ItemInfo.Position, &pItem) == TRUE)
		ITEMMGR->ItemDelete(pItem);
		else
		{
		ASSERT(0);
		}
		}
		break;
		case MP_ITEM_ADDTO_MUNPAWARE_NOTIFY:
		{
		MSG_ITEM * pItembase = (MSG_ITEM*)pMsg;
		GAMEIN->GetMunpaDialog()->AddItem(&pItembase->ItemInfo);
		}
		break;

		case MP_ITEM_MUNPA_WAREHOUSE_INFO_NACK:
		{
		//â�� �̿������ �����ϴ�.
		CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(48) );
		}
		break;*/

		/*
		case MP_ITEM_PYOGUK_INFO_ACK:
		{
		SEND_PYOGUK_ITEM_INFO* pmsg = (SEND_PYOGUK_ITEM_INFO*)pMsg;
		//Ȥ�ó� �ؼ� ���� â ����.
		GAMEIN->GetMunpaDialog()->SetActive(FALSE);
		GAMEIN->GetInventoryDialog()->SetActive(TRUE);
		GAMEIN->GetPyogukDialog()->SetActive(TRUE);
		GAMEIN->GetPyogukDialog()->SetPyogukItem(pmsg);
		GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_PyogukWare1);
		GAMEIN->GetPyogukDialog()->AddPyogukMode(ePyogukMode_PyogukWare1);
		RefreshAllItem();
		}
		break;*/

	case MP_ITEM_PYOGUK_MOVE_ACK:
		{
			MoveItemToPyoguk( (MSG_ITEM_MOVE_ACK*)pMsg );
			WINDOWMGR->DragWindowNull();
		}
		break;

	case MP_ITEM_DISSOLUTION_GETITEM:
		{
			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem	= NULL;
			CItem* pPreItem = NULL;
			for( int i = 0 ; i < pmsg->ItemNum ; ++i )
			{
				if( pPreItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					DeleteItem( pmsg->GetItem(i)->Position, &pPreItem );
					ASSERT( pPreItem );
				}
				pItem = MakeNewItem( pmsg->GetItem(i),"MP_ITEM_DISSOLUTION_GETITEM" );	
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem( pItem );
				if( !rt )
				{
					ASSERT(0);					
				}
			}		
		}
		break;
	case MP_ITEM_DISSOLUTION_ACK:
		{
			if( GAMEIN->GetDissolutionDialog() )
				GAMEIN->GetDissolutionDialog()->EndDissolution( TRUE );
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg( 692 ) );
		}
		break;
	case MP_ITEM_DISSOLUTION_NACK:
		{
			if( GAMEIN->GetDissolutionDialog() )
				GAMEIN->GetDissolutionDialog()->EndDissolution( FALSE );
		}
		break;

		//////////////////////////////////////////////////////////////////////////
		//nack
		/*
		case MP_ITEM_PYOGUK_INFO_NACK:
		GAMEIN->GetPyogukDialog()->ShowPyogukMode(ePyogukMode_NoWare); break;
		*/
	case MP_ITEM_ERROR_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);
			WINDOWMGR->BackDragWindow();
			MSG_ITEM_ERROR * pmsg = (MSG_ITEM_ERROR*)pMsg;
			if(pmsg->ECode == eItemUseErr_PreInsert)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(678));
			else if(pmsg->ECode == eItemUseErr_Discard)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(799));
			else if(pmsg->ECode == eItemUseErr_AlreadyUse)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(752) );
			else if(pmsg->ECode == eItemUseErr_Mix)
			{
				if( GAMEIN->GetMixDialog() )
					GAMEIN->GetMixDialog()->SetNowMixing( FALSE );
			}
			// magi82 - Titan(070118) ////////////////////////////////////////////////
			else if(pmsg->ECode == eItemUseErr_TitanPartsMake)
			{
				if( GAMEIN->GetTitanPartsMakeDlg() )
					GAMEIN->GetTitanPartsMakeDlg()->SetNowMixing( FALSE );
			}
			//////////////////////////////////////////////////////////////////////////
		}
		break;
	case MP_ITEM_CHANGEITEM_NACK: //������ ��ȯ ����
		{
			MSG_WORD * pmsg = (MSG_WORD*)pMsg;

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(354) );
		}
		break;

	case MP_ITEM_USE_CHANGEITEM_ACK:
		{
			ITEMOBTAINARRAY* pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				if( pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					pItem->SetItemBaseInfo(pmsg->GetItem(i));
				}
				else
				{
					pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_MONSTER_OBTAIN_NOTIFY");	
					BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
					if(!rt)
					{
						ASSERT(0);
					}
				}
			}

			//�������ϡ�aA��AI AOA����e Ao��i��U.
			cDialog* pDlg = WINDOWMGR->GetWindowForID( DBOX_DIVIDE_INV );
			if( pDlg )
			{
				((cDivideBox*)pDlg)->ExcuteDBFunc( 0 );	//Ae��O��o���� ���Ϣ��̡�a
			}

			if( pItem )
			{
				CHATMGR->AddMsg( CTC_GETITEM, CHATMGR->GetChatMsg( 121 ), pItem->GetItemInfo()->ItemName );
				//				EFFECTMGR->StartHeroEffectProcess(eEffect_GetItem);
				ItemDropEffect( pItem->GetItemIdx() );
			}

			QUICKMGR->RefreshQickItem();
		}
		break;
	case MP_ITEM_SHOPITEM_INFO_ACK:
		{
			SEND_SHOPITEM_INFO* pmsg = (SEND_SHOPITEM_INFO*)pMsg;

			if( pmsg->ItemCount )
			{
				GAMEIN->GetItemShopDialog()->SetItemInfo(pmsg);
			}
			else
			{
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(785) );
			}

			GAMEIN->GetItemShopDialog()->SetActive(TRUE);
			GAMEIN->GetInventoryDialog()->SetActive( TRUE );
			GAMEIN->GetInventoryDialog()->OnShopItemBtn();
			GAMEIN->GetInventoryDialog()->SetShopItemInvenBtn( TRUE );			
		}
		break;
	case MP_ITEM_SHOPITEM_USEDINFO:
		{
			SEND_SHOPITEM_USEDINFO* pmsg = (SEND_SHOPITEM_USEDINFO*)pMsg;

			for(int i=0; i<pmsg->ItemCount; i++)
			{
				AddUsedItemInfo(&pmsg->Item[i]);

				if( pmsg->Item[i].Param == eShopItemUseParam_EquipAvatar )
				{
					CItem* pItem = GetItem( pmsg->Item[i].ItemBase.dwDBIdx );
					if(pItem)
						pItem->SetUseParam( 1 );
				}

				// �ɼ��� ���ξ����. �׳� �ϵ��ڵ�.
				if( pmsg->Item[i].ItemBase.wIconIdx == eIncantation_MemoryMoveExtend ||
					pmsg->Item[i].ItemBase.wIconIdx == eIncantation_MemoryMoveExtend7 ||
					pmsg->Item[i].ItemBase.wIconIdx == eIncantation_MemoryMove2 ||
					pmsg->Item[i].ItemBase.wIconIdx == eIncantation_MemoryMoveExtend30 )
					HERO->CalcShopItemOption(pmsg->Item[i].ItemBase.wIconIdx, TRUE);
			}
		}
		break;
	case MP_ITEM_SHOPITEM_MPINFO:
		{
			SEND_MOVEDATA_INFO* pmsg = (SEND_MOVEDATA_INFO*)pMsg;			

			if( pmsg->Count == 0 && pmsg->bInited )
			{
				SetDisableDialog(FALSE, eItemTable_Inventory);
				SetDisableDialog(FALSE, eItemTable_Pyoguk);
				SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
				SetDisableDialog(FALSE, eItemTable_Shop);

				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(794) );

				OBJECTSTATEMGR->EndObjectState( HERO, eObjectState_Deal );
			}

			GAMEIN->GetMoveDialog()->SetMoveInfo( pmsg );
			if( pmsg->bInited )
				GAMEIN->GetMoveDialog()->SetActive( TRUE );
		}
		break;
	case MP_ITEM_SHOPITEM_USEEND:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			// �Ⱓ�� �̵��ֹ����� ������� �Ѵ�.
			ITEM_INFO* pInfo = GetItemInfo((WORD)pmsg->dwData );
			if( !pInfo )		return;
			SHOPITEMBASE* pShopItemBase = m_UsedItemList.GetData((WORD)pmsg->dwData);
			if( !pShopItemBase )		return;

			if( pInfo->ItemType == 11 )
			{
				BOOL bfind = FALSE;
				for(int i=0; i<SLOT_SHOPINVEN_NUM/2; i++)
				{
					ITEMBASE* pItemBase = (ITEMBASE*)GetItemInfoAbsIn( HERO, i+TP_SHOPINVEN_START );					
					if( pItemBase && pItemBase->dwDBIdx == pShopItemBase->ItemBase.dwDBIdx )
					{
						CItem* pOutItem = NULL;
						DeleteItem( i+TP_SHOPINVEN_START, &pOutItem );

						if( pOutItem )
						{
							char buf[256] = { 0, };
							sprintf( buf, CHATMGR->GetChatMsg(750), pOutItem->GetItemInfo()->ItemName );
							CHATMGR->AddMsg(CTC_SYSMSG, buf);
						}
						bfind = TRUE;
						break;
					}
				}
				if( !bfind )
				{
					for(int i=0; i<SLOT_WEAR_NUM; i++)
					{
						ITEMBASE* pItemBase = (ITEMBASE*)GetItemInfoAbsIn( HERO, i+TP_WEAR_START );					
						if( pItemBase && pItemBase->dwDBIdx == pShopItemBase->ItemBase.dwDBIdx )
						{
							CItem* pOutItem = NULL;
							DeleteItem( i+TP_WEAR_START, &pOutItem );

							if( pOutItem )
							{
								char buf[256] = { 0, };
								sprintf( buf, CHATMGR->GetChatMsg(750), pOutItem->GetItemInfo()->ItemName );
								CHATMGR->AddMsg(CTC_SYSMSG, buf);
							}

							bfind = TRUE;
							break;
						}
					}
				}
			}

			RemoveUsedItemInfo((WORD)pmsg->dwData, pShopItemBase->ItemBase.dwDBIdx);
			HERO->CalcShopItemOption((WORD)pmsg->dwData, FALSE);
		}
		break;
	case MP_ITEM_SHOPITEM_ONEMINUTE:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			STATUSICONDLG->SetOneMinuteToShopItem(pmsg->dwData);
		}
		break;		
	case MP_ITEM_SHOPITEM_USE_ACK:
		{
			SEND_SHOPITEM_BASEINFO* pmsg = (SEND_SHOPITEM_BASEINFO*)pMsg;

			CItem * item = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ShopItemPos);
			if( item )
			{
				eITEM_KINDBIT bits = item->GetItemKind();

				if( bits & eSHOP_ITEM )
				{
					ITEM_INFO* pInfo = GetItemInfo( item->GetItemBaseInfo()->wIconIdx );

					if( bits != eSHOP_ITEM_MAKEUP && bits != eSHOP_ITEM_DECORATION && bits != eSHOP_ITEM_PET )
					{
						if( item->GetDurability() > 1)
						{
							GAMEIN->GetInventoryDialog()->UpdateItemDurabilityAdd(pmsg->ShopItemPos,-1);
						}
						//
						else
						{							
							if( pInfo && pInfo->ItemType == 10 && pInfo->ItemIdx != eIncantation_ChangeName &&
								pInfo->ItemIdx != eIncantation_ChangeName_Dntrade )
							{
								CItem* pOutItem = NULL;
								DeleteItem( pmsg->ShopItemPos, &pOutItem);
								if( pOutItem )
									ReLinkQuickPosition( pOutItem );
							}
						}

						char buf[256] = { 0, };
						wsprintf( buf, CHATMGR->GetChatMsg(795), item->GetItemInfo()->ItemName );
						CHATMGR->AddMsg( CTC_SYSMSG, buf );
					}

					if( pInfo->ItemType == 10 && pmsg->ShopItemBase.Param )
						AddUsedItemInfo(&pmsg->ShopItemBase);

					HERO->CalcShopItemOption(pmsg->ShopItemIdx, TRUE);

					/*
					#ifdef _TL_LOCAL_
					if( pInfo->ItemIdx == eIncantation_MugongExtend || pInfo->ItemIdx == eIncantation_CharacterSlot ||
					pInfo->ItemIdx == eIncantation_InvenExtend || pInfo->ItemIdx == eIncantation_PyogukExtend )
					CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1305) );
					#endif
					*/


					// use sound
					// AUDIOMGR->Play(68, HERO);
				}

				QUICKMGR->RefreshQickItem();
			}
		}
		break;
	case MP_ITEM_SHOPITEM_USE_NACK:
		{
			MSG_ITEM_ERROR* pmsg = (MSG_ITEM_ERROR*)pMsg;

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			if( HERO->GetState() == eObjectState_Deal )			
				OBJECTSTATEMGR->EndObjectState( HERO, eObjectState_Deal );

			switch(pmsg->ECode)
			{
			case eItemUseErr_Lock:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(751) );
				break;
			case eItemUseErr_AlreadyUse:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(752) );
				break;
			case eItemUseErr_Unabletime:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(757) );
				break;
			case eItemUseErr_AlredyChange:
				//CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(912) );
				break;
			case eItemuseErr_DontUseToday:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1079) );
				break;
			case eItemUseErr_DontDupUse:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1076) );
				break;
			case eItemUseErr_UseFull:
				CHATMGR->AddMsg( CTC_ALERT_YELLOW, CHATMGR->GetChatMsg(1435) );
				break;
			}
		}
		break;
	case MP_ITEM_SHOPITEM_MSG:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

		}
		break;
	case MP_ITEM_SHOPITEM_UNSEAL_ACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			GAMEIN->GetInventoryDialog()->ItemUnSealing(pmsg->dwData);
			QUICKMGR->RefreshQickItem();
		}
		break;
	case MP_ITEM_SHOPITEM_UNSEAL_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(777) );
		}
		break;
	case MP_ITEM_SHOPITEM_MONEYPROTECT:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(753) );
			RemoveUsedItemInfo( (WORD)pmsg->dwData );
		}
		break;
	case MP_ITEM_SHOPITEM_EXPPROTECT:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(754) );
			RemoveUsedItemInfo( (WORD)pmsg->dwData );
		}
		break;
	case MP_ITEM_SHOPITEM_MIXUP:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(755) );
			RemoveUsedItemInfo( (WORD)pmsg->dwData );
		}
		break;
	case MP_ITEM_SHOPITEM_STATEMINUS:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;
			GAMEIN->GetCharacterDialog()->RefreshPointInfo();
		}
		break;
	case MP_ITEM_SHOPITEM_SAVEPOINT_ADD_ACK:
		{
			SEND_MOVEDATA_SIMPLE* pmsg = (SEND_MOVEDATA_SIMPLE*)pMsg;

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			if( HERO->GetState() == eObjectState_Deal )
				OBJECTSTATEMGR->EndObjectState( HERO, eObjectState_Deal );

			GAMEIN->GetMoveDialog()->AddMoveInfo( &pmsg->Data );

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(792) );
		}
		break;
	case MP_ITEM_SHOPITEM_SAVEPOINT_ADD_NACK:
		{
			SEND_MOVEDATA_SIMPLE* pmsg = (SEND_MOVEDATA_SIMPLE*)pMsg;

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			if( HERO->GetState() == eObjectState_Deal )
				OBJECTSTATEMGR->EndObjectState( HERO, eObjectState_Deal );

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(793) );
		}
		break;
	case MP_ITEM_SHOPITEM_SAVEPOINT_UPDATE_ACK:
		{
			SEND_MOVEDATA_SIMPLE* pmsg = (SEND_MOVEDATA_SIMPLE*)pMsg;

			GAMEIN->GetMoveDialog()->UpdateMoveInfo( &pmsg->Data );
		}
		break;
	case MP_ITEM_SHOPITEM_SAVEPOINT_DEL_ACK:
		{
			SEND_MOVEDATA_SIMPLE* pmsg = (SEND_MOVEDATA_SIMPLE*)pMsg;

			GAMEIN->GetMoveDialog()->DelMoveInfo( &pmsg->Data );
		}
		break;
	case MP_ITEM_SHOPITEM_REVIVEOTHER_SYN:
		{
			SEND_REVIVEOTHER* pmsg = (SEND_REVIVEOTHER*)pMsg;

			if( !GAMEIN->GetReviveDialog()->IsActive() )
			{
				MSG_DWORD2 msg;
				msg.Category = MP_ITEM;
				msg.Protocol = MP_ITEM_SHOPITEM_REVIVEOTHER_NACK;
				msg.dwObjectID = HEROID;
				msg.dwData1 = pmsg->TargetID;
				msg.dwData2 = eShopItemErr_Revive_NotReady;			// Not Ready
				NETWORK->Send( &msg, sizeof(msg) );
				return;
			}

			GAMEIN->GetInventoryDialog()->SetReviveData( pmsg->TargetID, 0, 0 );

			char buf[128] = { 0, };
			wsprintf( buf, CHATMGR->GetChatMsg(762), pmsg->TargetName );
			WINDOWMGR->MsgBox( MBI_REVIVECONFIRM, MBT_YESNO, buf );
		}
		break;
	case MP_ITEM_SHOPITEM_REVIVEOTHER_ACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);	

			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->dwData);
			if(!pPlayer)		return;
		}
		break;
	case MP_ITEM_SHOPITEM_REVIVEOTHER_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			switch( pmsg->dwData )
			{
			case eShopItemErr_Revive_Fail:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(765) );
				break;
			case eShopItemErr_Revive_NotDead:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(765) );
				break;
			case eShopItemErr_Revive_NotUse:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(786) );
				break;
			case eShopItemErr_Revive_Refuse:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(787) );
				break;
			case eShopItemErr_Revive_TooFar:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(789) );
				break;
			case eShopItemErr_Revive_TimeOver:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(790) );
				break;
			case eShopItemErr_Revive_NotReady:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(920) );
				break;
			}

			cDialog* pDlg = WINDOWMGR->GetWindowForID( MBI_REVIVECONFIRM );
			if( pDlg )
				WINDOWMGR->AddListDestroyWindow( pDlg );
		}
		break;

	case MP_ITEM_SHOPITEM_AVATAR_PUTON:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CObject* pObject = (CObject*)OBJECTMGR->GetObject(pmsg->dwData1);
			if( pObject && pObject->GetObjectKind() == eObjectKind_Player )
			{
				APPEARANCEMGR->SetAvatarItem( (CPlayer*)pObject, (WORD)pmsg->dwData2 );				
			}
			else
			{
				ASSERT(0);
			}
		}
		break;
	case MP_ITEM_SHOPITEM_AVATAR_TAKEOFF:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CObject* pObject = (CObject*)OBJECTMGR->GetObject(pmsg->dwData1);
			if( pObject && pObject->GetObjectKind() == eObjectKind_Player )
			{
				APPEARANCEMGR->SetAvatarItem( (CPlayer*)pObject, (WORD)pmsg->dwData2, FALSE );
			}
			else
			{
				ASSERT(0);
			}
		}
		break;
	case MP_ITEM_SHOPITEM_AVATAR_USE:	
		{
			SEND_SHOPITEM_USEONE* pmsg = (SEND_SHOPITEM_USEONE*)pMsg;

			AddUsedItemInfo(&pmsg->ShopItemBase);
		}
		break;


	case MP_ITEM_SHOPITEM_AVATAR_INFO:
		{
			SEND_AVATARITEM_INFO* pmsg = (SEND_AVATARITEM_INFO*)pMsg;
			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->PlayerId);
			if(!pPlayer)		return;

			AVATARITEM* pInfo= GAMERESRCMNGR->m_AvatarEquipTable.GetData( pmsg->ItemIdx );
			if(!pInfo)		return;	

			if( pmsg->PlayerId == HERO->GetID() )
				HERO->CheckShopItemUseInfoToInventory( pmsg->ItemIdx, pmsg->ItemPos );

			if( pInfo->Position == eAvatar_Hair )
			{
				int MotionIdx = -1;
				WORD Weapontype = pPlayer->GetWeaponEquipType();
				if( Weapontype==WP_GUM || Weapontype==WP_GWUN || Weapontype==WP_GUNG || Weapontype==WP_AMGI ||
					Weapontype==WP_EVENT || Weapontype == WP_EVENT_HAMMER )
					MotionIdx = eMotion_Item_ChangeHair_1;
				else if( Weapontype==WP_DO )
					MotionIdx = eMotion_Item_ChangeHair_2;
				else if( Weapontype==WP_CHANG )
					MotionIdx = eMotion_Item_ChangeHair_3;

				if( pPlayer->GetID() == HERO->GetID() )
					MOVEMGR->HeroMoveStop();

				if( pPlayer->GetState() == eObjectState_None )
				{
					pPlayer->SetItemUseMotionIdx(MotionIdx);
					pPlayer->GetEngineObject()->ChangeMotion(MotionIdx, FALSE);
				}

				OBJECTSTATEMGR->StartObjectState(pPlayer, eObjectState_ItemUse);
				OBJECTSTATEMGR->EndObjectState(pPlayer, eObjectState_ItemUse, pPlayer->GetEngineObject()->GetAnimationTime(MotionIdx));				
				memcpy( pPlayer->GetShopItemStats()->Avatar, pmsg->Avatar, sizeof(WORD)*eAvatar_Max );
			}
			else
			{
				memcpy( pPlayer->GetShopItemStats()->Avatar, pmsg->Avatar, sizeof(WORD)*eAvatar_Max );
				APPEARANCEMGR->InitAppearance( pPlayer );
			}

			pPlayer->CalcAvatarOption();
		}
		break;
	case MP_ITEM_SHOPITEM_AVATAR_USE_ACK:
		{
			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->wData2 );
			if( !pItem )		return;
			pItem->SetUseParam( 1 );

			char buf[256] = { 0, };
			wsprintf( buf, CHATMGR->GetChatMsg(779), pItem->GetItemInfo()->ItemName );
			CHATMGR->AddMsg( CTC_SYSMSG, buf );
		}
		break;
	case MP_ITEM_SHOPITEM_AVATAR_USE_ACKDB:
		{
			SEND_SHOPITEM_BASEINFO* pmsg = (SEND_SHOPITEM_BASEINFO*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->ShopItemPos );
			if( !pItem )		return;
			pItem->SetUseParam( 1 );

			char buf[256] = { 0, };
			wsprintf( buf, CHATMGR->GetChatMsg(779), pItem->GetItemInfo()->ItemName );
			CHATMGR->AddMsg( CTC_SYSMSG, buf );

			AddUsedItemInfo( &pmsg->ShopItemBase );
		}
		break;
	case MP_ITEM_SHOPITEM_AVATAR_USE_NACK:
		{
			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->wData2 );
			if( !pItem )		return;
			pItem->SetUseParam( 0 );

			char buf[256] = { 0, };
			wsprintf( buf, CHATMGR->GetChatMsg(780), pItem->GetItemInfo()->ItemName );
			CHATMGR->AddMsg( CTC_SYSMSG, buf );
		}
		break;
	case MP_ITEM_SHOPITEM_AVATAR_DISCARD:
		{
			SEND_AVATARITEM_INFO* pmsg = (SEND_AVATARITEM_INFO*)pMsg;
			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->PlayerId);
			if(!pPlayer)		return;

			memcpy( pPlayer->GetShopItemStats()->Avatar, pmsg->Avatar, sizeof(WORD)*eAvatar_Max );
			APPEARANCEMGR->InitAppearance( pPlayer );
		}
		break;
	case MP_ITEM_SHOPITEM_EQUIP_USEINFODB:
		{
			SEND_SHOPITEM_BASEINFO* pmsg = (SEND_SHOPITEM_BASEINFO*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->ShopItemPos );
			if( !pItem )
			{
				pItem = GAMEIN->GetPetInventoryDialog()->GetItemForPos( pmsg->ShopItemPos );
			}

			if( !pItem )
				return;

			AddUsedItemInfo( &pmsg->ShopItemBase );
		}
		break;
	case MP_ITEM_SHOPITEM_PET_USE:
		{
			SEND_SHOPITEM_USEONE* pmsg = (SEND_SHOPITEM_USEONE*)pMsg;

			//!!!Ŭ���̾�Ʈ���� �����. �Ǵ� �ʼ��� ������ �� ������ ����
			//!!!������ ������ ������ ��ȭ.
			AddUsedItemInfo(&pmsg->ShopItemBase);
		}
		break;
	case MP_ITEM_SHOPITEM_EFFECT:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;

			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->dwData1);
			if(!pPlayer)		return;

			EFFECTMGR->StartPlayerEffectProcess( pPlayer, pmsg->dwData2 );
		}
		break;
	case MP_ITEM_SHOPITEM_SLOTOVERITEM:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			if( pmsg->dwData > SLOT_SHOPITEM_IMSI )
			{				
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(796) );
			}
		}
		break;
	case MP_ITEM_SHOPITEM_PROTECTALL:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(729) );								
			HERO->GetShopItemStats()->ProtectCount = (char)pmsg->dwData;
		}
		break;
	case MP_ITEM_SHOPITEM_CHASE_ACK:
		{
			SEND_CHASEINFO* pmsg = (SEND_CHASEINFO*)pMsg;

			//if( MAP->GetMapNum() == RUNNINGMAP || MAP->GetMapNum() == PKEVENTMAP )
			if( MAP->IsMapSame(eRunningMap) || MAP->GetMapNum() == PKEVENTMAP )
			{
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1457) );
				return;
			}

			VECTOR3 pos;
			pmsg->Pos.Decompress( &pos );
			if( GAMEIN->GetChaseDlg()->InitMiniMap( pmsg->MapNum, (WORD)pos.x, (WORD)pos.z, pmsg->WandtedName, pmsg->EventMapNum ) )
			{
				GAMEIN->GetChaseDlg()->SetActive( TRUE );
			}
			else
			{
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(908) );
			}
		}
		break;
	case MP_ITEM_SHOPITEM_CHASE_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			GAMEIN->GetChaseDlg()->SetActive( FALSE );

			if(pmsg->dwData == 1)
				WINDOWMGR->MsgBox( MBI_CHASE_NACK, MBT_OK, CHATMGR->GetChatMsg(907) );
			else if(pmsg->dwData == 2)
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(908) );
			else if(pmsg->dwData == 3 )
				WINDOWMGR->MsgBox( MBI_CHASE_NACK, MBT_OK, CHATMGR->GetChatMsg(921) );
		}
		break;
	case MP_ITEM_SHOPITEM_CHASE_TRACKING:
		{
			//			CHATMGR->AddMsg( CTC_TRACKING, CHATMGR->GetChatMsg(910) );
		}
		break;
	case MP_ITEM_SHOPITEM_NCHANGE_ACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			WINDOWMGR->MsgBox( MBI_CHANGENAME_ACK, MBT_OK, CHATMGR->GetChatMsg(917) );

			// Item Position Ȯ��
			for(int i=0; i<SLOT_SHOPINVEN_NUM/2; i++)
			{
				ITEMBASE* pItemBase = (ITEMBASE*)GetItemInfoAbsIn( HERO, i+TP_SHOPINVEN_START );
				if( !pItemBase )	continue;

				if( pItemBase->dwDBIdx == GAMEIN->GetNameChangeDlg()->GetItemDBIdx() )
				{
					CItem* pItem = NULL;
					DeleteItem( i+TP_SHOPINVEN_START, &pItem );
					break;
				}
			}
		}
		break;
	case MP_ITEM_SHOPITEM_NCHANGE_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			switch( pmsg->dwData )
			{
			case 6:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(912) );
				break;
			case 5:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(916) );
				break;
			case 4:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(913) );
				break;
			case 3:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(914) );
				break;
			case 2:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(915) );
				break;
			}
		}
		break;
	case MP_ITEM_SHOPITEM_CHARCHANGE_ACK:
		{
			MSGBASE* pmsg = (MSGBASE*)pMsg;

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			OBJECTSTATEMGR->EndObjectState( HERO, eObjectState_Deal );

			GAMEIN->GetCharChangeDlg()->Reset( TRUE );
			GAMEIN->GetCharChangeDlg()->SetActive( FALSE );
		}
		break;
	case MP_ITEM_SHOPITEM_CHARCHANGE_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			switch( pmsg->dwData )
			{
			case 1:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(786) );
				break;
			case 2:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1185) );
				break;
			case 3:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1185) );
				break;
			case 4:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1185) );
				break;
			case 5:				
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(786) );
				break;
			case 6:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1184) );
				break;
			}

			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			OBJECTSTATEMGR->EndObjectState( HERO, eObjectState_Deal );

			GAMEIN->GetCharChangeDlg()->Reset( FALSE );
			GAMEIN->GetCharChangeDlg()->SetActive( FALSE );
		}
		break;
	case MP_ITEM_SHOPITEM_CHARCHANGE:
		{
			SEND_CHARACTERCHANGE_INFO* pmsg = (SEND_CHARACTERCHANGE_INFO*)pMsg;

			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject( pmsg->Param );
			if(!pPlayer)		return;

			pPlayer->SetCharChangeInfo( &pmsg->Info );
		}
		break;
	case MP_ITEM_SHOPITEM_SEAL_ACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			if( GAMEIN->GetSealDlg() )
				GAMEIN->GetSealDlg()->ItemSealAck();
		}
		break;
	case MP_ITEM_SHOPITEM_SEAL_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			if( GAMEIN->GetSealDlg() )
				GAMEIN->GetSealDlg()->ItemSealNAck();
		}
		break;
	case MP_ITEM_SHOPITEM_JOBCHANGE_ACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			WINDOWMGR->MsgBox( MBI_JOBCHANGE_ACK, MBT_OK, CHATMGR->GetChatMsg(1143) );
			CChangeJobDialog* pDlg = GAMEIN->GetChangeJobDlg();
			if( !pDlg )		return;

			// Item Position Ȯ��
			for(int i=0; i<SLOT_SHOPINVEN_NUM/2; i++)
			{
				ITEMBASE* pItemBase = (ITEMBASE*)GetItemInfoAbsIn( HERO, i+TP_SHOPINVEN_START );
				if( !pItemBase )	continue;

				if( pItemBase->dwDBIdx == pDlg->GetItemDBIdx() && pItemBase->Position == pDlg->GetItemPos() )
				{
					CItem* pItem = NULL;
					DeleteItem( i+TP_SHOPINVEN_START, &pItem );
					break;
				}
			}
		}
		break;
	case MP_ITEM_SHOPITEM_JOBCHANGE_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			switch( pmsg->dwData )
			{
			case 1:			// ȭ���̳� �ظ��� �ƴ�
				break;
			case 2:			// �������� ����� �� ����
				break;
			}

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(786) );
			GAMEIN->GetChangeJobDlg()->CancelChangeJob();
		}
		break;
	case MP_ITEM_SHOPITEM_REINFORCERESET_ACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			if( GAMEIN->GetReinforceResetDlg() )
				GAMEIN->GetReinforceResetDlg()->ItemResetAck();
		}
		break;
	case MP_ITEM_SHOPITEM_REINFORCERESET_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			if( GAMEIN->GetReinforceResetDlg() )
				GAMEIN->GetReinforceResetDlg()->ItemResetNAck();

			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1233) );
		}
		break;
	case MP_ITEM_SHOPITEM_RARECREATE_ACK:
		{
			MSG_ITEM_RAREITEM_GET* pmsg = (MSG_ITEM_RAREITEM_GET*)pMsg;

			CItem* pItem = NULL;
			pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->RareItemBase.Position);

			if( !pItem )
			{
				GAMEIN->GetRareCreateDlg()->ItemRareCreateNAck();
				return;
			}

			ITEMBASE* pItemBase = (ITEMBASE*)pItem->GetItemBaseInfo();
			if( !pItemBase )
			{
				GAMEIN->GetRareCreateDlg()->ItemRareCreateNAck();
				return;
			}

			if( pItemBase->RareIdx )
				RemoveItemRareOption( pItem->GetItemBaseInfo()->RareIdx );			
			pItemBase->RareIdx = pmsg->RareInfo.dwRareOptionIdx;
			AddItemRareOption(&pmsg->RareInfo);

			//
			GAMEIN->GetRareCreateDlg()->ItemRareCreateAck();
			QUICKMGR->RefreshQickItem();
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1235) );
		}
		break;
	case MP_ITEM_SHOPITEM_RARECREATE_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			GAMEIN->GetRareCreateDlg()->ItemRareCreateNAck();
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1236) );
		}
		break;
	case MP_ITEM_SHOPITEM_SHOUT_SENDACK:
		{
			SEND_SHOUTRECEIVE* pmsg = (SEND_SHOUTRECEIVE*)pMsg;

			WINDOWMGR->MsgBox( MBI_SHOUT_ACK, MBT_OK, CHATMGR->GetChatMsg(905), pmsg->Receive.Time/60+1, pmsg->Receive.Count );
		}
		break;

	case MP_ITEM_SHOPITEM_SHOUT_NACK:
		{
			WINDOWMGR->MsgBox( MBI_SHOUT_NACK, MBT_OK, CHATMGR->GetChatMsg(904) );
		}
		break;

#ifdef TAIWAN_LOCAL
	case MP_ITEM_EVENTITEM_USE:
		{
			if( pmsg->dwData2 == 53074 )
			{
				EFFECTMGR->StartPlayerEffectProcess( pPlayer, FindEffectNum("m_ba_079.beff") );
			}
			// 2005 ũ�������� �̺�Ʈ �ڵ�
			if( pmsg->dwData2 == EVENT_ITEM_FIRECRACKER )
			{
				EFFECTMGR->StartPlayerEffectProcess( pPlayer, EVENT_EFFECT_FIRECRACKER );
			}

			// 06. 03. ȭ��Ʈ���� �̺�Ʈ
			if( pmsg->dwData2 = 53151 )
			{
				EFFECTMGR->StartPlayerEffectProcess( pPlayer, FindEffectNum("m_ba_082.beff") );
			}
		}
		break;
#else
		// 2005 ũ�������� �̺�Ʈ �ڵ�
	case MP_ITEM_EVENTITEM_USE:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->dwData1);

			if( pmsg->dwData2 == EVENT_ITEM_FIRECRACKER )
			{
				EFFECTMGR->StartPlayerEffectProcess( pPlayer, EVENT_EFFECT_FIRECRACKER );
			}

			// 06. 03. ȭ��Ʈ���� �̺�Ʈ
			if( pmsg->dwData2 == 53151 )
			{
				EFFECTMGR->StartPlayerEffectProcess( pPlayer, FindEffectNum("m_ba_082.beff") );
			}
		}
		break;
#endif
	case MP_ITEM_FULLINVEN_ALERT:
		{
			CHATMGR->AddMsg( CTC_ALERT_YELLOW, CHATMGR->GetChatMsg( 1435 ) );
		}
		break;
		// magi82 - Titan(070319)
		//case MP_ITEM_TITAN_MUGONG_DELETEITEM:
		//	{
		//		MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

		//		WORD wTableIdx = GetTableIdxForAbsPos((POSTYPE)pmsg->dwData);
		//		DeleteItemofTable(wTableIdx, (POSTYPE)pmsg->dwData);
		//	}
		//	break;
		//////////////////////////////////////////////////////////////////////////
		// magi82 - Titan(070509)
	case MP_ITEM_TITAN_DISCARD_EQUIPITEM:
		{
			MSG_ITEM_DISCARD_ACK* pmsg = (MSG_ITEM_DISCARD_ACK*)pMsg;
			WORD wTableIdx = GetTableIdxForAbsPos(pmsg->TargetPos);

			DeleteItemofTable(wTableIdx, pmsg->TargetPos);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);
			QUICKMGR->RefreshQickItem();

			//TITANMGR->CheckNotifyMsg( TNM_EQUIP_NUM );
		}
		break;
		//////////////////////////////////////////////////////////////////////////
	case MP_ITEM_TITAN_EQUIP_REPAIR_ACK:
		{
			MSG_TITAN_REPAIR_EQUIPITEM_SYN* pmsg = (MSG_TITAN_REPAIR_EQUIPITEM_SYN*)pMsg;

			TITAN_ENDURANCE_ITEMINFO* pEndurance = TITANMGR->GetTitanEnduranceInfo(pmsg->RepairInfo.ItemDBIdx);
			if( !pEndurance )
			{
				return;
			}

			CopyMemory( pEndurance, &pmsg->RepairInfo, sizeof(TITAN_ENDURANCE_ITEMINFO) );
			RefreshItemToolTip(pmsg->RepairInfo.ItemDBIdx);
			TITANMGR->SetTitanEnduranceView(pmsg->RepairInfo.ItemDBIdx);
			CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1540));
		}
		break;
	case MP_ITEM_TITAN_EQUIP_REPAIR_NACK:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;

			// 0�̸� ������ ����, 1�̸� �ش�������� ������ Ǯ, 2�� ��� �������� ������ Ǯ
			if(pmsg->dwData == 0)
			{
				CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1579));
			}
			else if(pmsg->dwData == 1)
			{
				CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1541));
			}
			else if(pmsg->dwData == 2)
			{
				CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1582));
			}
		}
		break;
	case MP_ITEM_TITAN_EQUIP_REPAIR_TOTAL_ACK:
		{
			MSG_TITAN_REPAIR_TOTAL_EQUIPITEM_ACK* pmsg = (MSG_TITAN_REPAIR_TOTAL_EQUIPITEM_ACK*)pMsg;

			for( int i = 0; i < pmsg->wRepairNum; i++ )
			{
				TITAN_ENDURANCE_ITEMINFO* pEndurance = TITANMGR->GetTitanEnduranceInfo(pmsg->RepairInfo[i].ItemDBIdx);
				CopyMemory(pEndurance, &pmsg->RepairInfo[i], sizeof(TITAN_ENDURANCE_ITEMINFO));
				RefreshItemToolTip(pmsg->RepairInfo[i].ItemDBIdx);
				TITANMGR->SetTitanEnduranceView(pmsg->RepairInfo[i].ItemDBIdx);
			}

			CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1542));
		}
		break;
	default:
		break;
	}
}



void CItemManager::NetworkMsgParseExt(BYTE Protocol,void* pMsg)
{
	switch(Protocol)
	{
	case MP_ITEMEXT_SHOPITEM_CURSE_CANCELLATION_ADDITEM_ACK:

		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);
		}
		break;
	case MP_ITEMEXT_SHOPITEM_CURSE_CANCELLATION_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			pItem->SetLock(FALSE);
			GAMEIN->GetUniqueItemCurseCancellationDlg()->Release();
		}
		break;
	case MP_ITEMEXT_SHOPITEM_CURSE_CANCELLATION_ACK:
		{
			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			CItem* pPreItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				if( pPreItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					DeleteItem(pmsg->GetItem(i)->Position, &pPreItem);
					ASSERT(pPreItem);
				}
				pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_BUY_ACK");	
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
				if(!rt)
				{
					ASSERT(0);					
				}
			}

			CHATMGR->AddMsg(CTC_SYSMSG, CHATMGR->GetChatMsg(1627));

			if( GAMEIN->GetUniqueItemCurseCancellationDlg()->IsActive() == TRUE )
			{
				GAMEIN->GetUniqueItemCurseCancellationDlg()->SetActive(FALSE);
			}
		}
		break;
		// magi82 - UniqueItem(070705)
	case MP_ITEMEXT_SHOPITEM_CURSE_CANCELLATION_DELETEITEM:
		{
			MSG_ITEM_DISCARD_ACK* pmsg = (MSG_ITEM_DISCARD_ACK*)pMsg;

			GAMEIN->GetUniqueItemCurseCancellationDlg()->Release(FALSE);

			WORD wTableIdx = GetTableIdxForAbsPos(pmsg->TargetPos);

			DeleteItemofTable(wTableIdx, pmsg->TargetPos);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_GuildWarehouse);
			SetDisableDialog(FALSE, eItemTable_PetInven);
		}
		break;
		// magi82 - UniqueItem(070710)
	case MP_ITEMEXT_UNIQUEITEM_MIX_ADDITEM_ACK:

		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			MSG_ITEM* pmsg = (MSG_ITEM*)pMsg;

			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->ItemInfo.Position);
		}
		break;
	case MP_ITEMEXT_UNIQUEITEM_MIX_ADDITEM_NACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			MSG_WORD2* pmsg = (MSG_WORD2*)pMsg;
			CItem* pItem = GAMEIN->GetInventoryDialog()->GetItemForPos(pmsg->wData1);
			pItem->SetLock(FALSE);
			GAMEIN->GetUniqueItemMixDlg()->Release();
		}
		break;
	case MP_ITEMEXT_UNIQUEITEM_MIX_DELETEITEM:
		{
			MSG_ITEM_DISCARD_ACK* pmsg = (MSG_ITEM_DISCARD_ACK*)pMsg;

			GAMEIN->GetUniqueItemMixDlg()->Release(FALSE);

			WORD wTableIdx = GetTableIdxForAbsPos(pmsg->TargetPos);

			DeleteItemofTable(wTableIdx, pmsg->TargetPos);
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);
		}
		break;
	case MP_ITEMEXT_UNIQUEITEM_MIX_ACK:
		{
			SetDisableDialog(FALSE, eItemTable_Inventory);
			SetDisableDialog(FALSE, eItemTable_Pyoguk);
			SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
			SetDisableDialog(FALSE, eItemTable_Shop);

			ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;

			CItem* pItem = NULL;
			CItem* pPreItem = NULL;
			for(int i = 0 ; i < pmsg->ItemNum ; ++i)
			{
				if( pPreItem = GAMEIN->GetInventoryDialog()->GetItemForPos( pmsg->GetItem(i)->Position ) )
				{
					DeleteItem(pmsg->GetItem(i)->Position, &pPreItem);
					ASSERT(pPreItem);
				}
				pItem = MakeNewItem(pmsg->GetItem(i),"MP_ITEM_BUY_ACK");	
				BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
				if(!rt)
				{
					ASSERT(0);					
				}
			}
		}
		break;
    case MP_ITEMEXT_SKINITEM_SELECT_ACK:	//2007. 12. 5. CBH - ��Ų ������ ���� ó�� ����
		{
			SEND_SKIN_INFO* pmsg = (SEND_SKIN_INFO*)pMsg;
			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->dwObjectID);
			if(!pPlayer)		return;
			
			memcpy( &pPlayer->GetShopItemStats()->wSkinItem, pmsg->wSkinItem, sizeof(WORD)*eSkinItem_Max);

			APPEARANCEMGR->InitAppearance( pPlayer );

			pPlayer->StartSkinDelayTime();			
		}
		break;
	case MP_ITEMEXT_SKINITEM_SELECT_NACK:	//2007. 12. 5. CBH - ��Ų ������ ���� ó�� ����
		{
			MSG_DWORD3* pmsg = (MSG_DWORD3*)pMsg;
			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->dwObjectID);
			if(!pPlayer)		return;

			switch(pmsg->dwData1)
			{
			case eSkinResult_Fail:	//��� ����
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(786));
				break;
			case eSkinResult_DelayFail:	//������
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1355), pmsg->dwData2/1000+1);
				break;
			case eSkinResult_LevelFail:
				CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(238), pmsg->dwData3);
				break;
			}
		}
		break;
	case MP_ITEMEXT_SKINITEM_DISCARD_ACK:	//2007. 12. 11. CBH - ��Ų ������ ������ ó��
		{
			SEND_SKIN_INFO* pmsg = (SEND_SKIN_INFO*)pMsg;
			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(pmsg->dwObjectID);
			if(!pPlayer)		return;

			//pPlayer->GetShopItemStats()->dwSkinItemIndex = pmsg->dwData;
			memcpy( &pPlayer->GetShopItemStats()->wSkinItem, pmsg->wSkinItem, sizeof(WORD)*eSkinItem_Max);

			if( pPlayer->GetObjectKind() == eObjectKind_Player )
			{
				GAMEIN->GetSkinSelectDlg()->SetActive(FALSE);
				GAMEIN->GetCostumeSkinSelectDlg()->SetActive(FALSE);
			}
			
			APPEARANCEMGR->InitAppearance( pPlayer );
		}
		break;
	case MP_ITEMEXT_SHOPITEM_DECORATION_ON: 
		{
			CObject* pObject;
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			pObject = OBJECTMGR->GetObject(pmsg->dwObjectID);
			ASSERT(pObject);

			if( pObject->GetObjectKind() == eObjectKind_Player )
			{
				((CPlayer*)pObject)->GetShopItemStats()->dwStreetStallDecoration = pmsg->dwData1;

				if( pmsg->dwData1 == 0 )
				{
					OBJECTMGR->SetRemoveDecorationInfo((CPlayer*)pObject);
				}
				else
				{
					if( ((CPlayer*)pObject)->GetState() == eObjectState_StreetStall_Owner )
                        STREETSTALLMGR->AddStreetStallDecoration((CPlayer*)pObject, pmsg->dwData2);
				}
			}
		}
		break;
	default:
		break;
	}
}



BOOL CItemManager::IsEqualTableIdxForPos(WORD TableIdx, POSTYPE absPos )
{
	if(TP_INVENTORY_START <= absPos && absPos < TP_INVENTORY_END)
	{
		return (TableIdx == eItemTable_Inventory);
	}
	else if(TP_WEAR_START <= absPos && absPos < TP_WEAR_END)
	{
		return (TableIdx == eItemTable_Weared);
	}
	else if(TP_GUILDWAREHOUSE_START <= absPos && absPos < TP_GUILDWAREHOUSE_END)
	{
		return (TableIdx == eItemTable_GuildWarehouse);
	}
	else if(TP_PYOGUK_START <= absPos && absPos < TP_PYOGUK_END)
	{
		return (TableIdx == eItemTable_Pyoguk);
	}
	else if(TP_SHOPINVEN_START <= absPos && absPos < TP_SHOPINVEN_END)
	{
		return (TableIdx == eItemTable_ShopInven);
	}
	else
	{
		return FALSE;
	}
}
BYTE CItemManager::GetTableIdxForAbsPos(POSTYPE absPos)
{
	if(TP_INVENTORY_START <= absPos && absPos < TP_INVENTORY_END)
	{
		return eItemTable_Inventory;
	}
	else if(TP_WEAR_START <= absPos && absPos < TP_WEAR_END)
	{
		return eItemTable_Weared;
	}
	else if(TP_GUILDWAREHOUSE_START <= absPos && absPos < TP_GUILDWAREHOUSE_END)
	{
		return eItemTable_GuildWarehouse; 
	}
	else if(TP_PYOGUK_START <= absPos && absPos < TP_PYOGUK_END)
	{
		return eItemTable_Pyoguk;
	}
	else if(TP_SHOPITEM_START <= absPos && absPos < TP_SHOPITEM_END)
	{
		return eItemTable_Shop;
	}
	else if(TP_SHOPINVEN_START <= absPos && absPos < TP_SHOPINVEN_END)
	{
		return eItemTable_ShopInven;
	}
	else if(TP_PETINVEN_START <= absPos && absPos < TP_PETINVEN_END)
	{
		return eItemTable_PetInven;
	}
	else if(TP_PETWEAR_START <= absPos && absPos < TP_PETWEAR_END)
	{
		return eItemTable_PetWeared;
	}
	else if(TP_TITANWEAR_START <= absPos && absPos < TP_TITANWEAR_END)
	{
		return eItemTable_Titan;
	}
	// magi82 - Titan(070227)
	else if(TP_TITANSHOPITEM_START <= absPos && absPos < TP_TITANSHOPITEM_END)
	{
		return eItemTable_TitanShopItem;
	}
	else
	{
		return 255;
	}
/*
	if(INVENTORY_STARTPOSITION <= absPos && absPos < INVENTORY_ENDPOSITION)
	{
		return eItemTable_Inventory;
	}
	else if(WEAR_STARTPOSITION <= absPos && absPos < WEAR_ENDPOSITION)
	{
		return eItemTable_Weared;
	}
	else if(STALL_STARTPOSITION <= absPos && absPos < STALL_ENDPOSITION)
	{
		return eItemTable_StreetStall;
	}
	else if(MUNPAITEM_STARTPOSITION <= absPos && absPos < MUNPAITEM_ENDPOSITION)
	{
		return eItemTable_MunpaWarehouse;
	}
	else if(PYOGUKITEM_STARTPOSITION <= absPos && absPos < PYOGUKITEM_ENDPOSITION)
	{
		return eItemTable_Pyoguk;
	}
	else
	{
		return 255;
	}
*/
}


void CItemManager::RefreshAllItem()
{
	CItem* pItem;
	m_ItemHash.SetPositionHead();
	
	while(pItem = m_ItemHash.GetData())
	{
		if(pItem->GetItemKind() & eEQUIP_ITEM)
		{
			if(CanEquip(pItem->GetItemIdx()))
			{
				pItem->SetImageRGB( ICONCLR_USABLE );
				pItem->SetToolTipImageRGB( TTCLR_ITEM_CANEQUIP );
			}
			else
			{
				pItem->SetImageRGB( ICONCLR_DISABLE );
				pItem->SetToolTipImageRGB( TTCLR_ITEM_CANNOTEQUIP );
			}
		}
		else if(pItem->GetItemKind() & eMUGONG_ITEM)
		{
			if(CanConvertToMugong(pItem->GetItemIdx()))
			{
				pItem->SetImageRGB( ICONCLR_USABLE );
				pItem->SetToolTipImageRGB( TTCLR_MUGONGBOOK_CANCONVERT );
			}
			else
			{
				pItem->SetImageRGB( ICONCLR_DISABLE );
				pItem->SetToolTipImageRGB( TTCLR_MUGONGBOOK_CANNOTCONVERT );
			}
		}
		else if(pItem->GetItemKind() & eTITAN_EQUIPITEM)
		{
			if(HERO->InTitan() == TRUE)
			{
				pItem->SetImageRGB( ICONCLR_USABLE );
			}
			else
			{
				pItem->SetImageRGB( ICONCLR_DISABLE );
			}
		}

//		if(pItem->GetDurability() != 0 && !IsDupItem(pItem->GetItemIdx()))
//			SetToolTipIcon(pItem, GetItemOption(pItem->GetDurability()));
		//SW050920 Rare
		if( pItem->GetDurability() != 0 && !IsDupItem(pItem->GetItemIdx()) )
		{
			SetToolTipIcon( pItem, GetItemOption(pItem->GetDurability()), GetItemRareOption(pItem->GetRareness()) );
		}
		else
			SetToolTipIcon( pItem, NULL, GetItemRareOption(pItem->GetRareness()) );
		
		if( m_bAddPrice )
		{
			char buf[128];
//			wsprintf( buf, CHATMGR->GetChatMsg(214), AddComma(pItem->GetItemInfo()->SellPrice) );
			DWORD SellPrice = SWPROFIT->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
			// �����
			SellPrice = FORTWARMGR->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
			wsprintf( buf, CHATMGR->GetChatMsg(214), AddComma(SellPrice) );
			pItem->AddToolTipLine( buf, TTTC_SELLPRICE );
		}
	}
}

void CItemManager::RefreshItem( CItem* pItem )
{
	if(pItem->GetItemKind() & eEQUIP_ITEM)
	{
		if(CanEquip(pItem->GetItemIdx()))
		{
			pItem->SetImageRGB( ICONCLR_USABLE );
			pItem->SetToolTipImageRGB( TTCLR_ITEM_CANEQUIP );
		}
		else
		{
			pItem->SetImageRGB( ICONCLR_DISABLE );
			pItem->SetToolTipImageRGB( TTCLR_ITEM_CANNOTEQUIP );
		}
	}
	else if(pItem->GetItemKind() & eMUGONG_ITEM)
	{
		if(CanConvertToMugong(pItem->GetItemIdx()))
		{
			pItem->SetImageRGB( ICONCLR_USABLE );
			pItem->SetToolTipImageRGB( TTCLR_MUGONGBOOK_CANCONVERT );
		}
		else
		{
			pItem->SetImageRGB( ICONCLR_DISABLE );
			pItem->SetToolTipImageRGB( TTCLR_MUGONGBOOK_CANNOTCONVERT );
		}
	}
	else if(pItem->GetItemKind() & eTITAN_EQUIPITEM)
	{
		if(HERO->InTitan() == TRUE)
		{
			pItem->SetImageRGB( ICONCLR_USABLE );
		}
		else
		{
			pItem->SetImageRGB( ICONCLR_DISABLE );
		}
	}

//	if(pItem->GetDurability() != 0 && !IsDupItem(pItem->GetItemIdx()))
//		SetToolTipIcon(pItem, GetItemOption(pItem->GetDurability()));
	//!!!NULL Ȯ�� SW050920 Rare
	if( pItem->GetDurability() != 0 && !IsDupItem(pItem->GetItemIdx()) )
	{
		SetToolTipIcon( pItem, GetItemOption(pItem->GetDurability()), GetItemRareOption(pItem->GetRareness()) );
	}
	else
		SetToolTipIcon( pItem, NULL, GetItemRareOption(pItem->GetRareness()) );
	if( m_bAddPrice )
	{
		char buf[128];
//		wsprintf( buf, CHATMGR->GetChatMsg(214), AddComma(pItem->GetItemInfo()->SellPrice) );
		DWORD SellPrice = SWPROFIT->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
		// �����
		SellPrice = FORTWARMGR->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
		wsprintf( buf, CHATMGR->GetChatMsg(214), AddComma(SellPrice) );
		pItem->AddToolTipLine( buf, TTTC_SELLPRICE );
	}	
}

void CItemManager::RefreshItemToolTip( DWORD ItemDBIdx )
{
	CItem* pItem = GetItem(ItemDBIdx);

	ASSERT(pItem);

	if( pItem->GetDurability() != 0 && !IsDupItem((pItem->GetItemIdx())) )
	{
		SetToolTipIcon(pItem, GetItemOption(pItem->GetDurability()), GetItemRareOption(pItem->GetRareness()) );
	}
	else
		SetToolTipIcon( pItem, NULL, GetItemRareOption(pItem->GetRareness()) );



}

void CItemManager::SetPriceToItem( BOOL bAddPrice )
{
	CItem* pItem;
	m_ItemHash.SetPositionHead();

	if( bAddPrice )
	{
		char buf[128];

		while(pItem = m_ItemHash.GetData())
		{
//			wsprintf( buf, CHATMGR->GetChatMsg(214), AddComma(pItem->GetItemInfo()->SellPrice) );
			DWORD SellPrice = SWPROFIT->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
			// �����
			SellPrice = FORTWARMGR->CalTexRateForSell( pItem->GetItemInfo()->SellPrice );
			wsprintf( buf, CHATMGR->GetChatMsg(214), AddComma(SellPrice) );
			pItem->AddToolTipLine( buf, TTTC_SELLPRICE );			
		}
	}
	else
	{
		while(pItem = m_ItemHash.GetData())
		{
//			if(pItem->GetDurability() != 0 && !IsDupItem(pItem->GetItemIdx()))
//				SetToolTipIcon(pItem, GetItemOption(pItem->GetDurability()));
	//!!!NULL Ȯ�� SW050920 Rare
//			if( (pItem->GetDurability() != 0 || pItem->GetRareness() != 0)
//				&& !IsDupItem(pItem->GetItemIdx()) )
			if(pItem->GetDurability() != 0 && !IsDupItem(pItem->GetItemIdx()))
			{
				SetToolTipIcon( pItem, GetItemOption(pItem->GetDurability()), GetItemRareOption(pItem->GetRareness()) );
			}
			else
				SetToolTipIcon( pItem, NULL, GetItemRareOption(pItem->GetRareness()) );
		}
	}

	m_bAddPrice = bAddPrice;
}



// ���������� �������� ��ȯ
BOOL CItemManager::CanConvertToMugong(WORD wItemIdx,MUGONG_TYPE MugongType)
{
	ITEM_INFO * pInfo = GetItemInfo(wItemIdx);
	if(pInfo == NULL)
	{
		ASSERT(pInfo);
		return FALSE;
	}	

	/*
	WORD Limit_Level;		//�������ɷ���	
	WORD Limit_Job;			//������������

	WORD Limit_GuenRyuk;	//�������ɱٷ�	
	WORD Limit_MinChub;		//�������ɹ�ø	
	WORD Limit_CheRyuk;		//�������� ü��	
	WORD Limit_SimMeak;		//�������� �ɸ�

	WORD Limit_NaeGong;		//�������ɳ���	
	WORD Limit_Element;		//�������ɼӼ�	
	WORD Limit_Kindness;	//�������ɰ���	
	*/

	
	//////////////////////////////////////////////////////////////////////////	
	// YH2DO  ItemKind �� ���� ����� ���� �ʿ�
	if(MugongType == MUGONGTYPE_NORMAL)
	{
		if(1025 <= pInfo->ItemKind && pInfo->ItemKind <= 1036)
		{
		}
		else if(pInfo->ItemKind == eMUGONG_ITEM_TITAN)	// magi82 - Titan(070910) Ÿ��ź ����������Ʈ
		{
		}
		else
			return FALSE;
	}
	if(MugongType == MUGONGTYPE_JINBUB)
	{
		if(1038 == pInfo->ItemKind)
		{
		}
		else
			return FALSE;
	}
	if(MugongType == MUGONGTYPE_SIMBUB)
	{
		if(1037 == pInfo->ItemKind)
		{
		}
		else
			return FALSE;
	}
	if(MugongType == MUGONGTYPE_JOB)	//2007. 10. 28. CBH - ������� ���� �߰�
	{
		if(eMUGONG_ITEM_JOB == pInfo->ItemKind)
		{
		}
		else
			return FALSE;
	}
	//////////////////////////////////////////////////////////////////////////
	
	
	if(pInfo->LimitLevel > HERO->GetLevel())
	{
		return FALSE;
	}

	// �ٷ�.��ø,�ɸ�,ü��
	if(pInfo->LimitGenGol > HERO->GetGenGol())
	{
		return FALSE;
	}
	if(pInfo->LimitMinChub > HERO->GetMinChub())
	{
		return FALSE;
	}
	if(pInfo->LimitSimMek > HERO->GetSimMek())
	{
		return FALSE;
	}
	if(pInfo->LimitCheRyuk > HERO->GetCheRyuk())
	{
		return FALSE;
	}

	// stage
	if( pInfo->LimitJob != eItemStage_Normal )
	{
		if( !CheckItemStage( (BYTE)pInfo->LimitJob ) )
			return FALSE;
	}	

	// for japan
#ifdef _JAPAN_LOCAL_
	if( !CheckItemAttr( pInfo->wItemAttr ) )
		return FALSE;
	if( pInfo->wAcquireSkillIdx1 )
	{
		if( !CheckItemAquireSkill( pInfo->wAcquireSkillIdx1, pInfo->wAcquireSkillIdx2 ) )
			return FALSE;
	}
#endif

	return TRUE;
}
// ���� ���� �����۸���
BOOL CItemManager::CanEquip(WORD wItemIdx)
{
	ITEM_INFO * pInfo = GetItemInfo(wItemIdx);

	// �������� ����
	/*
	��ü:0
	��:1
	��:2
	*/
	if(pInfo->LimitGender)
	if(pInfo->LimitGender != HERO->GetGender()+1)
	{
		return FALSE;
	}
	
	// �������� ����
	if(pInfo->LimitLevel > HERO->GetLevel())
	{
		switch(pInfo->ItemKind)
		{
		case 1: // ������
			break;
		case 2: // ���� ������
			break;
		default:
			break;
		}

		int lev = 0;
		
		if( HERO->GetShopItemStats()->EquipLevelFree )
		{
			int gap = pInfo->LimitLevel - HERO->GetLevel();
			gap -= HERO->GetShopItemStats()->EquipLevelFree;
			if( gap > 0 )
				return FALSE;
		}
		else
			return FALSE;
	}
	

	// �ٷ�.��ø,�ɸ�,ü��
	if(pInfo->LimitGenGol > HERO->GetGenGol())
	{
		return FALSE;
	}
	if(pInfo->LimitMinChub > HERO->GetMinChub())
	{
		return FALSE;
	}
	if(pInfo->LimitSimMek > HERO->GetSimMek())
	{
		return FALSE;
	}
	if(pInfo->LimitCheRyuk > HERO->GetCheRyuk())
	{
		return FALSE;
	}

	// �������� ����
	/*
	��:0
	��:1
	��:2
	/*
	
	// �������� �Ӽ�
	/*
	��:0
	ȭ:1
	��:2
	��:3
	��:4
	��:5
	��:6
	 */

	// �������� ����
	// stage
	if( pInfo->LimitJob != eItemStage_Normal )
	{
		if( !CheckItemStage( (BYTE)pInfo->LimitJob ) )
			return FALSE;
	}	

	// for japan
#ifdef _JAPAN_LOCAL_
	if( !CheckItemAttr( pInfo->wItemAttr ) )
		return FALSE;
	if( pInfo->wAcquireSkillIdx1 )
	{
		if( !CheckItemAquireSkill( pInfo->wAcquireSkillIdx1, pInfo->wAcquireSkillIdx2 ) )
			return FALSE;
	}
#endif

	return TRUE;
}

ITEM_INFO * CItemManager::GetItemInfo(WORD wItemIdx)
{
	return m_ItemInfoList.GetData(wItemIdx);
}

#define TOOLTIPINFO_LEN		24
#ifdef _TL_LOCAL_
#define TOOLTIPINFO_EXTENT		144
#endif

void CItemManager::LoadItemToolTipList()
{
	CMHFile file;
#ifdef _FILE_BIN_
//	if( !file.Init( "Resource/TooltipInfo.bin", "rb" ) )
	if( !file.Init( "Resource/Client/TooltipInfo.bin", "rb" ) )
		return;
#else
	if( !file.Init( "Resource/TooltipInfo.txt", "rt" ) )
		return;
#endif

	char buf[512];	//������ ��������...22		//256���� �����ϰ�..
	TOOLTIP_TEXT* pToolTipText;
	TOOLTIP_TEXT* pTemp;
	DWORD dwIndex;
	int nCpyNum = 0;
	int nLen;
	char* str;
	while( 1 )
	{
		if( file.IsEOF() ) break;

		pToolTipText = NULL;

//		file.GetString( buf );
		SafeStrCpy( buf, file.GetString(), 512 );

		if( strcmp( buf, "#Msg" ) == 0 )
		{
			dwIndex = file.GetDword();	//msg index
//			ASSERT( dwIndex >= 0 && dwIndex <= 10000 );	//index�� �ùٸ���?//confirm

			//file.GetString( buf );
			SafeStrCpy( buf, file.GetString(), 512 );
			
			if( strcmp( buf, "{" ) == 0 )
			{
				file.GetLine( buf, 512 );
				str = buf;
				nLen = strlen( buf );

				while( *buf != '}' )
				{
#ifdef _TL_LOCAL_
					BYTE brk[512];
					int nBrkNum = g_TBreak.FindThaiWordBreak( str, nLen, brk, 512, FTWB_SEPARATESYMBOL );
					int nCutPos = 0;

					for( int i = 0 ; i < nBrkNum ; ++i )
					{
						nCutPos += brk[i];
						if( CFONT_OBJ->GetTextExtentEx(0, str, nCutPos ) > TOOLTIPINFO_EXTENT )
						{
							nCutPos -= brk[i];

							pTemp = new TOOLTIP_TEXT;
							pTemp->strToolTip = new char[nCutPos+1];
							strncpy( pTemp->strToolTip, str, nCutPos );
							pTemp->strToolTip[nCutPos] = 0;

							--i;
							str += nCutPos;
							nCutPos = 0;

							if( *str == ' ' )
							{
								++str;	//���� ù���ڰ� �����̽��� �ƴϵ���...
								++i;
							}

							if( pToolTipText )
								pToolTipText->pNext = pTemp;
							else
								m_ItemToolTipTextList.Add( pTemp, dwIndex );
	
							pToolTipText = pTemp;

						}
					}

					if( nCutPos > 0 )
					{
						pTemp = new TOOLTIP_TEXT;
						pTemp->strToolTip = new char[nCutPos+1];
						strncpy( pTemp->strToolTip, str, nCutPos );
						pTemp->strToolTip[nCutPos] = 0;

						if( pToolTipText )
							pToolTipText->pNext = pTemp;
						else
							m_ItemToolTipTextList.Add( pTemp, dwIndex );

						pToolTipText = pTemp;
					}

#else
					while( nLen > TOOLTIPINFO_LEN )
					{
						if( ( str + TOOLTIPINFO_LEN ) != CharNext( CharPrev( str, str + TOOLTIPINFO_LEN ) ) )
							nCpyNum = TOOLTIPINFO_LEN - 1;
						else
							nCpyNum = TOOLTIPINFO_LEN;
						
						pTemp = new TOOLTIP_TEXT;
						pTemp->strToolTip = new char[nCpyNum+1];
						strncpy( pTemp->strToolTip, str, nCpyNum );
						pTemp->strToolTip[nCpyNum] = 0;
						nLen -= nCpyNum;
						str += nCpyNum;
						if( *str == ' ' ) ++str;

						if( pToolTipText )
							pToolTipText->pNext = pTemp;
						else
							m_ItemToolTipTextList.Add( pTemp, dwIndex );

						pToolTipText = pTemp;
					}

					if( nLen > 0 )
					{
						pTemp = new TOOLTIP_TEXT;
						pTemp->strToolTip = new char[nLen+1];
						strncpy( pTemp->strToolTip, str, nLen );
						pTemp->strToolTip[nLen] = 0;

						if( pToolTipText )
							pToolTipText->pNext = pTemp;
						else
							m_ItemToolTipTextList.Add( pTemp, dwIndex );

						pToolTipText = pTemp;
					}
#endif

					file.GetLine( buf, 512 );
					str = buf;
					nLen = strlen( buf );
				}
			}
		}		
	}	
}

TOOLTIP_TEXT* CItemManager::GetItemToolTipInfo( WORD wIdx )
{
	return m_ItemToolTipTextList.GetData( wIdx );
}

void CItemManager::LoadItemList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	if(!file.Init("Resource/ItemList.bin", "rb"))
		return;
#else
	if(!file.Init("Resource/ItemList.txt", "rt"))
		return;
#endif	// _FILE_BIN_

	ITEM_INFO * pInfo = NULL;
	while(1)
	{
		if(file.IsEOF())
			break;

		ASSERT(!pInfo);
		pInfo = new ITEM_INFO;

		pInfo->ItemIdx = file.GetWord();
		SafeStrCpy( pInfo->ItemName, file.GetString(), MAX_ITEMNAME_LENGTH+1 );
		pInfo->ItemTooltipIdx = file.GetWord(); //1
		pInfo->Image2DNum = file.GetWord();	
		pInfo->ItemKind = file.GetWord();			
		pInfo->BuyPrice = file.GetDword();			
		pInfo->SellPrice = file.GetDword();			
		pInfo->Rarity = file.GetDword();			
		pInfo->WeaponType = file.GetWord();		
		pInfo->GenGol = file.GetWord();			
		pInfo->MinChub = file.GetWord();			
		pInfo->CheRyuk = file.GetWord();		//10	
		pInfo->SimMek = file.GetWord();			
		pInfo->Life = file.GetDword();			
		pInfo->Shield = file.GetDword();				
		pInfo->NaeRyuk = file.GetWord();			
		pInfo->AttrRegist.SetElement_Val(ATTR_FIRE,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_WATER,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_TREE,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_IRON,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_EARTH,file.GetFloat());
		pInfo->LimitJob = file.GetWord();		//20		
		pInfo->LimitGender = file.GetWord();		
		pInfo->LimitLevel = file.GetLevel();		
		pInfo->LimitGenGol = file.GetWord();		
		pInfo->LimitMinChub = file.GetWord();		
		pInfo->LimitCheRyuk = file.GetWord();		
		pInfo->LimitSimMek = file.GetWord();		
		pInfo->ItemGrade = file.GetWord();			
		pInfo->RangeType = file.GetWord();			
		pInfo->EquipKind = file.GetWord();			
		pInfo->Part3DType = file.GetWord();		//30
		pInfo->Part3DModelNum = file.GetWord();	
		pInfo->MeleeAttackMin = file.GetWord();	
		pInfo->MeleeAttackMax = file.GetWord();	
		pInfo->RangeAttackMin = file.GetWord();	
		pInfo->RangeAttackMax = file.GetWord();	
		pInfo->CriticalPercent = file.GetWord();	
		pInfo->AttrAttack.SetElement_Val(ATTR_FIRE,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_WATER,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_TREE,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_IRON,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_EARTH,file.GetFloat());
		pInfo->PhyDef = file.GetWord();			//42	
		pInfo->Plus_MugongIdx = file.GetWord();	
		pInfo->Plus_Value = file.GetWord();		
		pInfo->AllPlus_Kind = file.GetWord();		
		pInfo->AllPlus_Value = file.GetWord();		
		pInfo->MugongNum = file.GetWord();			
		pInfo->MugongType = file.GetWord();		
		pInfo->LifeRecover = file.GetWord();		
		pInfo->LifeRecoverRate = file.GetFloat();
		pInfo->NaeRyukRecover = file.GetWord();	
		pInfo->NaeRyukRecoverRate = file.GetFloat();	
		pInfo->ItemType = file.GetWord();
		
#ifdef _JAPAN_LOCAL_		
		pInfo->wItemAttr = file.GetWord();
		pInfo->wAcquireSkillIdx1 = file.GetWord();
		pInfo->wAcquireSkillIdx2 = file.GetWord();
		pInfo->wDeleteSkillIdx = file.GetWord();
#endif
		pInfo->wSetItemKind = file.GetWord();	// 2007. 6. 7. CBH ��Ʈ�̾��� ���� �߰�

		ASSERT(m_ItemInfoList.GetData(pInfo->ItemIdx) == FALSE);
		
		m_ItemInfoList.Add(pInfo, pInfo->ItemIdx);
		pInfo = NULL;
	}
	file.Release();

#ifdef _FILE_BIN_
	if(!file.Init("Resource/HideItemLock.bin", "rb"))
		return;
#else
	if(!file.Init("Resource/HideItemLock.txt", "rt"))
		return;
#endif	// _FILE_BIN_

	while(1)
	{
		if(file.IsEOF())
			break;
		
		pInfo = NULL;
		WORD Index = file.GetWord();
		pInfo = m_ItemInfoList.GetData(Index);

		if(pInfo)
			m_UnfitItemInfoListForHide.Add(pInfo, pInfo->ItemIdx);
	}

	file.Release();
}

/*
BOOL CItemManager::CanGradeUp(CItem * pItem)
{
	eITEM_KINDBIT bits = pItem->GetItemKind();
	if(bits & eEQUIP_ITEM)
	{
		if(m_dwStateParam == 50700)
		{
			// ����
			if((bits >= eEQUIP_ITEM_WEAPON && bits < eEQUIP_ITEM_WEAPON+6))
			{
				return TRUE;
			}

		}
		else if(m_dwStateParam == 50701)
		{
			// ��  
			if(bits == eEQUIP_ITEM_DRESS ||
				bits == eEQUIP_ITEM_HAT ||
				bits == eEQUIP_ITEM_SHOES)
			{
				return TRUE;
			}
		}
		
	}
	return FALSE;
}*/
BOOL CItemManager::IsTitanCallItem( WORD wItemIdx )
{
	ITEM_INFO* pItemInfo = GetItemInfo(wItemIdx);

	if(!pItemInfo)
	{
		ASSERT(0);
		return FALSE;
	}

	if(pItemInfo->ItemKind == eTITAN_ITEM_PAPER )
		return TRUE;
	else
		return FALSE;
}

BOOL CItemManager::IsTitanEquipItem( WORD wItemIdx )
{
	ITEM_INFO* pItemInfo = GetItemInfo(wItemIdx);

	if(!pItemInfo)
	{
		ASSERT(0);
		return FALSE;
	}

	if(pItemInfo->ItemKind & eTITAN_EQUIPITEM )
		return TRUE;
	else
		return FALSE;
}

BOOL CItemManager::IsPetSummonItem( WORD wItemIdx )
{
	ITEM_INFO* pItemInfo = GetItemInfo(wItemIdx);

	ASSERT(pItemInfo);

	if(pItemInfo->ItemKind == eQUEST_ITEM_PET || pItemInfo->ItemKind == eSHOP_ITEM_PET)
		return TRUE;
	else
		return FALSE;
}

BOOL CItemManager::IsRareOptionItem( WORD wItemIdx, DWORD dwRareIdx )	//RareIdx �� DB�� Id
{
	if(dwRareIdx && !IsDupItem(wItemIdx))
		return TRUE;
	return FALSE;
}


BOOL CItemManager::IsOptionItem( WORD wItemIdx, DURTYPE wDurability )
{
	if(wDurability != 0 && !IsDupItem(wItemIdx))
		return TRUE;
	return FALSE;
}
BOOL CItemManager::IsDupItem( WORD wItemIdx )
{
	//�������� �ƴϸ�... //KES 040503
//	if( wItemIdx < MIN_YOUNGYAKITEM_INDEX || wItemIdx >= MIN_ABILITY_INDEX )
//		return FALSE;

	switch( GetItemKind( wItemIdx ) )
	{
	case eYOUNGYAK_ITEM:
	case eYOUNGYAK_ITEM_PET:
	case eYOUNGYAK_ITEM_UPGRADE_PET:
	case eYOUNGYAK_ITEM_TITAN:
	case eEXTRA_ITEM_JEWEL:
	case eEXTRA_ITEM_MATERIAL:
	case eEXTRA_ITEM_METAL:
	case eEXTRA_ITEM_BOOK:
	case eEXTRA_ITEM_HERB:
	case eEXTRA_ITEM_ETC:
	case eEXTRA_ITEM_USABLE:
	case eSHOP_ITEM_CHARM:
	case eSHOP_ITEM_HERB:
		return TRUE;
	case eSHOP_ITEM_SUNDRIES:
		{
			ITEM_INFO* pItem = GetItemInfo( wItemIdx );
			if( !pItem )			return FALSE;

			if( pItem->SimMek )			// ShopItem���� ChangeItem �ΰ͵�
				return FALSE;
			else if( pItem->CheRyuk )	// ����
				return FALSE;
			else if( eSundries_Shout == wItemIdx )
				return FALSE;				
		}
		return TRUE;
	case eSHOP_ITEM_INCANTATION:
		{
			if( wItemIdx == eIncantation_TownMove15 || wItemIdx == eIncantation_MemoryMove15 ||
				wItemIdx == eIncantation_TownMove7 || wItemIdx == eIncantation_MemoryMove7 ||
				wItemIdx == eIncantation_TownMove7_NoTrade || wItemIdx == eIncantation_MemoryMove7_NoTrade ||
				wItemIdx == 55357 || wItemIdx == 55362 || wItemIdx == eIncantation_MemoryMoveExtend || wItemIdx == eIncantation_MemoryMoveExtend7 ||
				wItemIdx == eIncantation_MemoryMove2 || wItemIdx == eIncantation_MemoryMoveExtend30 ||
				wItemIdx == eIncantation_ShowPyoguk || wItemIdx == eIncantation_ChangeName ||
				wItemIdx == eIncantation_ChangeName_Dntrade ||
				wItemIdx == eIncantation_Tracking || wItemIdx == eIncantation_Tracking_Jin ||
				wItemIdx == eIncantation_ChangeJob ||
				wItemIdx == eIncantation_ShowPyoguk7 || wItemIdx == eIncantation_ShowPyoguk7_NoTrade ||
				wItemIdx == eIncantation_Tracking7 || wItemIdx == eIncantation_Tracking7_NoTrade ||
				wItemIdx== eIncantation_MugongExtend ||	wItemIdx == eIncantation_PyogukExtend ||
				wItemIdx == eIncantation_InvenExtend ||	wItemIdx == eIncantation_CharacterSlot ||
				wItemIdx== eIncantation_MugongExtend2 || wItemIdx == eIncantation_PyogukExtend2 ||
				wItemIdx == eIncantation_InvenExtend2 || wItemIdx == eIncantation_CharacterSlot2
				)
				return FALSE;

			ITEM_INFO* pItem = GetItemInfo( wItemIdx );
			if( !pItem )			return FALSE;

            if( pItem->LimitLevel && pItem->SellPrice )
				return FALSE;
		}
		return TRUE;
	case eSHOP_ITEM_NOMALCLOTHES_SKIN:
	case eSHOP_ITEM_COSTUME_SKIN:	
		{
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


CItem* CItemManager::GetItemofTable(WORD TableIDX, POSTYPE absPos)
{
	CItem * pItem = NULL;
	switch(TableIDX)
	{
	case eItemTable_Weared:
	case eItemTable_Inventory:
		{
			CInventoryExDialog * pDlg = GAMEIN->GetInventoryDialog();
			pItem = pDlg->GetItemForPos( absPos );			
		}
		break;
	case eItemTable_Pyoguk:
		{
			CPyogukDialog * pDlg = GAMEIN->GetPyogukDialog();
			pItem = pDlg->GetItemForPos( absPos );			
		}
		break;
	case eItemTable_GuildWarehouse:
		{
			CGuildWarehouseDialog * pDlg = GAMEIN->GetGuildWarehouseDlg();
			pItem = pDlg->GetItemForPos( absPos );			
		}
		break;
	case eItemTable_Shop:
		{
			CItemShopDialog* pDlg = GAMEIN->GetItemShopDialog();
			pItem = pDlg->GetItemForPos( absPos );
		}
		break;
	case eItemTable_ShopInven:
		{
			CInventoryExDialog * pDlg = GAMEIN->GetInventoryDialog();
			pItem = pDlg->GetItemForPos( absPos );
		}
		break;
	case eItemTable_PetInven:
		{
			CPetInventoryDlg* pDlg = GAMEIN->GetPetInventoryDialog();
			pItem = pDlg->GetItemForPos( absPos );
		}
		break;
	case eItemTable_Titan:
		{
			CTitanInventoryDlg* pDlg = GAMEIN->GetTitanInventoryDlg();
			pItem = pDlg->GetItemForPos( absPos );
		}
		break;
	}
	return pItem;
}

// yunho StatsCalcManager ������ ����
const ITEMBASE* CItemManager::GetItemInfoAbsIn(CHero* pHero,POSTYPE absPos)
{
	BYTE TableIdx = GetTableIdxForAbsPos(absPos);
	if(TableIdx == 255)
		return NULL;

	CItem* pItem = GetItemofTable(TableIdx,absPos);
	if(pItem == NULL)
		return NULL;

	return pItem->GetItemBaseInfo();
}

void CItemManager::SetDisableDialog(BOOL val, BYTE TableIdx)
{
	switch(TableIdx)
	{
	case eItemTable_Weared:
	case eItemTable_Inventory:
	case eItemTable_ShopInven:
		{
			CInventoryExDialog * pDlg = GAMEIN->GetInventoryDialog();
			if( pDlg )
				pDlg->SetDisable( val );
		}
		break;
	case eItemTable_Pyoguk:
		{
			CPyogukDialog * pDlg = GAMEIN->GetPyogukDialog();
			if( pDlg )
				pDlg->SetDisable( val );			
		}
		break;
	case eItemTable_PetInven:
		{
			CPetInventoryDlg* pDlg = GAMEIN->GetPetInventoryDialog();
			if( pDlg )
				pDlg->SetDisable( val );
		}
		break;
	case eItemTable_GuildWarehouse:
		{
			CGuildWarehouseDialog * pDlg = GAMEIN->GetGuildWarehouseDlg();
			if( pDlg )
				pDlg->SetDisable( val );
		}
		break;
	case eItemTable_Deal:
		{
			CDealDialog * pDlg = GAMEIN->GetDealDialog();
			if( pDlg )
				pDlg->SetDisable( val );
		}
		break;
	case eItemTable_Shop:
		{
			CItemShopDialog* pDlg = GAMEIN->GetItemShopDialog(); 
			if( pDlg )
				pDlg->SetDisable( val );
		}
		break;
	case eItemTable_Titan:
		{
			CTitanRepairDlg* pDlg = GAMEIN->GetTitanRepairDlg(); 
			if( pDlg )
				pDlg->SetDisable( val );
		}
		break;
	default:
		{
			ASSERTMSG(0, "���̺� �ε����� �������.DisableDlg()-Taiyo. Invalid Table Index.");
			break;
		}
	}
}

void CItemManager::DivideItem(ITEMOBTAINARRAY * pMsg )
{
	ITEMOBTAINARRAY * pmsg = (ITEMOBTAINARRAY *)pMsg;
	
	CItem* pItem = NULL;
	CItem* pPreItem = NULL;
	for(int i = 0 ; i < pmsg->ItemNum ; ++i)
	{
		DeleteItem(pmsg->GetItem(i)->Position, &pPreItem);

		pItem = MakeNewItem(pmsg->GetItem(i),"DivideItem");	
	//	BOOL rt = GAMEIN->GetInventoryDialog()->AddItem(pItem);
		if(!AddItem(pItem))
		{
			ASSERT(0);
		}
	}
}

BOOL CItemManager::AddItem(CItem* pItem)
{
	BYTE TableIdx = GetTableIdxForAbsPos(pItem->GetPosition());
	switch(TableIdx)
	{
	case eItemTable_Inventory:
	case eItemTable_Weared:
	case eItemTable_ShopInven:
		return GAMEIN->GetInventoryDialog()->AddItem(pItem);
	case eItemTable_Pyoguk:
		return GAMEIN->GetPyogukDialog()->AddItem(pItem);
	case eItemTable_GuildWarehouse:
		return GAMEIN->GetGuildWarehouseDlg()->AddItem(pItem);
	case eItemTable_PetInven:
		return GAMEIN->GetPetInventoryDialog()->AddItem(pItem);
	}
	
	return FALSE;
}

ITEM_INFO* CItemManager::FindItemInfoForName( char* strName )
{
	m_ItemInfoList.SetPositionHead();
	ITEM_INFO* pInfo = NULL;
	while( pInfo = m_ItemInfoList.GetData() )
	{
		if( strcmp( strName, pInfo->ItemName ) == 0 )
		{
			return pInfo;
		}
	}

	return NULL;
}

/*
BOOL CItemManager::CheckQPosForItemIdx( WORD wItemIdx )
{
	CItem* pItem;
	m_ItemHash.SetPositionHead();
	
	while(pItem = m_ItemHash.GetData())
	{
		if( GetTableIdxForAbsPos( pItem->GetPosition() ) == eItemTable_Inventory )
		if( pItem->GetItemIdx() == wItemIdx )
		if( pItem->GetQuickPosition() != 0 )
			return FALSE;
	}

	return TRUE;
}
*/

void CItemManager::ReLinkQuickPosition(CItem* pItem)
{
	if(IsDupItem(pItem->GetItemIdx()))
	{
		if(pItem->GetQuickPosition())
		{
//			if(pItem->GetDurability() == 1)		//1???????	//1�Ʃ�AI�ҡ� ��c��eC��A���ҡע���
			{			
				DURTYPE dur = GAMEIN->GetInventoryDialog()->GetTotalItemDurability(pItem->GetItemIdx());
				if(dur)
				{
					// ������ Quickposition�ٲ���ٰ� ����
					// �ٸ� Link�� ������ �ش�.	
					// ������ ����
					// 06. 01 �̿��� - ����â ����
					//GAMEIN->GetQuickDialog()->FakeAddItem(pItem->GetQuickPosition(), GAMEIN->GetInventoryDialog()->GetItemLike(pItem->GetItemIdx()));
					CItem* pNewItem = GAMEIN->GetInventoryDialog()->GetItemLike(pItem->GetItemIdx());
					WORD QuickPos = pItem->GetQuickPosition() | pNewItem->GetQuickPosition();
					
					QUICKMGR->SetQuickItem(QuickPos, pNewItem);
				}
			}
		}
	}
}


void CItemManager::SetPreItemData(sPRELOAD_INFO* pPreLoadInfo, int* Level, int Count)
{
	ITEM_INFO* pItemInfo = NULL;
	int ItemIndex = 0;

	m_ItemInfoList.SetPositionHead();
	while( pItemInfo = m_ItemInfoList.GetData() )
	{
		for(int i=0; i<Count; i++)
		{
			if(pItemInfo->LimitLevel == Level[i])
			{
				// �������� ������ �⺻���⿡ +�� ���� �������̴�. �׷��� �н�~
				if( pItemInfo->ItemIdx%10 )			continue;
				
				ItemIndex = pPreLoadInfo->Count[ePreLoad_Item];
				pPreLoadInfo->Kind[ePreLoad_Item][ItemIndex] = pItemInfo->ItemIdx;
				++pPreLoadInfo->Count[ePreLoad_Item];
				
				// Max�� �Ѿ�� �׸���~~
				if(pPreLoadInfo->Count[ePreLoad_Item] >= MAX_KIND_PERMAP)		return;
			}
		}

		/*
		if(pItemInfo->LimitLevel == Level)					
		{
			// �������� ������ �⺻���⿡ +�� ���� �������̴�. �׷��� �н�~
			if( pItemInfo->ItemIdx%10 )			continue;

			ItemIndex = pPreLoadInfo->Count[ePreLoad_Item];
			pPreLoadInfo->Kind[ePreLoad_Item][ItemIndex] = pItemInfo->ItemIdx;
			++pPreLoadInfo->Count[ePreLoad_Item];

			// Max�� �Ѿ�� �׸���~~
			if(pPreLoadInfo->Count[ePreLoad_Item] >= MAX_KIND_PERMAP)		break;
		}
		*/
	}
}

void CItemManager::ItemDropEffect( WORD wItemIdx )
{
	eITEM_KINDBIT bits = GetItemKind(wItemIdx);

	if( bits & eEQUIP_ITEM )
	{
		if( bits == eEQUIP_ITEM_WEAPON )
		{
			// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(��������� ȹ��)
			if( HERO->InTitan() == TRUE )
				EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetItem_Weapon);
			else
				EFFECTMGR->StartHeroEffectProcess(eEffect_GetItem_Weapon);
		}
		else if( bits == eEQUIP_ITEM_DRESS || bits == eEQUIP_ITEM_HAT || bits == eEQUIP_ITEM_SHOES)
		{
			// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(�������� ȹ��)
			if( HERO->InTitan() == TRUE )
				EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetItem_Dress);
			else
				EFFECTMGR->StartHeroEffectProcess(eEffect_GetItem_Dress);
		}
		else
		{
			// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(�Ǽ��縮������ ȹ��)
			if( HERO->InTitan() == TRUE )
				EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetItem_Accessory);
			else
				EFFECTMGR->StartHeroEffectProcess(eEffect_GetItem_Accessory);
//			EFFECTMGR->StartHeroEffectProcess(eEffect_GetMoney);
		}
	}
	else
	{
		// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(��������� ȹ��)
		if( HERO->InTitan() == TRUE )
			EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetItem_Youngyak);
		else
			EFFECTMGR->StartHeroEffectProcess(eEffect_GetItem_Youngyak);
	}
}

void CItemManager::MoneyDropEffect()
{
	// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(�� ȹ��)
	if( HERO->InTitan() == TRUE )
		EFFECTMGR->StartHeroEffectProcess(eEffect_Titan_GetMoney);
	else
		EFFECTMGR->StartHeroEffectProcess(eEffect_GetMoney);
}

void CItemManager::AddUsedItemInfo(SHOPITEMBASE* pInfo)
{
	if(!pInfo)		return;
	if( pInfo->ItemBase.dwDBIdx == 0 )		return;
	
	ITEM_INFO* pItemInfo = GetItemInfo( pInfo->ItemBase.wIconIdx );
	if( !pItemInfo || (!(pItemInfo->ItemKind & eSHOP_ITEM)) )
		return;
	
	SHOPITEMBASE* pShopItemBase = NULL;
	pShopItemBase = m_UsedItemList.GetData(pInfo->ItemBase.wIconIdx );
	if( pShopItemBase )
	{
		// �����÷���Ÿ��üũ
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM && pItemInfo->MeleeAttackMin )
		{
			SAFE_DELETE( pShopItemBase );
			m_UsedItemList.Remove( pItemInfo->ItemIdx );
		}
		else
			return;
	}
	
	pShopItemBase = new SHOPITEMBASE;
	memcpy(pShopItemBase, pInfo, sizeof(SHOPITEMBASE));
	m_UsedItemList.Add(pShopItemBase, pShopItemBase->ItemBase.wIconIdx);

	// �����÷���Ÿ��üũ
	if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM && pItemInfo->MeleeAttackMin )
	{
		if( pShopItemBase->Remaintime == 0 )
			return;
	}
	//
#ifdef _JAPAN_LOCAL_
#elif defined _HK_LOCAL_
#elif defined _TL_LOCAL_
#else
	if( pItemInfo->ItemIdx == eIncantation_MugongExtend )
		GAMEIN->GetMugongDialog()->SetMugongExtend( TRUE );
#endif

	if( pItemInfo->ItemKind != eSHOP_ITEM_EQUIP && pItemInfo->ItemKind != eSHOP_ITEM_PET_EQUIP && pItemInfo->BuyPrice )
		STATUSICONDLG->AddIcon(HERO, (WORD)(pItemInfo->BuyPrice+1), pShopItemBase->ItemBase.wIconIdx);
	
	if( pItemInfo->SellPrice == eShopItemUseParam_Playtime )
	if( pShopItemBase->Remaintime <= MINUTETOSECOND*1000 )
		STATUSICONDLG->SetOneMinuteToShopItem( pShopItemBase->ItemBase.wIconIdx );

	// �ƹ�Ÿ �������� ������ ���� ����
	if( pItemInfo->ItemKind == eSHOP_ITEM_MAKEUP || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
	{
		AddUsedAvatarItemToolTip( pInfo );
	}
	else if( pItemInfo->ItemKind == eSHOP_ITEM_EQUIP )
	{
		AddUsedShopEquipItemToolTip( pInfo );
	}
	// �Ⱓ�� ���� ���� ����
	else if( pItemInfo->ItemType == 11 )
	{
		CItem* pItem = GetItem( pInfo->ItemBase.dwDBIdx );
		if( pItem )
			SetToolTipIcon( (cIcon*)pItem );
	}
}


void CItemManager::RemoveUsedItemInfo(WORD wIndex, DWORD dwDBIdx)
{
	SHOPITEMBASE* pShopItemBase = m_UsedItemList.GetData(wIndex);
	if( !pShopItemBase )		return;
	if( dwDBIdx != 0 && pShopItemBase->ItemBase.dwDBIdx != dwDBIdx )		return;
	
	SAFE_DELETE(pShopItemBase);
	m_UsedItemList.Remove(wIndex);

	ITEM_INFO* pItemInfo = GetItemInfo(wIndex);	
	if( pItemInfo &&  pItemInfo->ItemKind != eSHOP_ITEM_EQUIP && pItemInfo->ItemKind != eSHOP_ITEM_PET_EQUIP && pItemInfo->BuyPrice )
		STATUSICONDLG->RemoveIcon(HERO, (WORD)(pItemInfo->BuyPrice+1), wIndex);
}

SHOPITEMBASE* CItemManager::GetUsedItemInfo(DWORD Index)
{
	return m_UsedItemList.GetData(Index);
}


void CItemManager::Process()
{
	if( m_UsedItemList.GetDataNum() == 0 )
		return;


	SHOPITEMBASE* pShopItem = NULL;	
	m_UsedItemList.SetPositionHead();

	// RaMa - 05.4.18
	ITEM_INFO* pInfo = NULL;	
	while( pShopItem = m_UsedItemList.GetData() )
	{
		pInfo = GetItemInfo( pShopItem->ItemBase.wIconIdx );
		// �÷���Ÿ��
		if( pInfo && pInfo->SellPrice == eShopItemUseParam_Playtime )
		{
			if( NOTIFYMGR->IsEventNotifyUse() )
			{
				if( pInfo->ItemKind == eSHOP_ITEM_CHARM && pInfo->MeleeAttackMin && pShopItem->Remaintime )
				{
					if( NOTIFYMGR->IsApplyEvent( pInfo->MeleeAttackMin ) )
						continue;												
				}
			}

			if( pShopItem->Remaintime > gTickTime )
				pShopItem->Remaintime -= gTickTime;
			else
				pShopItem->Remaintime = 0;
		}
	}
}


void CItemManager::DeleteShopItemInfo()
{
	SHOPITEMBASE* pShopItem = NULL;

	m_UsedItemList.SetPositionHead();

	while(	pShopItem = m_UsedItemList.GetData() )
	{
		RemoveUsedItemInfo( pShopItem->ItemBase.wIconIdx, pShopItem->ItemBase.dwDBIdx );
	}
}


void CItemManager::ReviveOtherOK()
{
}


void CItemManager::ReviveOtherSync()
{
	CObject* pObject = OBJECTMGR->GetSelectedObject();
	if( pObject == NULL || pObject == HERO || pObject->GetObjectKind() != eObjectKind_Player )
	{
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Inventory);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Pyoguk);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Shop);

		CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(763) );
		return;
	}
	/*
	if( MOVEMGR->CalcDistance( HERO, pObject ) > REVIVE_OTHER_DIST )
	{
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Inventory);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Pyoguk);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Shop);

		CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(764) );
		return;
	}
	*/

	CItem* pItem = GAMEIN->GetInventoryDialog()->GetSelectedShopItem();
	if( !pItem )
	{
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Inventory);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Pyoguk);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
		ITEMMGR->SetDisableDialog(FALSE, eItemTable_Shop);
		
		CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(786) );
		return;
	}

	MSG_DWORD3 msg;
	msg.Category = MP_ITEM;
	msg.Protocol = MP_ITEM_SHOPITEM_REVIVEOTHER_SYN;
	msg.dwObjectID = HERO->GetID();
	msg.dwData1 = OBJECTMGR->GetSelectedObject()->GetID();	
	msg.dwData2 = pItem->GetItemIdx();
	msg.dwData3 = pItem->GetPosition();
	NETWORK->Send( &msg, sizeof(msg) );

	GAMEIN->GetInventoryDialog()->ResetSelectedShopItem();
}


void CItemManager::ReviveOtherCancel()
{
	SetDisableDialog(FALSE, eItemTable_Inventory);
	SetDisableDialog(FALSE, eItemTable_Pyoguk);
	SetDisableDialog(FALSE, eItemTable_MunpaWarehouse);
	SetDisableDialog(FALSE, eItemTable_Shop);

	GAMEIN->GetInventoryDialog()->ResetSelectedShopItem();
}


void CItemManager::ReviveOtherConfirm( BOOL bRevive )
{
	if( GAMEIN->GetInventoryDialog()->GetReviveData().TargetID == 0 )
		return;

	MSG_DWORD2 msg;
	msg.Category = MP_ITEM;
	msg.dwObjectID = HERO->GetID();
	msg.dwData1 = GAMEIN->GetInventoryDialog()->GetReviveData().TargetID;
	msg.dwData2 = eShopItemErr_Revive_Refuse;
	
	if( bRevive )
		msg.Protocol = MP_ITEM_SHOPITEM_REVIVEOTHER_ACK;	
	else
		msg.Protocol = MP_ITEM_SHOPITEM_REVIVEOTHER_NACK;

	NETWORK->Send( &msg, sizeof(msg) );

	//
	GAMEIN->GetInventoryDialog()->SetReviveData( 0, 0, 0 );
}

BOOL CItemManager::FakeDeleteItem(CItem* pItem)
{
	if( pItem->GetPosition() >= TP_WEAR_START && pItem->GetPosition() < TP_WEAR_END )
		return FALSE;
	m_TempDeleteItem.Durability = pItem->GetDurability();
	m_TempDeleteItem.Position = pItem->GetPosition();
	m_TempDeleteItem.wIconIdx = pItem->GetItemIdx();

	return TRUE;
}

void CItemManager::SendDeleteItem()
{
	if( m_TempDeleteItem.wIconIdx == 0 ) 
	{
		ASSERT(m_TempDeleteItem.wIconIdx != 0);
		GAMEIN->GetInventoryDialog()->SetDisable(FALSE);
		GAMEIN->GetPyogukDialog()->SetDisable(FALSE);
		return;
	}

	CItem* pItem = GetItemofTable(GetTableIdxForAbsPos(m_TempDeleteItem.Position), m_TempDeleteItem.Position);

	if( !pItem )	return;
	// magi82 - Titan(070430)
	// ��ϵ� ������ ����������.
	if(pItem->GetDBIdx() == TITANMGR->GetRegistedTitanItemDBIdx())
	{
		CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1535) );
		GAMEIN->GetInventoryDialog()->SetDisable(FALSE);
		return;
	}

	if((pItem == 0) || (pItem->GetItemIdx() != m_TempDeleteItem.wIconIdx))
	{
		CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(153) );
		GAMEIN->GetInventoryDialog()->SetDisable(FALSE);
		return;
	}
			
	MSG_ITEM_DISCARD_SYN msg;
	msg.Category = MP_ITEM;
	msg.Protocol = MP_ITEM_DISCARD_SYN;
	msg.dwObjectID = HEROID;
	msg.TargetPos =m_TempDeleteItem.Position;
	msg.wItemIdx = m_TempDeleteItem.wIconIdx;
	msg.ItemNum = m_TempDeleteItem.Durability;

	NETWORK->Send(&msg,sizeof(msg));
	memset(&m_TempDeleteItem, 0, sizeof(ITEMBASE));
}

// 060911 KKR 
// �� ������ ���ð� �Ѹ��� 
void CItemManager::PrintShopItemUseTime( CItem* pItem, ITEM_INFO* pItemInfo )
{
	// ���Ⱓ ǥ��.
	pItem->AddToolTipLine("");
	pItem->AddToolTipLine( CHATMGR->GetChatMsg(1442), TTTC_DEFAULT );

	// ����ð� ���Ǵ°�
	if( pItemInfo->SellPrice == eShopItemUseParam_Realtime )
	{
		pItem->AddToolTipLine( CHATMGR->GetChatMsg(1444), TTTC_QUESTITEM );
	}
	// �ð����� ����
	else if( pItemInfo->SellPrice == eShopItemUseParam_Forever )
	{
		pItem->AddToolTipLine( CHATMGR->GetChatMsg(1443), TTTC_QUESTITEM );
	}
	// �÷��̽ð� 
	else if( pItemInfo->SellPrice == eShopItemUseParam_Playtime )
	{
		pItem->AddToolTipLine( CHATMGR->GetChatMsg(1444), TTTC_QUESTITEM );
	}
	// ����� ���� ���߿� ȿ���� �ߵ��Ǵ°� 
	else if( pItemInfo->SellPrice == eShopItemUseParam_Continue )
	{
		pItem->AddToolTipLine( CHATMGR->GetChatMsg(1444), TTTC_QUESTITEM );
	}
}

void CItemManager::AddUsedAvatarItemToolTip( SHOPITEMBASE* pInfo )
{
	if( !pInfo )		return;

	ITEM_INFO* pItemInfo = GetItemInfo( pInfo->ItemBase.wIconIdx );
	if( !pItemInfo )	return;

	CItem* pItem = GetItem( pInfo->ItemBase.dwDBIdx );
	if( !pItem )		return;
	stTIME time;
	time.value = pInfo->Remaintime;
	
	// �ʱ�ȭ
	cImage imgToolTip;
	SCRIPTMGR->GetImage( 63, &imgToolTip, PFT_HARDPATH );		
	pItem->SetToolTip( "", RGBA_MAKE(255, 255, 255, 255), &imgToolTip, TTCLR_ITEM_CANEQUIP );
	
	// ������ �̸�
	char buf[256] = { 0, };
	wsprintf( buf, "[%s]", pItemInfo->ItemName );
	pItem->AddToolTipLine( buf );

	//2007. 9. 21. CBH - ���� �ִ� �� ������ ���� �ѷ��ֱ�
	if( pItemInfo->NaeRyukRecoverRate != 0 )
	{
		pItem->AddToolTipLine("");
		sprintf(buf, CHATMGR->GetChatMsg(238), (int)pItemInfo->NaeRyukRecoverRate);
		if( HERO->GetCharacterTotalInfo()->Level < pItemInfo->NaeRyukRecoverRate )
			pItem->AddToolTipLine( buf, TTTC_LIMIT );
		else
			pItem->AddToolTipLine( buf, TTTC_FREELIMIT );
	}

	// 060908 KKR
	AddShopItemToolTip( (cIcon*)pItem, pItemInfo );
	
	// ������ �ɼ�  05.02.17
	//AddAvatarItemOptionTooltip( (cIcon*)pItem, pItemInfo );
	
	
	
	// ������ ����
	AddItemDescriptionToolTip( pItem, pItemInfo->ItemTooltipIdx );
	
	// ����ð�
	pItem->AddToolTipLine("");
	pItem->AddToolTipLine( CHATMGR->GetChatMsg(766), TTTC_DEFAULT );
	
	// ����ð� ���Ǵ°�
	if( pItemInfo->SellPrice == eShopItemUseParam_Realtime )
	{
		wsprintf( buf, CHATMGR->GetChatMsg(767), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute() );			
		pItem->AddToolTipLine( buf, TTTC_QUESTITEM );
	}
	// �ð����� ����
	else if( pItemInfo->SellPrice == eShopItemUseParam_Forever )
	{
		pItem->AddToolTipLine( CHATMGR->GetChatMsg(749), TTTC_QUESTITEM );
	}
}

void CItemManager::AddUsedShopEquipItemToolTip( SHOPITEMBASE* pInfo )
{
	CItem* pItem = GetItem( pInfo->ItemBase.dwDBIdx );
	ITEM_INFO* pItemInfo = GetItemInfo( pInfo->ItemBase.wIconIdx );
	if( !pItem || !pItemInfo )		return;

	// �ʱ�ȭ
	cImage imgToolTip;
	SCRIPTMGR->GetImage( 63, &imgToolTip, PFT_HARDPATH );		
	pItem->SetToolTip( "", RGBA_MAKE(255, 255, 255, 255), &imgToolTip, TTCLR_ITEM_CANEQUIP );

	// KKR
	SetEquipItemToolTip( pItem, pItemInfo, NULL, NULL );
	//AddShopItemToolTip( (cIcon*)pItem, pItemInfo );

	char buf[256]= { 0, };
	stTIME time;
	time.value = pInfo->Remaintime;
	
	// ����ð�
	pItem->AddToolTipLine("");
	pItem->AddToolTipLine( CHATMGR->GetChatMsg(766), TTTC_DEFAULT );

	// ����ð� ���Ǵ°�
	if( pItemInfo->SellPrice == eShopItemUseParam_Realtime )
	{
		wsprintf( buf, CHATMGR->GetChatMsg(767), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute() );			
		pItem->AddToolTipLine( buf, TTTC_QUESTITEM );
	}
	// �ð����� ����
	else if( pItemInfo->SellPrice == eShopItemUseParam_Forever )
	{
		pItem->AddToolTipLine( CHATMGR->GetChatMsg(749), TTTC_QUESTITEM );
	}
	
}

void CItemManager::AddUsedPetEquipItemToolTip( SHOPITEMBASE* pInfo )
{
	if(!pInfo)	return;

	ITEM_INFO* pItemInfo = GetItemInfo( pInfo->ItemBase.wIconIdx );
	CItem* pItem = GetItem( pInfo->ItemBase.dwDBIdx );
	if( !pItemInfo || !pItem )	return;

	stTIME time;
	time.value = pInfo->Remaintime;

	// �ʱ�ȭ
	cImage imgToolTip;
	SCRIPTMGR->GetImage( 63, &imgToolTip, PFT_HARDPATH );		
	pItem->SetToolTip( "", RGBA_MAKE(255, 255, 255, 255), &imgToolTip, TTCLR_ITEM_CANEQUIP );

	// ������ �̸�
	char buf[256] = { 0, };
	wsprintf( buf, "[%s]", pItemInfo->ItemName );
	pItem->AddToolTipLine( buf );

	//!!!
	//AddPetEquipItemOptionToolTip();

	// 060908 KKR
	AddShopItemToolTip( (cIcon*)pItem, pItemInfo );

	// ������ ����
	AddItemDescriptionToolTip( pItem, pItemInfo->ItemTooltipIdx );

	// ����ð�
	pItem->AddToolTipLine("");
	pItem->AddToolTipLine( CHATMGR->GetChatMsg(766), TTTC_DEFAULT );

	// ����ð� ���Ǵ°�
	if( pItemInfo->SellPrice == eShopItemUseParam_Realtime )
	{
		wsprintf( buf, CHATMGR->GetChatMsg(767), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute() );			
		pItem->AddToolTipLine( buf, TTTC_QUESTITEM );
	}
	// �ð����� ����
	else if( pItemInfo->SellPrice == eShopItemUseParam_Forever )
	{
		pItem->AddToolTipLine( CHATMGR->GetChatMsg(749), TTTC_QUESTITEM );
	}
}



void CItemManager::RefreshStatsBuffIcon()
{
	SHOPITEMBASE* pItemBase = NULL;

	m_UsedItemList.SetPositionHead();

	while( pItemBase = m_UsedItemList.GetData() )
	{
		ITEM_INFO* pItemInfo = GetItemInfo( pItemBase->ItemBase.wIconIdx );
		if( !pItemInfo)		continue;

		if(  pItemInfo->ItemKind != eSHOP_ITEM_EQUIP && pItemInfo->BuyPrice )
			STATUSICONDLG->AddIcon(HERO, (WORD)(pItemInfo->BuyPrice+1), pItemBase->ItemBase.wIconIdx);
	}
}

// 060908 KKR
// �� ������ ���ε� ������ ���� �Ѹ��� 
void CItemManager::AddShopItemToolTip( cIcon* pIcon, ITEM_INFO* pItemInfo )
{
	char line[128];	
	char buf[256] = { 0, };
	pIcon->AddToolTipLine("");


    if( pItemInfo->GenGol != 0 )
	{
		// ����ġ�߰� ȹ��Ȯ��
		if( pItemInfo->ItemKind == eSHOP_ITEM_PREMIUM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1395), pItemInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �߰��Ǵ� �ٷ�
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1437), pItemInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ������ ȸ����
		if( pItemInfo->ItemKind == eSHOP_ITEM_HERB )
		{
			sprintf(line, CHATMGR->GetChatMsg(1409), pItemInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ��������Ʈ
		if( pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1470), pItemInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ��ȭ���ѽ��۷���
		/*if( pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(1), pItemInfo->GenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}*/
	}
	if( pItemInfo->MinChub != 0 )
	{
		// �� �߰� ��� Ȯ��
		if( pItemInfo->ItemKind == eSHOP_ITEM_PREMIUM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1403), pItemInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �߰��Ǵ� ��ø
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1438), pItemInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ȣ�Ű��� ȸ����
		if( pItemInfo->ItemKind == eSHOP_ITEM_HERB )
		{
			sprintf(line, CHATMGR->GetChatMsg(1433), pItemInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ��ȭ�������᷹��
		/*if( pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(57), pItemInfo->MinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}*/
	}
	if( pItemInfo->CheRyuk != 0 )
	{
		// �Ϸ絿�� ����ġ �г�Ƽ ���� Ƚ��
		if( pItemInfo->ItemKind == eSHOP_ITEM_PREMIUM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1430), pItemInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �߰��Ǵ� ü��
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1441), pItemInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ���� ȸ����
		if( pItemInfo->ItemKind == eSHOP_ITEM_HERB )
		{
			sprintf(line, CHATMGR->GetChatMsg(1399), pItemInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ����ġ+����ȣ�ֹ��� Ƚ��
		if( pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1396), pItemInfo->CheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	/*
	if( pItemInfo->CheRyuk == 0 )
	{
		// ������ �Ұ���
		if( pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(1402), "�Ұ���");
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	*/
	if( pItemInfo->CheRyuk == 1 )
	{
		// �������
		if( pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(1402));
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}

	if( pItemInfo->SimMek != 0 )
	{
		// �Ϸ絿�� �� ���Ƽ ���� Ƚ��
		if( pItemInfo->ItemKind == eSHOP_ITEM_PREMIUM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1431), pItemInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �߰��Ǵ� �ɸ�
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1439), pItemInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �߰� ����ȸ���ӵ�(100%-100)
		if( pItemInfo->ItemKind == eSHOP_ITEM_HERB )
		{
			sprintf(line, CHATMGR->GetChatMsg(1421), pItemInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ü���������� �ε���
		/*if( pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(33), pItemInfo->SimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}*/
	}
	if( pItemInfo->Life != 0 )
	{
		// ������ �����ȶ� ���� �߰�ȹ�� Ȯ�� (100% = 100)
		/*if( pItemInfo->ItemKind == eSHOP_ITEM_PREMIUM )
		{
			sprintf(line, CHATMGR->GetChatMsg(22), pItemInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}*/
		// ���������� ������(100% - 100)
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1397), pItemInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �ִ������ ������
		if( pItemInfo->ItemKind == eSHOP_ITEM_HERB || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1419), pItemInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ��й��� �� �ִ� Ư��ġ
		if( pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1417), pItemInfo->Life);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->Shield != 0 )
	{
		// �ܰ������� ������(100% - 100)
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1416), pItemInfo->Shield);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �ִ�ȣ�Ű��� ������
		if( pItemInfo->ItemKind == eSHOP_ITEM_HERB || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1420), pItemInfo->Shield);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->NaeRyuk != 0 )
	{
		// Ư��ġ ������
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1428), pItemInfo->NaeRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �ִ볻�� ������
		if( pItemInfo->ItemKind == eSHOP_ITEM_HERB || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1418), pItemInfo->NaeRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->LimitJob != 0 )
	{
		// �޺������� ������(100%-100)
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1422), pItemInfo->LimitJob);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �����Ǿ� �������� ����
		if( pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION  )
		{            
			sprintf(line, CHATMGR->GetChatMsg(1432), pItemInfo->LimitJob);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ���������� ������(100% - 100)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1397), pItemInfo->LimitJob);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->LimitGender != 0 )
	{
		// ũ��Ƽ�� ������
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1423), pItemInfo->LimitGender);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �ܰ������� ������(100% - 100)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1416), pItemInfo->LimitGender);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->LimitLevel != 0 )
	{
		// ũ��Ƽ�ý� ����Ȯ��(100%-100)
		if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1424), pItemInfo->LimitLevel);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// Ÿ���� �������� �϶�(100%-100)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1426), pItemInfo->LimitLevel);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->LimitGenGol != 0 )
	{
		// ������������(100%-100)
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1406), pItemInfo->LimitGenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// Ÿ���� �Ӽ����׷°���(100%-100)
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1427), pItemInfo->LimitGenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ���¹̳� �Һ� ���ҷ�(%)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1450), pItemInfo->LimitGenGol);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->LimitMinChub != 0 )
	{
		// �Ӽ���������(100%-100)
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1410), pItemInfo->LimitMinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// Ÿ���� ���ݷ°���(100%-100)
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1425), pItemInfo->LimitMinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ���¹̳� �ִ�ġ ������(val)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1451), pItemInfo->LimitMinChub);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		
	}
	if( pItemInfo->LimitCheRyuk != 0 )
	{
		// ���¼Ҹ𰨼���(100%-100)
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1400), pItemInfo->LimitCheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ���¹̳� ȸ�� ������(%)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1448), pItemInfo->LimitCheRyuk);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->LimitSimMek != 0 )
	{
		// ����ġ �߰�(100% - 100)
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1395), pItemInfo->LimitSimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ȸ���� �߰�����ġ
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1434), pItemInfo->LimitSimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ���¹̳� ȸ�� �߰���(val)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1448), pItemInfo->LimitSimMek);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->ItemGrade != 0 )
	{
		// Ư��ġ�߰�(100% - 100)
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1429), pItemInfo->ItemGrade);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ����ӵ��������
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1394), pItemInfo->ItemGrade);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ģ�е� �߰� ȹ�淮(%)
 		if( pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1447), pItemInfo->ItemGrade);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->RangeType != 0 )
	{
		// ��������ġ�߰�(100% - 100)
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1452), pItemInfo->RangeType);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ��ȥ������ �߰��� ���¼ӵ�( 100 ->100%�߰�)
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1405), pItemInfo->RangeType);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ģ�е� ���� ��ȣ��(%)
		if( pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1551), pItemInfo->RangeType);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->EquipKind != 0 )
	{
		// ������ѷ���
       /* if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(63), pItemInfo->RangeType);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		*/
		// ������� ���¼Ҹ𿩺�(�Ҹ�ȵǸ� 1 )
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1393), pItemInfo->RangeType);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->MeleeAttackMin != 0 )
	{
		// ��ġ�� �ȵǴ� �÷���Ÿ���� �̺�Ʈ ��ȣ
        /*if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(4), pItemInfo->MeleeAttackMin);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}*/
		// ���� ���ݷ�
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1449), pItemInfo->MeleeAttackMin);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->RangeAttackMin != 0 )
	{
		// ��������� ������ 1
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1392));
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ����ȸ���� ȣ�Ű��� ȸ���� (100->100%ȸ��)
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1401), pItemInfo->RangeAttackMin);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->RangeAttackMax != 0 )
	{
		// ����ӵ� �������
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1394), pItemInfo->RangeAttackMax);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ��ȥ���� ���ݷ�����
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION )
		{
			sprintf(line, CHATMGR->GetChatMsg(1474), pItemInfo->RangeAttackMax);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->CriticalPercent != 0 )
	{
		// ��ȭ������ġ
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1390), pItemInfo->CriticalPercent);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �߰��Ǵ� �ϰ�
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			//sprintf(line, CHATMGR->GetChatMsg(1440), pItemInfo->CriticalPercent);
			sprintf(line, CHATMGR->GetChatMsg(1681), pItemInfo->CriticalPercent);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->PhyDef != 0 )
	{
		// �������߰� ���Ȯ��(100% - 100)
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1415), pItemInfo->PhyDef);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->Plus_MugongIdx != 0 )
	{
		// �ִ������������
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1419), pItemInfo->Plus_MugongIdx);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �ݰ� Ȯ�� 100%=100
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1407), pItemInfo->Plus_MugongIdx);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->Plus_Value != 0 )
	{
		// �ִ�ȣ�Ű���������
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1420), pItemInfo->Plus_Value);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// �ݰݽ� ������% 100%=100
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1408), pItemInfo->Plus_Value);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->AllPlus_Kind != 0 )
	{
		// �ִ볻��������
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM )
		{
			sprintf(line, CHATMGR->GetChatMsg(1418), pItemInfo->AllPlus_Kind);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
		// ��������̰� ������ 1
		if( pItemInfo->ItemKind == eSHOP_ITEM_DECORATION  )
		{
			sprintf(line, CHATMGR->GetChatMsg(1392));
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	/*if( pItemInfo->AllPlus_Value != 0 )
	{
		// �ߺ����ȵǴ� ������
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_HERB ||
			pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION || pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(30), pItemInfo->AllPlus_Value);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->MugongNum != 0 )
	{
		// �ߺ����ȵǴ� �����
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_HERB ||
			pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION || pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(29), pItemInfo->MugongNum);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->MugongType != 0 )
	{
		// �ߺ����ȵǴ� �ֹ�����
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_HERB ||
			pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION || pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(32), pItemInfo->MugongType);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->LifeRecover != 0 )
	{
		// �ߺ����ȵǴ� ��ȭ��
        if( pItemInfo->ItemKind == eSHOP_ITEM_CHARM || pItemInfo->ItemKind == eSHOP_ITEM_HERB ||
			pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION || pItemInfo->ItemKind == eSHOP_ITEM_SUNDRIES )
		{
			sprintf(line, CHATMGR->GetChatMsg(31), pItemInfo->LifeRecover);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	*/
	/*
	if( pItemInfo->LifeRecover != 0 )
	{
		// ��������
        if( pItemInfo->ItemKind == eSHOP_ITEM_PREMIUM || pItemInfo->ItemKind == eSHOP_ITEM_CHARM ||
			pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION || pItemInfo->ItemKind == eSHOP_ITEM_MAKEUP ||
			pItemInfo->ItemKind == eSHOP_ITEM_MAKEUP || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION ||
			pItemInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP )
		{
			sprintf(line, CHATMGR->GetChatMsg(1398), pItemInfo->LifeRecover);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	*/
	//2007. 12. 24. CBH - �����ϰ� ������ ��ġ �߸� ó���� �κ� ����
	if( pItemInfo->NaeRyukRecover != 0 )
	{
		// ��������
		if( pItemInfo->ItemKind == eSHOP_ITEM_PREMIUM || pItemInfo->ItemKind == eSHOP_ITEM_CHARM ||
			pItemInfo->ItemKind == eSHOP_ITEM_INCANTATION || pItemInfo->ItemKind == eSHOP_ITEM_MAKEUP ||
			pItemInfo->ItemKind == eSHOP_ITEM_MAKEUP || pItemInfo->ItemKind == eSHOP_ITEM_DECORATION ||
			pItemInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemInfo->ItemKind == eSHOP_ITEM_PET_EQUIP )
		{
			sprintf(line, CHATMGR->GetChatMsg(1682), pItemInfo->NaeRyukRecover);
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->BuyPrice  == 1 )
	{
		// �ŷ��Ұ���
		if( pItemInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemInfo->ItemKind == eSHOP_ITEM_PET )
		{
			sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1465));
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->BuyPrice  == 0 )
	{
		// �ŷ�����
		if( pItemInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemInfo->ItemKind == eSHOP_ITEM_PET )
		{
			if( pItemInfo->ItemType == 11 )
			{
				if( !(((CItem*)pIcon)->GetItemBaseInfo()->ItemParam & ITEM_PARAM_SEAL) )
					sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1465));
				else
					sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1464));
			}
			else
				sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1464));

			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->MeleeAttackMax == 0 )
	{
		// �ŷ����� 
		if( pItemInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemInfo->ItemKind == eSHOP_ITEM_PET )
		{

			if( pItemInfo->ItemType == 11 )
			{
				if( !(((CItem*)pIcon)->GetItemBaseInfo()->ItemParam & ITEM_PARAM_SEAL) )
					sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1465));
				else
					sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1464));
			}
			else
				sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1464));

			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
	if( pItemInfo->MeleeAttackMax == 1 )
	{
		// �ŷ��Ұ��� 
		if( pItemInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemInfo->ItemKind == eSHOP_ITEM_PET )
		{
			sprintf(line, CHATMGR->GetChatMsg(1391), CHATMGR->GetChatMsg(1465));
			pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
		}
	}
}


void CItemManager::AddAvatarItemOptionTooltip( cIcon* pIcon, ITEM_INFO* pItemInfo )
{
/*	char line[128];	
	pIcon->AddToolTipLine("");

	// �ٰ�
	if( pItemInfo->GenGol > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(382), pItemInfo->GenGol);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// ��ø
	if( pItemInfo->MinChub > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(383), pItemInfo->MinChub);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// ü��
	if( pItemInfo->CheRyuk > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(384), pItemInfo->CheRyuk);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// �ɸ�
	if( pItemInfo->SimMek > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(385), pItemInfo->SimMek);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// �ִ������
	if( pItemInfo->Life > 0 )
	{
		sprintf(line, "%s +%d", 
			CHATMGR->GetChatMsg(386), pItemInfo->Life);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// �ִ�ȣ�Ű���
	if( pItemInfo->Shield > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(388), pItemInfo->Shield);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// �ִ볻��
	if( pItemInfo->NaeRyuk > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(387), pItemInfo->NaeRyuk);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// ������ݷ�
	if( pItemInfo->MeleeAttackMin > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(747), pItemInfo->MeleeAttackMin);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// ũ��Ƽ�ü�ġ
	if( pItemInfo->CriticalPercent > 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(390), pItemInfo->CriticalPercent);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// �ݰ�Ȯ��
	if( pItemInfo->Plus_MugongIdx > 0 )
	{
		sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(746), pItemInfo->Plus_MugongIdx);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// �ݰݽ� ��������
	if( pItemInfo->Plus_Value > 0 )
	{
		sprintf(line, "%s +%d%%", CHATMGR->GetChatMsg(745), pItemInfo->Plus_Value);
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
	// ���������
	if( pItemInfo->AllPlus_Kind == 1 )
	{
		sprintf(line, "%s", CHATMGR->GetChatMsg(744));
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}

	
	
#ifdef TAIWAN_LOCAL	
	//�ϵ��ڵ�!!! avartar item
	if( pItemInfo->ItemIdx == 55533 || pItemInfo->ItemIdx == 55534 )
	{
		sprintf(line, "�Ṧ���� +%d", 100 );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );		
	}
	//�ϵ��ڵ�!!! avartar item
	if( pItemInfo->ItemIdx == 55535 || pItemInfo->ItemIdx == 55536 )
	{
		sprintf(line, "�ָ��� +%d%%", 10 );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}
#else
	//�ϵ��ڵ�!!! avartar item
	if( pItemInfo->ItemIdx == 55533 || pItemInfo->ItemIdx == 55534 ||
		pItemInfo->ItemIdx == 57569 || pItemInfo->ItemIdx == 57570 ||
		pItemInfo->ItemIdx == 57571 || pItemInfo->ItemIdx == 57572 ||
		pItemInfo->ItemIdx == 57573 || pItemInfo->ItemIdx == 57574 )
	{
		sprintf(line, CHATMGR->GetChatMsg(1290), 100 );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );		
	}
	//�ϵ��ڵ�!!! avartar item
	if( pItemInfo->ItemIdx == 55535 || pItemInfo->ItemIdx == 55536 ||
		pItemInfo->ItemIdx == 57575 || pItemInfo->ItemIdx == 57576 ||
		pItemInfo->ItemIdx == 57577 || pItemInfo->ItemIdx == 57578 ||
		pItemInfo->ItemIdx == 57579 || pItemInfo->ItemIdx == 57580 )
	{
		sprintf(line, CHATMGR->GetChatMsg(1291), 10 );
		pIcon->AddToolTipLine( line, TTTC_EXTRAATTR );
	}

#endif
*/
}

BOOL CItemManager::CheckItemStage( BYTE bItemStage )
{
	// stage
	if( bItemStage == eItemStage_Normal )		return TRUE;
	
	BYTE stage = HERO->GetStage();
	if( stage == eStage_Normal )				return FALSE;
	if( bItemStage == eItemStage_ChangeStage )	return TRUE;
	
	switch( stage )
	{
	case eStage_Hwa:
		{
			if( bItemStage == eItemStage_OverHwa || bItemStage == eItemStage_Hwa )
				return TRUE;
		}
		break;
	case eStage_Hyun:
		{
			if( bItemStage == eItemStage_ChangeStage2 || bItemStage == eItemStage_OverHwa || bItemStage == eItemStage_Hyun )
				return TRUE;
		}
		break;
	case eStage_Geuk:
		{
			if( bItemStage == eItemStage_OverGeuk || bItemStage == eItemStage_Geuk )
				return TRUE;
		}
		break;
	case eStage_Tal:
		{
			if( bItemStage == eItemStage_ChangeStage2 || bItemStage == eItemStage_OverGeuk || bItemStage == eItemStage_Tal )
				return TRUE;
		}
		break;
	}

	return FALSE;
}

BOOL CItemManager::LoadRareItemInfo()
{
	CMHFile fp;

	char szBuf[256] = {0,};
	char line[512];

	char FileName[256];

#ifdef _FILE_BIN_
	sprintf(FileName, "Resource/Item_RareItemInfo.bin");
	if( !fp.Init(FileName,"rb") ) return FALSE;
#else
	sprintf(FileName, "Resource/Item_RareItemInfo.txt");
	if( !fp.Init(FileName,"rt") ) return FALSE;
#endif

	while( !fp.IsEOF() )
	{
		fp.GetString(szBuf);

		if( szBuf[0] == '@' )
		{
			fp.GetLineX(line,512);
			continue;
		}

		//		strupr(szBuf);

		if( szBuf[0] == '*' )
		{
			sRareItemInfo* pRareItemInfo = new sRareItemInfo;
			pRareItemInfo->ItemIdx = fp.GetDword();
			pRareItemInfo->RareProb = fp.GetDword();

			ASSERT(!m_RareItemInfoTable.GetData(pRareItemInfo->ItemIdx));
			m_RareItemInfoTable.Add(pRareItemInfo, pRareItemInfo->ItemIdx);
		}
	}

	fp.Release();

	return TRUE;
}

void CItemManager::SetIconColorFilter( DWORD dwDBIdx, DWORD clrRGBA )
{
	CItem* pItem = GetItem(dwDBIdx);
	pItem->SetImageRGB(clrRGBA);
}

void CItemManager::SetIconColorFilter( cIcon* pIcon, DWORD clrRGBA )
{
	pIcon->SetImageRGB(clrRGBA);
}

#ifdef _JAPAN_LOCAL_
BOOL CItemManager::CheckItemAttr( WORD wItemAttr )
{
	if( wItemAttr == eIA_All )	return TRUE;

	int nMainAttr = HERO->GetMainCharAttr();
	int	nSubAttr = HERO->GetSubCharAttr();

	int nRelation = wItemAttr - 10;
	if( nRelation > 0 )
	{
		if( nMainAttr == nRelation )	return TRUE;
	}
	else
	{
		if( nMainAttr == wItemAttr || nSubAttr == wItemAttr )	return TRUE;
	}

	return FALSE;
}

BOOL CItemManager::CheckItemAquireSkill( WORD wAcquireSkill1, WORD wAcquireSkill2 )
{
	if( wAcquireSkill1 == 0 )	return TRUE;

	CMugongBase* pMugong = NULL;
	for( int i = wAcquireSkill1; i < wAcquireSkill2+1; ++i )
	{
		pMugong = MUGONGMGR->GetMugongByMugongIdx( i );
		if( pMugong )
		if( pMugong->GetSung() > 0 )
			return TRUE;
	}

	return FALSE;
}
#endif	// _JAPAN_LOCAL_

void CItemManager::CheckInvenFullForAlert(int flg)
{
#ifndef TAIWAN_LOCAL
#ifndef _JAPAN_LOCAL_
#ifndef _HK_LOCAL_
#ifndef _TL_LOCAL_
	return;
#endif
#endif
#endif
#endif
	//�Ϲ� �����۰� �����۸� ������ Ǯ�κ� ���¸� üũ. ��� �޽����� �����ش�.
	CInventoryExDialog* pInven = GAMEIN->GetInventoryDialog();
	CItemShopInven* pSInven = pInven->GetShopInven();

	char buf[256] = {0,};

	if( flg & eCBS_Inven )
	{
		if( 0 == pInven->GetBlankNum() )
		{
			sprintf( buf, "%s %s", CHATMGR->GetChatMsg(1481), CHATMGR->GetChatMsg(122) );
			CHATMGR->AddMsg(CTC_ALERT_YELLOW, buf );
			WINDOWMGR->MsgBox( MBI_INVEN_FULL_ALERT, MBT_OK, CHATMGR->GetChatMsg(122) );
		}
	}

	if( flg & eCBS_SInven )
	{
		if( 0 == pSInven->GetBlankNum() )
		{
			sprintf( buf, "%s %s", CHATMGR->GetChatMsg(1481), CHATMGR->GetChatMsg(1480) );
			CHATMGR->AddMsg(CTC_ALERT_YELLOW, buf );
			WINDOWMGR->MsgBox( MBI_SHOPINVEN_FULL_ALERT, MBT_OK, CHATMGR->GetChatMsg(1480) );
		}
	}
}

void CItemManager::GetItemOptionsAndToolTipFromInfoMsg( cIcon* pIcon, MSG_LINKITEMOPTIONS* pOptionsMsg )
{
	CAddableInfoIterator iter(&pOptionsMsg->AddableInfo);
	BYTE AddInfoKind;

	ITEM_OPTION_INFO OptionInfo;
	memset(&OptionInfo,0,sizeof(OptionInfo));
	ITEM_RARE_OPTION_INFO RareOptionInfo;
	PET_TOTALINFO PetInfo;
	TITAN_TOTALINFO TitanInfo;
	TITAN_ENDURANCE_ITEMINFO ItemInfo;

	while( (AddInfoKind = iter.GetInfoKind()) != CAddableInfoList::None )
	{
		switch(AddInfoKind)
		{
		case(CAddableInfoList::ItemOption):
			{
				//ITEM_OPTION_INFO OptionInfo;
				iter.GetInfoData(&OptionInfo);
				ITEMMGR->AddItemOption(&OptionInfo);
			}
			break;
		case(CAddableInfoList::ItemRareOption):
			{
				//ITEM_RARE_OPTION_INFO RareOptionInfo;
				iter.GetInfoData(&RareOptionInfo);
				ITEMMGR->AddItemRareOption(&RareOptionInfo);
			}
			break;
		case(CAddableInfoList::PetTotalInfo):
			{
				//PET_TOTALINFO PetInfo;
				iter.GetInfoData(&PetInfo);
				PETMGR->AddPetInfo(&PetInfo);
			}
			break;
		case(CAddableInfoList::TitanTotalInfo):
			{
				//TITAN_TOTALINFO TitanInfo;
				iter.GetInfoData(&TitanInfo);
				TITANMGR->AddTitanInfo(&TitanInfo);
			}
			break;
		case(CAddableInfoList::TitanEndrncInfo):
			{
				//TITAN_ENDURANCE_ITEMINFO ItemInfo;
				iter.GetInfoData(&ItemInfo);
				TITANMGR->AddTitanEquipItemEnduranceInfo(&ItemInfo);
			}
			break;
		}
		iter.ShiftToNextData();
	}

	//ToolTips
	int optionsKind = pOptionsMsg->eOptionKind;

	if(optionsKind & eOPTS_ItemOption)
	{
		SetToolTipIcon( pIcon, &OptionInfo, NULL );
	}

	if(optionsKind & eOPTS_ItemRareOption)
	{
		SetToolTipIcon( pIcon, &OptionInfo, &RareOptionInfo );
	}

	if(optionsKind & eOPTS_PetTotalInfo)
	{
		SetToolTipIcon( pIcon, NULL, NULL, pOptionsMsg->ItemInfo.dwDBIdx );
	}

	if(optionsKind & eOPTS_TitanTotalInfo)
	{
		SetToolTipIcon( pIcon );
	}

	if(optionsKind & eOPTS_TitanEquipEndurance)
	{
		SetToolTipIcon( pIcon );
	}

	if(0 == optionsKind)
	{
		SetToolTipIcon( pIcon );
	}
}

///// 2007. 6. 8. CBH - ��Ʈ������ ���� �Լ� �߰� ///////////////////////////
BOOL CItemManager::LoadSetItemOption()
{
	CMHFile file;
#ifdef _FILE_BIN_
	if(!file.Init("Resource/setitem_opt.bin", "rb"))
		return FALSE;
#else
	if(!file.Init("Resource/setitem_opt.txt", "rt"))
		return FALSE;
#endif	// _FILE_BIN_

	SET_ITEM_OPTION* pInfo = NULL;
	while(!file.IsEOF())
	{
		ASSERT(!pInfo);
		pInfo = new SET_ITEM_OPTION;

		pInfo->wIndex = file.GetWord();
		pInfo->wSetItemKind = file.GetWord();
		SafeStrCpy( pInfo->szSetItemName, file.GetString(), MAX_NAME_LENGTH+1 );
		
		pInfo->wGenGol = file.GetWord();
		pInfo->wMinChub = file.GetWord();
		pInfo->wCheRyuk = file.GetWord();
		pInfo->wSimMek = file.GetWord();
		pInfo->dwLife = file.GetDword();
		pInfo->dwShield = file.GetDword();
		pInfo->dwNaeRyuk = file.GetDword();

		pInfo->AttrRegist.SetElement_Val(ATTR_FIRE,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_WATER,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_TREE,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_IRON,file.GetFloat());
		pInfo->AttrRegist.SetElement_Val(ATTR_EARTH,file.GetFloat());
		pInfo->wLimitLevel = file.GetWord();
		SafeStrCpy( pInfo->szEffect, file.GetString(), 100 );		 
        pInfo->wLimitGenGol = file.GetWord();
		pInfo->wLimitMinChub = file.GetWord();
		pInfo->wLimitCheRyuk = file.GetWord();
		pInfo->wLimitSimMek = file.GetWord();

		pInfo->wMeleeAttackMin = file.GetWord();
		pInfo->wMeleeAttackMax = file.GetWord();
		pInfo->wRangeAttackMin = file.GetWord();
		pInfo->wRangeAttackMax = file.GetWord();
		pInfo->wCriticalPercent = file.GetWord();        		
		pInfo->AttrAttack.SetElement_Val(ATTR_FIRE,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_WATER,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_TREE,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_IRON,file.GetFloat());
		pInfo->AttrAttack.SetElement_Val(ATTR_EARTH,file.GetFloat());

		pInfo->wPhyDef = file.GetWord();
		pInfo->wLifeRecover = file.GetWord();
		pInfo->fLifeRecoverRate = file.GetFloat();
		pInfo->wNaeRyukRecover = file.GetWord();
		pInfo->fNaeRyukRecoverRate = file.GetFloat();
		pInfo->wSetValue = file.GetWord();
		pInfo->wApplicationValue = file.GetWord();
		
		ASSERT(!m_SetItemOptionList.GetData(pInfo->wIndex));

		m_SetItemOptionList.Add(pInfo, pInfo->wIndex);
		pInfo = NULL;
	}
	file.Release();

	return TRUE;
}

SET_ITEM_OPTION* CItemManager::GetSetItemOption(WORD wSetItemKind, WORD wSetValue)
{
	SET_ITEM_OPTION* pSetItemOption = NULL;

	m_SetItemOptionList.SetPositionHead();
	while(pSetItemOption = m_SetItemOptionList.GetData())
	{
		if(pSetItemOption->wSetItemKind == wSetItemKind)
		{
			if(wSetValue == pSetItemOption->wSetValue)
			{				
				return pSetItemOption;
			}
		}
	}

	return NULL;
}

SET_ITEM_OPTION* CItemManager::GetSetItemOption(WORD wSetItemKind)
{
	//�ش� ��Ʈ ������ ������ �ְ� Ǯ�� �ɼ� ������ ��ȯ
	SET_ITEM_OPTION* pSetItemOption = NULL;

	m_SetItemOptionList.SetPositionHead();
	while(pSetItemOption = m_SetItemOptionList.GetData())
	{
		if(pSetItemOption->wSetItemKind == wSetItemKind)
		{
			if(pSetItemOption->wSetValue == pSetItemOption->wApplicationValue)
			{
				return pSetItemOption;
			}
		}
	}

	return NULL;
}

void CItemManager::RemoveSetItemOption(WORD wIndex, SET_ITEM_OPTION* pSetItemOptionOut)
{
	SET_ITEM_OPTION* pInfo = m_SetItemOptionList.GetData(wIndex);

	if( pInfo == NULL )
	{
		ASSERT(0);		
		return;
	}

	if(pSetItemOptionOut)
		*pSetItemOptionOut = *pInfo;
	SAFE_DELETE( pInfo );
	m_SetItemOptionList.Remove(wIndex);
}

void CItemManager::AddSetItemToolTip(cIcon* pIcon, ITEM_INFO* pItemInfo )
{
	char line[128];	
	DWORD dwSetItemCount = 0;
		
	dwSetItemCount = GetSetItemEquipValue(pItemInfo);

	//���� �������� ��Ʈ������ �ְ� �Ʈ �������� �ִ��� �˻�		
	m_SetItemOptionList.SetPositionHead();	
	SET_ITEM_OPTION* pSetItemOption = NULL;
	while( pSetItemOption = m_SetItemOptionList.GetData() )
	{
		if(pSetItemOption->wSetItemKind == pItemInfo->wSetItemKind)
		{
			WORD wSetValue = pSetItemOption->wSetValue;            

			sprintf( line, CHATMGR->GetChatMsg(1577), wSetValue);
			pIcon->AddToolTipLine(line);

			if(dwSetItemCount >= wSetValue)
			{
				SetSetItemToolTip( pIcon, pItemInfo, TRUE, wSetValue );
			}
			else
			{
				SetSetItemToolTip( pIcon, pItemInfo, FALSE, wSetValue );
			}
		}		
	}
}

DWORD CItemManager::GetSetItemEquipValue(ITEM_INFO* pItemInfo)
{
	CItem* pItem = NULL;
	DWORD dwSetItemCount = 0;

	//���â�� ���� ������ ��Ʈ �������� � ���� ���ִ��� �˻�
	for(unsigned int ui = TP_WEAR_START ; ui < TP_WEAR_END ; ui++)
	{
		pItem = GAMEIN->GetInventoryDialog()->GetItemForPos( ui );
		if( pItem )
		{
			if(pItem->GetItemInfo()->wSetItemKind == pItemInfo->wSetItemKind)
			{
				dwSetItemCount++;
			}		
		}

		pItem = NULL;
	}

	return dwSetItemCount;
}

void CItemManager::SetSetItemToolTip( cIcon* pIcon, ITEM_INFO* pItemInfo, BOOL bSetItemEnable, WORD wSetValue)
{
	char line[128];
	DWORD dwColor;
	float attrvalue = 0;
	
	if(bSetItemEnable == TRUE)
	{
		dwColor = TTTC_SETITEMOPTION_ENABLE;
	}
	else
	{
		dwColor = TTTC_SETITEMOPTION_DISABLE;
	}

	
	SET_ITEM_OPTION* pSetItemOption = NULL;	
	pSetItemOption = GetSetItemOption(pItemInfo->wSetItemKind, wSetValue);
	
	if(pSetItemOption == NULL)
	{
		return;
	}	

	/// �ٰ�
	if( pSetItemOption->wGenGol != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(382), pSetItemOption->wGenGol);		
		pIcon->AddToolTipLine( line, dwColor );
	}
	//��ø
	if( pSetItemOption->wMinChub != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(383), pSetItemOption->wMinChub);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//ü��
	if( pSetItemOption->wCheRyuk != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(384), pSetItemOption->wCheRyuk);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//�ɸ�
	if( pSetItemOption->wSimMek != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(385), pSetItemOption->wSimMek);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//�ִ������
	if( pSetItemOption->dwLife != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(386), pSetItemOption->dwLife);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//ȣ�Ű���
	if( pSetItemOption->dwShield != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(388), pSetItemOption->dwShield);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//�ִ볻��
	if( pSetItemOption->dwNaeRyuk != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(387), pSetItemOption->dwNaeRyuk);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//�Ӽ� ���׷�
	for(int i = ATTR_FIRE; i <= ATTR_MAX; ++i )
	{
		attrvalue = pSetItemOption->AttrRegist.GetElement_Val(i);
		if(attrvalue != 0)
		{
			sprintf( line, "%s +%d%%", CHATMGR->GetChatMsg(265+i-1), (int) (attrvalue * 100) );
			pIcon->AddToolTipLine( line, dwColor );
		}
		attrvalue = 0;
	}
	//�ٰŸ� ���ݷ�
	if( pSetItemOption->wMeleeAttackMin != 0 || pSetItemOption->wMeleeAttackMax != 0)
	{
		sprintf( line, "%s %d ~ %d", CHATMGR->GetChatMsg(389), pSetItemOption->wMeleeAttackMin, pSetItemOption->wMeleeAttackMax );
		pIcon->AddToolTipLine( line, dwColor );
	}	
	//���Ÿ� ���ݷ�
	if( pSetItemOption->wRangeAttackMin != 0 || pSetItemOption->wRangeAttackMax != 0)
	{
		sprintf( line, "%s %d ~ %d", CHATMGR->GetChatMsg(391), pSetItemOption->wRangeAttackMin, pSetItemOption->wRangeAttackMax );
		pIcon->AddToolTipLine( line, dwColor );
	}	
	//ũ��Ƽ��
	if( pSetItemOption->wCriticalPercent != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(390), pSetItemOption->wCriticalPercent);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//�Ӽ� ���ݷ�
	for(int i = ATTR_FIRE; i <= ATTR_MAX; ++i )
	{
		attrvalue = pSetItemOption->AttrAttack.GetElement_Val(i);
		if(attrvalue != 0)
		{
			sprintf( line, "%s +%d%%", CHATMGR->GetChatMsg(392+i-1), (int) (attrvalue * 100) );
			pIcon->AddToolTipLine( line, dwColor );
		}
		attrvalue = 0;
	}		
	//��������
	if( pSetItemOption->wPhyDef != 0 )
	{
		sprintf(line, "%s +%d", CHATMGR->GetChatMsg(397), pSetItemOption->wPhyDef);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//������ ȸ�� (����)	
	if(pSetItemOption->wLifeRecover != 0)
	{
		sprintf(line, CHATMGR->GetChatMsg(1564), pSetItemOption->wLifeRecover);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//������ ȸ��
	if(pSetItemOption->fLifeRecoverRate != 0)
	{
		sprintf(line, CHATMGR->GetChatMsg(1566), (WORD)(pSetItemOption->fLifeRecoverRate * 100));
		pIcon->AddToolTipLine( line, dwColor );
	}
	//���� ȸ�� (����)
	if(pSetItemOption->wNaeRyukRecover != 0)
	{
		sprintf(line, CHATMGR->GetChatMsg(1565), pSetItemOption->wNaeRyukRecover);
		pIcon->AddToolTipLine( line, dwColor );
	}
	//���� ȸ��
	if(pSetItemOption->fNaeRyukRecoverRate != 0)
	{
		sprintf(line, CHATMGR->GetChatMsg(1567), (WORD)(pSetItemOption->fNaeRyukRecoverRate * 100));
		pIcon->AddToolTipLine( line, dwColor );
	}
}

CYHHashTable<SET_ITEM_OPTION>* CItemManager::GetSetItemOptionList()
{
	return &m_SetItemOptionList;
}

void CItemManager::SetItemEffectProcess(CPlayer* pPlayer)
{
	//ĳ���� ����Ʈ ó�� 
	SET_ITEM_INFO setItemInfo[MAX_SETITEM_KIND_NUM];
	memset(setItemInfo, 0, sizeof(SET_ITEM_INFO) * MAX_SETITEM_KIND_NUM);		

	//Ÿ��ź�� Ÿ�� ������ ����Ʈ ����
	if( (pPlayer->InTitan() == TRUE) && (pPlayer->GetSetItemEffectID() != 0) )
	{
		pPlayer->RemoveObjectEffect( pPlayer->GetSetItemEffectID() );
		pPlayer->SetSetItemEffectID( 0 );							
		return;
	}

	//ĳ������ ���� ������ �˻��Ͽ� �迭�� ����
	CHARACTER_TOTALINFO* pCharacterinfo = pPlayer->GetCharacterTotalInfo();

	for(unsigned int i = 0 ; i < eWearedItem_Max ; i++)
	{
		WORD wWearedItemIdx = pCharacterinfo->WearedItemIdx[i];
		ITEM_INFO* pItemInfo = GetItemInfo(wWearedItemIdx);
		if( !pItemInfo )
			continue;

		if(pItemInfo->wSetItemKind != 0)
		{
			for(unsigned int ui = 0 ; ui < MAX_SETITEM_KIND_NUM ; ui++)
			{
				if(setItemInfo[ui].wKind == pItemInfo->wSetItemKind)
				{
					setItemInfo[ui].wCount++;
					break;
				}
				else
				{
					if(setItemInfo[ui].wKind == 0)
					{
						setItemInfo[ui].wKind = pItemInfo->wSetItemKind;
						setItemInfo[ui].wCount++;
						break;
					}					
				}				
			}
		}
	}	
	
	SET_ITEM_OPTION* pSetItemOption = NULL;		
	for(unsigned int ui = 0 ; ui < MAX_SETITEM_KIND_NUM ; ui++)
	{
		//3�� ��Ʈ �̻� ����ú��� �˻�
		if(setItemInfo[ui].wCount != 0)
		{			
			pSetItemOption = GetSetItemOption(setItemInfo[ui].wKind);

			if(pSetItemOption != NULL)
			{
				if( setItemInfo[ui].wCount == pSetItemOption->wSetValue )
				{
					if( pPlayer->GetSetItemEffectID() != SETITEM_STARTEFFECTID+pSetItemOption->wIndex )
					{
						if( pPlayer->GetSetItemEffectID() != 0 )
						{							
							pPlayer->RemoveObjectEffect( pPlayer->GetSetItemEffectID() );
							pPlayer->SetSetItemEffectID( 0 );							
						}

						if(pPlayer->InTitan() == FALSE)
						{
							OBJECTEFFECTDESC desc(FindEffectNum(pSetItemOption->szEffect));
							pPlayer->AddObjectEffect( SETITEM_STARTEFFECTID+pSetItemOption->wIndex, &desc, 1 );
							pPlayer->SetSetItemEffectID( SETITEM_STARTEFFECTID+pSetItemOption->wIndex );
						}						
					}
				}
				else
				{
					if( pPlayer->GetSetItemEffectID() == SETITEM_STARTEFFECTID+pSetItemOption->wIndex )
					{
						pPlayer->RemoveObjectEffect( pPlayer->GetSetItemEffectID() );
						pPlayer->SetSetItemEffectID( 0 );
					}
				}
			}		
		}		
	}	
}
/////////////////////////////////////////////////////////////////////////////

// magi82 - UniqueItem(070628)
void CItemManager::AddUniqueItemToolTip(cIcon* pIcon, ITEM_INFO* pItemInfo )
{
	char line[128];	
	char buf[256] = { 0, };
	UNIQUE_ITEM_OPTION_INFO* pUniqueInfo = GAMERESRCMNGR->GetUniqueItemOptionList(pItemInfo->ItemIdx);
	if( !pUniqueInfo )
	{
		return;
	}

	pIcon->AddToolTipLine("");

	// �ŷ�, ������
	if(pUniqueInfo->ExchangeFlag == 1)
	{
		sprintf(line, CHATMGR->GetChatMsg(1607));
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	}
	else
	{
		sprintf(line, CHATMGR->GetChatMsg(1608));
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );
	}

	// ����
	if(pUniqueInfo->MixFlag == 1)
	{
		sprintf(line, CHATMGR->GetChatMsg(1609));
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	}
	else
	{
		sprintf(line, CHATMGR->GetChatMsg(1610));
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );
	}

	// HP
	sprintf(line, CHATMGR->GetChatMsg(1611), pUniqueInfo->nHp);
	if(pUniqueInfo->nHp > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nHp < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// MP
	sprintf(line, CHATMGR->GetChatMsg(1612), pUniqueInfo->nMp);
	if(pUniqueInfo->nMp > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nMp < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// ȣ�Ű���
	sprintf(line, CHATMGR->GetChatMsg(1613), pUniqueInfo->nShield);
	if(pUniqueInfo->nShield > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nShield < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// ����������
	sprintf(line, CHATMGR->GetChatMsg(1614), pUniqueInfo->nPhyDamage);
	if(pUniqueInfo->nPhyDamage > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nPhyDamage < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// �ϰݵ�����
	sprintf(line, CHATMGR->GetChatMsg(1615), pUniqueInfo->nCriDamage);
	if(pUniqueInfo->nCriDamage > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nCriDamage < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// �ϰ�Ȯ��
	sprintf(line, CHATMGR->GetChatMsg(1616), pUniqueInfo->nCriRate);
	if(pUniqueInfo->nCriRate > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nCriRate < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// �ٰ�
	sprintf(line, CHATMGR->GetChatMsg(1617), pUniqueInfo->nGengol);
	if(pUniqueInfo->nGengol > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nGengol < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// ��ø
	sprintf(line, CHATMGR->GetChatMsg(1618), pUniqueInfo->nMinChub);
	if(pUniqueInfo->nMinChub > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nMinChub < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// ü��
	sprintf(line, CHATMGR->GetChatMsg(1619), pUniqueInfo->nCheRyuk);
	if(pUniqueInfo->nCheRyuk > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nCheRyuk < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// �ɸ�
	sprintf(line, CHATMGR->GetChatMsg(1620), pUniqueInfo->nSimMek);
	if(pUniqueInfo->nSimMek > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nSimMek < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// ����
	sprintf(line, CHATMGR->GetChatMsg(1621), pUniqueInfo->nDefen);
	if(pUniqueInfo->nDefen > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nDefen < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// �����Ÿ�
	sprintf(line, CHATMGR->GetChatMsg(1622), pUniqueInfo->nRange);
	if(pUniqueInfo->nRange > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nRange < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// �Ӽ�����
	sprintf(line, CHATMGR->GetChatMsg(1623), pUniqueInfo->nAttR);
	if(pUniqueInfo->nAttR > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nAttR < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

	// ������
	sprintf(line, CHATMGR->GetChatMsg(1624), pUniqueInfo->nEnemyDefen);
	if(pUniqueInfo->nEnemyDefen > 0)
		pIcon->AddToolTipLine( line, TTTC_MUGONGKIND );
	else if( pUniqueInfo->nEnemyDefen < 0)
		pIcon->AddToolTipLine( line, TTCLR_ENERMY );

}
