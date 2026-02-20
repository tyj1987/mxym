//----------------------------------------------------------------------------------------------------
//  StallFindDlg   version:  1.0   ��  date: 03/31/2008
//
//  Copyright (C) 2008 - All Rights Reserved
//----------------------------------------------------------------------------------------------------
///	Index	Stiner(8)
///	@file	StallFindDlg.cpp
///	@author	�̼���
///	@brief	���� �˻��� ���� ���̾�α�
//----------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include ".\stallfinddlg.h"
#include "WindowIDEnum.h"
#include "./Interface\cListDialog.h"
#include "./Interface\cButton.h"
#include "./Interface\cStatic.h"
#include "./interface/cComboBox.h"
#include "./interface/cPushupButton.h"
#include "./StallKindSelectDlg.h"
#include "./Input/Mouse.h"
#include "./Interface/cWindowManager.h"
#include "MHFile.h"
#include "ItemManager.h"
#include "cResourceManager.h"
#include "GameResourceStruct.h"
#include "PtrList.h"
#include "cIMEex.h"
#include "ChatManager.h"
#include "cMsgBox.h"
#include "GameIn.h"
#include "./Interface/cScriptManager.h"
#include "ObjectManager.h"
#include "StreetstallItemViewDlg.h"
#include "StreetBuyStall.h"
#include "StreetStall.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStallFindDlg::CStallFindDlg(void)
{
	ZeroMemory( m_arrStallInfo, MAX_STALLITEM_NUM * sizeof(STREETSTALL_PRICE_INFO) );
	m_nStallCount = 0;

	m_pItemTypeCombo = NULL;
	ZeroMemory( m_arrItemDetailTypeCombo, 2 * sizeof(cComboBox*) );
	m_pItemList = NULL;
	m_pClassList = NULL;	

	m_pNameStatic = NULL;

	m_pStallList = NULL;
	ZeroMemory( m_parrPageBtn, MAX_RESULT_PAGE * sizeof(cPushupButton*) );
	ZeroMemory( m_parrPageUpDownBtn, 2 * sizeof(cButton*) );

	m_dwSearchType = eSK_SELL;

	m_nCurrentPage = -1;
	m_nBasePage = 0;
	m_nMaxPage = 0;

	m_nItemType = 0;
	m_nItemDetailType = 0;
	m_nSelectedItemListIdx = -1;
	m_nSelectedClassListIdx = -1;
	m_nSelectedStallListIdx = -1;
}

CStallFindDlg::~CStallFindDlg(void)
{
	PTRLISTSEARCHSTART(m_ptrItemInfo, TItemInfo*, pItemInfo);
	m_ptrItemInfo.Remove( (void*)pItemInfo );
	delete pItemInfo;
	PTRLISTSEARCHEND
	m_ptrItemInfo.RemoveAll();
}

void CStallFindDlg::LoadItemList()
{
	CMHFile	fp;
	// ������ ����Ʈ ��ũ��Ʈ �ε�
#ifdef _FILE_BIN_
	if( !fp.Init( "./Resource/Client/SFList.bin", "rb" ) )	return;
#else
	if( !fp.Init( ".\\Resource\\SFList.txt", "rt" ) )	return;
#endif

	// ������ ����Ʈ �Ľ�
	while( !fp.IsEOF() )
	{
		TItemInfo * itemInfo = new TItemInfo();

		itemInfo->Type			= fp.GetWord();
		itemInfo->DetailType	= fp.GetWord();
		itemInfo->ItemIdx		= fp.GetWord();

		m_ptrItemInfo.AddTail( itemInfo );
	}

	fp.Release();

	UpdateItemList();
}

