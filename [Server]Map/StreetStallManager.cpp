#include "stdafx.h"
#include "ServerSystem.h"
#include "StreetStall.h"
#include "StreetStallManager.h"
#include "usertable.h"
#include "PackedData.h"
#include "ObjectStateManager.h"
#include "mapdbmsgparser.h"
#include "Player.h"
#include "itemmanager.h"
#include "CheckRoutine.h"
#include "..\[CC]Header\GameResourceStruct.h"
#include "..\[CC]Header\GameResourceManager.h"
#include "Battle.h"

GLOBALTON(cStreetStallManager)

cStreetStallManager::cStreetStallManager()
{
	m_mpStreetStall = new CMemoryPoolTempl<cStreetStall>;
	m_mpStreetStall->Init( 100, 50, "StreetStallPool" );

	m_StallTable.Initialize(128);

	// magi82  ˻
    InitStallData();
}

cStreetStallManager::~cStreetStallManager()
{
	cStreetStall* pStall;
	m_StallTable.SetPositionHead();
	while(pStall = m_StallTable.GetData())
	{
		m_mpStreetStall->Free(pStall);
	}
	m_StallTable.RemoveAll();

	SAFE_DELETE(m_mpStreetStall);

	// magi82  ˻
	ReleaseStallData();
}

cStreetStall* cStreetStallManager::CreateStreetStall( CPlayer* pOwner, WORD StallKind, char* title )
{
	//  ÷̾   ϴ Ȯؾϳ ??
	ASSERT( !m_StallTable.GetData(pOwner->GetID()) );

	cStreetStall* pStall = m_mpStreetStall->Alloc();
	pStall->Init();

	ASSERT( pStall );
	if( pStall == NULL )
		return NULL;

	pStall->SetOwner( pOwner );
	pStall->SetStallKind( StallKind );
	pOwner->SetStreetStallTitle( title );
	
	m_StallTable.Add(pStall,pOwner->GetID());

	return pStall;
}

void cStreetStallManager::DeleteStreetStall( CPlayer* pOwner )
{
	ASSERT(m_StallTable.GetData(pOwner->GetID()));
	cStreetStall* pStall = m_StallTable.GetData(pOwner->GetID());
	
	if( pStall )
	{
		// magi82  ˻
		if( pStall->GetStallKind() == eSK_SELL )
            ResetStallData( pOwner->GetID(), eFind_Sell);
		else if( pStall->GetStallKind() == eSK_BUY )
			ResetStallData( pOwner->GetID(), eFind_Buy);

		pStall->EmptyCellAll();
		pStall->DeleteGuestAll();
		m_mpStreetStall->Free( pStall );
		m_StallTable.Remove(pOwner->GetID());
	}
}


BOOL cStreetStallManager::IsExist( cStreetStall* pStall )
{
	if( m_StallTable.GetData(pStall->GetOwner()->GetID()) == NULL ) 
		return FALSE;

	return TRUE;
}
/*
BOOL cStreetStallManager::GuestIn( CPlayer* pOwner, CPlayer* pGuest )
{
	cStreetStall* pStall = m_StallTable.GetData(pOwner->GetID());
	if( pStall )
	{
		pStall->AddGuest( pGuest );
		return TRUE;
	}

	return FALSE;
}

void cStreetStallManager::GuestOut( DWORD OwnerId, CPlayer* pGuest )
{
	CPlayer* pOwner = (CPlayer*)g_pUserTable->FindUser( OwnerId ); // ʰ ִ  .
	if( pOwner == NULL ) return;

	cStreetStall* pStall = m_StallTable.GetData(OwnerId);
	if( pStall )
		pStall->DeleteGuest( pGuest );
}
*/
cStreetStall* cStreetStallManager::FindStreetStall( CPlayer* pOwner )
{
	return m_StallTable.GetData(pOwner->GetID());
}


void cStreetStallManager::StreetStallMode( CPlayer* pPlayer, WORD StallKind, char* title, BOOL bMode )
{
	MSG_STREETSTALL_TITLE msg;
	msg.Category = MP_STREETSTALL;
	msg.dwObjectID = pPlayer->GetID();

	if(bMode ==  TRUE)
	{
		OBJECTSTATEMGR_OBJ->StartObjectState(pPlayer, eObjectState_StreetStall_Owner, 0);
		msg.Protocol = MP_STREETSTALL_START;
		//strcpy( msg.Title, title );
		msg.StallKind = StallKind;
		SafeStrCpy( msg.Title, title, MAX_STREETSTALL_TITLELEN+1 );
	}
	else
	{
		OBJECTSTATEMGR_OBJ->EndObjectState(pPlayer, eObjectState_StreetStall_Owner, 0);
		msg.Protocol = MP_STREETSTALL_END;
		msg.StallKind = StallKind;
	}

	PACKEDDATA_OBJ->QuickSendExceptObjectSelf(pPlayer,&msg,sizeof(msg));
}

void cStreetStallManager::UserLogOut( CPlayer* pPlayer )
{
	cStreetStall* pGestStall = pPlayer->GetGuestStall();
	cStreetStall* pOwnerStall = FindStreetStall( pPlayer );

//find
//m_StallTable.GetData(pOwner->GetID());
//m_StallTable.GetData(pStall->GetOwner()->GetID()

	if( pGestStall ) //  մ  ...
	{
		if( IsExist( pGestStall ) )
			pGestStall->DeleteGuest( pPlayer ); // մ ִ  ´.

		MSGBASE msg;
		msg.Category = MP_STREETSTALL;
		msg.Protocol = MP_STREETSTALL_CLOSE;
		msg.dwObjectID = pPlayer->GetID();
		pPlayer->SendMsg(&msg, sizeof(msg));
		return;
	}
	
	if( pOwnerStall ) //   ִ ...
	{
		// Ḧ ˸.(ڱڽ ºȭ)
		MSG_STREETSTALL_TITLE msg;
		msg.Category = MP_STREETSTALL;
		msg.Protocol = MP_STREETSTALL_END;
		msg.StallKind = pOwnerStall->GetStallKind();
		msg.dwObjectID = pPlayer->GetID();
		pPlayer->SendMsg(&msg, sizeof(msg));

		// մԵ鿡  Ǿ ˸.
		MSGBASE CloseSyn;
		CloseSyn.Category = MP_STREETSTALL;
		CloseSyn.Protocol = MP_STREETSTALL_CLOSE;
		CloseSyn.dwObjectID = pPlayer->GetID();
		pOwnerStall->SendMsgGuestAll( &CloseSyn, sizeof(MSGBASE), TRUE ); 
		StreetStallMode( pPlayer, pOwnerStall->GetStallKind(), NULL, FALSE );

		DeleteStreetStall(pPlayer);

		return;
	}
}

BOOL cStreetStallManager::BuyItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pBuyInfo )
{
	POSTYPE pos = pBuyInfo->StallPos;		//  ġ
	sCELLINFO* pItemInfo = pStall->GetCellInfo( pos );
	ITEMBASE ItemBase = pItemInfo->sItemBase;
	POSTYPE RealPos = ItemBase.Position;	//   
	MONEYTYPE money = pItemInfo->dwMoney;

	// RaMa - ShopItemInven
	CItemSlot* pGuestInventory;
	CItemSlot* pOwnerInventory;

	ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( pItemInfo->sItemBase.wIconIdx );
	if( !pItemKindInfo )	return FALSE;

	eITEMTABLE Table = eItemTable_Inventory;
	if( pItemKindInfo->ItemKind & eSHOP_ITEM )
	{
		pGuestInventory	= (CShopInvenSlot*)pGuest->GetSlot( eItemTable_ShopInven );
		pOwnerInventory = (CShopInvenSlot*)pOwner->GetSlot( eItemTable_ShopInven );
		Table = eItemTable_ShopInven;
	}
	else
	{
		pGuestInventory	= (CInventorySlot*)pGuest->GetSlot( eItemTable_Inventory );
		pOwnerInventory = (CInventorySlot*)pOwner->GetSlot( eItemTable_Inventory );
	}

	//
	
	MSG_WORD		msgNack; //  ޼
	MSG_LINKITEM	DelSyn; // 󿡼  ޼
	MSG_SELLITEM	sellMsg; // ο  ޼
	//MSG_ITEMEX		buyMsg; // մԿ  ޼
	//SW070308 ۺΰɼǵ 
	MSG_LINKITEMOPTIONS	buyMsg;
	MSG_STALLMSG	stallMsg;
	
	POSTYPE absPosOut;
	WORD EmptyCellPos[1];
	const ITEMBASE* pInvenItem;

	if(CanBuyItem(pOwner, pGuest, pItemInfo, EmptyCellPos, pBuyInfo, &ItemBase, 1) == FALSE)
		goto ITEMBUY_FAILED;

	absPosOut = EmptyCellPos[0]; //  ä  

	pInvenItem = pOwnerInventory->GetItemInfoAbs(pItemInfo->sItemBase.Position);
	
	if(!CHKRT->ItemOf(pOwner, pInvenItem->Position, pInvenItem->wIconIdx,0,0,CB_EXIST|CB_ICONIDX))
	{
		goto ITEMBUY_FAILED;
	}

	if(!CHKRT->ItemOf(pGuest, absPosOut,0,0,0,CB_ICONIDX))
	{
		goto ITEMBUY_FAILED;
	}	

	ITEMBASE temp;

	if(pOwnerInventory->DeleteItemAbs(pOwner, RealPos, &temp, SS_LOCKOMIT) != EI_TRUE)
		goto ITEMBUY_FAILED;
	
	temp.Position = absPosOut;
	if(pGuestInventory->InsertItemAbs(pGuest, absPosOut, &temp, SS_LOCKOMIT) !=  EI_TRUE)
	{	
		//rollback
		temp.Position = RealPos;
		if(pOwnerInventory->InsertItemAbs(pOwner, RealPos, &temp) != EI_TRUE)
		{
			ASSERT(0);
			goto ITEMBUY_FAILED;
		}
		goto ITEMBUY_FAILED;
	}
	// ɼ  ߰  
	if( ITEMMGR->IsOptionItem(ItemBase.wIconIdx, ItemBase.Durability ) )
	{
		pGuest->AddItemOption(pOwner->GetItemOption(ItemBase.Durability));
		if( FALSE == pOwner->RemoveItemOption(ItemBase.Durability) )
		{
			char szBuf[128];
			sprintf( szBuf, "StallOptItem_Error! PlayerID : %d\t ItemDBIdx : %d\n", pOwner->GetID()
				,ItemBase.wIconIdx );
			ASSERTMSG(0, szBuf);
		}
	}
	//SW050920 Rare
	if( ITEMMGR->IsRareOptionItem(ItemBase.wIconIdx, ItemBase.RareIdx) )
	{
		pGuest->AddItemRareOption(pOwner->GetItemRareOption(ItemBase.RareIdx));
		if( FALSE == pOwner->RemoveItemRareOption(ItemBase.RareIdx) )
		{
			char szBuf[128];
			sprintf( szBuf, "StallRareItem_Error! PlayerID : %d\t ItemDBIdx : %d\n", pOwner->GetID()
				,ItemBase.wIconIdx );
			ASSERTMSG(0, szBuf);
		}
	}
	//SW051129 Pet
	if( ITEMMGR->IsPetSummonItem(ItemBase.wIconIdx) )
	{
		BOOL bReduceFriendly = !(ItemBase.ItemParam & ITEM_PARAM_SEAL);	//λ¸ FALSE
		pOwner->GetPetManager()->ExchangePet(ItemBase.dwDBIdx, pGuest, bReduceFriendly);
	}

	//SW070127 Ÿź
	if( ITEMMGR->IsTitanCallItem(ItemBase.wIconIdx) )
	{
		pOwner->GetTitanManager()->ExchangeTitan(ItemBase.dwDBIdx, pGuest);
	}

	if( ITEMMGR->IsTitanEquipItem(ItemBase.wIconIdx) )
	{
		pOwner->GetTitanManager()->ExchangeTitanEquipItem(ItemBase.dwDBIdx, pGuest);
	}

	// RaMa - ShopItem
	pOwner->SetMoney( pItemInfo->dwMoney, MONEY_ADDITION, 0, eItemTable_Inventory, eMoneyLog_GetStreetStall, pGuest->GetID());
	pGuest->SetMoney( pItemInfo->dwMoney, MONEY_SUBTRACTION, 0, eItemTable_Inventory, eMoneyLog_LoseStreetStall, pOwner->GetID());

	ItemUpdateToDB( pGuest->GetID(), ItemBase.dwDBIdx ,ItemBase.wIconIdx, ItemBase.Durability, absPosOut, 0, ItemBase.RareIdx );

	// Log 
	LogItemMoney(pOwner->GetID(), pOwner->GetObjectName(), pGuest->GetID(), pGuest->GetObjectName(),
		eLog_StreetStallBuyAll, pOwner->GetMoney(), pGuest->GetMoney(), pItemInfo->dwMoney,
		temp.wIconIdx, temp.dwDBIdx, 0, temp.Position, temp.Durability, pOwner->GetPlayerExpPoint());

	// 󿡼 Ѵ.
	pStall->EmptyCell( &ItemBase, Table );

	// magi82  ˻
	DeleteStallData( pOwner->GetID(), ItemBase.wIconIdx, eFind_Sell );
	
	DelSyn.Category = MP_STREETSTALL;
	DelSyn.Protocol = MP_STREETSTALL_DELETEITEM;
	DelSyn.dwObjectID = pBuyInfo->dwObjectID;
	DelSyn.ItemInfo = ItemBase;
	DelSyn.wAbsPosition = pos;

	pStall->SendMsgGuestAll( &DelSyn, sizeof(DelSyn) );

	//////////////////////////////////////////////////////
	// οԼ    ־ ش./////////////////
	sellMsg.Category = MP_STREETSTALL;
	sellMsg.Protocol = MP_STREETSTALL_SELLITEM;
	sellMsg.dwObjectID = pBuyInfo->StallOwnerID;
	sellMsg.ItemInfo = ItemBase;
	sellMsg.count = 0;
	sellMsg.dwData = pBuyInfo->dwObjectID;

	pOwner->SendMsg( &sellMsg, sizeof(sellMsg) );
	/////////////////////////////////////////////////////////////

	ItemBase.Position = absPosOut;
	
	// մԿ  ÷ϰ  .////////////////////
	buyMsg.Category = MP_STREETSTALL;
	buyMsg.Protocol = MP_STREETSTALL_BUYITEM_ACK;
	buyMsg.dwObjectID = pBuyInfo->dwObjectID;
	buyMsg.ItemInfo = ItemBase;

	/*
	buyMsg.count = 0;

	if( ITEMMGR->IsOptionItem(ItemBase.wIconIdx, ItemBase.Durability) )
	{
		buyMsg.sOptionInfo = *(pGuest->GetItemOption(ItemBase.Durability));
		buyMsg.count = 1;
	}
	//SW050920 Rare
	if( ITEMMGR->IsRareOptionItem(ItemBase.wIconIdx, ItemBase.RareIdx) )
	{
		buyMsg.sRareOptionInfo = *(pGuest->GetItemRareOption(ItemBase.RareIdx));
		buyMsg.RareCount = 1;
	}
	//SW051129 Pet
	if( ITEMMGR->IsPetSummonItem(ItemBase.wIconIdx) )
	{
		buyMsg.sPetInfo = *(pGuest->GetPetManager()->GetPetTotalInfo(ItemBase.dwDBIdx));
		buyMsg.PetInfoCount = 1;
	}*/
	//SW070308 ۺΰɼǵ 
	ITEMMGR->SetItemOptionsInfoMsg(pGuest, &ItemBase, (MSG_LINKITEMOPTIONS*)&buyMsg);

	pGuest->SendMsg( &buyMsg, buyMsg.GetSize() );
	/////////////////////////////////////////////////////////////

	stallMsg.Category = MP_STREETSTALL;
	stallMsg.Protocol = MP_STREETSTALL_MESSAGE;
	stallMsg.dwObjectID = pOwner->GetID();
	stallMsg.SellerID = pOwner->GetID();
	stallMsg.BuyerID = pGuest->GetID();
	stallMsg.ItemIdx = ItemBase.wIconIdx;
	stallMsg.count = 1;
	stallMsg.money = money;
	
	pOwner->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );
	pGuest->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );
	
	return TRUE;

