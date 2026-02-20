// TitanItemManager.cpp: implementation of the CTitanItemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TitanItemManager.h"
#include "MHFile.h"
#include "ItemManager.h"
#include "CheckRoutine.h"
#include "Player.h"
#include "MapDBMsgParser.h"
#include "MugongManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTitanItemManager::CTitanItemManager()
{
	m_TPMInfoList.Initialize(MAX_ITEM_NUM);

	// ÆÄÃ÷ ¼Ó¼º Å×ÀÌºí
	m_TitanPartsKindPool = new CMemoryPoolTempl<TITAN_PARTS_KIND>;
	m_TitanPartsKindPool->Init( 100, 10, "TitanPartsKind" );
	m_TitanPartsKindTable.Initialize( 100 );

	// Å¸ÀÌÅº ¾÷±×·¹ÀÌµå Å×ÀÌºí
	m_TitanUpgradeInfoTable.Initialize(100);

	// Å¸ÀÌÅº ºÐÇØ Å×ÀÌºí
	m_TitanBreakInfoTable.Initialize(MAX_ITEM_NUM);


	// Å¸ÀÌÅº ºÐÇØ È®·üº¯¼ö ÃÊ±âÈ­
	m_TitanBreakRate = 0;
}

CTitanItemManager::~CTitanItemManager()
{
	ITEM_MIX_INFO * pTitanItemInfo = NULL;
	m_TPMInfoList.SetPositionHead();
	while(pTitanItemInfo = m_TPMInfoList.GetData())
	{
		if(pTitanItemInfo->psResultItemInfo)
		{
			for(int i = 0 ; i < pTitanItemInfo->wResultItemNum ; ++i)
			{
				if(pTitanItemInfo->psResultItemInfo[i].wResItemIdx != 0)
				{
					if(pTitanItemInfo->psResultItemInfo[i].psMaterialItemInfo)
					{
						delete [] pTitanItemInfo->psResultItemInfo[i].psMaterialItemInfo;
						pTitanItemInfo->psResultItemInfo[i].psMaterialItemInfo = NULL;
					}
				}
			}
			delete [] pTitanItemInfo->psResultItemInfo;
			pTitanItemInfo->psResultItemInfo = NULL;
		}
		delete pTitanItemInfo;
		pTitanItemInfo = NULL;
	}
	m_TPMInfoList.RemoveAll();

	// ÆÄÃ÷ ¼Ó¼º Å×ÀÌºí ¸±¸®Áî
	m_TitanPartsKindTable.SetPositionHead();
	TITAN_PARTS_KIND* pTitanPartsKind = NULL;
	while( pTitanPartsKind = m_TitanPartsKindTable.GetData() )
	{
		m_TitanPartsKindPool->Free( pTitanPartsKind );
	}
	m_TitanPartsKindTable.RemoveAll();

	SAFE_DELETE(m_TitanPartsKindPool);
	//////////////////////////////////////////////////////////////////////////

	// Å¸ÀÌÅº ¾÷±×·¹ÀÌµå Å×ÀÌºí ¸±¸®Áî
	TITAN_UPGRADEINFO* pTitanInfo;
	m_TitanUpgradeInfoTable.SetPositionHead();
	while(pTitanInfo = m_TitanUpgradeInfoTable.GetData())
	{
		delete pTitanInfo;
		pTitanInfo = NULL;
	}
	m_TitanUpgradeInfoTable.RemoveAll();
	//////////////////////////////////////////////////////////////////////////

	// Å¸ÀÌÅº ºÐÇØ Å×ÀÌºí ¸±¸®Áî
	TITAN_BREAKINFO* pTitanBreakInfo;
	m_TitanBreakInfoTable.SetPositionHead();
	while(pTitanBreakInfo = m_TitanBreakInfoTable.GetData())
	{
		delete pTitanBreakInfo;
		pTitanBreakInfo = NULL;
	}
	m_TitanBreakInfoTable.RemoveAll();
	//////////////////////////////////////////////////////////////////////////
}

ITEM_MIX_INFO * CTitanItemManager::GetTitanMixItemInfo(WORD wItemIdx)
{
	return m_TPMInfoList.GetData(wItemIdx);
}

void CTitanItemManager::LoadTitanMixResultItemInfo(CMHFile * fp, ITEM_MIX_RES * pResInfo)
{
	pResInfo->wResItemIdx = fp->GetWord();
	SafeStrCpy(pResInfo->szResItemName, fp->GetString(), MAX_ITEMNAME_LENGTH+1 );
	pResInfo->wMixKind = fp->GetWord();
	pResInfo->Money = fp->GetDword();
	pResInfo->SuccessRatio = fp->GetWord();
	pResInfo->wMaterialItemNum = fp->GetWord();
	pResInfo->psMaterialItemInfo = new ITEM_MIX_MATERIAL[pResInfo->wMaterialItemNum];
	for( int i = 0 ; i < pResInfo->wMaterialItemNum ; ++i)
	{
		pResInfo->psMaterialItemInfo[i].wMatItemIdx = fp->GetWord();
		SafeStrCpy(pResInfo->psMaterialItemInfo[i].szMatItemName, fp->GetString(), MAX_ITEMNAME_LENGTH+1);
		pResInfo->psMaterialItemInfo[i].wMatItemNum = fp->GetWord();
	}
}
void CTitanItemManager::LoadTitanMixList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	if(!file.Init("Resource/TitanMixList.bin", "rb"))
		return;
#else
	if(!file.Init("Resource/TitanMixList.txt", "rt"))
		return;
#endif

	ITEM_MIX_INFO * pInfo = NULL;
	WORD wTmpItemIdx = 0;
	WORD wTmpRestIdx = 0;
	while(1)
	{
		if(file.IsEOF())
			break;

		wTmpItemIdx = file.GetWord();
		pInfo = m_TPMInfoList.GetData(wTmpItemIdx);
		if(!pInfo)
		{
			pInfo = new ITEM_MIX_INFO;
			memset(pInfo, 0, sizeof(ITEM_MIX_INFO));
		}
		pInfo->wItemIdx = wTmpItemIdx;
		SafeStrCpy(pInfo->szItemName, file.GetString(), MAX_ITEMNAME_LENGTH+1);
		pInfo->wResultItemNum = file.GetWord();
		if(!pInfo->psResultItemInfo)
		{
			pInfo->psResultItemInfo = new ITEM_MIX_RES[pInfo->wResultItemNum];
			memset(pInfo->psResultItemInfo, 0, sizeof(ITEM_MIX_RES)*pInfo->wResultItemNum);
		}
		for(wTmpRestIdx = 0 ; wTmpRestIdx < pInfo->wResultItemNum ; ++wTmpRestIdx)
		{	
			if(pInfo->psResultItemInfo[wTmpRestIdx].wResItemIdx == 0)
			{
				LoadTitanMixResultItemInfo(&file, &pInfo->psResultItemInfo[wTmpRestIdx]);
				break;
			}
		}

		if(!m_TPMInfoList.GetData(pInfo->wItemIdx))
			m_TPMInfoList.Add(pInfo, pInfo->wItemIdx);
		pInfo = NULL;
	}
	file.Release();
}