void CStallFindDlg::Linking()
{
	int i;	// for���� ���� ���� ��,.��a
	m_pItemTypeCombo = (cComboBox*)GetWindowForID(SFR_TYPECOMBO);

	for( i = 0 ; i < ITEM_TYPE_COUNT ; i++ )
	{
		m_arrItemDetailTypeCombo[WEAPON + i] = (cComboBox*)GetWindowForID(SFR_WEAPON_DETAILTYPECOMBO + i);
		m_arrItemDetailTypeCombo[WEAPON + i]->SetActive(FALSE);
	}
	m_nItemType = 0;
	m_arrItemDetailTypeCombo[WEAPON]->SetActive(TRUE);

	m_pItemList = (cListDialog*)GetWindowForID(SFR_ITEMLIST);
	m_pClassList = (cListDialog*)GetWindowForID(SFR_CLASSLIST);

	m_pSellModeRadioBtn = (cPushupButton*)GetWindowForID(SFR_PB_SELLMODE);
	m_pBuyModeRadioBtn = (cPushupButton*)GetWindowForID(SFR_PB_BUYMODE);

	m_pNameStatic = (cStatic*)GetWindowForID(SFR_NAMESTATIC);
	m_pNameStatic->SetStaticText( RESRCMGR->GetMsg(1023) );
	m_pPriceStatic = (cStatic*)GetWindowForID(SFR_PRICESTATIC);
	m_pPriceStatic->SetStaticText( RESRCMGR->GetMsg(1026) );

	m_pStallList = (cListDialog*)GetWindowForID(SFR_RESULTLIST);

	for( i = 0; i < MAX_RESULT_PAGE; i++ )
		m_parrPageBtn[i] = (cPushupButton*)GetWindowForID(SFR_RESULTPAGEBTN1 + i);

	m_parrPageUpDownBtn[0] = (cButton*)GetWindowForID(SFR_RESULTPAGEBTNUP);
	m_parrPageUpDownBtn[1] = (cButton*)GetWindowForID(SFR_RESULTPAGEBTNDOWN);

	m_pItemList->SetShowSelect(TRUE);
	m_pClassList->SetShowSelect(TRUE);
	m_pStallList->SetShowSelect(TRUE);

	LoadItemList();
}

void CStallFindDlg::SetSearchType(DWORD val)
{
	m_dwSearchType = val;
}

void CStallFindDlg::SetStallPriceInfo( SEND_STREETSTALL_INFO * pStallInfo )
{
	// ������ �˻� ����� ������
	if( pStallInfo->wCount <= 0 )
	{
		if( m_bSearchedAll )
			WINDOWMGR->MsgBox( MBI_OK, MBT_OK, CHATMGR->GetChatMsg(1698) );
		else
			WINDOWMGR->MsgBox( MBI_OK, MBT_OK, CHATMGR->GetChatMsg(1695) );
	}

	// ������ ������ ����Ʈ�� ������ �ʱ�ȭ
	m_pStallList->RemoveAll();
	m_nCurrentPage = -1;

	// ������ ������ �ִ� �迭 �ʱ�ȭ
	ZeroMemory( m_arrStallInfo, MAX_STALLITEM_NUM * sizeof(STREETSTALL_PRICE_INFO) );
	m_nStallCount = pStallInfo->wCount;

	// ������ ����
	for( int i = 0 ; i < m_nStallCount ; i++ )
		m_arrStallInfo[i] = pStallInfo->sInfo[i];

	// ������ ����
	SortStallList( ( m_dwSearchType == eSK_BUY ) ? TRUE : FALSE );

	// ��ü ������ ����
	m_nMaxPage = m_nStallCount / MAX_LINE_PER_PAGE - 1;
	if( m_nStallCount % MAX_LINE_PER_PAGE )
		++m_nMaxPage;

	// 0�� ��������
	SetPage(0);
}

void CStallFindDlg::UpdateItemList()
{
	char buf[32] = {0,};

	// Ÿ��ź ��� ����Ʈ ����
	m_pClassList->RemoveAll();
	if( m_nItemType < ACCESSORY )
	{
		for( int i = 0 ; i < 10 ; i++ )
		{
			wsprintf(buf, "+%d", i);
			m_pClassList->AddItem(buf, RGB_HALF(255, 255, 255));
		}
	}
	else if( m_nItemType == TITAN_ITEM )
	{
		// Ÿ��ź �������� ��Ÿ�� ���� ���� ����� ����
		if( m_nItemDetailType != 2 )
			for( int i = 0 ; i < 4 ; i++ )
			{
				wsprintf(buf, "+%d", i);
				m_pClassList->AddItem(buf, RGB_HALF(255, 255, 255));
			}
	}
	m_pClassList->ResetGuageBarPos();

	// ������ ����Ʈ ��� ����
	m_pItemList->RemoveAll();
	PTRLISTSEARCHSTART(m_ptrItemInfo, TItemInfo*, pItemInfo);
	if( m_nItemType + 1 == pItemInfo->Type ) // �޺��ڽ� �ε������� 0�϶� +1�� ����, 0���� �Ⱦ���
	{
		if( m_nItemDetailType + 1 == pItemInfo->DetailType ) // �޺��ڽ� �ε������� 0�϶� +1�� ����, 0���� �Ⱦ���
		{
			ITEM_INFO * pItem = ITEMMGR->GetItemInfo( pItemInfo->ItemIdx );
			m_pItemList->AddItem( pItem->ItemName, RGB_HALF(255, 255, 255) );
		}
	}
	PTRLISTSEARCHEND
		m_pItemList->ResetGuageBarPos();
}