ITEMBUY_FAILED:
	msgNack.Category	= MP_STREETSTALL;
	msgNack.Protocol	= MP_STREETSTALL_BUYITEM_NACK;
	msgNack.dwObjectID	= pBuyInfo->dwObjectID;
	msgNack.wData		= 0;
	pGuest->SendMsg( &msgNack, sizeof(msgNack) );

	return FALSE;
}

BOOL cStreetStallManager::BuyDupItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pBuyInfo )
{
	POSTYPE pos = pBuyInfo->StallPos;	// â ġ
	sCELLINFO* pItemInfo = pStall->GetCellInfo( pos );
	ITEMBASE ItemBase = pItemInfo->sItemBase;
	POSTYPE RealPos = ItemBase.Position; //   
	DURTYPE wBuyNum = pBuyInfo->ItemInfo.Durability;
	MONEYTYPE money = pItemInfo->dwMoney*wBuyNum;
	
	// RaMa - ShopItemInven
	CItemSlot* pGuestInventory;
	CItemSlot* pOwnerInventory;

	ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( pItemInfo->sItemBase.wIconIdx );
	if( !pItemKindInfo )	return FALSE;

	eITEMTABLE Table = eItemTable_Inventory;
	if( pItemKindInfo->ItemKind &eSHOP_ITEM )
	{
		pGuestInventory	= (CShopInvenSlot*)pGuest->GetSlot( eItemTable_ShopInven );
		pOwnerInventory = (CShopInvenSlot*)pOwner->GetSlot( eItemTable_ShopInven );
		Table = eItemTable_ShopInven;
	}
	else
	{
		pGuestInventory	= (CInventorySlot*)pGuest->GetSlot( eItemTable_Inventory );
		pOwnerInventory = (CInventorySlot*)pOwner->GetSlot( eItemTable_Inventory );
	}
	
	WORD rt = NOT_ERROR;
	POSTYPE absPosOut;
	WORD EmptyCellPos[1];
	MSG_WORD msgNack; //  ޼
	MSG_LINKITEM DelSyn; // 󿡼  ޼
	MSG_SELLITEM sellMsg; // ο  ޼
	MSG_ITEMEX buyMsg; // մԿ  ޼
	MSG_SELLITEM updateMsg; // ġ   쿡  ó 
	MSG_STALLMSG stallMsg;
	const ITEMBASE* pInvenItem;

	if( ItemBase.Durability < wBuyNum )
	{
		rt = NOT_EXIST;
		goto ITEMBUY_FAILED;
	}
	if(CanBuyItem(pOwner, pGuest, pItemInfo, EmptyCellPos, pBuyInfo, &ItemBase, wBuyNum) == FALSE)
		goto ITEMBUY_FAILED;

	absPosOut =  EmptyCellPos[0];

	pInvenItem = pOwnerInventory->GetItemInfoAbs(pItemInfo->sItemBase.Position);
	
	if(!CHKRT->ItemOf(pOwner, pInvenItem->Position, pInvenItem->wIconIdx,0,0,CB_EXIST|CB_ICONIDX))
	{
		goto ITEMBUY_FAILED;
	}
	if(!CHKRT->ItemOf(pGuest, absPosOut,0,0,0,CB_ICONIDX))
	{
		goto ITEMBUY_FAILED;
	}
	
	if( ItemBase.Durability == wBuyNum )
	{
		ITEMBASE temp;
		if(pOwnerInventory->DeleteItemAbs(pOwner, RealPos, &temp, SS_LOCKOMIT) != EI_TRUE)
			goto ITEMBUY_FAILED;
		temp.Position = absPosOut;
		if(pGuestInventory->InsertItemAbs(pGuest, absPosOut, &temp, SS_LOCKOMIT) !=  EI_TRUE)
		{
			//rollback
			temp.Position = RealPos;
			if(pOwnerInventory->InsertItemAbs(pOwner, RealPos, &temp) != EI_TRUE)
			{
				ASSERT(0);
				goto ITEMBUY_FAILED;
			}
			goto ITEMBUY_FAILED;
		}

		ItemUpdateToDB( pGuest->GetID(), temp.dwDBIdx ,temp.wIconIdx, temp.Durability, absPosOut, 0 );
		
		LogItemMoney(pOwner->GetID(), pOwner->GetObjectName(), pGuest->GetID(), pGuest->GetObjectName(),
			eLog_StreetStallBuyAll, pOwner->GetMoney(), pGuest->GetMoney(), money,
			temp.wIconIdx, temp.dwDBIdx, temp.Position, absPosOut, temp.Durability, pOwner->GetPlayerExpPoint());
		// 󿡼 Ѵ.
		pStall->EmptyCell( &ItemBase, Table );

		// magi82  ˻
		DeleteStallData( pOwner->GetID(), ItemBase.wIconIdx, eFind_Sell );
		
		DelSyn.Category = MP_STREETSTALL;
		DelSyn.Protocol = MP_STREETSTALL_DELETEITEM;
		DelSyn.dwObjectID = pBuyInfo->dwObjectID;
		DelSyn.ItemInfo = ItemBase;
		DelSyn.wAbsPosition = pos;

		pStall->SendMsgGuestAll( &DelSyn, sizeof(DelSyn) );
		////////////////////////////////////////////////////// 
		// οԼ    ־ ش./////////////////
		sellMsg.Category = MP_STREETSTALL;
		sellMsg.Protocol = MP_STREETSTALL_SELLITEM;
		sellMsg.dwObjectID = pBuyInfo->StallOwnerID;
		sellMsg.ItemInfo = ItemBase;
		sellMsg.count = 0;
		sellMsg.dwData = pBuyInfo->dwObjectID;

		pOwner->SendMsg( &sellMsg, sizeof(sellMsg) );
		/////////////////////////////////////////////////////////////
		
		ItemBase.Position = absPosOut;
		
		// մԿ  ÷ϰ  .////////////////////
		buyMsg.Category = MP_STREETSTALL;
		buyMsg.Protocol = MP_STREETSTALL_BUYITEM_ACK;
		buyMsg.dwObjectID = pBuyInfo->StallOwnerID;
		buyMsg.ItemInfo = ItemBase;
		buyMsg.count = 0;

		pGuest->SendMsg( &buyMsg, sizeof(buyMsg) );
	}
	else
	{
		// ġ   δ. ()
		DURTYPE LeftDurability = ItemBase.Durability - wBuyNum;
		if( EI_TRUE != pOwnerInventory->UpdateItemAbs(pOwner, RealPos, ItemBase.dwDBIdx, ItemBase.wIconIdx, 0, 0, LeftDurability, UB_DURA, SS_LOCKOMIT|SS_CHKDBIDX ) )
			goto ITEMBUY_FAILED;

		pStall->UpdateCell( pos,  LeftDurability);
		
		updateMsg.Category = MP_STREETSTALL;
		updateMsg.Protocol = MP_STREETSTALL_UPDATEITEM;
		updateMsg.dwObjectID = pBuyInfo->StallOwnerID;
		updateMsg.dwData = pos;
		updateMsg.ItemInfo = ItemBase;
		updateMsg.count = LeftDurability;
		
		pOwner->SendMsg( &updateMsg, sizeof(updateMsg) );
		
		pStall->SendMsgGuestAll( &updateMsg, sizeof(updateMsg) );
		
		//////////////////////////////////////////////////////
		// οԼ    ־ ش./////////////////
		sellMsg.Category = MP_STREETSTALL;
		sellMsg.Protocol = MP_STREETSTALL_SELLITEM;
		sellMsg.dwObjectID = pBuyInfo->StallOwnerID;
		sellMsg.ItemInfo = ItemBase;
		sellMsg.count = LeftDurability;
		sellMsg.dwData = pBuyInfo->dwObjectID;

		pOwner->SendMsg( &sellMsg, sizeof(sellMsg) );
		
//		pOwnerInventory->UpdateItemAbs(pOwner, RealPos, ItemBase.dwDBIdx, ItemBase.wIconIdx, 0, 0, LeftDurability, UB_DURA, SS_LOCKOMIT );
		//copy: error check!

		pOwnerInventory->SetLock(RealPos, TRUE);
	
		ItemUpdateToDB( pOwner->GetID(), ItemBase.dwDBIdx ,ItemBase.wIconIdx, LeftDurability, RealPos, 0 );
		
		LogItemMoney(pOwner->GetID(), pOwner->GetObjectName(), pGuest->GetID(), pGuest->GetObjectName(),
			eLog_StreetStallSellDivide, pOwner->GetMoney(), pGuest->GetMoney(), money,
			ItemBase.wIconIdx, ItemBase.dwDBIdx, ItemBase.Position, absPosOut, LeftDurability, pOwner->GetPlayerExpPoint());

		/////////////////////////////////////////////////////////////
		
		/////////////////////////////////////////////////////////////
		// մԿ  ÷ϰ  .////////////////////
		ITEMBASE NewItemBase;  /* ϴ Ⱚ  */
		NewItemBase.dwDBIdx			= 9;
		NewItemBase.wIconIdx		= 9;
		NewItemBase.Position		= absPosOut;
		NewItemBase.QuickPosition	= 9;
		NewItemBase.Durability		= 9;
		// magi82(28)
		NewItemBase.ItemParam		= 9;
		NewItemBase.RareIdx			= 9;
		pGuestInventory->InsertItemAbs(pGuest, absPosOut, &NewItemBase, SS_PREINSERT);
				
		SSItemInsert( pGuest->GetID(), ItemBase.wIconIdx, wBuyNum, absPosOut, pOwner->GetID(), (WORD)(pInvenItem->ItemParam & ITEM_PARAM_SEAL) );
		/////////////////////////////////////////////////////////////
	}
	
	pOwner->SetMoney( money, MONEY_ADDITION, 0, eItemTable_Inventory, eMoneyLog_GetStreetStall, pGuest->GetID());
	pGuest->SetMoney( money, MONEY_SUBTRACTION, 0, eItemTable_Inventory, eMoneyLog_LoseStreetStall, pOwner->GetID());

	stallMsg.Category = MP_STREETSTALL;
	stallMsg.Protocol = MP_STREETSTALL_MESSAGE;
	stallMsg.dwObjectID = pOwner->GetID();
	stallMsg.SellerID = pOwner->GetID();
	stallMsg.BuyerID = pGuest->GetID();
	stallMsg.ItemIdx = ItemBase.wIconIdx;
	stallMsg.count = (WORD)wBuyNum;
	stallMsg.money = money;
	
	pOwner->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );
	pGuest->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );

	return TRUE;
	
ITEMBUY_FAILED:
	msgNack.Category	= MP_STREETSTALL;
	msgNack.Protocol	= MP_STREETSTALL_BUYITEM_NACK;
	msgNack.dwObjectID	= pBuyInfo->dwObjectID;
	msgNack.wData		= rt;
	pGuest->SendMsg( &msgNack, sizeof(MSG_WORD) );
	
	return FALSE;
}