BOOL CTitanItemManager::LoadTitanPartsKindList()
{
	TITAN_PARTS_KIND* pTemp = NULL;

	CMHFile file;
#ifdef _FILE_BIN_
	if(!file.Init("./Resource/TitanPartsKind.bin", "rb"))
		return FALSE;
#else
	if(!file.Init("./Resource/TitanPartsKind.txt", "rt"))
		return FALSE;
#endif

	while(1)
	{
		if(file.IsEOF())
			break;

		pTemp = m_TitanPartsKindPool->Alloc();
		ZeroMemory(pTemp, sizeof(TITAN_PARTS_KIND));

		pTemp->dwPartsIdx = file.GetDword();
		pTemp->dwPartsKind = file.GetDword();
		pTemp->dwResultTitanIdx = file.GetDword();

		m_TitanPartsKindTable.Add(pTemp, pTemp->dwPartsIdx);
	}

	return TRUE;
}

TITAN_PARTS_KIND* CTitanItemManager::GetTitanPartsKind(  DWORD partsIdx  )
{
	return m_TitanPartsKindTable.GetData( partsIdx );
}

void CTitanItemManager::LoadTitanUpgradeInfoResultItemInfo(CMHFile * fp, TITAN_UPGRADE_MATERIAL * pMaterialInfo)
{
	pMaterialInfo->dwIndex = fp->GetDword();
	pMaterialInfo->dwCount = fp->GetDword();
}
void CTitanItemManager::LoadTitanUpgradeInfoList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	if(!file.Init("Resource/TitanUpgradeInfo.bin", "rb"))
		return;
#else
	if(!file.Init("Resource/TitanUpgradeInfo.txt", "rt"))
		return;
#endif

	TITAN_UPGRADEINFO * pInfo = NULL;
	DWORD dwTitanIdx = 0;

	while(1)
	{
		if(file.IsEOF())
			break;

		dwTitanIdx = file.GetDword();

		pInfo = new TITAN_UPGRADEINFO;
		ZeroMemory(pInfo, sizeof(TITAN_UPGRADEINFO));
		pInfo->dwTitanIdx = dwTitanIdx;
		pInfo->dwNextTitanIdx = file.GetDword();
		pInfo->dwMoney = file.GetDword();
		pInfo->wMaterial_Num = file.GetWord();

		if(pInfo->wMaterial_Num)
		{
			pInfo->pTitanUpgradeMaterial = new TITAN_UPGRADE_MATERIAL[pInfo->wMaterial_Num];
			ZeroMemory(pInfo->pTitanUpgradeMaterial, sizeof(TITAN_UPGRADE_MATERIAL)*pInfo->wMaterial_Num);
		}

		for( int i = 0; i < pInfo->wMaterial_Num; i++ )
			LoadTitanUpgradeInfoResultItemInfo(&file, &pInfo->pTitanUpgradeMaterial[i]);

		m_TitanUpgradeInfoTable.Add(pInfo, pInfo->dwTitanIdx);

		pInfo = NULL;
	}
	file.Release();
}

TITAN_UPGRADEINFO * CTitanItemManager::GetTitanUpgradeInfoItemInfo(DWORD titanIdx)
{
	return m_TitanUpgradeInfoTable.GetData(titanIdx);
}

void CTitanItemManager::LoadTitanBreakInfoMaterialInfo(CMHFile * fp, TITAN_BREAK_MATERIAL * pMaterialInfo)
{
	pMaterialInfo->dwMaterialIdx = fp->GetDword();
	pMaterialInfo->wMaterCnt = fp->GetWord();
	pMaterialInfo->wRate = fp->GetWord();
}

void CTitanItemManager::LoadTitanBreakInfoList()
{
	CMHFile file;
#ifdef _FILE_BIN_
	if(!file.Init("Resource/TitanBreakList.bin", "rb"))
		return;
#else
	if(!file.Init("Resource/TitanBreakList.txt", "rt"))
		return;
#endif

	TITAN_BREAKINFO * pInfo = NULL;
	DWORD dwIdx = 0;

	while(1)
	{
		if(file.IsEOF())
			break;

		if(!m_TitanBreakRate)
			m_TitanBreakRate = file.GetDword();

		dwIdx = file.GetDword();

		pInfo = new TITAN_BREAKINFO;
		ZeroMemory(pInfo, sizeof(TITAN_BREAKINFO));
		pInfo->dwIdx = dwIdx;
		pInfo->dwMoney = file.GetDword();
		pInfo->wTotalCnt = file.GetWord();
		pInfo->wGetCnt = file.GetWord();

		if(pInfo->wTotalCnt)
		{
			pInfo->pTitanBreakMaterial = new TITAN_BREAK_MATERIAL[pInfo->wTotalCnt];
			ZeroMemory(pInfo->pTitanBreakMaterial, sizeof(TITAN_BREAK_MATERIAL)*pInfo->wTotalCnt);
		}

		for( int i = 0; i < pInfo->wTotalCnt; i++ )
			LoadTitanBreakInfoMaterialInfo(&file, &pInfo->pTitanBreakMaterial[i]);

		m_TitanBreakInfoTable.Add(pInfo, pInfo->dwIdx);

		pInfo = NULL;
	}
	file.Release();
}

TITAN_BREAKINFO * CTitanItemManager::GetTitanBreakInfoItemInfo(DWORD dwIdx)
{
	return m_TitanBreakInfoTable.GetData(dwIdx);
}