void CStallFindDlg::UpdateStallList()
{
	// ����Ʈ ����
	m_pStallList->RemoveAll();

	// ȭ�鿡 ���ϰ͵鸸 ����Ʈ�� �߰�
	int nCount = 0;
	int nViewStartIndex = MAX_LINE_PER_PAGE * m_nCurrentPage;

	while( nCount < m_nStallCount )
	{
		if( nCount >= nViewStartIndex && nCount < nViewStartIndex + MAX_LINE_PER_PAGE )
		{
			char buf[40];
			sprintf( buf, "%-16s%18s", m_arrStallInfo[nCount].strName, AddComma( m_arrStallInfo[nCount].dwPrice ) );

			m_pStallList->AddItem( buf, RGB_HALF(255, 255, 255) );
		}

		++nCount;
	}
}

void CStallFindDlg::SortStallList(BOOL flag)
{
	int k;  // 修复C2065: 在函数作用域声明k
	STREETSTALL_PRICE_INFO temp;
	for( int nIncrement = m_nStallCount ; nIncrement > 0 ; nIncrement /= 2 )
	{
		for( int j = nIncrement ; j < m_nStallCount ; j++ )
		{
			temp = m_arrStallInfo[j];

			for( int k = j ; k >= nIncrement ; k -= nIncrement )
			{
				if( flag )	// ��������
				{
					if( temp.dwPrice > m_arrStallInfo[ k - nIncrement ].dwPrice )
						m_arrStallInfo[k] = m_arrStallInfo[ k - nIncrement ];
					else
						break;
				}
				else		// ��������
				{
					if( temp.dwPrice < m_arrStallInfo[ k - nIncrement ].dwPrice )
						m_arrStallInfo[k] = m_arrStallInfo[ k - nIncrement ];
					else
						break;
				}
			}

			m_arrStallInfo[k] = temp;
		}
	}
}

void CStallFindDlg::SetBasePage(BOOL bNext)
{
	int BasePageBackup = m_nBasePage;

	if( bNext )
	{
		if( m_nBasePage + MAX_RESULT_PAGE <= m_nMaxPage )
			m_nBasePage += MAX_RESULT_PAGE;
	}
	else
	{
		if( m_nBasePage - MAX_RESULT_PAGE >= 0 )
			m_nBasePage -= MAX_RESULT_PAGE;
	}

	if( BasePageBackup != m_nBasePage )
		SetPage(0);
}

BOOL CStallFindDlg::CheckDelay(DWORD dwDelayTime, int nID)
{
	static DWORD	dwPrevTime[5];

	if( dwPrevTime[nID] == 0 )
		dwPrevTime[nID] = gCurTime;
	else
	{
		// dwDelayTime ����
		if( ( gCurTime - dwPrevTime[nID] ) > dwDelayTime )
		{
			dwPrevTime[nID] = 0;
			return TRUE;
		}
		// dwDelayTime ����
		else	
		{
			return FALSE;
		}
	}

	return TRUE;	
}

void CStallFindDlg::SetPage( int index )
{
	int ShowPage = m_nMaxPage - m_nBasePage;

	if( ShowPage > 4 )
		ShowPage = 4;

	char buf[16] = { 0, };
	for( int i = 0 ; i < MAX_RESULT_PAGE ; i++ )
	{
		if( i > ShowPage + 1 || m_nBasePage + i > m_nMaxPage )
			m_parrPageBtn[i]->SetActive( FALSE );
		else
		{
			sprintf( buf, "%d", m_nBasePage + i + 1 );
			m_parrPageBtn[i]->SetText( buf, RGB_HALF(255, 255, 255) );
			m_parrPageBtn[i]->SetActive( TRUE );
			m_parrPageBtn[i]->SetPush( FALSE );
		}
	}
	m_parrPageBtn[index]->SetPush( TRUE );

	// Page Down
	if( m_nBasePage >= MAX_RESULT_PAGE )
		m_parrPageUpDownBtn[0]->SetActive( TRUE );
	else
		m_parrPageUpDownBtn[0]->SetActive( FALSE );
	// Page Up
	if( ( m_nBasePage + MAX_RESULT_PAGE ) <= m_nMaxPage )
		m_parrPageUpDownBtn[1]->SetActive( TRUE );
	else
		m_parrPageUpDownBtn[1]->SetActive( FALSE );

	// ���� ��������ȣ�� ��������
	if( ( m_nBasePage + index ) == m_nCurrentPage )
		return;

	m_nCurrentPage = m_nBasePage + index;

	UpdateStallList();
}