BOOL cStreetStallManager::SellItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pSellInfo )
{
	POSTYPE pos = pSellInfo->StallPos;					// â ġ
	sCELLINFO* pItemInfo = pStall->GetCellInfo( pos );
	
	POSTYPE RealPos = pSellInfo->ItemInfo.Position;		//   
	
	ITEMBASE ItemBase = pSellInfo->ItemInfo;			//Ǹ  
	ItemBase.Durability = 0;
	DURTYPE wSellNum = pSellInfo->ItemInfo.Durability;
	MONEYTYPE money = pItemInfo->dwMoney*wSellNum;
	
	// RaMa - ShopItemInven
	CItemSlot* pGuestInventory;
	CItemSlot* pOwnerInventory;

	ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( pItemInfo->sItemBase.wIconIdx );
	if( !pItemKindInfo )	return FALSE;

	eITEMTABLE Table = eItemTable_Inventory;
	if( pItemKindInfo->ItemKind & eSHOP_ITEM )
	{
		pGuestInventory	= (CShopInvenSlot*)pGuest->GetSlot( eItemTable_ShopInven );
		pOwnerInventory = (CShopInvenSlot*)pOwner->GetSlot( eItemTable_ShopInven );
		Table = eItemTable_ShopInven;
	}
	else
	{
		pGuestInventory	= (CInventorySlot*)pGuest->GetSlot( eItemTable_Inventory );
		pOwnerInventory = (CInventorySlot*)pOwner->GetSlot( eItemTable_Inventory );
	}

	//

	WORD	rt = NOT_ERROR;
	MSG_WORD2		msgNack;	//  ޼
	MSG_LINKITEM	DelSyn;		// 󿡼  ޼
	MSG_SELLITEM	sellMsg;	// մԿ  ޼
	MSG_BUYITEM		buyMsg;		// ο  ޼
	MSG_SELLITEM	updateMsg;	//   ɶ
	MSG_STALLMSG	stallMsg;	// ŷ  ޼
	
	POSTYPE absPosOut;
	WORD EmptyCellPos[1];
	const ITEMBASE* pInvenItem;

	if(CanSellItem(pOwner, pGuest, pItemInfo, EmptyCellPos, pSellInfo, &ItemBase, 1, rt) == FALSE)
		goto ITEMSELL_FAILED;

	absPosOut = EmptyCellPos[0]; //  ä  

	pInvenItem = pGuestInventory->GetItemInfoAbs(RealPos);
	if( !pInvenItem ) goto ITEMSELL_FAILED;
	
	if(!pInvenItem)
	{
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}
	
	if( !(pItemKindInfo->ItemKind & eSHOP_ITEM ) )
	{
		// ȭ/ ŷ 
		if( ITEMMGR->IsOptionItem(pInvenItem->wIconIdx, pInvenItem->Durability ) )
		{
			pItemInfo->bLock = FALSE;
			goto ITEMSELL_FAILED;
		}
		if( ITEMMGR->IsRareOptionItem(pInvenItem->wIconIdx, pInvenItem->RareIdx) )
		{
			pItemInfo->bLock = FALSE;
			goto ITEMSELL_FAILED;
		}
	}

	if(!CHKRT->ItemOf(pGuest, pInvenItem->Position, pInvenItem->wIconIdx,0,0,CB_EXIST|CB_ICONIDX))
	{
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}

	if(!CHKRT->ItemOf(pOwner, absPosOut,0,0,0,CB_ICONIDX))
	{
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}	

	ITEMBASE temp;
	
	if(pGuestInventory->DeleteItemAbs(pGuest, RealPos, &temp, SS_LOCKOMIT) != EI_TRUE)
	{
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}
	
	temp.Position = absPosOut;
	if(pOwnerInventory->InsertItemAbs(pOwner, absPosOut, &temp, SS_LOCKOMIT) !=  EI_TRUE)
	{	
		//rollback
		temp.Position = RealPos;
		if(pGuestInventory->InsertItemAbs(pGuest, RealPos, &temp) != EI_TRUE)
		{
			ASSERT(0);
			pItemInfo->bLock = FALSE;
			goto ITEMSELL_FAILED;
		}
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}
	
	if( ITEMMGR->IsPetSummonItem(ItemBase.wIconIdx) )
	{
		PET_TOTALINFO* pPetInfo = pGuest->GetPetManager()->GetPetTotalInfo(ItemBase.dwDBIdx);
		if(NULL == pPetInfo)	return FALSE;

		pOwner->GetPetManager()->SendPetInfo(pPetInfo);
        
		BOOL bReduceFriendly = !(ItemBase.ItemParam & ITEM_PARAM_SEAL);	//λ¸ FALSE
		pGuest->GetPetManager()->ExchangePet(ItemBase.dwDBIdx, pOwner, bReduceFriendly);
	}

	//SW070127 Ÿź
	if( ITEMMGR->IsTitanCallItem(ItemBase.wIconIdx) )
	{
		TITAN_TOTALINFO* pTitanInfo = pGuest->GetTitanManager()->GetTitanTotalInfo(ItemBase.dwDBIdx);
		if(NULL == pTitanInfo)	return FALSE;

		pGuest->GetTitanManager()->ExchangeTitan(ItemBase.dwDBIdx, pOwner);
	}

	if( ITEMMGR->IsTitanEquipItem(ItemBase.wIconIdx) )
	{
		TITAN_ENDURANCE_ITEMINFO* pTitanEnduranceInfo = pGuest->GetTitanManager()->GetTitanItemEnduranceInfo(ItemBase.dwDBIdx);
		if(NULL == pTitanEnduranceInfo)	return FALSE;

		pGuest->GetTitanManager()->ExchangeTitanEquipItem(ItemBase.dwDBIdx, pOwner);
	}

	// RaMa - ShopItem
	pGuest->SetMoney( pItemInfo->dwMoney, MONEY_ADDITION, 0, eItemTable_Inventory, eMoneyLog_GetStreetStall, pOwner->GetID());
	pOwner->SetMoney( pItemInfo->dwMoney, MONEY_SUBTRACTION, 0, eItemTable_Inventory, eMoneyLog_LoseStreetStall, pGuest->GetID());
	
	ItemUpdateToDB( pOwner->GetID(), ItemBase.dwDBIdx ,ItemBase.wIconIdx, ItemBase.Durability, absPosOut, 0, ItemBase.RareIdx );
	
	// Log 
	LogItemMoney(pGuest->GetID(), pGuest->GetObjectName(), pOwner->GetID(), pOwner->GetObjectName(),
		eLog_StreetStallBuyAll, pGuest->GetMoney(), pOwner->GetMoney(), pItemInfo->dwMoney,
		temp.wIconIdx, temp.dwDBIdx, 0, temp.Position, temp.Durability, pGuest->GetPlayerExpPoint());
	
	if(pItemInfo->wVolume == 1)
	{
		// 󿡼 Ѵ.
		pStall->EmptyCell( pos );

		// magi82  ˻
		DeleteStallData( pOwner->GetID(), ItemBase.wIconIdx, eFind_Buy );
		
		DelSyn.Category = MP_STREETSTALL;
		DelSyn.Protocol = MP_STREETSTALL_DELETEITEM;
		DelSyn.dwObjectID = pSellInfo->dwObjectID;
		DelSyn.ItemInfo = ItemBase;
		DelSyn.wAbsPosition = pos;
		
		pStall->SendMsgGuestAll( &DelSyn, sizeof(DelSyn) );
		
		DelSyn.Protocol = MP_STREETSTALL_DELETEITEM_ACK;
		
		pOwner->SendMsg( &DelSyn, sizeof(DelSyn) );
	}
	else
	{	// 󿡼  δ.
		DURTYPE LeftDurability = pItemInfo->wVolume - wSellNum;
		pStall->UpdateCell( pos,  LeftDurability);
		pStall->GetCellInfo(pos)->bLock = FALSE;
		
		updateMsg.Category = MP_STREETSTALL;
		updateMsg.Protocol = MP_STREETSTALL_UPDATEITEM;
		updateMsg.dwObjectID = pSellInfo->StallOwnerID;
		updateMsg.dwData = pos;
		updateMsg.ItemInfo = ItemBase;
		updateMsg.count = LeftDurability;
		
		pOwner->SendMsg( &updateMsg, sizeof(updateMsg) );
		
		pStall->SendMsgGuestAll( &updateMsg, sizeof(updateMsg) );
	}
	
	//////////////////////////////////////////////////////
	// մԿԼ    ־ ش./////////////////
	sellMsg.Category = MP_STREETSTALL;
	sellMsg.Protocol = MP_STREETSTALL_SELLITEM_ACK;
	sellMsg.dwObjectID = pOwner->GetID();
	sellMsg.ItemInfo = ItemBase;
	sellMsg.count = 0;
	sellMsg.dwData = pOwner->GetID();
	
	pGuest->SendMsg( &sellMsg, sizeof(sellMsg) );
	/////////////////////////////////////////////////////////////
	
	ItemBase.Position = absPosOut;
	
	// ο  ÷ϰ  .////////////////////
	buyMsg.Category = MP_STREETSTALL;
	buyMsg.Protocol = MP_STREETSTALL_BUYITEM;
	buyMsg.dwObjectID = pGuest->GetID();
	buyMsg.ItemInfo = ItemBase;
	//buyMsg.count = 0;
	//buyMsg.dwData = pGuest->GetID();
	buyMsg.dwMoney = pGuest->GetID();	//SW070308 ۺΰɼǵ  //buyMsg.dwData Ŭ̾Ʈ  ʰ ִ.
	
	/*
	if( ITEMMGR->IsOptionItem(ItemBase.wIconIdx, ItemBase.Durability) )
	{
		buyMsg.sOptionInfo = *(pOwner->GetItemOption(ItemBase.Durability));
		buyMsg.count = 1;
	}

	if( ITEMMGR->IsRareOptionItem(ItemBase.wIconIdx, ItemBase.RareIdx) )
	{
		buyMsg.sRareOptionInfo = *(pOwner->GetItemRareOption(ItemBase.RareIdx));
		buyMsg.RareCount = 1;
	}
	
	if( ITEMMGR->IsPetSummonItem(ItemBase.wIconIdx) )
	{
		buyMsg.sPetInfo = *(pOwner->GetPetManager()->GetPetTotalInfo(ItemBase.dwDBIdx));
		buyMsg.PetInfoCount = 1;
	}*/
	//SW070308 ۺΰɼǵ 
	ITEMMGR->SetItemOptionsInfoMsg(pOwner, &buyMsg.ItemInfo, (MSG_LINKITEMOPTIONS*)&buyMsg);

	pOwner->SendMsg( &buyMsg, sizeof(buyMsg) );
	/////////////////////////////////////////////////////////////

	stallMsg.Category = MP_STREETSTALL;
	stallMsg.Protocol = MP_STREETSTALL_MESSAGE;
	stallMsg.dwObjectID = pOwner->GetID();
	stallMsg.SellerID = pGuest->GetID();
	stallMsg.BuyerID = pOwner->GetID();
	stallMsg.ItemIdx = ItemBase.wIconIdx;
	stallMsg.count = (WORD)wSellNum;
	stallMsg.money = money;

	pOwner->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );
	pGuest->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );

	return TRUE;
	
ITEMSELL_FAILED:
	msgNack.Category	= MP_STREETSTALL;
	msgNack.Protocol	= MP_STREETSTALL_SELLITEM_NACK;
	msgNack.dwObjectID	= pSellInfo->dwObjectID;
	msgNack.wData1		= rt;
	msgNack.wData2		= RealPos;
	pGuest->SendMsg( &msgNack, sizeof(MSG_WORD2) );

	return FALSE;
}

BOOL cStreetStallManager::SellDupItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pSellInfo )
{
	POSTYPE pos = pSellInfo->StallPos;					// â ġ
	sCELLINFO* pItemInfo = pStall->GetCellInfo( pos );	// 

	POSTYPE RealPos = pSellInfo->ItemInfo.Position;		//   

	ITEMBASE ItemBase = pSellInfo->ItemInfo;			// Ǹ  
	DURTYPE wSellNum = pSellInfo->ItemInfo.Durability;	// Ǹ 
	DWORD	dwDBIdx = pSellInfo->ItemInfo.dwDBIdx;		// ǸϷ  DBε
	MONEYTYPE money = pItemInfo->dwMoney;
	
	// RaMa - ShopItemInven
	CItemSlot* pGuestInventory;
	CItemSlot* pOwnerInventory;

	ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( pItemInfo->sItemBase.wIconIdx );
	if( !pItemKindInfo )	return FALSE;

	eITEMTABLE Table = eItemTable_Inventory;
	if( pItemKindInfo->ItemKind &eSHOP_ITEM )
	{
		pGuestInventory	= (CShopInvenSlot*)pGuest->GetSlot( eItemTable_ShopInven );
		pOwnerInventory = (CShopInvenSlot*)pOwner->GetSlot( eItemTable_ShopInven );
		Table = eItemTable_ShopInven;
	}
	else
	{
		pGuestInventory	= (CInventorySlot*)pGuest->GetSlot( eItemTable_Inventory );
		pOwnerInventory = (CInventorySlot*)pOwner->GetSlot( eItemTable_Inventory );
	}
	
	WORD rt = NOT_ERROR;
	POSTYPE absPosOut;
	WORD EmptyCellPos[1];
	MSG_WORD2 msgNack;			//  ޼

	MSG_LINKITEM DelSyn;		//  ɶ
	MSG_SELLITEM updateMsg;		//   ɶ
	
	MSG_SELLITEM sellMsg;		// Ǹ(ԽƮ)  ޼
	MSG_BUYITEM buyMsg;			// ()  ޼

	MSG_STALLMSG stallMsg;
	
	const ITEMBASE* pInvenItem;	// Ǹ(ԽƮ) κ ִ  

	if(pItemInfo->wVolume == 0)	//  ̹  ̸ 
	{
		rt = NOT_EXIST;
		goto ITEMSELL_FAILED;
	}

	if(CanSellItem(pOwner, pGuest, pItemInfo, EmptyCellPos, pSellInfo, &ItemBase, wSellNum, rt) == FALSE)
		goto ITEMSELL_FAILED;

	absPosOut =  EmptyCellPos[0];

	pInvenItem = pGuestInventory->GetItemInfoAbs(RealPos);

	// ȷ  κ ִ  ũٸ
	// κ ִ  
	if(pInvenItem->Durability < wSellNum)
		wSellNum = pInvenItem->Durability;


	if(!pInvenItem)
	{
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}

	if(!CHKRT->ItemOf(pGuest, pInvenItem->Position, pInvenItem->wIconIdx,0,0,CB_EXIST|CB_ICONIDX))
	{
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}
	if(!CHKRT->ItemOf(pOwner, absPosOut,0,0,0,CB_ICONIDX))
	{
		pItemInfo->bLock = FALSE;
		goto ITEMSELL_FAILED;
	}
	
	if( pItemInfo->wVolume == wSellNum ) 
	{	// Եϵ  ȷ   ġϸ
		//  Ѵ
		pStall->EmptyCell( pos );

		// magi82  ˻
		DeleteStallData( pOwner->GetID(), ItemBase.wIconIdx, eFind_Buy );
		
		DelSyn.Category = MP_STREETSTALL;
		DelSyn.Protocol = MP_STREETSTALL_DELETEITEM;
		DelSyn.dwObjectID = pSellInfo->dwObjectID;
		DelSyn.ItemInfo = ItemBase;
		DelSyn.wAbsPosition = pos;
		
		pStall->SendMsgGuestAll( &DelSyn, sizeof(DelSyn) );
		
		DelSyn.Protocol = MP_STREETSTALL_DELETEITEM_ACK;
		
		pOwner->SendMsg( &DelSyn, sizeof(DelSyn) );
		
	}
	else if(pItemInfo->wVolume > wSellNum)
	{	// Եϵ  ȷ   
		//  δ
		DURTYPE LeftDurability = pItemInfo->wVolume - wSellNum;
		pStall->UpdateCell( pos,  LeftDurability);
		pStall->GetCellInfo(pos)->bLock = FALSE;

		updateMsg.Category = MP_STREETSTALL;
		updateMsg.Protocol = MP_STREETSTALL_UPDATEITEM;
		updateMsg.dwObjectID = pSellInfo->StallOwnerID;
		updateMsg.dwData = pos;
		updateMsg.ItemInfo = ItemBase;
		updateMsg.count = LeftDurability;
		
		pOwner->SendMsg( &updateMsg, sizeof(updateMsg) );
		
		pStall->SendMsgGuestAll( &updateMsg, sizeof(updateMsg) );
	}
	else
	{	//  ϵ  ȷ  
		//  ϰ ȷ  Ѵ
		wSellNum = pItemInfo->wVolume;
		
		pStall->EmptyCell( pos );

		// magi82  ˻
		DeleteStallData( pOwner->GetID(), ItemBase.wIconIdx, eFind_Buy );
		
		DelSyn.Category = MP_STREETSTALL;
		DelSyn.Protocol = MP_STREETSTALL_DELETEITEM;
		DelSyn.dwObjectID = pSellInfo->dwObjectID;
		DelSyn.ItemInfo = ItemBase;
		DelSyn.wAbsPosition = pos;
		
		pStall->SendMsgGuestAll( &DelSyn, sizeof(DelSyn) );
		
		DelSyn.Protocol = MP_STREETSTALL_DELETEITEM_ACK;
		
		pOwner->SendMsg( &DelSyn, sizeof(DelSyn) );
	}

	money = money * wSellNum;		// Ǹ 
	
	if( wSellNum == pInvenItem->Durability)
	{	//ȷ  κ  ġϸ
		//ԽƮ κ ϰ  κ ִ´
		ITEMBASE temp;
		if(pGuestInventory->DeleteItemAbs(pGuest, RealPos, &temp, SS_LOCKOMIT) != EI_TRUE)
		{
			pItemInfo->bLock = FALSE;
			goto ITEMSELL_FAILED;
		}
	
		temp.Position = absPosOut;
		if(pOwnerInventory->InsertItemAbs(pGuest, absPosOut, &temp, SS_LOCKOMIT) !=  EI_TRUE)
		{
			//rollback
			temp.Position = RealPos;
			if(pGuestInventory->InsertItemAbs(pOwner, RealPos, &temp) != EI_TRUE)
			{
				ASSERT(0);
				pItemInfo->bLock = FALSE;
				goto ITEMSELL_FAILED;
			}
			pItemInfo->bLock = FALSE;
			goto ITEMSELL_FAILED;
		}
		
		ItemUpdateToDB( pOwner->GetID(), temp.dwDBIdx ,temp.wIconIdx, temp.Durability, absPosOut, 0 );
		
		LogItemMoney(pGuest->GetID(), pOwner->GetObjectName(), pGuest->GetID(), pGuest->GetObjectName(),
			eLog_StreetStallBuyAll, pOwner->GetMoney(), pGuest->GetMoney(), money,
			temp.wIconIdx, temp.dwDBIdx, temp.Position, absPosOut, temp.Durability, pOwner->GetPlayerExpPoint());
		
		//////////////////////////////////////////////////////
		// մԿԼ    ־ ش./////////////////
		sellMsg.Category = MP_STREETSTALL;
		sellMsg.Protocol = MP_STREETSTALL_SELLITEM_ACK;
		sellMsg.dwObjectID = pOwner->GetID();
		sellMsg.ItemInfo = ItemBase;
		sellMsg.count = 0;
		sellMsg.dwData = pOwner->GetID();
		
		pGuest->SendMsg( &sellMsg, sizeof(sellMsg) );
		/////////////////////////////////////////////////////////////
		
		ItemBase.Position = absPosOut;
		
		// ο  ÷ϰ  .////////////////////
		buyMsg.Category = MP_STREETSTALL;
		buyMsg.Protocol = MP_STREETSTALL_BUYITEM;
		buyMsg.dwObjectID = pGuest->GetID();
		buyMsg.ItemInfo = ItemBase;
		//buyMsg.count = 0;
		//buyMsg.dwData = pGuest->GetID();
		
		pOwner->SendMsg( &buyMsg, sizeof( buyMsg ) );
		/////////////////////////////////////////////////////////////
	}
	else
	{	//ȷ  κ  
		//ԽƮ κ   ̰  κ ִ´
		DURTYPE LeftDurability = pInvenItem->Durability - wSellNum;
		if( EI_TRUE != pGuestInventory->UpdateItemAbs(pGuest, RealPos, ItemBase.dwDBIdx, ItemBase.wIconIdx, 0, 0, LeftDurability, UB_DURA, SS_LOCKOMIT|SS_CHKDBIDX ) )
			goto ITEMSELL_FAILED;
		

		//////////////////////////////////////////////////////////////////////////
		// DB  ־ 
		// CreateDupItem Լ   ο ޼ ư
		ITEMBASE NewItemBase;  /* ϴ Ⱚ  */
		NewItemBase.dwDBIdx			= 9;
		NewItemBase.wIconIdx		= 9;
		NewItemBase.Position		= absPosOut;
		NewItemBase.QuickPosition	= 9;
		NewItemBase.Durability		= 9;
		// magi82(28)
		NewItemBase.ItemParam		= 9;
		NewItemBase.RareIdx			= 9;
		
		pOwnerInventory->InsertItemAbs(pGuest, absPosOut, &NewItemBase, SS_PREINSERT);
		
		SSItemInsert( pOwner->GetID(), ItemBase.wIconIdx, wSellNum, absPosOut, pGuest->GetID(), (WORD)(pInvenItem->ItemParam & ITEM_PARAM_SEAL));
		//////////////////////////////////////////////////////////////////////////

		ItemBase.Position = RealPos;

		/////////////////////////////////////////////////////////////
		// մԿԼ    ־ ش.////////////////////
		sellMsg.Category = MP_STREETSTALL;
		sellMsg.Protocol = MP_STREETSTALL_SELLITEM_ACK;
		sellMsg.dwObjectID = pOwner->GetID();
		sellMsg.ItemInfo = ItemBase;
		sellMsg.count = LeftDurability;
		sellMsg.dwData = pOwner->GetID();
		
		pGuest->SendMsg( &sellMsg, sizeof(sellMsg) );
		
//		pGuestInventory->UpdateItemAbs(pGuest, RealPos, ItemBase.dwDBIdx, ItemBase.wIconIdx, 0, 0, LeftDurability, UB_DURA, SS_LOCKOMIT );
		//copy: error check!

		pGuestInventory->SetLock(RealPos, FALSE);
		
		ItemUpdateToDB( pGuest->GetID(), ItemBase.dwDBIdx ,ItemBase.wIconIdx, LeftDurability, RealPos, 0 );
		
		LogItemMoney(pGuest->GetID(), pGuest->GetObjectName(), pOwner->GetID(), pOwner->GetObjectName(),
			eLog_StreetStallSellDivide, pGuest->GetMoney(), pOwner->GetMoney(), money,
			ItemBase.wIconIdx, ItemBase.dwDBIdx, ItemBase.Position, absPosOut, LeftDurability, pGuest->GetPlayerExpPoint());
		
		/////////////////////////////////////////////////////////////
	}
	
	pGuest->SetMoney( money, MONEY_ADDITION, 0, eItemTable_Inventory, eMoneyLog_GetStreetStall, pOwner->GetID());
	pOwner->SetMoney( money, MONEY_SUBTRACTION, 0, eItemTable_Inventory, eMoneyLog_LoseStreetStall, pGuest->GetID());

	stallMsg.Category = MP_STREETSTALL;
	stallMsg.Protocol = MP_STREETSTALL_MESSAGE;
	stallMsg.dwObjectID = pOwner->GetID();
	stallMsg.SellerID = pGuest->GetID();
	stallMsg.BuyerID = pOwner->GetID();
	stallMsg.ItemIdx = ItemBase.wIconIdx;
	stallMsg.count = (WORD)wSellNum;
	stallMsg.money = money;
	
	pOwner->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );
	pGuest->SendMsg( &stallMsg, sizeof(MSG_STALLMSG) );

	return TRUE;
	