void CTitanItemManager::TitanMixAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg )
{
	WORD wErrorCode = 0;
	CItemSlot * pSlot = NULL;
	MSG_ITEM msgAck;

	// ÀÎº¥Åä¸® ¾ÆÀÌÅÛÀÌ ¾Æ´Ï¸é ¿¡·¯
	if( GetTableIdxPosition( pmsg->ItemInfo.Position ) != eItemTable_Inventory )
		wErrorCode = 1;

	// ¼­¹ö¿Í Å¬¶óÀÇ ¾ÆÀÌÅÛÁ¤º¸°¡ °°ÀºÁö Ã¼Å©
	if(!CHKRT->ItemOf(pPlayer, pmsg->ItemInfo.Position, pmsg->ItemInfo.wIconIdx, 0,0, CB_EXIST|CB_ICONIDX))
		wErrorCode = 2;

	pSlot = pPlayer->GetSlot(eItemTable_Inventory);
	if( pSlot )
	{
		// ÇØ´ç ½½·ÔÀÌ ÀÌ¹Ì Àá°ÜÀÖÀ¸¸é ¿¡·¯
		if( pSlot->IsLock( pmsg->ItemInfo.Position ) )
			wErrorCode = 3;

		// ¿É¼Ç¾ÆÀÌÅÛÀÎÁö Ã¼Å©
		if(ITEMMGR->IsOptionItem( pmsg->ItemInfo.wIconIdx, pmsg->ItemInfo.Durability ) )
			wErrorCode = 4;

		// Á¤»óÀÏ¶§ Ã³¸®
		if(wErrorCode == 0)
		{
			pSlot->SetLock( pmsg->ItemInfo.Position, TRUE );

			msgAck.Category		= MP_ITEM;
			msgAck.Protocol		= MP_ITEM_TITANMIX_ADDITEM_ACK;
			msgAck.ItemInfo		= pmsg->ItemInfo;
			pPlayer->SendMsg( &msgAck, sizeof( msgAck ) );			
		}
		else
		{
			MSG_WORD2 msgNack;
			msgNack.Category	= MP_ITEM;
			msgNack.Protocol	= MP_ITEM_TITANMIX_ADDITEM_NACK;
			msgNack.wData1		= pmsg->ItemInfo.Position;
			msgNack.wData2		= wErrorCode;
			pPlayer->SendMsg( &msgNack, sizeof( msgNack ) );		
		}
	}
}

void CTitanItemManager::TitanMix( CPlayer* pPlayer, MSG_TITANMIX* pmsg )
{
	for( int i = 0; i < 4; i++ )
	{
		if(!CHKRT->ItemOf(pPlayer, pmsg->iconInfo[i].itempos, (WORD)pmsg->iconInfo[i].dwItemIdx, 0,0, CB_EXIST|CB_ICONIDX))
			return;
	}

	TITAN_PARTS_KIND* pTitanPartsKind = TITANITEMMGR->GetTitanPartsKind(pmsg->iconInfo[0].dwItemIdx);
	ITEMBASE Item;
	memset( &Item, 0, sizeof(ITEMBASE) );
	CItemSlot* pSlot = pPlayer->GetSlot(eItemTable_Inventory);

	// magi1127
	// ÆÄÃ÷µé »èÁ¦
	for( int i = 0; i < 4; i++ )
	{
		Item = *pSlot->GetItemInfoAbs(pmsg->iconInfo[i].itempos);
		if(EI_TRUE == ITEMMGR->DiscardItem(pPlayer, pmsg->iconInfo[i].itempos, (WORD)pmsg->iconInfo[i].dwItemIdx, 1))
		{
			POSTYPE TargetPos = 0;
			WORD wItemIdx = 0;
			DURTYPE ItemNum = 0;

			MSG_ITEM_DISCARD_ACK msg;
			msg.TargetPos = pmsg->iconInfo[i].itempos;
			msg.wItemIdx = (WORD)pmsg->iconInfo[i].dwItemIdx;
			msg.ItemNum = 1;
			msg.Category = MP_ITEM;
			msg.Protocol = MP_ITEM_TITANMIX_ACK_DELETEPARTS;
			ITEMMGR->SendAckMsg(pPlayer, &msg, sizeof(msg));

			// Log
			LogItemMoney(pmsg->dwObjectID, pPlayer->GetObjectName(), 0, "",
				eLog_ItemDiscard, pPlayer->GetMoney(eItemTable_Inventory), 0, 0,
				Item.wIconIdx, Item.dwDBIdx,  Item.Position, 0,
				Item.Durability, pPlayer->GetPlayerExpPoint());
		}
	}

	// ¾ÆÀÌÅÛ »ý¼º
	WORD EmptyCellPos[255];
	WORD EmptyCellNum;

	WORD obtainItemNum = ITEMMGR->GetCanBuyNumInSpace( pPlayer, pSlot, (WORD)pTitanPartsKind->dwResultTitanIdx, 1, EmptyCellPos, EmptyCellNum );
	if(obtainItemNum == 0)
		return;

	ITEMMGR->ObtainItemEx( pPlayer, ITEMMGR->Alloc(pPlayer, MP_ITEM, MP_ITEM_TITANMIX_ACK, pPlayer->GetID(), 0, eLog_ItemObtainTitan, obtainItemNum, (DBResult)(ITEMMGR->ObtainItemDBResult)), (WORD)pTitanPartsKind->dwResultTitanIdx, obtainItemNum, EmptyCellPos, EmptyCellNum, EmptyCellNum );
}

void CTitanItemManager::TitanupgradeAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg )
{
	WORD wErrorCode = 0;
	CItemSlot * pSlot = NULL;
	MSG_ITEM msgAck;

	// ÀÎº¥Åä¸® ¾ÆÀÌÅÛÀÌ ¾Æ´Ï¸é ¿¡·¯
	if( GetTableIdxPosition( pmsg->ItemInfo.Position ) != eItemTable_Inventory )
		wErrorCode = 1;

	// ¼­¹ö¿Í Å¬¶óÀÇ ¾ÆÀÌÅÛÁ¤º¸°¡ °°ÀºÁö Ã¼Å©
	if(!CHKRT->ItemOf(pPlayer, pmsg->ItemInfo.Position, pmsg->ItemInfo.wIconIdx, 0,0, CB_EXIST|CB_ICONIDX))
		wErrorCode = 2;

	pSlot = pPlayer->GetSlot(eItemTable_Inventory);
	if( pSlot )
	{
		// ÇØ´ç ½½·ÔÀÌ ÀÌ¹Ì Àá°ÜÀÖÀ¸¸é ¿¡·¯
		if( pSlot->IsLock( pmsg->ItemInfo.Position ) )
			wErrorCode = 3;

		// ¿É¼Ç¾ÆÀÌÅÛÀÎÁö Ã¼Å©
		if(ITEMMGR->IsOptionItem( pmsg->ItemInfo.wIconIdx, pmsg->ItemInfo.Durability ) )
			wErrorCode = 4;

		// 3´Ü°èÀÌÈÄ·Î´Â ¾÷±×·¹ÀÌµå ¾ÈµÊ
		int TitanGrade = ITEMMGR->GetItemInfo(pmsg->ItemInfo.wIconIdx)->Plus_Value;
		if( TitanGrade >= 3 )
			wErrorCode = 5;

		// Á¤»óÀÏ¶§ Ã³¸®
		if(wErrorCode == 0)
		{
			pSlot->SetLock( pmsg->ItemInfo.Position, TRUE );

			msgAck.Category		= MP_ITEM;
			msgAck.Protocol		= MP_ITEM_TITANUPGRADE_ADDITEM_ACK;
			msgAck.ItemInfo		= pmsg->ItemInfo;
			pPlayer->SendMsg( &msgAck, sizeof( msgAck ) );			
		}
		else
		{
			MSG_WORD2 msgNack;
			msgNack.Category	= MP_ITEM;
			msgNack.Protocol	= MP_ITEM_TITANUPGRADE_ADDITEM_NACK;
			msgNack.wData1		= pmsg->ItemInfo.Position;
			msgNack.wData2		= wErrorCode;
			pPlayer->SendMsg( &msgNack, sizeof( msgNack ) );		
		}
	}	
}