void CStallFindDlg::SetActive(BOOL val)
{
	if( !m_bDisable )
	{
		if( val == FALSE )
			OnClose();
		else
			UpdateItemList();
	}

	cDialog::SetActive(val);
}

void CStallFindDlg::OnActionEvent( LONG lId, void * p, DWORD we )
{
	switch(lId)
	{
	case SFR_TYPECOMBOBTN:
		break;

	case SFR_TYPECOMBO:
		if( we == WE_COMBOBOXSELECT )
			OnEventTypeCombo(lId, p, we);
		break;

	case SFR_DETAILTYPECOMBOBTN:
		break;

	case SFR_WEAPON_DETAILTYPECOMBO:
	case SFR_CLOTHES_DETAILTYPECOMBO:
	case SFR_ACCESSORY_DETAILTYPECOMBO:
	case SFR_POTION_DETAILTYPECOMBO:
	case SFR_MATERIAL_DETAILTYPECOMBO:
	case SFR_ETC_DETAILTYPECOMBO:
	case SFR_ITEMMALL_DETAILTYPECOMBO:
	case SFR_TITAN_DETAILTYPECOMBO:
		if( we == WE_COMBOBOXSELECT )
			OnEventDetailTypeCombo(lId, p, we);
		break;

	case SFR_ITEMLIST:
		OnEventItemList(lId, p, we);
		break;

	case SFR_CLASSLIST:
		OnEventClassList(lId, p, we);
		break;

	case SFR_SEARCHBTN:
		OnClickSearchBtn();
		break;

	case SFR_SEARCHALLBTN:
		OnClickSearchAllBtn();
		break;

	case SFR_PB_SELLMODE:
	case SFR_PB_BUYMODE:
		OnClickFindTypeBtn(lId, p, we);
		break;

	case SFR_RESULTLIST:
		OnEventStallList(lId, p, we);
		break;

	case SFR_RESULTPAGEBTN1:
	case SFR_RESULTPAGEBTN2:
	case SFR_RESULTPAGEBTN3:
	case SFR_RESULTPAGEBTN4:
	case SFR_RESULTPAGEBTN5:
		OnClickPageBtn(lId);
		break;

	case SFR_RESULTPAGEBTNUP:
	case SFR_RESULTPAGEBTNDOWN:
		OnClickPageUpDonwBtn(lId);
		break;

	case CMI_CLOSEBTN:
		OnClose();
		break;
	}
}