ITEMSELL_FAILED:
	msgNack.Category	= MP_STREETSTALL;
	msgNack.Protocol	= MP_STREETSTALL_SELLITEM_NACK;
	msgNack.dwObjectID	= pSellInfo->dwObjectID;
	msgNack.wData1		= rt;
	msgNack.wData2		= RealPos;
	pGuest->SendMsg( &msgNack, sizeof(MSG_WORD2) );
	
	return FALSE;
}

void cStreetStallManager::CreateDupItem( DWORD dwObjectId, ITEMBASE* pItemBase, DWORD FromChrID )
{
	CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( dwObjectId );
	
	if( pPlayer == NULL )
	{
		ASSERT(0);
		return;
	}

	cStreetStall* pStall = FindStreetStall( pPlayer );
	
	if(!pStall)	//  մ  Ǹų̴ 
	{
		MSG_ITEMEX buyMsg;
		buyMsg.Category = MP_STREETSTALL;
		buyMsg.Protocol = MP_STREETSTALL_BUYITEM_ACK;
		buyMsg.dwObjectID = dwObjectId;
		buyMsg.ItemInfo = *pItemBase;
		buyMsg.count = 0;
		pPlayer->SendMsg( &buyMsg, sizeof(MSG_ITEMEX) );
	}
	else
	{
		MSG_BUYITEM buyMsg;
		buyMsg.Category = MP_STREETSTALL;
		buyMsg.Protocol = MP_STREETSTALL_BUYITEM;
		buyMsg.dwObjectID = dwObjectId;
		buyMsg.ItemInfo = *pItemBase;
		//buyMsg.count = 0;
		//buyMsg.dwData = FromChrID;
		
		pPlayer->SendMsg( &buyMsg, sizeof( buyMsg ) );
		
	}
	
	// RaMa - ShopItemInven
	CItemSlot* pInventory;
	
	ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( pItemBase->wIconIdx );
	if( !pItemKindInfo )	return;
	
	eITEMTABLE Table = eItemTable_Inventory;	
	if( pItemKindInfo->ItemKind &eSHOP_ITEM )
	{
		Table = eItemTable_ShopInven;
		pInventory	= (CShopInvenSlot*)pPlayer->GetSlot( eItemTable_ShopInven );		
	}
	else
	{
		pInventory	= (CInventorySlot*)pPlayer->GetSlot( eItemTable_Inventory );
	}	
	
	pInventory->InsertItemAbs( pPlayer, pItemBase->Position, pItemBase, SS_LOCKOMIT);
	
	char FromName[MAX_NAME_LENGTH+1] = {0,};
	MONEYTYPE FromMoney = 0;
	EXPTYPE FromExpPoint = 0;
	CPlayer* pFromPlayer = (CPlayer*)g_pUserTable->FindUser( FromChrID );
	if(pFromPlayer)
	{
		SafeStrCpy(FromName, pFromPlayer->GetObjectName(), MAX_NAME_LENGTH+1);
		FromMoney = pFromPlayer->GetMoney();
		FromExpPoint = pFromPlayer->GetPlayerExpPoint();
	}
	LogItemMoney(FromChrID, FromName, dwObjectId, pPlayer->GetObjectName(),
		eLog_StreetStallBuyDivide, FromMoney, pPlayer->GetMoney(), 0,
		pItemBase->wIconIdx, pItemBase->dwDBIdx, 0, pItemBase->Position, pItemBase->Durability, 
		FromExpPoint);
}

void cStreetStallManager::NetworkMsgParse( BYTE Protocol, void* pMsg )
{
	switch( Protocol )
	{
	case MP_STREETSTALL_OPEN_SYN:
		{
			MSG_STREETSTALL_TITLE* msg = (MSG_STREETSTALL_TITLE*)pMsg;
		// 1.  
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) 
			{
				ASSERT(0);
				break;
			}

			/* State  Check Ѵ. */
			//KES VIMU 031103 ߰ 
			//SW061019 ̹  ߰
			if( pPlayer->IsVimuing() || pPlayer->GetExchangeContainer()->pRoom != NULL ||
				pPlayer->GetBattle()->GetBattleKind() == eBATTLE_KIND_GTOURNAMENT ||
				pPlayer->GetBattle()->GetBattleKind() == eBATTLE_KIND_SIEGEWAR ||
				//g_pServerSystem->CheckMapKindIs(eSurvival)	||
				g_pServerSystem->GetMap()->IsMapKind(eSurvivalMap) ||
				pPlayer->InTitan() )
			{
				MSGBASE CloseSyn;
				CloseSyn.Category = MP_STREETSTALL;
				CloseSyn.Protocol = MP_STREETSTALL_OPEN_NACK;
				pPlayer->SendMsg(&CloseSyn, sizeof(CloseSyn));
				return;
			}
			//
/*
#ifdef _HK_LOCAL_
			if( pPlayer->GetShopItemStats()->bStreetStall == 0 )
			{
				MSGBASE CloseSyn;
				CloseSyn.Category = MP_STREETSTALL;
				CloseSyn.Protocol = MP_STREETSTALL_OPEN_NACK;
				pPlayer->SendMsg(&CloseSyn, sizeof(CloseSyn));
				return;
			}
#endif
*/			
			//   ϸ ش.
			cStreetStall* pStall = FindStreetStall( pPlayer );
			if( pStall )
			{
//				ASSERT(0);
				DeleteStreetStall( pPlayer );

				MSGBASE CloseSyn;
				CloseSyn.Category = MP_STREETSTALL;
				CloseSyn.Protocol = MP_STREETSTALL_OPEN_NACK;
				pPlayer->SendMsg(&CloseSyn, sizeof(CloseSyn));

				return;
			}
/*
			if(msg->StallKind == eSK_BUY)
			{
				MSGBASE CloseSyn;
				CloseSyn.Category = MP_STREETSTALL;
				CloseSyn.Protocol = MP_STREETSTALL_OPEN_NACK;
				pPlayer->SendMsg(&CloseSyn, sizeof(CloseSyn));
				
				return;
			}
*/
			pStall = CreateStreetStall( pPlayer, msg->StallKind, msg->Title );
/*		
			//////////////////////////////////////////////////////////////////////////
			//    ߰ ̿ κ ͼ Ҵش
			if(msg->StallKind == eSK_BUY)
			{
				pStall->SetDefaultUsable();

				SHOPITEMOPTION*	pShopItemOption = pPlayer->GetShopItemStats();
				pStall->SetExtraUsable( pShopItemOption->BuyStallInven );

				//    κ ִ κ 
				// ִ κ ҴѴ
				if(pStall->GetUsable() >  MAX_STREETSTALL_CELLNUM)
					pStall->SetMaxUsable();
			}
			//////////////////////////////////////////////////////////////////////////
*/			
		// 2.  Message .
			MSG_STREETSTALL_TITLE OpenSyn;
			OpenSyn = *msg;
			OpenSyn.StallKind = msg->StallKind;
			OpenSyn.Protocol = MP_STREETSTALL_OPEN_ACK;
		
			pPlayer->SendMsg( &OpenSyn, sizeof(MSG_STREETSTALL_TITLE) );

		// 3.  ο ¸ ϵ Ѵ.
			StreetStallMode( pPlayer, msg->StallKind, msg->Title, TRUE );

			return;
		}
		break;
	case MP_STREETSTALL_CLOSE_SYN:
		{
			MSGBASE* msg = (MSGBASE*)pMsg;
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( pPlayer == NULL ) break;

			cStreetStall* pStall = FindStreetStall( pPlayer );
			if( pStall == NULL ) break;

			// 1.  ο ¸ ϵ Ѵ.
			StreetStallMode( pPlayer, pStall->GetStallKind(), NULL, FALSE ); // Ani 

			// 2. մԵ鿡 ˸. (մԵ ¸ Ͽ Ѵ.)
			MSGBASE CloseSyn;
			CloseSyn.Category = MP_STREETSTALL;
			CloseSyn.Protocol = MP_STREETSTALL_CLOSE;
			CloseSyn.dwObjectID = msg->dwObjectID;

			pStall->SendMsgGuestAll( &CloseSyn, sizeof(MSGBASE), TRUE );

			// 3.   Ѵ.
			DeleteStreetStall( pPlayer );

			// 4.  Message .
			CloseSyn.Protocol = MP_STREETSTALL_CLOSE_ACK;

			// StreetStallMode ̹ ȣѴ.
			pPlayer->SendMsg( &CloseSyn, sizeof(MSGBASE) );
		}
		break;
	case MP_STREETSTALL_LOCKITEM_SYN:
		{
			MSG_STREETSTALL_ITEMSTATUS* msg = (MSG_STREETSTALL_ITEMSTATUS*)pMsg;

			CPlayer* pPlayer = NULL;
			cStreetStall* pStall = NULL;
			sCELLINFO* pInfo = NULL;

			MSGBASE Ftemp;
			MSG_STREETSTALL_ITEMSTATUS Stemp;

			// 1.  ãƼ  Lock Ѵ.
			pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer )
			{
				ASSERT(0);
				return;
			}

			if( msg->wAbsPosition >= MAX_STREETSTALL_CELLNUM ) goto ITEMLOCKFAIL;

			pStall = FindStreetStall( pPlayer );
			if( pStall == NULL ) goto ITEMLOCKFAIL;

			pInfo = pStall->GetCellInfo( msg->wAbsPosition );
			if( pInfo->sItemBase.dwDBIdx != msg->ItemInfo.dwDBIdx ) goto ITEMLOCKFAIL;

			pStall->SetMoney( msg->wAbsPosition, msg->dwMoney );
			pStall->SetVolume( msg->wAbsPosition, msg->wVolume );
			pStall->ChangeCellState( msg->wAbsPosition, TRUE );


			// magi82  ˻
			InsertStallData( msg->dwObjectID, pInfo->sItemBase.wIconIdx, eFind_Sell, msg->dwMoney );
            
			Stemp = *msg;
			Stemp.Protocol = MP_STREETSTALL_LOCKITEM_ACK;
			pPlayer->SendMsg( &Stemp, sizeof(MSG_STREETSTALL_ITEMSTATUS) );

			// մ ¸ Ѵ.
			Stemp.Protocol = MP_STREETSTALL_LOCKITEM;
			pStall->SendMsgGuestAll( &Stemp, sizeof(MSG_STREETSTALL_ITEMSTATUS) );

			return;

ITEMLOCKFAIL:
			Ftemp.Category = MP_STREETSTALL;
			Ftemp.Protocol = MP_STREETSTALL_LOCKITEM_NACK;
			Ftemp.dwObjectID = msg->dwObjectID;
			pPlayer->SendMsg( &Ftemp, sizeof(MSGBASE) );
		}
		break;
	case MP_STREETSTALL_UNLOCKITEM_SYN:
		{
			MSG_STREETSTALL_ITEMSTATUS* msg = (MSG_STREETSTALL_ITEMSTATUS*)pMsg;

			CPlayer* pPlayer = NULL;
			cStreetStall* pStall = NULL;
			sCELLINFO* pInfo = NULL;

			MSGBASE Ftemp;
			MSG_STREETSTALL_ITEMSTATUS Stemp;

			// 1.  ãƼ  UnLock Ѵ.
			pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) 
			{
				ASSERT(0);
				return;
			}

			if( msg->wAbsPosition >= MAX_STREETSTALL_CELLNUM ) goto ITEMUNLOCKFAIL;

			pStall = FindStreetStall( pPlayer );
			if( pStall == NULL )  goto ITEMUNLOCKFAIL;

			pInfo = pStall->GetCellInfo( msg->wAbsPosition );
			if( pInfo->sItemBase.dwDBIdx != msg->ItemInfo.dwDBIdx) goto ITEMUNLOCKFAIL;

			pStall->SetMoney( msg->wAbsPosition, 0 );
			pStall->SetVolume( msg->wAbsPosition, 0 );
			pStall->ChangeCellState( msg->wAbsPosition, FALSE );
				
			Stemp = *msg;
			Stemp.Protocol = MP_STREETSTALL_UNLOCKITEM_ACK;
			pPlayer->SendMsg( &Stemp, sizeof(MSG_STREETSTALL_ITEMSTATUS) );
				
			// մ ¸ Ѵ.
			Stemp.Protocol = MP_STREETSTALL_UNLOCKITEM;
			pStall->SendMsgGuestAll( &Stemp, sizeof(MSG_STREETSTALL_ITEMSTATUS) );

			return;