void CTitanItemManager::Titanupgrade( CPlayer* pPlayer, MSG_TITAN_UPGRADE_SYN* pmsg )
{
	// Áõ¼­ À¯¹« Ã¼Å©
	if(!CHKRT->ItemOf(pPlayer, pmsg->wBasicItemPos, pmsg->wBasicItemIdx, 0,0, CB_EXIST|CB_ICONIDX))
		return;

	// °¢ Àç·á À¯¹« Ã¼Å©
	for( int i = 0; i < pmsg->wMaterialNum; i++ )
	{
		if(!CHKRT->ItemOf(pPlayer, pmsg->Material[i].ItemPos, pmsg->Material[i].wItemIdx, pmsg->Material[i].Dur,0, CB_EXIST|CB_ICONIDX))
		{
			MSG_BYTE errorMsg;
			ZeroMemory(&errorMsg, sizeof(MSG_BYTE));
			SetProtocol(&errorMsg, MP_ITEM, MP_ITEM_TITANUPGRADE_NACK);
			errorMsg.bData = 1;
			pPlayer->SendMsg( &errorMsg, sizeof( errorMsg ) );
			return;
		}
	}

	TITAN_UPGRADEINFO* pTitanUpgrade = TITANITEMMGR->GetTitanUpgradeInfoItemInfo(pmsg->wBasicItemIdx);

	// Å¸ÀÌÅºÀÌ 3´Ü°è¸é ´õÀÌ»ó ¾÷±×·¹ÀÌµåÇÒÀÏÀÌ ¾øÀ½
	if(pTitanUpgrade->dwNextTitanIdx == 0)
	{
		MSG_BYTE errorMsg;
		ZeroMemory(&errorMsg, sizeof(MSG_BYTE));
		SetProtocol(&errorMsg, MP_ITEM, MP_ITEM_TITANUPGRADE_NACK);
		errorMsg.bData = 2;
		pPlayer->SendMsg( &errorMsg, sizeof( errorMsg ) );
		return;
	}

	// µ· ±ïÀ½
	if(pPlayer->GetMoney() < pTitanUpgrade->dwMoney)
	{
		MSG_BYTE errorMsg;
		ZeroMemory(&errorMsg, sizeof(MSG_BYTE));
		SetProtocol(&errorMsg, MP_ITEM, MP_ITEM_TITANUPGRADE_NACK);
		errorMsg.bData = 3;
		pPlayer->SendMsg( &errorMsg, sizeof( errorMsg ) );
		return;
	}
	pPlayer->SetMoney(pTitanUpgrade->dwMoney, MONEY_SUBTRACTION);

	ITEMBASE Item;
	memset( &Item, 0, sizeof(ITEMBASE) );
	CItemSlot* pSlot = pPlayer->GetSlot(eItemTable_Inventory);

	// magi1127
	// ÆÄÃ÷µé »èÁ¦
	for( int i = 0; i < pmsg->wMaterialNum; i++ )
	{
		Item = *pSlot->GetItemInfoAbs(pmsg->Material[i].ItemPos);
		if(EI_TRUE == ITEMMGR->DiscardItem(pPlayer, pmsg->Material[i].ItemPos, pmsg->Material[i].wItemIdx, pmsg->Material[i].Dur))
		{
			POSTYPE TargetPos = 0;
			WORD wItemIdx = 0;
			DURTYPE ItemNum = 0;

			MSG_ITEM_DISCARD_ACK msg;
			msg.TargetPos = pmsg->Material[i].ItemPos;
			msg.wItemIdx = pmsg->Material[i].wItemIdx;
			msg.ItemNum = pmsg->Material[i].Dur;
			msg.Category = MP_ITEM;
			msg.Protocol = MP_ITEM_TITANUPGRADE_ACK_DELETEMATERIAL;
			ITEMMGR->SendAckMsg(pPlayer, &msg, sizeof(msg));

			// Log
			LogItemMoney(pmsg->dwObjectID, pPlayer->GetObjectName(), 0, "",
				eLog_ItemDiscard, pPlayer->GetMoney(eItemTable_Inventory), 0, 0,
				Item.wIconIdx, Item.dwDBIdx,  Item.Position, 0,
				Item.Durability, pPlayer->GetPlayerExpPoint());
		}
	}

	// ¿ø·¡ ÀÖ´ø Å¸ÀÌÅº Áõ¼­´Â »èÁ¦
	Item = *pSlot->GetItemInfoAbs(pmsg->wBasicItemPos);
	if(EI_TRUE == ITEMMGR->DiscardItem(pPlayer, pmsg->wBasicItemPos, pmsg->wBasicItemIdx, 1))
	{
		MSG_ITEM_DISCARD_ACK msg;
		msg.TargetPos = pmsg->wBasicItemPos;
		msg.wItemIdx = pmsg->wBasicItemIdx;
		msg.ItemNum = 1;
		msg.Category = MP_ITEM;
		msg.Protocol = MP_ITEM_TITANUPGRADE_ACK_DELETEMATERIAL;
		ITEMMGR->SendAckMsg(pPlayer, &msg, sizeof(msg));

		// Log
		LogItemMoney(pmsg->dwObjectID, pPlayer->GetObjectName(), 0, "",
			eLog_ItemDiscard, pPlayer->GetMoney(eItemTable_Inventory), 0, 0,
			Item.wIconIdx, Item.dwDBIdx,  Item.Position, 0,
			Item.Durability, pPlayer->GetPlayerExpPoint());

		return;
	}

	// ¾÷±×·¹ÀÌµåµÈ Å¸ÀÌÅº Áõ¼­ »ý¼º
	WORD EmptyCellPos[255];
	WORD EmptyCellNum;
	WORD obtainItemNum = ITEMMGR->GetCanBuyNumInSpace( pPlayer, pSlot, (WORD)pTitanUpgrade->dwNextTitanIdx, 1, EmptyCellPos, EmptyCellNum );
	if(obtainItemNum == 0)
		return;

	ITEMMGR->ObtainItemEx( pPlayer, ITEMMGR->Alloc(pPlayer, MP_ITEM, MP_ITEM_TITANUPGRADE_ACK, pPlayer->GetID(), 0, eLog_ItemObtainTitan, obtainItemNum, (DBResult)(ITEMMGR->ObtainItemDBResult)), (WORD)pTitanUpgrade->dwNextTitanIdx, obtainItemNum, EmptyCellPos, EmptyCellNum, EmptyCellNum );
}