void CStallFindDlg::OnClickSearchBtn()
{
	// �����ִ� ������ �� â �ݱ�
	if( TRUE == GAMEIN->GetStreetStallItemViewDlg()->IsActive() )
		GAMEIN->GetStreetStallItemViewDlg()->SetActive( FALSE );

	PTRLISTPOS pos = m_pItemList->GetListItem()->FindIndex( m_nSelectedItemListIdx );
	if( !pos )	// ���õ� �������� �ִ��� Ȯ��
	{
		// �������� ������ �ȵǾ� ����.. �޼��� �ڽ� ����..
		WINDOWMGR->MsgBox( MBI_OK, MBT_OK, CHATMGR->GetChatMsg(1691) );
		return;
	}

	// ����� ���� �������� ����� 0���� �ʱ�ȭ
	// Ȥ�ó� �ؼ� �ѹ���!
	if( m_nItemType >= ACCESSORY && m_nItemType != TITAN_ITEM )
		m_nSelectedClassListIdx = 0;

	if( m_nSelectedClassListIdx == -1 )
	{
		// ����� ������ �ȵǾ� ����.. �޼��� �ڽ� ����..
		WINDOWMGR->MsgBox( MBI_OK, MBT_OK, CHATMGR->GetChatMsg(1691) );
		return;
	}

	// ������ üũ
	if( CheckDelay( SEARCH_DELAY, 0 ) == FALSE )
	{
		WINDOWMGR->MsgBox( MBI_OK, MBT_OK, CHATMGR->GetChatMsg(1694) );
		return;
	}

	ITEM* pItem = (ITEM*)( m_pItemList->GetListItem()->GetAt( pos ) );
	if( !pItem )
		ASSERT( !pos );

	// ������ ���� ���
	ITEM_INFO *pItemInfo;
	if( m_nSelectedClassListIdx > 0 && m_nSelectedClassListIdx < 10 )
	{
		char ItemName[128] = { 0, };
		wsprintf( ItemName, "%s+%d", pItem->string, m_nSelectedClassListIdx );

		pItemInfo = ITEMMGR->FindItemInfoForName( ItemName );
	}
	else
		pItemInfo = ITEMMGR->FindItemInfoForName( pItem->string );

	if( pItemInfo )
	{
		// ������ �޼��� ����
		MSG_DWORD2	msg;
		ZeroMemory( &msg, sizeof(MSG_DWORD2) );
		msg.Category = MP_STREETSTALL;
		msg.Protocol = MP_STREETSTALL_FINDITEM_SYN;
		msg.dwObjectID = HEROID;
		msg.dwData1 = pItemInfo->ItemIdx;
		msg.dwData2 = m_dwSearchType;
		NETWORK->Send( &msg, sizeof(MSG_DWORD2) );

		// ��ü�˻��� �ƴ����� ����
		m_bSearchedAll = FALSE;

		// ���� �����Ҷ������� Disable
		SetDisable( TRUE );
	}
}


void CStallFindDlg::OnClickSearchAllBtn()
{
	// �����ִ� ������ �� â �ݱ�
	if( TRUE == GAMEIN->GetStreetStallItemViewDlg()->IsActive() )
		GAMEIN->GetStreetStallItemViewDlg()->SetActive( FALSE );

	// ������ üũ
	if( CheckDelay( SEARCH_DELAY, 0 ) == FALSE )
	{
		WINDOWMGR->MsgBox( MBI_OK, MBT_OK, CHATMGR->GetChatMsg(1694) );
		return;
	}

	// �����˻��� �޼��� �������� ������ ����
	MSG_DWORD2	msg;
	ZeroMemory( &msg, sizeof(MSG_DWORD2) );
	msg.Category = MP_STREETSTALL;
	msg.Protocol = MP_STREETSTALL_FINDITEM_SYN;
	msg.dwObjectID = HEROID;
	msg.dwData1 = NULL;		// ��ü�˻��� ������ �ε��� ���� NULL�� ����
	msg.dwData2 = m_dwSearchType;
	NETWORK->Send( &msg, sizeof(MSG_DWORD2) );

	// ��ü�˻����� ����
	m_bSearchedAll =  TRUE;

	// ���� �����Ҷ������� Disable
	SetDisable( TRUE );
}

void CStallFindDlg::OnClickFindTypeBtn(LONG lId, void * p, DWORD we)
{
	// ���� ��ư ó��
	if( we & WE_PUSHUP )	// ������ �� ������
	{
		if( lId == SFR_PB_SELLMODE )
			m_pSellModeRadioBtn->SetPush( TRUE );
		if( lId == SFR_PB_BUYMODE )
			m_pBuyModeRadioBtn->SetPush( TRUE );
	}
	else if( we & WE_PUSHDOWN )	// �ٸ��� ������
	{
		if( lId == SFR_PB_SELLMODE )
			m_pBuyModeRadioBtn->SetPush( FALSE );
		else if( lId == SFR_PB_BUYMODE )
			m_pSellModeRadioBtn->SetPush( FALSE );
	}

	// ���� ����Ʈ�� �ؽ�Ʈ ����
	if( lId == SFR_PB_BUYMODE )
	{
		m_dwSearchType = eSK_BUY;

		// static �ؽ�Ʈ ����
		m_pNameStatic->SetStaticText( RESRCMGR->GetMsg( 1024 ) );
		m_pPriceStatic->SetStaticText( RESRCMGR->GetMsg( 1027 ) );
	}
	else if( lId == SFR_PB_SELLMODE )
	{
		m_dwSearchType = eSK_SELL;

		// static �ؽ�Ʈ ����
		m_pNameStatic->SetStaticText( RESRCMGR->GetMsg( 1023 ) );
		m_pPriceStatic->SetStaticText( RESRCMGR->GetMsg( 1026 ) );
	}

	// ������ ������ ����Ʈ�� ������ �ʱ�ȭ
	m_pStallList->RemoveAll();
	m_nStallCount = 0;
	m_nCurrentPage = -1;
	m_nMaxPage = -1;
	m_nBasePage = 0;

	// ������ ������ �ִ� �迭 �ʱ�ȭ
	ZeroMemory( m_arrStallInfo, MAX_STALLITEM_NUM * sizeof(STREETSTALL_PRICE_INFO) );

	SetPage(0);
}