ITEMUNLOCKFAIL:
			Ftemp.Category = MP_STREETSTALL;
			Ftemp.Protocol = MP_STREETSTALL_UNLOCKITEM_NACK;
			Ftemp.dwObjectID = msg->dwObjectID;
			pPlayer->SendMsg( &Ftemp, sizeof(MSGBASE) );
		}
		break;

	case MP_STREETSTALL_FAKEREGISTITEM_SYN:
		{
			MSG_REGISTITEMEX* msg = (MSG_REGISTITEMEX*)pMsg;
			cStreetStall* pStall = NULL;
			CPlayer* pPlayer = NULL;
			CItemSlot* pSlot = NULL;
			ITEMBASE pBase;
			sCELLINFO* pInfo = NULL;
			DWORD erroflag = 0;			
			// RaMa
			eITEMTABLE Table = eItemTable_Inventory;
			ITEM_INFO* pItemKindInfo = NULL;
			
			MSG_DWORD2 RegistFail;
			MSG_LINKITEMEX RegistSyn;
			SEND_LINKITEM_TOTALINFO RegistGuest;

			// 1.  ãƼ  ÷Ѵ.
			pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer )
			{
				ASSERT(0);
				return;
			}
			
			if( pPlayer->GetGuestStall() ) goto ITEMREGISTFAILEX; // Guest  쿡   ó
			
			pStall = FindStreetStall( pPlayer );
			if( pStall == NULL ) goto ITEMREGISTFAILEX;
			if( pStall->IsFull() ) goto ITEMREGISTFAILEX; //  ..

			if(pStall->GetStallKind() == eSK_BUY)	goto ITEMREGISTFAILEX;

			
			
			// RaMa - ShopItemInven
			pItemKindInfo = ITEMMGR->GetItemInfo( msg->ItemInfo.wIconIdx );
			if( !pItemKindInfo )	goto ITEMREGISTFAILEX;
			
			// magi82 - UniqueItem(070628)  ũۿø üũ
			if(pItemKindInfo->ItemKind == eEQUIP_ITEM_UNIQUE && GAMERESRCMNGR->GetUniqueItemOptionList(msg->ItemInfo.wIconIdx)->ExchangeFlag == 0)
			{
				goto ITEMREGISTFAILEX;
			}

			// magi82(17) - Titan(07.11.01)  Ÿź 󿡼 ǸŰ ȵǰ 
			if(pItemKindInfo->ItemKind == eTITAN_ITEM_PAPER)
			{
				pSlot	= (CInventorySlot*)pPlayer->GetSlot( eItemTable_Inventory );
				ITEMBASE* pIBase = (ITEMBASE*)pSlot->GetItemInfoAbs( msg->ItemInfo.Position );
				if( pIBase )
				{
					if( pPlayer->GetTitanManager()->GetRegistTitanCallItemDBIdx() == pIBase->dwDBIdx )
						goto ITEMREGISTFAILEX;
				}
			}

			if( pItemKindInfo->ItemKind & eSHOP_ITEM )
			{
				pSlot	= (CShopInvenSlot*)pPlayer->GetSlot( eItemTable_ShopInven );
				Table = eItemTable_ShopInven;
				
				ITEM_INFO* pInfo = ITEMMGR->GetItemInfo( msg->ItemInfo.wIconIdx );
				if( pInfo && pInfo->ItemType == 11 )
				{
					ITEMBASE* pIBase = (ITEMBASE*)pSlot->GetItemInfoAbs( msg->ItemInfo.Position );
					if( !pIBase || !(pIBase->ItemParam & ITEM_PARAM_SEAL) )
						goto ITEMREGISTFAILEX;
				}

				// ȯҰ  ŷ ȵǵ
				if( pItemKindInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemKindInfo->ItemKind == eSHOP_ITEM_PET_EQUIP )
				{
					if( pItemKindInfo->BuyPrice )
						goto ITEMREGISTFAILEX;
				}
				else if( pItemKindInfo->MeleeAttackMax )
					goto ITEMREGISTFAILEX;
			}
			else
				pSlot	= (CInventorySlot*)pPlayer->GetSlot( eItemTable_Inventory );
			//
			
			if( !pSlot ) goto ITEMREGISTFAILEX;
			if( Table==eItemTable_Inventory && pSlot->IsLock((POSTYPE)msg->ItemInfo.Position) ) goto ITEMREGISTFAILEX;
			
			if( !CHKRT->ItemOf( pPlayer, msg->ItemInfo.Position, msg->ItemInfo.wIconIdx,
				msg->ItemInfo.Durability, msg->ItemInfo.QuickPosition ) )
				goto ITEMREGISTFAILEX;
			
			//5̻ ģ   ø  
			if( ITEMMGR->IsDupItem(msg->ItemInfo.wIconIdx) && msg->ItemInfo.Durability > MAX_YOUNGYAKITEM_DUPNUM )
			{
				goto ITEMREGISTFAILEX;
			}
			
			if( GetItemKind( msg->ItemInfo.wIconIdx ) == eYOUNGYAK_ITEM )
			{
				if( pSlot->CheckQPosForItemIdx( msg->ItemInfo.wIconIdx ) == FALSE )
					goto ITEMREGISTFAILEX;
			}
			else
			{
				if( pSlot->GetItemInfoAbs( msg->ItemInfo.Position ) == NULL )
					goto ITEMREGISTFAILEX;
					
				if( pSlot->GetItemInfoAbs( msg->ItemInfo.Position )->QuickPosition != 0 )
					goto ITEMREGISTFAILEX;
			}
			
			pBase =  *pSlot->GetItemInfoAbs((POSTYPE)msg->ItemInfo.Position);
			if( !pStall->CheckItemDBIdx(pBase.dwDBIdx) ) 
			{
				erroflag = 1;
				goto ITEMREGISTFAILEX; // ̹ ϵǾ ִ ...
			}
			
			//			if( !pStall->FillCell( &pBase, msg->dwMoney, TRUE ) ) goto ITEMREGISTFAILEX;
			if( !pStall->FillCell( &pBase, msg->dwMoney, FALSE ) ) goto ITEMREGISTFAILEX;

			//SW051129 Pet
			// ȯ   ŷ ģе ˻
			ITEM_INFO* pItemInfo = ITEMMGR->GetItemInfo( msg->ItemInfo.wIconIdx );
			if( pItemInfo->ItemKind == eQUEST_ITEM_PET || pItemInfo->ItemKind == eSHOP_ITEM_PET )
			{
				if( FALSE == pPlayer->GetPetManager()->CheckPetDefaultFriendship(msg->ItemInfo.dwDBIdx) 
					|| TRUE == pPlayer->GetPetManager()->IsCurPetSummonItem(msg->ItemInfo.dwDBIdx) )
				{
					ASSERT(0);	// ⺻ ģе ̸ε ŷ Ͽ. ClientCheck
					goto ITEMREGISTFAILEX;
				}
			}

			//!!!ӽ  ڵ..α Ȯ Ұ
			if( eTITAN_ITEM_PAPER == pItemInfo->ItemKind )
			{
				TITAN_TOTALINFO* pCheckInfo = pPlayer->GetTitanManager()->GetTitanTotalInfo(msg->ItemInfo.dwDBIdx);

				if( NULL == pCheckInfo )
				{
					char buf[256] = {0,};
					sprintf(buf, "ownerID: %d, TitanPaperDBIdx: %d", pPlayer->GetID(), msg->ItemInfo.dwDBIdx);
					ASSERTMSG(0, buf);
					goto ITEMREGISTFAILEX;
				}
			}
			
			RegistSyn.Category = MP_STREETSTALL;
			RegistSyn.Protocol = MP_STREETSTALL_FAKEREGISTITEM_ACK;
			RegistSyn.dwObjectID = msg->dwObjectID;
			RegistSyn.ItemInfo = pBase;
			RegistSyn.dwMoney = msg->dwMoney;
			
			pPlayer->SendMsg( &RegistSyn, sizeof(MSG_LINKITEMEX) );
			
			// մԿ ǰ ش.

			RegistGuest.Category = MP_STREETSTALL;
			RegistGuest.Protocol = MP_STREETSTALL_FAKEREGISTITEM;
			RegistGuest.dwObjectID = msg->dwObjectID;
			RegistGuest.ItemInfo = pBase;
			RegistGuest.dwMoney = msg->dwMoney;

			/*
			RegistGuest.PetInfoCount = 0;
			RegistGuest.count = 0;
			RegistGuest.RareCount = 0;

			if( ITEMMGR->IsOptionItem(pBase.wIconIdx, pBase.Durability) )
			{
				RegistGuest.count = 1;
				RegistGuest.sOptionInfo = *(pPlayer->GetItemOption(pBase.Durability));
			}
			//SW050920 Rare
			if( ITEMMGR->IsRareOptionItem(pBase.wIconIdx, pBase.RareIdx) )
			{
				RegistGuest.RareCount = 1;
				RegistGuest.sRareOptionInfo = *(pPlayer->GetItemRareOption(pBase.RareIdx));
			}
			if( ITEMMGR->IsPetSummonItem(pBase.wIconIdx) )
			{
				 RegistGuest.PetInfoCount = 1;
				 RegistGuest.sPetTotalInfo = *(pPlayer->GetPetManager()->GetPetTotalInfo(pBase.dwDBIdx));
			}*/
//SW070308 ۺΰɼǵ 
			ITEMMGR->SetItemOptionsInfoMsg(pPlayer, &msg->ItemInfo, (MSG_LINKITEMOPTIONS*)&RegistGuest);

			pStall->SendMsgGuestAll( &RegistGuest, RegistGuest.GetSize() );
			
			return;
			
ITEMREGISTFAILEX:
			RegistFail.Category = MP_STREETSTALL;
			RegistFail.Protocol = MP_STREETSTALL_FAKEREGISTITEM_NACK;
			RegistFail.dwObjectID = msg->dwObjectID;
			RegistFail.dwData1 = msg->ItemInfo.Position;
			RegistFail.dwData2 = erroflag;
			
			pPlayer->SendMsg( &RegistFail, sizeof(MSG_DWORD2) );
			
			return;
		}
		break;
	case MP_STREETSTALL_FAKEREGISTBUYITEM_SYN:
		{
			MSG_REGISTBUYITEMEX* msg = (MSG_REGISTBUYITEMEX*)pMsg;
			cStreetStall* pStall = NULL;
			CPlayer* pPlayer = NULL;
			CItemSlot* pSlot = NULL;
			ITEMBASE pBase;
			sCELLINFO* pInfo = NULL;
			DWORD erroflag = 0;			
			// RaMa
			eITEMTABLE Table = eItemTable_Inventory;
			ITEM_INFO* pItemKindInfo = NULL;
			DWORD TotalMoney = 0;
			
			MSG_DWORD2 RegistFail;
			MSG_LINKBUYITEMEX RegistSyn;
			SEND_LINKBUYITEM_TOTALINFO RegistGuest;
			
			// 1.  ãƼ  ÷Ѵ.
			pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer )
			{
				ASSERT(0);
				return;
			}
			
			if( pPlayer->GetGuestStall() ) goto BUYITEMREGISTFAILEX; // Guest  쿡   ó
			
			pStall = FindStreetStall( pPlayer );
			if( pStall == NULL ) goto BUYITEMREGISTFAILEX;
			if( pStall->IsFull() ) goto BUYITEMREGISTFAILEX; //  ..
			
			if(pStall->GetStallKind() == eSK_SELL) goto BUYITEMREGISTFAILEX;

			//////////////////////////////////////////////////////////////////////////
			//ϰ ϴ   κ  Ұ ϶