void CTitanItemManager::TitanBreak( CPlayer* pPlayer, MSG_DWORD2* pmsg )
{
	// Å¸ÀÌÅº ¾ÆÀÌÅÛ À¯¹« Ã¼Å©
	if(!CHKRT->ItemOf(pPlayer, (POSTYPE)pmsg->dwData2, (WORD)pmsg->dwData1, 0,0, CB_EXIST|CB_ICONIDX))
		return;

	TITAN_BREAKINFO* pTitanBreakInfo = TITANITEMMGR->GetTitanBreakInfoItemInfo(pmsg->dwData1);

	// µ· ±ïÀ½
	if(pPlayer->GetMoney() < pTitanBreakInfo->dwMoney)
	{
		MSG_BYTE errorMsg;
		ZeroMemory(&errorMsg, sizeof(MSG_BYTE));
		SetProtocol(&errorMsg, MP_ITEM, MP_ITEM_TITANBREAK_NACK);
		errorMsg.bData = 3;
		pPlayer->SendMsg( &errorMsg, sizeof( errorMsg ) );
		return;
	}
	pPlayer->SetMoney(pTitanBreakInfo->dwMoney, MONEY_SUBTRACTION);

	ITEMBASE Item;
	memset( &Item, 0, sizeof(ITEMBASE) );
	CItemSlot* pSlot = pPlayer->GetSlot(eItemTable_Inventory);

	// Å¸ÀÌÅº ¾ÆÀÌÅÛ »èÁ¦
	Item = *pSlot->GetItemInfoAbs((POSTYPE)pmsg->dwData2);
	if(EI_TRUE == ITEMMGR->DiscardItem(pPlayer, (POSTYPE)pmsg->dwData2, (WORD)pmsg->dwData1, 1))
	{
		MSG_ITEM_DISCARD_ACK msg;
		msg.TargetPos = (POSTYPE)pmsg->dwData2;
		msg.wItemIdx = (WORD)pmsg->dwData1;
		msg.ItemNum = 1;
		msg.Category = MP_ITEM;
		msg.Protocol = MP_ITEM_TITANBREAK_ACK_DELETEITEM;
		ITEMMGR->SendAckMsg(pPlayer, &msg, sizeof(msg));

		// Log	
		LogItemMoney(pmsg->dwObjectID, pPlayer->GetObjectName(), 0, "",
			eLog_ItemDiscard, pPlayer->GetMoney(eItemTable_Inventory), 0, 0,
			Item.wIconIdx, Item.dwDBIdx,  Item.Position, 0,
			Item.Durability, pPlayer->GetPlayerExpPoint());
	}


	// ºÐÇØµÇ´Â ¾ÆÀÌÅÛÀÇ Àç·á¸¦ È®·ü·Î »Ì¾Æ³¿
	int totalrate = m_TitanBreakRate;
	TITAN_BREAK_ACK sResult;
	BOOL flag[20] = { 0, };

	for( int i = 0; i < pTitanBreakInfo->wGetCnt; i++ )
	{
		int ran = random(0, totalrate);
		int dup = 0;

		for( int j = 0; j < pTitanBreakInfo->wTotalCnt; j++ )
		{
			if(!flag[j])
			{
				dup += pTitanBreakInfo->pTitanBreakMaterial[j].wRate;
				if(dup >= ran)
				{
					// ¼º°ø
					totalrate -= pTitanBreakInfo->pTitanBreakMaterial[j].wRate;
					flag[j] = TRUE;
					sResult.AddMaterial((WORD)pTitanBreakInfo->pTitanBreakMaterial[j].dwMaterialIdx, i, pTitanBreakInfo->pTitanBreakMaterial[j].wMaterCnt);
					break;
				}
			}
		}
	}

	// ºÐÇØµÈ Àç·á »ý¼º
	for( int i = 0; i < sResult.wMaterialNum; i++ )
	{
		WORD EmptyCellPos[255];
		WORD EmptyCellNum;
		pSlot = pPlayer->GetSlot(eItemTable_Inventory);
		WORD obtainItemNum = ITEMMGR->GetCanBuyNumInSpace( pPlayer, pSlot, (WORD)sResult.sMaterial[i].dwItemIdx, sResult.sMaterial[i].wMaterialDur, EmptyCellPos, EmptyCellNum );
		if(obtainItemNum == 0)
			return;

		ITEMMGR->ObtainItemEx( pPlayer, ITEMMGR->Alloc(pPlayer, MP_ITEM, MP_ITEM_TITANBREAK_ACK, pPlayer->GetID(), 0, eLog_ItemObtainTitan, obtainItemNum, (DBResult)(ITEMMGR->ObtainItemDBResult)), (WORD)sResult.sMaterial[i].dwItemIdx, obtainItemNum, EmptyCellPos, EmptyCellNum, EmptyCellNum );
	}	
}

void CTitanItemManager::TitanBreakAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg )
{
	WORD wErrorCode = 0;
	CItemSlot * pSlot = NULL;
	MSG_ITEM msgAck;

	// ÀÎº¥Åä¸® ¾ÆÀÌÅÛÀÌ ¾Æ´Ï¸é ¿¡·¯
	if( GetTableIdxPosition( pmsg->ItemInfo.Position ) != eItemTable_Inventory )
		wErrorCode = 1;

	// ºÐÇØ °¡´ÉÇÑ ¾ÆÀÌÅÛÀÎÁö Ã¼Å©
	TITAN_BREAKINFO* pTitanBreakInfo = TITANITEMMGR->GetTitanBreakInfoItemInfo(pmsg->ItemInfo.wIconIdx);
	if(!pTitanBreakInfo)
		wErrorCode = 5;

	// ¼­¹ö¿Í Å¬¶óÀÇ ¾ÆÀÌÅÛÁ¤º¸°¡ °°ÀºÁö Ã¼Å©
	if(!CHKRT->ItemOf(pPlayer, pmsg->ItemInfo.Position, pmsg->ItemInfo.wIconIdx, 0,0, CB_EXIST|CB_ICONIDX))
		wErrorCode = 2;

	pSlot = pPlayer->GetSlot(eItemTable_Inventory);
	if( pSlot )
	{
		// ÇØ´ç ½½·ÔÀÌ ÀÌ¹Ì Àá°ÜÀÖÀ¸¸é ¿¡·¯
		if( pSlot->IsLock( pmsg->ItemInfo.Position ) )
			wErrorCode = 3;

		// ¿É¼Ç¾ÆÀÌÅÛÀÎÁö Ã¼Å©
		if(ITEMMGR->IsOptionItem( pmsg->ItemInfo.wIconIdx, pmsg->ItemInfo.Durability ) )
			wErrorCode = 4;

		// Á¤»óÀÏ¶§ Ã³¸®
		if(wErrorCode == 0)
		{
			pSlot->SetLock( pmsg->ItemInfo.Position, TRUE );

			msgAck.Category		= MP_ITEM;
			msgAck.Protocol		= MP_ITEM_TITANBREAK_ADDITEM_ACK;
			msgAck.ItemInfo		= pmsg->ItemInfo;
			pPlayer->SendMsg( &msgAck, sizeof( msgAck ) );			
		}
		else
		{
			MSG_WORD2 msgNack;
			msgNack.Category	= MP_ITEM;
			msgNack.Protocol	= MP_ITEM_TITANBREAK_ADDITEM_NACK;
			msgNack.wData1		= pmsg->ItemInfo.Position;
			msgNack.wData2		= wErrorCode;
			pPlayer->SendMsg( &msgNack, sizeof( msgNack ) );		
		}
	}
}