void CStallFindDlg::OnEventTypeCombo(LONG lId, void * p, DWORD we)
{
	// ������ ���� �Ǿ��� ����Ʈ �ε��� ���� �ʱ�ȭ
	m_nSelectedClassListIdx = -1;
	m_nSelectedItemListIdx = -1;
	m_nItemDetailType = 0;

	int nPrevItemType = m_nItemType;

	if( m_nItemType != -1 )
	{
		m_arrItemDetailTypeCombo[nPrevItemType]->SelectComboText(0);
		m_arrItemDetailTypeCombo[nPrevItemType]->SetActive(FALSE);
		m_nItemType = m_pItemTypeCombo->GetCurSelectedIdx();

		// ù ���̾�α� â �㶧 �� �޺���ư�� ������ -1������ ������ �ǹ���, �װ��� �з��� ��
		if( m_nItemType != -1 )
		{
			m_arrItemDetailTypeCombo[m_nItemType]->SetActive(TRUE);
		}
		else
		{
			m_arrItemDetailTypeCombo[nPrevItemType]->SetActive(TRUE);
			m_nItemType = nPrevItemType;
		}
	}
	else
		m_nItemType = nPrevItemType;

	UpdateItemList();
}


void CStallFindDlg::OnEventDetailTypeCombo(LONG lId, void * p, DWORD we)
{
	switch( lId )
	{
	case SFR_WEAPON_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[WEAPON]->GetCurSelectedIdx();
		break;
	case SFR_CLOTHES_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[CLOTHES]->GetCurSelectedIdx();
		break;
	case SFR_ACCESSORY_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[ACCESSORY]->GetCurSelectedIdx();
		break;
	case SFR_POTION_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[POTION]->GetCurSelectedIdx();
		break;
	case SFR_MATERIAL_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[MATERIAL]->GetCurSelectedIdx();
		break;
	case SFR_ETC_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[ETC]->GetCurSelectedIdx();
		break;
	case SFR_ITEMMALL_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[ITEM_MALL]->GetCurSelectedIdx();
		break;
	case SFR_TITAN_DETAILTYPECOMBO:
		m_nItemDetailType = m_arrItemDetailTypeCombo[TITAN_ITEM]->GetCurSelectedIdx();
		break;
	}

	UpdateItemList();
}


void CStallFindDlg::OnEventItemList(LONG lId, void * p, DWORD we)
{
	m_nSelectedClassListIdx = -1;
	m_pClassList->SetCurSelectedRowIdx( -1 );

	m_nSelectedItemListIdx = m_pItemList->GetCurSelectedRowIdx();
}


void CStallFindDlg::OnEventClassList(LONG lId, void * p, DWORD we)
{
	m_nSelectedClassListIdx = m_pClassList->GetCurSelectedRowIdx();
}


void CStallFindDlg::OnEventStallList( LONG lId, void * p, DWORD we )
{
}


void CStallFindDlg::OnClickPageBtn(LONG lId)
{
	switch( lId )
	{
	case SFR_RESULTPAGEBTN1:
		SetPage( 0 );
		break;

	case SFR_RESULTPAGEBTN2:
		SetPage( 1 );
		break;

	case SFR_RESULTPAGEBTN3:
		SetPage( 2 );
		break;

	case SFR_RESULTPAGEBTN4:
		SetPage( 3 );
		break;

	case SFR_RESULTPAGEBTN5:
		SetPage( 4 );
		break;
	}
}

void CStallFindDlg::OnClickPageUpDonwBtn(LONG lId)
{
	switch( lId )
	{
	case SFR_RESULTPAGEBTNUP:
		SetBasePage( FALSE );
		break;

	case SFR_RESULTPAGEBTNDOWN:
		SetBasePage( TRUE );
		break;
	}
}