//			if(msg->wAbsPosition >= pStall->GetUsable()) goto BUYITEMREGISTFAILEX;
			//////////////////////////////////////////////////////////////////////////
			
			//԰  ʰҶ

			pInfo = pStall->GetCellInfo(msg->wAbsPosition);

			if(!pInfo) goto BUYITEMREGISTFAILEX;

			TotalMoney = pStall->GetTotalMoney();

			if(pInfo->bFill)	//    Ǿִٸ  ϵ  
				TotalMoney -= (pInfo->dwMoney * pInfo->wVolume);

			if(pPlayer->GetMoney() < TotalMoney + (msg->dwMoney * msg->wVolume)) 
			{
				erroflag = 1;
				goto BUYITEMREGISTFAILEX;
			}
			
			//////////////////////////////////////////////////////////////////////////
			// ŵ ۿ شϴ κ  ִ Ǵ
			WORD EmptyCellPos;

			pItemKindInfo = ITEMMGR->GetItemInfo( msg->ItemInfo.wIconIdx );
			
			if( !pItemKindInfo )	goto BUYITEMREGISTFAILEX;
			
			if( pItemKindInfo->ItemKind & eSHOP_ITEM )
				pSlot	= (CShopInvenSlot*)pPlayer->GetSlot( eItemTable_ShopInven );
			else
				pSlot = (CInventorySlot*)pPlayer->GetSlot( eItemTable_Inventory );
			
			if( pSlot->GetEmptyCell(&EmptyCellPos) == 0 ) 
			{
				erroflag = 2;
				goto BUYITEMREGISTFAILEX;
			}
			//////////////////////////////////////////////////////////////////////////

			
			pBase = msg->ItemInfo;
			
			if( !pStall->FillCell( &pBase, msg->dwMoney, FALSE, msg->wVolume, msg->wAbsPosition ) )  goto BUYITEMREGISTFAILEX;

			// magi82  ˻
			InsertStallData( msg->dwObjectID, pBase.wIconIdx, eFind_Buy, msg->dwMoney );

			RegistSyn.Category = MP_STREETSTALL;
			RegistSyn.Protocol = MP_STREETSTALL_FAKEREGISTBUYITEM_ACK;
			RegistSyn.dwObjectID = msg->dwObjectID;
			RegistSyn.ItemInfo = pBase;
			RegistSyn.dwMoney = msg->dwMoney;
			RegistSyn.wVolume = msg->wVolume;
			RegistSyn.wAbsPosition = msg->wAbsPosition;
			
			pPlayer->SendMsg( &RegistSyn, sizeof(MSG_LINKBUYITEMEX) );
			
			// մԿ ǰ ش.
			
			RegistGuest.Category = MP_STREETSTALL;
			RegistGuest.Protocol = MP_STREETSTALL_FAKEREGISTBUYITEM;
			RegistGuest.dwObjectID = msg->dwObjectID;
			RegistGuest.ItemInfo = pBase;
			RegistGuest.dwMoney = msg->dwMoney;
			//RegistGuest.count = 0;;
			//RegistGuest.RareCount = 0;
			RegistGuest.wVolume = msg->wVolume;
			RegistGuest.wAbsPosition = msg->wAbsPosition;
			
			/*
			if( ITEMMGR->IsOptionItem(pBase.wIconIdx, pBase.Durability) )
			{
				RegistGuest.count = 1;
				RegistGuest.sOptionInfo = *(pPlayer->GetItemOption(pBase.Durability));
			}
			//SW050920 Rare
			if( ITEMMGR->IsRareOptionItem(pBase.wIconIdx, pBase.RareIdx) )
			{
				RegistGuest.RareCount = 1;
				RegistGuest.sRareOptionInfo = *(pPlayer->GetItemRareOption(pBase.RareIdx));
			}*/
			//SW070308 ۺΰɼǵ 
			ITEMMGR->SetItemOptionsInfoMsg(pPlayer, &RegistGuest.ItemInfo, (MSG_LINKITEMOPTIONS*)&RegistGuest);
			
			pStall->SendMsgGuestAll( &RegistGuest, sizeof(SEND_LINKBUYITEM_TOTALINFO) );

			return;