// magi82 - Titan(070611) Å¸ÀÌÅº ¹«°øº¯È¯ ÁÖ¼®Ã³¸®
/*
void CTitanItemManager::TitanMugongChange( CPlayer* pPlayer, MSG_WORD3* pmsg )
{
	WORD wMugongIdx = pmsg->wData1;
	WORD wItemIdx = pmsg->wData2;
	WORD wItemPos = pmsg->wData3;

	CItemSlot * pSlot = pPlayer->GetSlot(wItemPos);
	const ITEMBASE * pItemBase = pSlot->GetItemInfoAbs(wItemPos);
	ITEMBASE Item = *pItemBase;
	ITEM_INFO * pItemInfo = ITEMMGR->GetItemInfo(wItemIdx);

	if(pItemInfo->ItemKind != eMUGONG_ITEM_TITAN)
		return;

	if(!MUGONGMNGR->AddTitanMugong(pPlayer, pmsg->wData1))
		return;

	if(EI_TRUE == ITEMMGR->DiscardItem(pPlayer, wItemPos, wItemIdx, 1))
	{
		MSG_DWORD msg;
		msg.dwData = wItemPos;
		msg.Category = MP_ITEM;
		msg.Protocol = MP_ITEM_TITAN_MUGONG_DELETEITEM;
		ITEMMGR->SendAckMsg(pPlayer, &msg, sizeof(msg));

		// Log
		LogItemMoney(pPlayer->GetID(), pPlayer->GetObjectName(), 0, "",
			eLog_ItemUse, pPlayer->GetMoney(eItemTable_Inventory), 0, 0,
			Item.wIconIdx, Item.dwDBIdx,  Item.Position, 0,
			Item.Durability, pPlayer->GetPlayerExpPoint());	
	}
}
*/

void CTitanItemManager::TitanRegisterAdditem( CPlayer* pPlayer, MSG_ITEM* pmsg )
{
	WORD wErrorCode = 0;
	CItemSlot * pSlot = NULL;
	MSG_ITEM msgAck;

	// ÀÎº¥Åä¸® ¾ÆÀÌÅÛÀÌ ¾Æ´Ï¸é ¿¡·¯
	if( GetTableIdxPosition( pmsg->ItemInfo.Position ) != eItemTable_Inventory )
		wErrorCode = 1;

	// ¼­¹ö¿Í Å¬¶óÀÇ ¾ÆÀÌÅÛÁ¤º¸°¡ °°ÀºÁö Ã¼Å©
	if(!CHKRT->ItemOf(pPlayer, pmsg->ItemInfo.Position, pmsg->ItemInfo.wIconIdx, 0,0, CB_EXIST|CB_ICONIDX))
		wErrorCode = 2;

	pSlot = pPlayer->GetSlot(eItemTable_Inventory);
	if( pSlot )
	{
		// ÇØ´ç ½½·ÔÀÌ ÀÌ¹Ì Àá°ÜÀÖÀ¸¸é ¿¡·¯
		if( pSlot->IsLock( pmsg->ItemInfo.Position ) )
			wErrorCode = 3;

		// ¿É¼Ç¾ÆÀÌÅÛÀÎÁö Ã¼Å©
		if(ITEMMGR->IsOptionItem( pmsg->ItemInfo.wIconIdx, pmsg->ItemInfo.Durability ) )
			wErrorCode = 4;

		// Á¤»óÀÏ¶§ Ã³¸®
		if(wErrorCode == 0)
		{
			pSlot->SetLock( pmsg->ItemInfo.Position, TRUE );

			msgAck.Category		= MP_ITEM;
			msgAck.Protocol		= MP_ITEM_TITAN_REGISTER_ADDITEM_ACK;
			msgAck.ItemInfo		= pmsg->ItemInfo;
			pPlayer->SendMsg( &msgAck, sizeof( msgAck ) );			
		}
		else
		{
			MSG_WORD2 msgNack;
			msgNack.Category	= MP_ITEM;
			msgNack.Protocol	= MP_ITEM_TITAN_REGISTER_ADDITEM_NACK;
			msgNack.wData1		= pmsg->ItemInfo.Position;
			msgNack.wData2		= wErrorCode;
			pPlayer->SendMsg( &msgNack, sizeof( msgNack ) );		
		}
	}
}