void CStallFindDlg::OnClose()
{
	// �����ִ� ������ �� â �ݱ�
	if( TRUE == GAMEIN->GetStreetStallItemViewDlg()->IsActive() )
		GAMEIN->GetStreetStallItemViewDlg()->SetActive( FALSE );

	// ���� ����
	m_pStallList->RemoveAll();

	// �����ʱ�ȭ
	m_pItemTypeCombo->SelectComboText( 0 );
	for( int i = 0 ; i < ITEM_TYPE_COUNT ; i++ )
	{
		m_arrItemDetailTypeCombo[i]->SetActive( ( i == 0 )?TRUE:FALSE );
		m_arrItemDetailTypeCombo[i]->SelectComboText( 0 );
	}

	// ������ ��ư ��Ȱ��ȭ
	for( int j = 0 ; j < MAX_RESULT_PAGE ; j++ )
		m_parrPageBtn[j]->SetActive( FALSE );

	m_parrPageUpDownBtn[0]->SetActive( FALSE );
	m_parrPageUpDownBtn[1]->SetActive( FALSE );

	// ���� ��ư �ʱ�ȭ
	m_pSellModeRadioBtn->SetPush(TRUE);
	m_pBuyModeRadioBtn->SetPush(FALSE);
	m_dwSearchType = eSK_SELL;

	// Static�ؽ�Ʈ �ʱ�ȭ
	m_pNameStatic->SetStaticText( RESRCMGR->GetMsg( 1023 ) );
	m_pPriceStatic->SetStaticText( RESRCMGR->GetMsg( 1026 ) );

	m_nStallCount = 0;
	m_nItemType = 0;
	m_nItemDetailType = 0;
	m_nSelectedItemListIdx = -1;
	m_nSelectedClassListIdx = -1;
	m_nSelectedStallListIdx = -1;
}

DWORD CStallFindDlg::ActionEvent(CMouse * mouseInfo)
{
	if( !m_bActive ) return WE_NULL;

	DWORD we = WE_NULL;
	we |= cWindow::ActionEvent(mouseInfo);

	if( !m_bDisable )
	{
		we |= ActionEventWindow(mouseInfo);
		we |= ActionEventComponent(mouseInfo);
	}

	if( we == ( WE_LBTNDBLCLICK | WE_MOUSEOVER ) )
	{
		if( CheckDelay( ITEMVIEW_DELAY, 1 ) == FALSE )
		{
			CHATMGR->AddMsg( CTC_SYSMSG, CHATMGR->GetChatMsg(1694) );
			return we;
		}

		if( ( m_nSelectedStallListIdx = m_pStallList->GetCurSelectedRowIdx() ) != -1 )
		{
			// ���õ� ������ ��ǳ�� �̹��� ����
			if( 0 < m_dwSelectedObjectIndex )
			{
				CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(m_dwSelectedObjectIndex);
				if( pPlayer )
				{
					pPlayer->SetStreetStallBalloonImage( m_dwPrevSelectedType, FALSE );
				}
			}

			int nStallArrayIndex = MAX_LINE_PER_PAGE * m_nCurrentPage + m_nSelectedStallListIdx;
			m_dwSelectedObjectIndex = m_arrStallInfo[nStallArrayIndex].dwOwnerIdx;
			m_dwPrevSelectedType = m_dwSearchType;

			CPlayer* pPlayer = (CPlayer*)OBJECTMGR->GetObject(m_dwSelectedObjectIndex);
			if( pPlayer )
				pPlayer->SetStreetStallBalloonImage( m_dwPrevSelectedType, TRUE );

			// �޼��� ����
			SendItemViewMsg();
		}
	}

	return we;	
}

void CStallFindDlg::SendItemViewMsg()
{
	// �������� ������ �ȵǾ� ����..
	if( m_nSelectedStallListIdx == -1 )
		return;

	int nStallArrayIndex = MAX_LINE_PER_PAGE * m_nCurrentPage + m_nSelectedStallListIdx;

	// ������ �޼��� ����
	MSG_DWORD	msg;
	ZeroMemory( &msg, sizeof(MSG_DWORD) );
	msg.Category = MP_STREETSTALL;
	msg.Protocol = MP_STREETSTALL_ITEMVIEW_SYN;
	msg.dwObjectID = HEROID;
	msg.dwData = m_arrStallInfo[nStallArrayIndex].dwOwnerIdx;
	NETWORK->Send( &msg, sizeof(MSG_DWORD) );

	// ���� �����Ҷ������� Disable
	SetDisable( TRUE );
}