BUYITEMREGISTFAILEX:
			RegistFail.Category = MP_STREETSTALL;
			RegistFail.Protocol = MP_STREETSTALL_FAKEREGISTBUYITEM_NACK;
			RegistFail.dwObjectID = msg->dwObjectID;
			RegistFail.dwData1 = msg->ItemInfo.Position;
			RegistFail.dwData2 = erroflag;
			
			pPlayer->SendMsg( &RegistFail, sizeof(MSG_DWORD2) );
			
			return;
			
		}
		break;
	case MP_STREETSTALL_DELETEITEM_SYN:
		{
			MSG_LINKITEM* msg = (MSG_LINKITEM*)pMsg;

			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) break;

			cStreetStall* pStall = FindStreetStall( pPlayer );
			if( pStall == NULL ) break;

			// RaMa - ShopItemInven
			ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( msg->ItemInfo.wIconIdx );
			if( !pItemKindInfo )	return;

			eITEMTABLE Table = eItemTable_Inventory;	
			if( pItemKindInfo->ItemKind &eSHOP_ITEM )
				Table = eItemTable_ShopInven;			
			//

			// magi82  ˻ -> ش  鼭  ˻ ̺  ° ؾѴ.
			StreetStallFindState eFindState;

			if(pStall->GetStallKind() == eSK_SELL)
			{
				pStall->EmptyCell( &msg->ItemInfo, Table );
				eFindState = eFind_Sell;
			}
			else if(pStall->GetStallKind() == eSK_BUY)
			{
				pStall->EmptyCell( msg->wAbsPosition );
				eFindState = eFind_Buy;
			}

			// magi82  ˻
            DeleteStallData( msg->dwObjectID, msg->ItemInfo.wIconIdx, eFindState );            

			// ޼ .
			MSG_LINKITEM DelSyn;
			DelSyn.Category = MP_STREETSTALL;
			DelSyn.Protocol = MP_STREETSTALL_DELETEITEM_ACK;
			DelSyn.dwObjectID = msg->dwObjectID;
			DelSyn.ItemInfo = msg->ItemInfo;
			DelSyn.wAbsPosition = msg->wAbsPosition;
			pPlayer->SendMsg( &DelSyn, sizeof(MSG_LINKITEM) );

			// մԿԼ ǰ  Ѵ.
			DelSyn.Protocol = MP_STREETSTALL_DELETEITEM;
			pStall->SendMsgGuestAll( &DelSyn, sizeof(MSG_LINKITEM) );
		}
		break;
	case MP_STREETSTALL_BUYITEM_SYN:
		{
			STREETSTALL_BUYINFO* msg = (STREETSTALL_BUYINFO*)pMsg;
			CPlayer* pOwner = (CPlayer *)g_pUserTable->FindUser(msg->StallOwnerID);
			if(pOwner == NULL) return;

			CPlayer* pGuest = (CPlayer *)g_pUserTable->FindUser(msg->dwObjectID);
			if(pGuest == NULL) return;

			if( FindStreetStall(pGuest) != NULL )
			{
				goto BUYITEMFAILEX;
				//SendNackMsg(pGuest, MP_STREETSTALL_BUYITEM_NACK);
//				return;
			}

			cStreetStall* pStall = FindStreetStall( pOwner );
			if( pStall == NULL ) return;
			if( pGuest->GetGuestStall() == NULL ) 
			{
				char buf[128];
				sprintf(buf, " µ   Ѵ. %d", pGuest->GetID());
				ASSERTMSG(0, buf);
				
				goto BUYITEMFAILEX;
				//SendNackMsg(pGuest, MP_STREETSTALL_BUYITEM_NACK);
//				return;
			}

			if(pGuest->GetGuestStall()->GetOwner()->GetID() != msg->StallOwnerID)
			{
				char buf[128];
				sprintf(buf, "ϵ  ٸ   Ѵ: %d", pGuest->GetID());
				ASSERTMSG(0, buf);
				
				goto BUYITEMFAILEX;
//				SendNackMsg(pGuest, MP_STREETSTALL_BUYITEM_NACK);
//				return;
			}
			
			if( pStall->CheckItemDBIdx(msg->ItemInfo.dwDBIdx) ) // Ͼ üũѴ.
			{
				MSG_WORD msgNack;
				msgNack.Category	= MP_STREETSTALL;
				msgNack.Protocol	= MP_STREETSTALL_BUYITEM_NACK;
				msgNack.dwObjectID	= msg->dwObjectID;
				msgNack.wData		= NOT_EXIST;
				pGuest->SendMsg( &msgNack, sizeof(msgNack) );

				return;
			}

			if( ITEMMGR->IsDupItem( msg->ItemInfo.wIconIdx ) )
			{
				BuyDupItem( pOwner, pStall, pGuest, msg );
			}
			else
			{
				BuyItem( pOwner, pStall, pGuest, msg );
			}

			return;

BUYITEMFAILEX:
		MSG_WORD msgNack;
		msgNack.Category	= MP_STREETSTALL;
		msgNack.Protocol	= MP_STREETSTALL_BUYITEM_NACK;
		msgNack.dwObjectID	= msg->dwObjectID;
		msgNack.wData		= NOT_EXIST;
		pGuest->SendMsg( &msgNack, sizeof(msgNack) );
		return;

		}
		break;

	case MP_STREETSTALL_SELLITEM_SYN:
		{
			STREETSTALL_BUYINFO* msg = (STREETSTALL_BUYINFO*)pMsg;
			CPlayer* pOwner = NULL;
			CPlayer* pGuest = NULL;
			cStreetStall* pStall = NULL;

			pOwner = (CPlayer *)g_pUserTable->FindUser(msg->StallOwnerID);
			pGuest = (CPlayer *)g_pUserTable->FindUser(msg->dwObjectID);
			
			if( !pOwner || !pGuest ) goto SELLITEMFAILEX;

			//copy: check!
			if( FindStreetStall( pGuest ) != NULL ) 
				goto SELLITEMFAILEX;

			pStall = FindStreetStall( pOwner );		
			
			if( !pStall ) goto SELLITEMFAILEX;


			
			if( !pGuest->GetGuestStall() ) 	goto SELLITEMFAILEX;

			if(pGuest->GetGuestStall()->GetOwner()->GetID() != msg->StallOwnerID)
				goto SELLITEMFAILEX;
			
			if( !(pStall->GetCellInfo(msg->StallPos)->bFill)			//   ȵ 
				|| ( (pStall->GetCellInfo(msg->StallPos)->sItemBase.wIconIdx) != msg->ItemInfo.wIconIdx ) 
					//  ε ġ
				|| (pStall->GetCellInfo(msg->StallPos)->wVolume < 1)	//  ϵ  0 
				|| (pStall->GetCellInfo(msg->StallPos)->bLock) )		// ִ 
				goto SELLITEMFAILEX;

			if( ITEMMGR->IsDupItem( msg->ItemInfo.wIconIdx ) )
				SellDupItem( pOwner, pStall, pGuest, msg );
			else
				SellItem( pOwner, pStall, pGuest, msg );

			return;

SELLITEMFAILEX:
			MSG_WORD2 msgNack;
			msgNack.Category	= MP_STREETSTALL;
			msgNack.Protocol	= MP_STREETSTALL_SELLITEM_NACK;
			msgNack.dwObjectID	= msg->dwObjectID;
			msgNack.wData1		= NOT_EXIST;
			msgNack.wData2		= msg->ItemInfo.Position;
			pGuest->SendMsg( &msgNack, sizeof(MSG_WORD2) );
			
			return;
		}
		break;

	case MP_STREETSTALL_GUESTIN_SYN:
		{
			MSG_DWORD* msg = (MSG_DWORD*)pMsg;
			BOOL bVal = TRUE;
			CPlayer* pOwner = (CPlayer*)g_pUserTable->FindUser( msg->dwData );

			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) break;
			
			/* State  Check Ѵ. */
			//KES VIMU 031103 ߰
			if( pPlayer->IsVimuing() ) bVal = FALSE;	//
			if( pPlayer->GetExchangeContainer()->pRoom != NULL ) bVal = FALSE;	//ȯ
			if( (pPlayer->GetState() == eObjectState_StreetStall_Guest) || (pPlayer->GetState() == eObjectState_StreetStall_Owner))
			{
				char buf[128];
				sprintf(buf, " ̹  : %d", pPlayer->GetID());
				ASSERTMSG(0, buf);
				SendNackMsg(pPlayer, MP_STREETSTALL_GUESTIN_NACK);
				return;
			}
				
			cStreetStall* pStall = NULL;

			if( !pOwner )
			{
				bVal = FALSE;
			}
			else if( bVal )
			{
				pStall = FindStreetStall( pOwner );
				if( !pStall ) 
				{
					bVal = FALSE;
				}
				else
				{
					ASSERT( pPlayer->GetGuestStall() == NULL );	// null̾Ѵ.

					if( pPlayer->GetGuestStall() )
					{
						pPlayer->GetGuestStall()->DeleteGuest( pPlayer );
					}
					

					pStall->AddGuest( pPlayer );
				}
			}

			if( bVal )
			{
				STREETSTALL_INFO sInfo;
				memset(&sInfo, 0, sizeof(STREETSTALL_INFO));
				sInfo.count = 0;
				sInfo.RareCount = 0;
				pStall->GetStreetStallInfo( sInfo );
				sInfo.Category = MP_STREETSTALL;
				sInfo.Protocol = MP_STREETSTALL_GUESTIN_ACK;
				sInfo.StallKind = pStall->GetStallKind();
				sInfo.dwObjectID = msg->dwObjectID;
				sInfo.StallOwnerID = msg->dwData;

				OBJECTSTATEMGR_OBJ->StartObjectState(pPlayer, eObjectState_StreetStall_Guest, 0);

				pPlayer->SendMsg( &sInfo, sInfo.GetSize() );
			}
			else
			{
				SendNackMsg(pPlayer, MP_STREETSTALL_GUESTIN_NACK);
			}
		}
		break;
	case MP_STREETSTALL_GUESTOUT_SYN:
		{
			MSG_DWORD* msg = (MSG_DWORD*)pMsg;
			CPlayer* pOwner = (CPlayer*)g_pUserTable->FindUser( msg->dwData );
			if( !pOwner ) break;

			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) break;
			cStreetStall * pPlayerStall = pPlayer->GetGuestStall();
			if(!pPlayerStall)
			{
				char buf[128];
				sprintf(buf, "÷̾ gueststall µ  ޽ : %d", pPlayer->GetID());
				ASSERTMSG(0, buf);
				SendNackMsg(pPlayer, MP_STREETSTALL_GUESTOUT_NACK);
				if(pPlayer->GetState() == eObjectState_StreetStall_Guest)
				{				
					char buf[128];
					sprintf(buf, "÷̾ gueststall µ ´  : %d", pPlayer->GetID());
					ASSERTMSG(0, buf);
				}
				return;
			}
			if(pPlayerStall->GetOwner()->GetID() != pOwner->GetID())
			{
				char buf[128];
				sprintf(buf, " ִ  ٸ  ٰ ޽ : %d", pPlayer->GetID());
				ASSERTMSG(0, buf);
				return;
			}
			if( pPlayer->GetState() != eObjectState_StreetStall_Guest)
			{
				char buf[128];
				sprintf(buf, " մԻ° ƴѵ  ޽ : %d", pPlayer->GetID());
				ASSERTMSG(0, buf);
				SendNackMsg(pPlayer, MP_STREETSTALL_GUESTOUT_NACK);
				return;
			}

			OBJECTSTATEMGR_OBJ->EndObjectState(pPlayer, eObjectState_StreetStall_Guest);
			cStreetStall* pStall = FindStreetStall( pOwner );
			if( pStall )
				pStall->DeleteGuest( pPlayer );

			MSGBASE OutSyn;
			OutSyn.Category = MP_STREETSTALL;
			OutSyn.Protocol = MP_STREETSTALL_GUESTOUT_ACK;
			OutSyn.dwObjectID = msg->dwObjectID;

			pPlayer->SendMsg( &OutSyn, sizeof(MSGBASE) );
		}
		break;
	case MP_STREETSTALL_EDITTITLE_SYN:
		{
			MSG_STREETSTALL_TITLE* msg = (MSG_STREETSTALL_TITLE*)pMsg;

			// 1.  ãƼ ŸƲ Ѵ.
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) break;

			cStreetStall* pStall = FindStreetStall( pPlayer );

			if( pStall == NULL )
			{
				MSGBASE temp;
				temp.Category = MP_STREETSTALL;
				temp.dwObjectID = msg->dwObjectID;
				temp.Protocol = MP_STREETSTALL_EDITTITLE_NACK;

				pPlayer->SendMsg( &temp, sizeof(MSGBASE) );
			}
			else
			{
				pPlayer->SetStreetStallTitle( msg->Title );

				MSG_STREETSTALL_TITLE temp;
				temp.Category = MP_STREETSTALL;
				temp.dwObjectID = msg->dwObjectID;
				temp.StallKind = msg->StallKind;
				SafeStrCpy( temp.Title, msg->Title, MAX_STREETSTALL_TITLELEN+1 );
				temp.Protocol = MP_STREETSTALL_EDITTITLE_ACK;
				pPlayer->SendMsg( &temp, sizeof(MSG_STREETSTALL_TITLE) );

				// մԿ ŸƲ ˸.
				temp.Protocol = MP_STREETSTALL_EDITTITLE;
				PACKEDDATA_OBJ->QuickSendExceptObjectSelf(pPlayer,&temp,sizeof(MSG_STREETSTALL_TITLE));
			}
		}
		break;
	case MP_STREETSTALL_UPDATE_SYN:
		{
			MSG_WORD* msg = (MSG_WORD*)pMsg;
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) break;
			
			cStreetStall* pStall = FindStreetStall( pPlayer );
			if( !pStall ) break;
			
			
			sCELLINFO* pCellInfo = pStall->GetCellInfo( msg->wData );
			
			if(( pCellInfo ) && ( pCellInfo->bFill ) && ( !pCellInfo->bLock ))
			{
				pCellInfo->bLock = TRUE;

				MSG_WORD success;

				success.Category = MP_STREETSTALL;
				success.Protocol = MP_STREETSTALL_UPDATE_ACK;
				success.dwObjectID = pPlayer->GetID();
				success.wData = msg->wData;
								
				pPlayer->SendMsg( &success, sizeof(MSG_WORD) );

				success.Protocol = MP_STREETSTALL_UPDATE;

				pStall->SendMsgGuestAll( &success, sizeof(MSG_WORD) );
			}
			else
			{
				MSG_WORD fail;

				fail.Category = MP_STREETSTALL;
				fail.Protocol = MP_STREETSTALL_UPDATE_NACK;
				fail.dwObjectID = pPlayer->GetID();
				fail.wData = msg->wData;

				pPlayer->SendMsg( &fail, sizeof(MSG_WORD) );
			}
		}
		break;
	case MP_STREETSTALL_UPDATEEND_SYN:
		{
			MSGBASE* msg = (MSGBASE*)pMsg;
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( msg->dwObjectID );
			if( !pPlayer ) break;
			
			cStreetStall* pStall = FindStreetStall( pPlayer );
			if( !pStall ) break;
			
			for(int i = 0; i < SLOT_STREETBUYSTALL_NUM; i++)
			{
				sCELLINFO* pCellInfo = pStall->GetCellInfo(i);
				
				if(pCellInfo)
					pCellInfo->bLock = FALSE;
			}
			
			MSGBASE success;

			success.Category = MP_STREETSTALL;
			success.dwObjectID = pPlayer->GetID();
			
			success.Protocol = MP_STREETSTALL_UPDATEEND;

			pStall->SendMsgGuestAll( &success, sizeof(MSGBASE) );

			success.Protocol = MP_STREETSTALL_UPDATEEND_ACK;

			pPlayer->SendMsg( &success, sizeof(MSGBASE) );
		}
		break;
		// magi82  ˻
	case MP_STREETSTALL_FINDITEM_SYN:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;
			
			// ˻ õϴ  ִ üũ
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( pmsg->dwObjectID );
			if( !pPlayer )
				break;

            if( FALSE == CheckDelayTime(STALL_SEARCH_DELAY_TIME, eDelay_StallSearch) )
			{
                MSG_DWORD msg;
				SetProtocol( &msg, MP_STREETSTALL, MP_STREETSTALL_FINDITEM_NACK );
				msg.dwObjectID = pmsg->dwObjectID;
				msg.dwData = 1;

				pPlayer->SendMsg( &msg, sizeof(MSG_DWORD) );
				return;
			}

			SEND_STREETSTALL_INFO msg;
			memset( &msg, 0, sizeof(SEND_STREETSTALL_INFO) );
			SetProtocol( &msg, MP_STREETSTALL, MP_STREETSTALL_FINDITEM_ACK );
			msg.dwObjectID = pPlayer->GetID();

			int nCount = 0;
			//  ε  ü ˻̴.
			if( 0 == pmsg->dwData1 )
			{
				// ü ˻Ҷ  üũ  Ƿ    ̺  ʿ
				//  ִ ش   ̺   ° .
				CPlayer* pOwner = NULL;
				cStreetStall* pInfo;
				m_StallTable.SetPositionHead();
				while( pInfo = m_StallTable.GetData() )
				{
					pOwner = pInfo->GetOwner();
					if( pmsg->dwData2 == pInfo->GetStallKind() )
					{
						if( pPlayer->GetChannelID() == pOwner->GetChannelID() )
						{
							// ش  ˻Ϸ  Ÿ԰ ƾ ȴ.( .. Ǹ ..)
							//  äε ƾ Ѵ.
							msg.sInfo[nCount].dwOwnerIdx = pOwner->GetID();
							msg.sInfo[nCount].dwPrice = 0;	// ü ˻Ҷ  ˷ ʿ䰡 .
							SafeStrCpy( msg.sInfo[nCount].strName, pOwner->GetObjectName(), MAX_NAME_LENGTH );

							nCount++;
						}
					}
				}

				msg.wCount = nCount;
			}
			else
			{
				//  ¿  ° 
				StreetStallFindState eFindState;
				if( pmsg->dwData2 == eSK_SELL )
					eFindState = eFind_Sell;
				else if( pmsg->dwData2 == eSK_BUY )
					eFindState = eFind_Buy;

				CPlayer* pOwner = NULL;
				STALL_DEALITEM_INFO* pDealItem = m_pDealItemTable[eFindState].GetData(pmsg->dwData1);
				if( pDealItem )
				{
					STALL_DEALER_INFO* pDealer;
					pDealItem->Info.SetPositionHead();
					while( pDealer = pDealItem->Info.GetData() )
					{
						pOwner = (CPlayer*)g_pUserTable->FindUser( pDealer->dwCharIdx );
						if( pOwner && pPlayer->GetChannelID() == pOwner->GetChannelID() )
						{
							//  ä  ˻  Ͽ ߰Ѵ.
							msg.sInfo[nCount].dwOwnerIdx = pDealer->dwCharIdx;
							msg.sInfo[nCount].dwPrice = pDealer->dwPrice;
							SafeStrCpy( msg.sInfo[nCount].strName, pOwner->GetObjectName(), MAX_NAME_LENGTH );

							nCount++;
						}
					}

					msg.wCount = nCount;
				}
			}

			pPlayer->SendMsg( &msg, msg.GetSize() );
		}
		break;
		// magi82  ˻
	case MP_STREETSTALL_ITEMVIEW_SYN:
		{
			MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( pmsg->dwObjectID );
			if( !pPlayer )
				break;

			if( FALSE == CheckDelayTime(ITEM_VIEW_DELAY_TIME, eDelay_ItemView) )
			{
				MSG_DWORD msg;
				SetProtocol( &msg, MP_STREETSTALL, MP_STREETSTALL_ITEMVIEW_NACK );
				msg.dwObjectID = pmsg->dwObjectID;
				msg.dwData = 1;

				pPlayer->SendMsg( &msg, sizeof(MSG_DWORD) );
				return;
			}


			ITEM_OPTION_INFO OptionInfo[SLOT_STREETSTALL_NUM];
			ITEM_RARE_OPTION_INFO RareOptionInfo[SLOT_STREETSTALL_NUM];
			PET_TOTALINFO PetInfo[SLOT_STREETSTALL_NUM];
			TITAN_TOTALINFO TitanInfo[SLOT_STREETSTALL_NUM];
			TITAN_ENDURANCE_ITEMINFO TitanEquipInfo[SLOT_STREETSTALL_NUM];
			WORD OptionNum = 0;
			WORD RareOptionNum = 0;
			WORD PetInfoNum = 0;
			WORD TitanInfoNum = 0;
			WORD TitanEquipNum = 0;


			MSG_STREETSTALL_ITEMVIEW msg;
			memset( &msg, 0, sizeof(MSG_STREETSTALL_ITEMVIEW) );
			SetProtocol( &msg, MP_STREETSTALL, MP_STREETSTALL_ITEMVIEW_ACK );

			cStreetStall* pInfo = m_StallTable.GetData(pmsg->dwData);
			if( pInfo )
			{
				int nMaxCount = 0;
				if( pInfo->GetStallKind() == eSK_SELL )
                    nMaxCount = pInfo->GetCurRegistItemNum();
				else if( pInfo->GetStallKind() == eSK_BUY )
					nMaxCount = SLOT_STREETBUYSTALL_NUM;

				for( int i = 0; i < nMaxCount; i++ )
				{
					sCELLINFO* pItemInfo = pInfo->GetCellInfo( i );
					if( pItemInfo )
					{
						msg.sInfo[i].dwItemIdx = pItemInfo->sItemBase.wIconIdx;
						msg.sInfo[i].dwPrice = pItemInfo->dwMoney;
						msg.sInfo[i].dwRareIdx = pItemInfo->sItemBase.RareIdx;
						msg.sInfo[i].dwDBIdx = pItemInfo->sItemBase.dwDBIdx;
						if( eSK_SELL == pInfo->GetStallKind() )
							msg.sInfo[i].dwDur = (DWORD)(pItemInfo->sItemBase.Durability);
						else
						{
							//   ƴ üũ
							if( FALSE == ITEMMGR->IsDupItem(msg.sInfo[i].dwItemIdx) )
								msg.sInfo[i].dwDur = (DWORD)(pItemInfo->sItemBase.Durability);
							else
								msg.sInfo[i].dwDur = pItemInfo->wVolume;
						}

						if( ITEMMGR->IsOptionItem(msg.sInfo[i].dwItemIdx, msg.sInfo[i].dwDur) )
						{
							memcpy(&OptionInfo[OptionNum], pInfo->GetOwner()->GetItemOption(msg.sInfo[i].dwDur), sizeof(ITEM_OPTION_INFO));
							++OptionNum;
						}
						if( ITEMMGR->IsRareOptionItem(msg.sInfo[i].dwItemIdx, msg.sInfo[i].dwRareIdx) )
						{
							memcpy(&RareOptionInfo[RareOptionNum], pInfo->GetOwner()->GetItemRareOption(msg.sInfo[i].dwRareIdx), sizeof(ITEM_RARE_OPTION_INFO));
							++RareOptionNum;
						}
						if( ITEMMGR->IsPetSummonItem((WORD)msg.sInfo[i].dwItemIdx))
						{
							memcpy(&PetInfo[PetInfoNum], pInfo->GetOwner()->GetPetManager()->GetPetTotalInfo(msg.sInfo[i].dwDBIdx), sizeof(PET_TOTALINFO));
							++PetInfoNum;
						}
						if( ITEMMGR->IsTitanCallItem(msg.sInfo[i].dwItemIdx))
						{
							memcpy(&TitanInfo[TitanInfoNum], pInfo->GetOwner()->GetTitanManager()->GetTitanTotalInfo(msg.sInfo[i].dwDBIdx), sizeof(TITAN_TOTALINFO));
							++TitanInfoNum;
						}
						if( ITEMMGR->IsTitanEquipItem(msg.sInfo[i].dwItemIdx))
						{
							TITAN_ENDURANCE_ITEMINFO* pTitanEnduranceInfo = pInfo->GetOwner()->GetTitanManager()->GetTitanItemEnduranceInfo(msg.sInfo[i].dwDBIdx);
							if( pInfo )
							{
								memcpy(&TitanEquipInfo[TitanEquipNum], pTitanEnduranceInfo, sizeof(TITAN_ENDURANCE_ITEMINFO));
								++TitanEquipNum;
							}
							else
							{
								char temp[256] = {0,};
								sprintf( temp, "GetTitanItemEnduranceInfo is NULL : %d", msg.sInfo[i].dwDBIdx );
								ASSERTMSG(0,temp);
							}
						}
					}
				}
				msg.wCount = nMaxCount;

				if( OptionNum )
				{
					msg.AddableInfo.AddInfo( CAddableInfoList::ItemOption, sizeof(ITEM_OPTION_INFO)*OptionNum, OptionInfo );
					msg.wOptioncount = OptionNum;
				}
				if( RareOptionNum )
				{
					msg.AddableInfo.AddInfo( CAddableInfoList::ItemRareOption, sizeof(ITEM_RARE_OPTION_INFO)*RareOptionNum, RareOptionInfo );
					msg.wRareCount = RareOptionNum;
				}
				if( PetInfoNum )
				{
					msg.AddableInfo.AddInfo( CAddableInfoList::PetTotalInfo, sizeof(PET_TOTALINFO)*PetInfoNum, PetInfo );
					msg.wPetItemCount = PetInfoNum;
				}
				if( TitanInfoNum )
				{
					msg.AddableInfo.AddInfo( CAddableInfoList::TitanTotalInfo, sizeof(TITAN_TOTALINFO)*TitanInfoNum, TitanInfo );
					msg.wTitanItemCount = TitanInfoNum;
				}
				if( TitanEquipNum )
				{
					msg.AddableInfo.AddInfo( CAddableInfoList::TitanEndrncInfo, sizeof(TITAN_ENDURANCE_ITEMINFO)*TitanEquipNum, TitanEquipInfo );
					msg.wTitanEquipItemCount = TitanEquipNum;
				}
			}

			pPlayer->SendMsg( &msg, msg.GetMsgSize() );
		}
		break;
	}
}

void cStreetStallManager::SendNackMsg(CPlayer* pPlayer, BYTE Protocol)
{
	MSGBASE nmsg;
	nmsg.Category = MP_STREETSTALL;
	nmsg.Protocol = Protocol;
	pPlayer->SendMsg(&nmsg, sizeof(nmsg));
}