void CTitanItemManager::TitanDissolutionAdditem( CPlayer* pPlayer, MSG_ITEM* pmsg )
{
	WORD wErrorCode = 0;
	CItemSlot * pSlot = NULL;
	MSG_ITEM msgAck;

	// ÀÎº¥Åä¸® ¾ÆÀÌÅÛÀÌ ¾Æ´Ï¸é ¿¡·¯
	if( GetTableIdxPosition( pmsg->ItemInfo.Position ) != eItemTable_Inventory )
		wErrorCode = 1;

	// ¼­¹ö¿Í Å¬¶óÀÇ ¾ÆÀÌÅÛÁ¤º¸°¡ °°ÀºÁö Ã¼Å©
	if(!CHKRT->ItemOf(pPlayer, pmsg->ItemInfo.Position, pmsg->ItemInfo.wIconIdx, 0,0, CB_EXIST|CB_ICONIDX))
		wErrorCode = 2;

	pSlot = pPlayer->GetSlot(eItemTable_Inventory);
	if( pSlot )
	{
		// ÇØ´ç ½½·ÔÀÌ ÀÌ¹Ì Àá°ÜÀÖÀ¸¸é ¿¡·¯
		if( pSlot->IsLock( pmsg->ItemInfo.Position ) )
			wErrorCode = 3;

		// ¿É¼Ç¾ÆÀÌÅÛÀÎÁö Ã¼Å©
		if(ITEMMGR->IsOptionItem( pmsg->ItemInfo.wIconIdx, pmsg->ItemInfo.Durability ) )
			wErrorCode = 4;

		// Á¤»óÀÏ¶§ Ã³¸®
		if(wErrorCode == 0)
		{
			pSlot->SetLock( pmsg->ItemInfo.Position, TRUE );

			msgAck.Category		= MP_ITEM;
			msgAck.Protocol		= MP_ITEM_TITAN_DISSOLUTION_ADDITEM_ACK;
			msgAck.ItemInfo		= pmsg->ItemInfo;
			pPlayer->SendMsg( &msgAck, sizeof( msgAck ) );			
		}
		else
		{
			MSG_WORD2 msgNack;
			msgNack.Category	= MP_ITEM;
			msgNack.Protocol	= MP_ITEM_TITAN_DISSOLUTION_ADDITEM_NACK;
			msgNack.wData1		= pmsg->ItemInfo.Position;
			msgNack.wData2		= wErrorCode;
			pPlayer->SendMsg( &msgNack, sizeof( msgNack ) );		
		}
	}
}

void CTitanItemManager::TitanPartsMakeAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg )
{
	WORD wErrorCode = 0;
	ITEM_MIX_INFO* pMixItemInfo = NULL;
	CItemSlot * pSlot = NULL;
	MSG_ITEM msgAck;

	//AI¨¬¡ÍAa¢ç A¡¿AI¨¬iAC 'A®AIAo
	if( GetTableIdxPosition( pmsg->ItemInfo.Position ) != eItemTable_Inventory )
	{
		wErrorCode = 1;
		goto go_TPM_ADDITEM_NACK;				
	}			
	//d¨¬¡Æ ¡©öo¯I AIA®CI¢¥AAo
	if(!CHKRT->ItemOf(pPlayer, pmsg->ItemInfo.Position, pmsg->ItemInfo.wIconIdx, 0,0, CB_EXIST|CB_ICONIDX))
	{
		wErrorCode = 2;
		goto go_TPM_ADDITEM_NACK;
	}
	//¢ÒoAI ¡ÆE¡¤A AO¢¥AAo
	pSlot = pPlayer->GetSlot(eItemTable_Inventory);
	if( pSlot )
	{
		if( pSlot->IsLock( pmsg->ItemInfo.Position ) )
			wErrorCode = 3;
			goto go_TPM_ADDITEM_NACK;
		}
		// ¯EùC ¨¡AIAU úE¥ìE
		if(ITEMMGR->IsOptionItem( pmsg->ItemInfo.wIconIdx, pmsg->ItemInfo.Durability ) )
		{
			wErrorCode = 4;
			goto go_TPM_ADDITEM_NACK;
		}

		//SW050920 Rare
		//			if( ITEMMGR->IsRareOptionItem( pmsg->ItemInfo.wIconIdx, pmsg->ItemInfo.RareIdx ) )
		//			{
		//				wErrorCode = 5;
		//				goto go_TPM_ADDITEM_NACK;
		//			}

		// view list
		pMixItemInfo = TITANITEMMGR->GetTitanMixItemInfo( pmsg->ItemInfo.wIconIdx );
		if(!pMixItemInfo)
		{
			wErrorCode = 5;
			goto go_TPM_ADDITEM_NACK;
		}

		//¿µ¾à¾ÆÀÌÅÛÀÌ ¾Æ´Ñ Àç·á¾ÆÀÌÅÛÀÏ°æ¿ì ÇÏ³ª¹Û¿¡ ¾È¿Ã¶ó°£´Ù.
		if( GetItemKind(pmsg->ItemInfo.wIconIdx) != eYOUNGYAK_ITEM &&
			GetItemKind(pmsg->ItemInfo.wIconIdx) != eEXTRA_ITEM_JEWEL &&
			pmsg->ItemInfo.Durability > 1 )
		{
			wErrorCode = 6;
			goto go_TPM_ADDITEM_NACK;
		}

		//////////Ae¡ÆuOK
		pSlot->SetLock( pmsg->ItemInfo.Position, TRUE );//¢Òo; ¡ÆC¢¥U.

		msgAck.Category		= MP_ITEM;
		msgAck.Protocol		= MP_ITEM_TPM_ADDITEM_ACK;
		msgAck.ItemInfo		= pmsg->ItemInfo;
		pPlayer->SendMsg( &msgAck, sizeof( msgAck ) );			
		return;

go_TPM_ADDITEM_NACK:
		MSG_WORD2 msgNack;
		msgNack.Category	= MP_ITEM;
		msgNack.Protocol	= MP_ITEM_TPM_ADDITEM_NACK;
		msgNack.wData1		= pmsg->ItemInfo.Position;
		msgNack.wData2		= wErrorCode;
		pPlayer->SendMsg( &msgNack, sizeof( msgNack ) );		
}