BOOL cStreetStallManager::CanBuyItem(CPlayer* pOwner, CPlayer *pGuest, sCELLINFO* pItemInfo, WORD* EmptyCellPos, STREETSTALL_BUYINFO* pBuyInfo, ITEMBASE* pStallItem, DWORD dwBuyNum )
{
	//KES:   ȭ
	if( pItemInfo->dwMoney != pBuyInfo->dwBuyPrice )
	{
		AssertBuyErr(NOT_SAMEPRICE);
		return FALSE;
	}

	//   üũ
	if( !pItemInfo || !pItemInfo->bFill || !pItemInfo->bLock || pStallItem->dwDBIdx != pBuyInfo->ItemInfo.dwDBIdx )
	{
		AssertBuyErr(NOT_EXIST);
		return FALSE;
	}

	// HAVE MONEY ?
	if( pGuest->GetMoney() < pItemInfo->dwMoney * dwBuyNum )
	{
		AssertBuyErr(NOT_MONEY);
		return FALSE;
	}

	// RaMa - ShopItemInven
	CItemSlot* pGuestInventory;

	ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( pItemInfo->sItemBase.wIconIdx );
	if( !pItemKindInfo )	return FALSE;

	eITEMTABLE Table = eItemTable_Inventory;
	if( pItemKindInfo->ItemKind &eSHOP_ITEM )
	{
		pGuestInventory	= (CShopInvenSlot*)pGuest->GetSlot( eItemTable_ShopInven );
		Table = eItemTable_ShopInven;
	}
	else
	{
		pGuestInventory	= (CInventorySlot*)pGuest->GetSlot( eItemTable_Inventory );
	}	
	//

	// κ丮 üũ
	if( pGuestInventory->GetEmptyCell(EmptyCellPos) == 0 ) 
	{
		AssertBuyErr(NOT_SPACE);
		return FALSE;
	}
	if( !pOwner->IsEnoughAdditionMoney(pItemInfo->dwMoney) )
	{
		AssertBuyErr(NOT_OWNERMONEY);
		
		MSG_WORD msg;
		msg.Category = MP_STREETSTALL;
		msg.Protocol = MP_STREETSTALL_SELLITEM_ERROR;
		msg.wData = pBuyInfo->StallPos;
		pOwner->SendMsg(&msg, sizeof(msg));

		return FALSE;
	}

	return TRUE;
}

BOOL cStreetStallManager::CanSellItem(CPlayer* pOwner, CPlayer *pGuest, sCELLINFO* pItemInfo, WORD* EmptyCellPos, STREETSTALL_BUYINFO* pSellInfo, ITEMBASE* pSellItem, DWORD dwSellNum, WORD& result )
{
	//KES:   ȭ
	if( pItemInfo->dwMoney != pSellInfo->dwBuyPrice )
	{
		AssertSellErr(NOT_SAMEPRICE);
		return FALSE;
	}
	
	//   üũ
	if( !pItemInfo || !pItemInfo->bFill || pItemInfo->bLock || pSellItem->wIconIdx != pItemInfo->sItemBase.wIconIdx || pItemInfo->wVolume < 1)
	{
		AssertSellErr(NOT_EXIST);
		result = NOT_EXIST;

		return FALSE;
	}
	
	// HAVE MONEY ?
	if( pOwner->GetMoney() < pItemInfo->dwMoney * dwSellNum )
	{
		AssertSellErr(NOT_MONEY);
		result = NOT_MONEY;

		MSG_WORD msg;
		msg.Category = MP_STREETSTALL;
		msg.Protocol = MP_STREETSTALL_BUYITEM_ERROR;
		msg.wData = NOT_MONEY;
		pOwner->SendMsg(&msg, sizeof(msg));
			
		return FALSE;
	}

	//2007. 8. 30. CBH - Ĵ  ݾ ִġ  üũ ߰
	if(pGuest->IsEnoughAdditionMoney(pItemInfo->dwMoney) == FALSE)
	{
		return FALSE;
	}
	
	
	// RaMa - ShopItemInven
	CItemSlot* pOwnerInventory;
	
	ITEM_INFO* pItemKindInfo = ITEMMGR->GetItemInfo( pItemInfo->sItemBase.wIconIdx );
	if( !pItemKindInfo )	return FALSE;
	
	eITEMTABLE Table = eItemTable_Inventory;
	if( pItemKindInfo->ItemKind & eSHOP_ITEM )
	{
		pOwnerInventory	= (CInventorySlot*)pOwner->GetSlot( eItemTable_ShopInven );
		Table = eItemTable_ShopInven;
		
		// ȯ üũ
		CShopInvenSlot* pGuestInventory	= (CShopInvenSlot*)pGuest->GetSlot( eItemTable_ShopInven );
		if( pItemKindInfo && pItemKindInfo->ItemType == 11 )
		{
			ITEMBASE* pIBase = (ITEMBASE*)pGuestInventory->GetItemInfoAbs( pSellItem->Position );
			if( !pIBase || !(pIBase->ItemParam & ITEM_PARAM_SEAL) )
				return FALSE;
		}
		// ȯҰ  ŷ ȵǵ
		if( pItemKindInfo->ItemKind == eSHOP_ITEM_EQUIP || pItemKindInfo->ItemKind == eSHOP_ITEM_PET_EQUIP )
		{
			if( pItemKindInfo->BuyPrice )
				return FALSE;
		}
		else if( pItemKindInfo->MeleeAttackMax )
			return FALSE;
	}
	else
	{
		pOwnerInventory	= (CInventorySlot*)pOwner->GetSlot( eItemTable_Inventory );
	}	
	//
	
	// κ丮 üũ
	if( pOwnerInventory->GetEmptyCell(EmptyCellPos) == 0 ) 
	{
		AssertSellErr(NOT_SPACE);
		result = NOT_SPACE;

		MSG_WORD msg;
		msg.Category = MP_STREETSTALL;
		msg.Protocol = MP_STREETSTALL_BUYITEM_ERROR;
		msg.wData = NOT_SPACE;
		pOwner->SendMsg(&msg, sizeof(msg));
		
		return FALSE;
	}
/*	if( !pGuest->IsEnoughAdditionMoney(pItemInfo->dwMoney) )
	{
		AssertSellErr(NOT_OWNERMONEY);
		
		MSG_WORD msg;
		msg.Category = MP_STREETSTALL;
		msg.Protocol = MP_STREETSTALL_SELLITEM_NACK;
		msg.wData = pSellInfo->StallPos;
		pGuest->SendMsg(&msg, sizeof(msg));
		
		return FALSE;
	}
*/	

	pItemInfo->bLock = TRUE;
	
	return TRUE;
}

void cStreetStallManager::AssertBuyErr(WORD Err)
{
	char buf[128];
	sprintf(buf, "  : %d", Err);
	ASSERTMSG(0, buf);
}

void cStreetStallManager::AssertSellErr(WORD Err)
{
	char buf[128];
	sprintf(buf, " ȱ : %d", Err);
	ASSERTMSG(0, buf);
}

// magi82  ˻ ->  ˻ ޸Ǯ, ̺ ʱȭ
void cStreetStallManager::InitStallData()
{
	memset( &m_dwSearchDelayTime, 0, sizeof(m_dwSearchDelayTime) );

	// Ǹ 
	m_pDealItemPool[eFind_Sell] = new CMemoryPoolTempl<STALL_DEALITEM_INFO>;
	m_pDealItemPool[eFind_Sell]->Init( 100, 50, "SellStallDealerPool" );
	m_pDealerPool[eFind_Sell] = new CMemoryPoolTempl<STALL_DEALER_INFO>;
	m_pDealerPool[eFind_Sell]->Init( 100, 50, "SellStallDealItemPool" );
	m_pDealItemTable[eFind_Sell].Initialize(1000);

	//  
	m_pDealItemPool[eFind_Buy] = new CMemoryPoolTempl<STALL_DEALITEM_INFO>;
	m_pDealItemPool[eFind_Buy]->Init( 100, 50, "BuyStallDealerPool" );
	m_pDealerPool[eFind_Buy] = new CMemoryPoolTempl<STALL_DEALER_INFO>;
	m_pDealerPool[eFind_Buy]->Init( 100, 50, "BuyStallDealItemPool" );
	m_pDealItemTable[eFind_Buy].Initialize(1000);
}

// magi82  ˻ ->  ˻ ޸Ǯ, ̺ 
void cStreetStallManager::ReleaseStallData()
{
	for( int i = 0; i < eFind_Max; i++ )
	{
		STALL_DEALITEM_INFO* pDealItem;
		m_pDealItemTable[i].SetPositionHead();
		while( pDealItem = m_pDealItemTable[i].GetData() )
		{
			//  Ĵ  
			if( pDealItem->Info.GetDataNum() )
			{
				// ش  Ĵ   
				STALL_DEALER_INFO* pDealer;
				pDealItem->Info.SetPositionHead();
				while( pDealer = pDealItem->Info.GetData() )
				{
					m_pDealerPool[i]->Free(pDealer);
				}
				pDealItem->Info.RemoveAll();
			}

			m_pDealItemPool[i]->Free(pDealItem);
		}
		m_pDealItemTable[i].RemoveAll();

		SAFE_DELETE(m_pDealerPool[i]);
		SAFE_DELETE(m_pDealItemPool[i]);
	}
}

// magi82  ˻ ->  ˻  
void cStreetStallManager::ResetStallData( DWORD dwOwnerIdx, StreetStallFindState State )
{
	cStreetStall* pStall = m_StallTable.GetData(dwOwnerIdx);
	if( !pStall )
		return;

	int nCount = 0;
	if( pStall->GetStallKind() == eSK_SELL )
		nCount = pStall->GetCurRegistItemNum();
	else if( pStall->GetStallKind() == eSK_BUY )
		nCount = MAX_STREETBUYSTALL_CELLNUM;

	STALL_DEALITEM_INFO* pDealItem;
	STALL_DEALER_INFO* pDealer;
	// ش   ִ    ̺ ؾ Ѵ.
	for( int i = 0; i < nCount; i++ )
	{
		sCELLINFO* pCell = pStall->GetCellInfo(i);
		// ش  ̺ ִ°?
		pDealItem = m_pDealItemTable[State].GetData( pCell->sItemBase.wIconIdx );
		if( pDealItem )
		{
			// ش    ִ°?
			pDealer = pDealItem->Info.GetData( dwOwnerIdx );
			if( pDealer )
			{
				//  ִ   ְ ̺ 
				m_pDealerPool[State]->Free(pDealer);
				pDealItem->Info.Remove(pDealer->dwCharIdx);
				pDealer = NULL;

				// ̺  üũؼ ̺   ̻ ش  Ĵ  °̹Ƿ  ̺ ش.
				if( 0 == pDealItem->Info.GetDataNum() )
				{
					m_pDealItemPool[State]->Free(pDealItem);
					m_pDealItemTable[State].Remove(pDealItem->ItemIdx);
					pDealItem = NULL;
				}
			}
		}
	}
}

// magi82  ˻ ->  ˻  ߰
void cStreetStallManager::InsertStallData( DWORD dwOwnerIdx, DWORD dwItemIdx, StreetStallFindState State, DWORD dwMoney )
{
	StreetStallPriceState ePriceState;
	// Ǹ ̸ ְ,  ̸ 
	if( State == eFind_Sell )
	{
		ePriceState = ePrice_Max;
	}
	else if( State == eFind_Buy )
	{
		ePriceState = ePrice_Min;
	}

	STALL_DEALITEM_INFO* pDealItem = m_pDealItemTable[State].GetData(dwItemIdx);
	if( pDealItem )
	{
		STALL_DEALER_INFO* pDealer = pDealItem->Info.GetData(dwOwnerIdx);
		if( pDealer )
		{
			DWORD dwPrice = GetPrice( dwOwnerIdx, dwItemIdx, ePriceState );

			if( dwPrice > 0 )
			{
				pDealer->dwPrice = dwPrice;
			}
		}
		else
		{
			pDealer = m_pDealerPool[State]->Alloc();
			pDealer->dwCharIdx = dwOwnerIdx;
			pDealer->dwPrice = dwMoney;
			pDealItem->Info.Add( pDealer, pDealer->dwCharIdx );
		}   
	}
	else
	{
		pDealItem = m_pDealItemPool[State]->Alloc();
		pDealItem->ItemIdx = dwItemIdx;
		pDealItem->Info.Initialize(100);

		STALL_DEALER_INFO* pDealer = m_pDealerPool[State]->Alloc();
		pDealer->dwCharIdx = dwOwnerIdx;
		pDealer->dwPrice = dwMoney;
		pDealItem->Info.Add( pDealer, pDealer->dwCharIdx );

		m_pDealItemTable[State].Add( pDealItem, pDealItem->ItemIdx );
	}

}

// magi82  ˻ ->  ˻  
void cStreetStallManager::DeleteStallData( DWORD dwOwnerIdx, DWORD dwItemIdx, StreetStallFindState State )
{
	StreetStallPriceState ePriceState;
	// Ǹ ̸ ְ,  ̸ 
	if( State == eFind_Sell )
	{
		ePriceState = ePrice_Max;
	}
	else if( State == eFind_Buy )
	{
		ePriceState = ePrice_Min;
	}

	STALL_DEALITEM_INFO* pDealItem = m_pDealItemTable[State].GetData(dwItemIdx);
	if( pDealItem )
	{
		STALL_DEALER_INFO* pDealer = pDealItem->Info.GetData(dwOwnerIdx);
		if( pDealer )
		{
			DWORD dwPrice = GetPrice( dwOwnerIdx, dwItemIdx, ePriceState );

			if( dwPrice > 0 )
			{
				pDealer->dwPrice = dwPrice;
			}
			else
			{
				m_pDealerPool[State]->Free(pDealer);
				pDealItem->Info.Remove(pDealer->dwCharIdx);
				pDealer = NULL;
			}

			if( 0 == pDealItem->Info.GetDataNum() )
			{
				m_pDealItemPool[State]->Free(pDealItem);
				m_pDealItemTable[State].Remove(pDealItem->ItemIdx);
				pDealItem = NULL;
			}
		}
	}
}

// magi82  ˻ -> ش  ְ,  
DWORD cStreetStallManager::GetPrice( DWORD dwOwnerIdx, DWORD dwItemIdx, StreetStallPriceState State )
{
	cStreetStall* pStall = m_StallTable.GetData(dwOwnerIdx);
	if( !pStall )
		return 0;

	DWORD dwPrice = 0;
	if( State == ePrice_Max )
	{
		for( int i = 0; i < pStall->GetCurRegistItemNum(); i++ )
		{
			sCELLINFO* pCell = pStall->GetCellInfo(i);
			if( pCell->sItemBase.wIconIdx == dwItemIdx && pCell->dwMoney > dwPrice )
			{
				dwPrice = pCell->dwMoney;
			}
		}
	}
	else if( State == ePrice_Min )
	{
		for( int i = 0; i < MAX_STREETBUYSTALL_CELLNUM; i++ )
		{
			sCELLINFO* pCell = pStall->GetCellInfo(i);
			if( pCell->sItemBase.wIconIdx == dwItemIdx && ( dwPrice == 0 || pCell->dwMoney < dwPrice ) )
			{
				dwPrice = pCell->dwMoney;
			}
		}
	}

    return dwPrice;    
}

// magi82  ˻ ->  ˻  üũ
BOOL cStreetStallManager::CheckDelayTime( DWORD dwTime, StreetStallDelayState State )
{
	if( m_dwSearchDelayTime[State] > gCurTime )
		return FALSE;

	m_dwSearchDelayTime[State] = gCurTime + dwTime;
    return TRUE;
}