void CTitanItemManager::TitanPartsMake( CPlayer* pPlayer, MSG_ITEM_MIX_SYN* pmsg )
{
	int rt = 0;
	if(EI_TRUE == (rt = ITEMMGR->MixItem(pPlayer, pmsg->wBasicItemIdx, pmsg->BasicItemPos, pmsg->ResultIndex, pmsg->Material, pmsg->wMaterialNum, pmsg->ShopItemIdx, pmsg->ShopItemPos, TRUE)))
	{
		MSG_ITEM_MIX_ACK msg;
		memcpy(&msg, pmsg, sizeof(MSG_ITEM_MIX_SYN));
		msg.Protocol = MP_ITEM_TPM_SUCCESS_ACK;
		ITEMMGR->SendAckMsg(pPlayer, &msg, msg.GetSize());
	}
	else
	{
		switch(rt)
		{
		case 1000:
			{
				// ¡Ë¡Íe¡§oC¡§¡§¡Ë
				MSG_ITEM_MIX_ACK msg;
				memcpy(&msg, pmsg, sizeof(MSG_ITEM_MIX_SYN));
				msg.Protocol = MP_ITEM_TPM_BIGFAILED_ACK;
				ITEMMGR->SendAckMsg(pPlayer, &msg, msg.GetSize());
			}
			break;
		case 1001:
			{
				// ¡§oC¡§¡§¡Ë
				MSG_ITEM_MIX_ACK msg;
				memcpy(&msg, pmsg, sizeof(MSG_ITEM_MIX_SYN));
				msg.Protocol = MP_ITEM_TPM_FAILED_ACK;
				ITEMMGR->SendAckMsg(pPlayer, &msg, msg.GetSize());
			}
			break;
		case 20:
		case 21:
		case 22:
		case 23:
			{
				MSG_DWORD2 msg;
				msg.Category = MP_ITEM;
				msg.Protocol = MP_ITEM_TPM_MSG;
				msg.dwObjectID = pPlayer->GetID();
				msg.dwData1 = rt;
				msg.dwData2 = pmsg->BasicItemPos;
				pPlayer->SendMsg( &msg, sizeof(msg) );
			}
			break;
		default:
			{
				if( rt != 2 )	//2¹ø ASSERT°¡ Á¦ÀÏ ¸¹´Ù. DB´À·Á ¼­¹ö¿Í Å¬¶óÀÌ¾ðÆ®ÀÇ ¾ÆÀÌÅÛÁ¤º¸°¡ ²¿ÀÎ´Ù.
				{
					char temp[256];
					sprintf(temp,"¾ÆÀÌÅÛ Á¶ÇÕ ¿¡·¯ %d",rt);
					ASSERTMSG(0, temp);
				}
				MSG_ITEM_ERROR msg;
				msg.Category = MP_ITEM;
				msg.Protocol = MP_ITEM_ERROR_NACK;
				msg.dwObjectID = pPlayer->GetID();
				msg.ECode = eItemUseErr_TitanPartsMake;
				ITEMMGR->SendErrorMsg(pPlayer, &msg, sizeof(msg), rt);
			}
			break;
		}				
	}
}

void CTitanItemManager::TitanRegister( CPlayer* pPlayer, MSG_DWORD2* pmsg )
{
	if(pPlayer->GetTitanManager()->GetCurRidingTitan() != NULL)
	{
		return;
	}

	DWORD dwIdx = pmsg->dwData1;
	DWORD dwPos = pmsg->dwData2;

	CItemSlot * pSlot = pPlayer->GetSlot( (POSTYPE)dwPos );

	if(!CHKRT->ItemOf(pPlayer, (POSTYPE)dwPos, (WORD)dwIdx,0,0,CB_EXIST|CB_ICONIDX))
		return;

	ITEMBASE* pItem = (ITEMBASE*)pSlot->GetItemInfoAbs((POSTYPE)dwPos);

	if(!pItem)
	{
		ASSERT(0);
		return;
	}

	if( TRUE == pPlayer->GetTitanManager()->RegistTitan(pItem->dwDBIdx) )
	{	// ACK

		// Å¸ÀÌÅº Áõ¼­ ÆÄ¶ó¸ÞÅÍ¸¦ DB¿¡ °»½Å
		pItem->ItemParam |= ITEM_PARAM_TITAN_REGIST;
		ShopItemParamUpdateToDB(pPlayer->GetID(), pItem->dwDBIdx, pItem->ItemParam);

		MSG_DWORD2 msg;
		SetProtocol( &msg, MP_ITEM, MP_ITEM_TITAN_REGISTER_ACK );
		msg.dwObjectID = pPlayer->GetID();
		TITAN_TOTALINFO* pInfo = pPlayer->GetTitanManager()->GetTitanTotalInfo(pItem->dwDBIdx);
		if(!pInfo)
		{
			ASSERT(0);
			return;
		}
		msg.dwData1 = pInfo->RegisterCharacterIdx;
		msg.dwData2 = dwPos;
		pPlayer->SendMsg(&msg,sizeof(msg));
	}
	else
	{	// NACK
		MSG_DWORD2 msg;
		SetProtocol( &msg, MP_ITEM, MP_ITEM_TITAN_REGISTER_NACK );
		msg.dwObjectID = pPlayer->GetID();
		TITAN_TOTALINFO* pInfo = pPlayer->GetTitanManager()->GetTitanTotalInfo(pItem->dwDBIdx);
		if(!pInfo)
		{
			ASSERT(0);
			return;
		}
		msg.dwData1 = pInfo->RegisterCharacterIdx;
		msg.dwData2 = dwPos;
		pPlayer->SendMsg(&msg,sizeof(msg));
	}
}

void CTitanItemManager::TitanCancellation( CPlayer* pPlayer, MSG_DWORD2* pmsg )
{
	if(pPlayer->GetTitanManager()->GetCurRidingTitan() != NULL)
	{
		return;
	}

	DWORD dwIdx = pmsg->dwData1;
	DWORD dwPos = pmsg->dwData2;

	CItemSlot * pSlot = pPlayer->GetSlot((POSTYPE)dwPos);

	if(!CHKRT->ItemOf(pPlayer, (POSTYPE)dwPos, (WORD)dwIdx,0,0,CB_EXIST|CB_ICONIDX))
		return;

	ITEMBASE* pItem = (ITEMBASE*)pSlot->GetItemInfoAbs((POSTYPE)dwPos);

	if(!pItem)
	{
		ASSERT(0);
		return;
	}

	if( TRUE == pPlayer->GetTitanManager()->CancleTitanRegister(pItem->dwDBIdx) )
	{	// ACK

		// Å¸ÀÌÅº Áõ¼­ ÆÄ¶ó¸ÞÅÍ¸¦ DB¿¡ °»½Å
		pItem->ItemParam ^= ITEM_PARAM_TITAN_REGIST;			
		ShopItemParamUpdateToDB(pPlayer->GetID(), pItem->dwDBIdx, pItem->ItemParam);

		MSG_DWORD2 msg;
		SetProtocol( &msg, MP_ITEM, MP_ITEM_TITAN_DISSOLUTION_ACK );
		msg.dwObjectID = pPlayer->GetID();
		TITAN_TOTALINFO* pInfo = pPlayer->GetTitanManager()->GetTitanTotalInfo(pItem->dwDBIdx);
		if(!pInfo)
		{
			ASSERT(0);
			return;
		}
		msg.dwData1 = pInfo->RegisterCharacterIdx;
		msg.dwData2 = dwPos;
		pPlayer->SendMsg(&msg,sizeof(msg));
	}
	else
	{	// NACK
		MSG_DWORD2 msg;
		SetProtocol( &msg, MP_ITEM, MP_ITEM_TITAN_DISSOLUTION_NACK );
		msg.dwObjectID = pPlayer->GetID();
		TITAN_TOTALINFO* pInfo = pPlayer->GetTitanManager()->GetTitanTotalInfo(pItem->dwDBIdx);
		if(!pInfo)
		{
			ASSERT(0);
			return;
		}
		msg.dwData1 = pInfo->RegisterCharacterIdx;
		msg.dwData2 = dwPos;
		pPlayer->SendMsg(&msg,sizeof(msg));
	}

}