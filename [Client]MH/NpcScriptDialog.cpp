#include "stdafx.h"
#include "NpcScriptDialog.h"
#include "NpcScriptManager.h"
#include "cDialogueList.h"
#include "cHyperTextList.h"
#include "WindowIDEnum.h"
#include "ObjectStateManager.h"
#include "ObjectManager.h"

#include "./Interface/cStatic.h"
#include "ChatManager.h"
#include "./Interface/cWindowManager.h"
#include "./Input/UserInput.h"

#include "Quest.h"
#include "QuestManager.h"
#include "ChatManager.h"

cNpcScriptDialog::cNpcScriptDialog()
{
	// LBS 03.10.16
	m_type = WT_NPCSCRIPDIALOG;
	m_nHyperCount = 0;
	m_dwCurNpc = 0;

	m_pListDlg = NULL;
	m_pTitleText = NULL;

	for(int i=0 ; i< MAX_REGIST_HYPERLINK;++i)
		m_sHyper[i].Init();

	m_dwQuestIdx = 0;
}

cNpcScriptDialog::~cNpcScriptDialog()
{
}

void cNpcScriptDialog::Linking()
{
	m_pListDlg = (cListDialogEx*)this->GetWindowForID( NSI_LISTDLG );

	m_pTitleText = new cStatic;
	if( m_pTitleText )
	{
		m_pTitleText->Init( 13, 4, 0, 0, NULL, -1 );
		m_pTitleText->SetShadow( TRUE );
		m_pTitleText->SetFontIdx( 2 );
		m_pTitleText->SetAlign( TXT_LEFT );
		
		m_pTitleText->SetStaticText( "" );
		
		Add( m_pTitleText );
	}
}

BOOL cNpcScriptDialog::OpenDialog( DWORD dwNpcId )
{
	// ÃÊ±âÈ­
	int j;  // ä¿®å¤C2065: åœ¨å‡½æ•°ä½œç”¨åŸŸå£°æ˜j
	for(int i=0 ; i< MAX_REGIST_HYPERLINK;++i)
		m_sHyper[i].Init();

	m_pListDlg->RemoveAll();
	m_nHyperCount = 0;

	// ÆäÀÌÁö¿¡ ´ëÇÑ Á¤º¸¸¦ ¿äÃ»ÇÑ´Ù.
	m_dwCurNpc = dwNpcId; // ¼±ÅÃ Npc¸¦ ¼³Á¤ÇÑ´Ù.

	cPage* pMainPage = NPCSCRIPTMGR->GetMainPage( m_dwCurNpc );
	if( pMainPage == NULL ) return FALSE;

	m_dwCurPageId = pMainPage->GetPageId(); // ÀÓ½Ã·Î ÇöÀç Ç¥½Ã ÆäÀÌÁö¸¦ °¡Áø´Ù.

	// ÆäÀÌÁö Á¤º¸¸¦ ÀÌ¿ëÇÏ¿© ÆäÀÌÁö¸¦ ±¸¼ºÇÑ´Ù.
	DWORD dwMsg = pMainPage->GetRandomDialogue(); // ¸Ş¼¼ÁöÀÇ ¹øÈ£

	// ¸Ş¼¼Áö ¹øÈ£¸¦ ÀÌ¿ëÇÏ¿© »ç¿ëÇÒ ¸Ş¼¼Áö¸¦ °¡Á®¿Â´Ù.
	cDialogueList* pList = NPCSCRIPTMGR->GetDialogueList();
	if( pList == NULL ) return FALSE;

	cHyperTextList* pHyper = NPCSCRIPTMGR->GetHyperTextList();
	if( pHyper == NULL ) return FALSE;

// ´ë»ç Á¤º¸ ¼ÂÆÃ ½ÃÀÛ
	WORD wIdx = 0;
	WORD LineInfo = 0;
	DIALOGUE* temp = NULL;

	LINKITEM* pItem = NULL;
	LINKITEM* Prev = NULL;

	while(1)
	{
		temp = pList->GetDialogue( dwMsg, wIdx );
		if( temp == NULL ) break;

		// ¶óÀÎÁ¤º¸°¡ ÀÖÀ¸¸é ¾ÆÀÌÅÛÀ» Ã·°¡ÇÑ´Ù.(ÇÊ¼ö!!)
		pItem = new LINKITEM;
//		pItem->Init();
		strcpy(pItem->string, temp->str);
		pItem->rgb = temp->dwColor;

		if( Prev == NULL )
		{
			m_pListDlg->cListItem::AddItem(pItem);
			LineInfo = temp->wLine;
		}
		else
		{
			if( LineInfo == temp->wLine )
			{
				Prev->NextItem = pItem;
			}
			else
			{
				m_pListDlg->cListItem::AddItem(pItem);
				LineInfo = temp->wLine;
			}
		}
		
		Prev = pItem;

		++wIdx;
	}


// ¸µÅ© Á¤º¸¸¦ °¡Á®¿Í¼­ ¼³Á¤ÇÑ´Ù. (ÃßÈÄ ÀÛ¾÷ : Á¶°ÇÀ» °Ë»öÇÏ¿© º¸¿©ÁÙ ¸µÅ©¿¡ ´ëÇÑ Ç¥½Ã)
	HYPERLINK* pLink;
	int nLinkCount = pMainPage->GetHyperLinkCount(); 


// ¸µÅ©¸¦ È­¸é¿¡ Ãâ·ÂÇÏ±âÀü¿¡ ¶óÀÎÀ» ¶ç¿î´Ù.
	if( nLinkCount > 0 )
	{
		for( int i=0; i< 3;++i)
		{
			LINKITEM* pItem = new LINKITEM;
//			pItem->Init();
			char* temp = " ";
			strcpy(pItem->string, temp);
			
			m_pListDlg->cListItem::AddItem( pItem );
		}
	}

	// ÇùÇà º¸ÀÏÁö ¸»Áö °áÁ¤
	BOOL bQuest = QUESTMGR->IsNpcRelationQuest( m_dwCurNpc );
/*	DWORD data = QUESTMGR->GetNpcScriptPage( m_dwCurNpc );
	BOOL aa = FALSE;
	if( data == 0 || data == 30 )
		aa = TRUE;
*/
	for( j = 0 ; j < nLinkCount ; ++j )
	{
		pLink = pMainPage->GetHyperText(j);
		if( pLink == NULL ) break;

//		if( pLink->wLinkType == emLink_Quest && aa == TRUE )
		if( pLink->wLinkType == emLink_Quest && !bQuest )
			continue;

		temp = pHyper->GetHyperText( pLink->wLinkId );
		if( temp )
		{
			// Ãß°¡
			LINKITEM* pItem = new LINKITEM;
//			pItem->Init();
			strcpy(pItem->string, temp->str);
			pItem->dwType = pLink->wLinkType;
			
			m_pListDlg->cListItem::AddItem( pItem );

			// Data
			m_sHyper[m_nHyperCount].bUse = TRUE;
			m_sHyper[m_nHyperCount].dwListItemIdx = m_pListDlg->GetItemCount()-1;
			m_sHyper[m_nHyperCount].sHyper = *pLink;
			++m_nHyperCount;
		}
	}

//	SetActiveRecursive( TRUE );
	SetActive( TRUE );

	//¾×¼Ç¿¡¼­ ¹Ì¸® Ã³¸®ÇÔ. - hs
//	OBJECTSTATEMGR->StartObjectState(HERO, eObjectState_Deal);

	CNpc* pObj = NPCSCRIPTMGR->GetSelectedNpc();
	if( pObj )
		SetDlgTitleText( pObj->GetObjectName() );
	else
		SetDlgTitleText( CHATMGR->GetChatMsg(232) );

	return TRUE;
}

BOOL cNpcScriptDialog::OpenLinkPage( DWORD dwPageId )
{
	int j;  // ä¿®å¤C2065: åœ¨å‡½æ•°ä½œç”¨åŸŸå£°æ˜j
	// ÃÊ±âÈ­
	for(int i=0 ; i< MAX_REGIST_HYPERLINK;++i)
		m_sHyper[i].Init();

	m_pListDlg->RemoveAll();
	m_nHyperCount = 0;

	// ÆäÀÌÁö¿¡ ´ëÇÑ Á¤º¸¸¦ ¿äÃ»ÇÑ´Ù.
	cPage* pPage = NPCSCRIPTMGR->GetPage( m_dwCurNpc, dwPageId );
	if( pPage == NULL ) return FALSE;

	m_dwCurPageId = dwPageId; // ÀÓ½Ã·Î ÇöÀç Ç¥½Ã ÆäÀÌÁö¸¦ °¡Áø´Ù.

	// ÆäÀÌÁö Á¤º¸¸¦ ÀÌ¿ëÇÏ¿© ÆäÀÌÁö¸¦ ±¸¼ºÇÑ´Ù.
	DWORD dwMsg = pPage->GetRandomDialogue(); // ¸Ş¼¼ÁöÀÇ ¹øÈ£

	// ¸Ş¼¼Áö ¹øÈ£¸¦ ÀÌ¿ëÇÏ¿© »ç¿ëÇÒ ¸Ş¼¼Áö¸¦ °¡Á®¿Â´Ù.
	cDialogueList* pList = NPCSCRIPTMGR->GetDialogueList();
	if( pList == NULL ) return FALSE;

	cHyperTextList* pHyper = NPCSCRIPTMGR->GetHyperTextList();
	if( pHyper == NULL ) return FALSE;

// ´ë»ç Á¤º¸ ¼ÂÆÃ ½ÃÀÛ
	WORD wIdx = 0;
	WORD LineInfo = 0;
	DIALOGUE* temp = NULL;

	LINKITEM* pItem = NULL;
	LINKITEM* Prev = NULL;

	while(1)
	{
		temp = pList->GetDialogue( dwMsg, wIdx );
		if( temp == NULL ) break;

		// ¶óÀÎÁ¤º¸°¡ ÀÖÀ¸¸é ¾ÆÀÌÅÛÀ» Ã·°¡ÇÑ´Ù.(ÇÊ¼ö!!)
		pItem = new LINKITEM;
//		pItem->Init();
		strcpy(pItem->string, temp->str);
		pItem->rgb = temp->dwColor;

		if( Prev == NULL )
		{
			m_pListDlg->cListItem::AddItem(pItem);
			LineInfo = temp->wLine;
		}
		else
		{
			if( LineInfo == temp->wLine )
			{
				Prev->NextItem = pItem;
			}
			else
			{
				m_pListDlg->cListItem::AddItem(pItem);
				LineInfo = temp->wLine;
			}
		}
		
		Prev = pItem;

		++wIdx;
	}


// ¸µÅ© Á¤º¸¸¦ °¡Á®¿Í¼­ ¼³Á¤ÇÑ´Ù. (ÃßÈÄ ÀÛ¾÷ : Á¶°ÇÀ» °Ë»öÇÏ¿© º¸¿©ÁÙ ¸µÅ©¿¡ ´ëÇÑ Ç¥½Ã)
	HYPERLINK* pLink;
	int nLinkCount = pPage->GetHyperLinkCount(); 


// ¸µÅ©¸¦ È­¸é¿¡ Ãâ·ÂÇÏ±âÀü¿¡ ¶óÀÎÀ» ¶ç¿î´Ù.
	if( nLinkCount > 0 )
	{
		for( int i=0; i< 3;++i)
		{
			LINKITEM* pItem = new LINKITEM;
//			pItem->Init();
			char* temp = " ";
			strcpy(pItem->string, temp);
			
			m_pListDlg->cListItem::AddItem( pItem );
		}
	}
/*
	for( j = 0 ; j < nLinkCount ; ++j )
	{
		pLink = pPage->GetHyperText(j);
		if( pLink == NULL ) break;

		temp = pHyper->GetHyperText( pLink->wLinkId );
		if( temp )
		{
			// Ãß°¡
			LINKITEM* pItem = new LINKITEM;
//			pItem->Init();
			strcpy(pItem->string, temp->str);
			pItem->dwType = pLink->wLinkType;
			
			m_pListDlg->cListItem::AddItem( pItem );

			// Data
			m_sHyper[m_nHyperCount].bUse = TRUE;
			m_sHyper[m_nHyperCount].dwListItemIdx = m_pListDlg->GetItemCount()-1;
			m_sHyper[m_nHyperCount].sHyper = *pLink;
			++m_nHyperCount;
		}
	}
*/	
	// ÇùÇà º¸ÀÏÁö ¸»Áö °áÁ¤
	BOOL bQuest = QUESTMGR->IsNpcRelationQuest( m_dwCurNpc );
	for( j = 0 ; j < nLinkCount ; ++j )
	{
		pLink = pPage->GetHyperText(j);
		if( pLink == NULL ) break;
		if( pLink->wLinkType == emLink_Quest && !bQuest )
			continue;

		temp = pHyper->GetHyperText( pLink->wLinkId );
		if( temp )
		{
			// Ãß°¡
			LINKITEM* pItem = new LINKITEM;
			strcpy(pItem->string, temp->str);
			pItem->dwType = pLink->wLinkType;
			
			m_pListDlg->cListItem::AddItem( pItem );

			// Data
			m_sHyper[m_nHyperCount].bUse = TRUE;
			m_sHyper[m_nHyperCount].dwListItemIdx = m_pListDlg->GetItemCount()-1;
			m_sHyper[m_nHyperCount].sHyper = *pLink;
			++m_nHyperCount;
		}
	}

	return TRUE;
}

void cNpcScriptDialog::EndDialog()
{
	for(int i=0 ; i< MAX_REGIST_HYPERLINK;++i)
		m_sHyper[i].Init();

	m_pListDlg->RemoveAll();

	m_nHyperCount = 0;
	m_dwCurNpc = 0;

	NPCSCRIPTMGR->SetSelectedNpc( NULL );
	
	//Á×ÀºÈÄ¿¡ µé¾î¿Ã ¼öµµ ÀÖ´Ù.
	if( HERO->GetState() != eObjectState_Die )
		OBJECTSTATEMGR->EndObjectState(HERO, eObjectState_Deal);

//	SetActive( FALSE );
	SetDlgTitleText("");
}

HYPER* cNpcScriptDialog::GetHyperInfo( DWORD dwIdx )
{
	for(int i=0;i<m_nHyperCount;++i)
	{
		if( m_sHyper[i].bUse && m_sHyper[i].dwListItemIdx == dwIdx)
			return &m_sHyper[i];
	}

	return NULL;
}

void cNpcScriptDialog::HyperLinkParser( DWORD dwIdx )
{
	if( m_nHyperCount == 0 ) return;

	int nType = -1;
	DWORD dwData;
	int	hypertext = 0;

	for( int i=0 ; i<m_nHyperCount ; ++i)
	{
		if( m_sHyper[i].dwListItemIdx == dwIdx )
		{
			nType = m_sHyper[i].sHyper.wLinkType;
			dwData = m_sHyper[i].sHyper.dwData;
			hypertext = m_sHyper[i].sHyper.wLinkId;
			break;
		}
	}

	switch( nType )
	{
	case emLink_Page:
		{
			OpenLinkPage( dwData );
			m_pListDlg->ResetGuageBarPos();
		}
		break;
	case emLink_Open:
		{
			// °Å·¡Ã¢ ¿­±â
			NPCSCRIPTMGR->StartNpcBusiness( dwData );		
		}
		break;
	case emLink_End:
		{
			EndDialog();
			DisActiveEx();
		}
		break;
	case emLink_Quest:
		{
			OpenQuestLinkPage( dwData );
		}
		break;
	case emLink_QuestLink:
		{
			m_dwQuestIdx = 0;
			DWORD dwPage = 0;
			CQuest* pQuest = QUESTMGR->GetQuest( dwData );
			if( pQuest )
			{
				dwPage = pQuest->GetNpcScriptPage( m_dwCurNpc );
				if( dwPage == 0 )
					dwPage = 30;
				OpenLinkPage( dwPage );

				m_dwQuestIdx = dwData;
			}						
		}
		break;
	case emLink_QuestStart:
		{
			// for quest - jsd : ¿ÏÀü hard coding ¤Ñ.¤Ñ
			if( m_dwQuestIdx == 45 || m_dwQuestIdx == 46 || m_dwQuestIdx == 47 )
			{
				CQuest* pQuest = QUESTMGR->GetQuest( m_dwQuestIdx );
				if( pQuest )
				if( pQuest->IsTimeCheck() )
				{
				}
				else
				{
                    if( hypertext == 347 || hypertext == 349 )
					{
						HERO->StartSocietyActForQuest( 259, -1, -1, TRUE );
					}
					OpenLinkPage( dwData );
				}
			}
			else
			{
				OpenLinkPage( dwData );
			}
//			OpenLinkPage( dwData );
			if( m_dwQuestIdx )
				QUESTMGR->SendNpcTalkEvent( m_dwCurNpc, m_dwQuestIdx );
			if(NPCSCRIPTMGR->GetSelectedNpc())
			if(NPCSCRIPTMGR->GetSelectedNpc()->GetNpcJob() == BOMUL_ROLE)
			{
				MSG_DWORD msg;
				msg.Category = MP_NPC;
				msg.Protocol = MP_NPC_DOJOB_SYN;
				msg.dwData = NPCSCRIPTMGR->GetSelectedNpc()->GetID();
				msg.dwObjectID = HEROID;
				NETWORK->Send(&msg, sizeof(msg));
			}
		}
		break;
	case emLink_EventQuestStart:
		{
			if( dwData )
				QUESTMGR->SendNpcTalkEventForEvent( m_dwCurNpc, dwData );
			EndDialog();
			DisActiveEx();
		}
		break;
	case emLink_QuestContinue:
		{
			OpenLinkPage( dwData );
		}
		break;
	case emLink_QuestEnd:
		{
			QUESTMGR->QuestAbandon( m_dwQuestIdx );
			OpenLinkPage( dwData );
		}
		break;
	case emLink_MapChange:
		{
			g_UserInput.GetHeroMove()->MapChange((WORD)dwData );
		}
		break;
	}	
}

void cNpcScriptDialog::SetDlgTitleText( char* strTitle )
{
	if( !m_pTitleText ) return;

	m_pTitleText->SetStaticText( strTitle );
}

void cNpcScriptDialog::SetActive( BOOL val )
{
	if( m_bActive == val ) return;

	if(!val)
	{
		CNpc* pNpc = NPCSCRIPTMGR->GetSelectedNpc();
		if(pNpc) 
		{			
			if( pNpc->GetNpcJob() == BOMUL_ROLE)
			{
				MSG_DWORD msg;
				msg.Category = MP_NPC;
				msg.Protocol = MP_NPC_CLOSEBOMUL_SYN;
				msg.dwObjectID = HERO->GetID();
				msg.dwData = NPCSCRIPTMGR->GetSelectedNpc()->GetID();
				NETWORK->Send(&msg, sizeof(msg));
			}
		}
		EndDialog();
	}

	cDialog::SetActiveRecursive( val );
}

// ´Ù¸¥°÷¿¡¼­ Àı´ë È£ÃâÇÏÁö ¸¶½Ã¿À!!
void cNpcScriptDialog::DisActiveEx()
{
	for(int i=0 ; i< MAX_REGIST_HYPERLINK;++i)
		m_sHyper[i].Init();

	m_pListDlg->RemoveAll();

	m_nHyperCount = 0;
	m_dwCurNpc = 0;

	NPCSCRIPTMGR->SetSelectedNpc( NULL );

	SetDlgTitleText("");

	cDialog::SetActiveRecursive( FALSE );
	WINDOWMGR->SetMouseInputProcessed();
}

void cNpcScriptDialog::OpenQuestLinkPage( DWORD dwPageId )
{
	// ÃÊ±âÈ­
	int j;  // ä¿®å¤C2065: åœ¨å‡½æ•°ä½œç”¨åŸŸå£°æ˜j
	for( int i = 0; i < MAX_REGIST_HYPERLINK; ++i )
		m_sHyper[i].Init();

	m_pListDlg->RemoveAll();
	m_nHyperCount = 0;

	// ÆäÀÌÁö¿¡ ´ëÇÑ Á¤º¸¸¦ ¿äÃ»ÇÑ´Ù.
	cPage* pPage = NPCSCRIPTMGR->GetPage( m_dwCurNpc, dwPageId );
	if( pPage == NULL ) return;

	// ÀÓ½Ã·Î ÇöÀç Ç¥½Ã ÆäÀÌÁö¸¦ °¡Áø´Ù.
	m_dwCurPageId = dwPageId; 

	// ÆäÀÌÁö Á¤º¸¸¦ ÀÌ¿ëÇÏ¿© ÆäÀÌÁö¸¦ ±¸¼ºÇÑ´Ù.
	DWORD dwMsg = pPage->GetRandomDialogue();

	// ¸Ş¼¼Áö ¹øÈ£¸¦ ÀÌ¿ëÇÏ¿© »ç¿ëÇÒ ¸Ş¼¼Áö¸¦ °¡Á®¿Â´Ù.
	cDialogueList* pList = NPCSCRIPTMGR->GetDialogueList();
	if( pList == NULL ) return;

	cHyperTextList* pHyper = NPCSCRIPTMGR->GetHyperTextList();
	if( pHyper == NULL ) return;

	// ´ë»ç Á¤º¸ ¼ÂÆÃ ½ÃÀÛ
	WORD wIdx = 0;
	WORD LineInfo = 0;
	DIALOGUE* temp = NULL;

	LINKITEM* pItem = NULL;
	LINKITEM* Prev = NULL;

	while(1)
	{
		temp = pList->GetDialogue( dwMsg, wIdx );
		if( temp == NULL ) break;

		// ¶óÀÎÁ¤º¸°¡ ÀÖÀ¸¸é ¾ÆÀÌÅÛÀ» Ã·°¡ÇÑ´Ù.(ÇÊ¼ö!!)
		pItem = new LINKITEM;
		strcpy(pItem->string, temp->str);
		pItem->rgb = temp->dwColor;

		if( Prev == NULL )
		{
			m_pListDlg->cListItem::AddItem(pItem);
			LineInfo = temp->wLine;
		}
		else
		{
			if( LineInfo == temp->wLine )
			{
				Prev->NextItem = pItem;
			}
			else
			{
				m_pListDlg->cListItem::AddItem(pItem);
				LineInfo = temp->wLine;
			}
		}
		Prev = pItem;
		++wIdx;
	}

	// ¸µÅ© Á¤º¸¸¦ °¡Á®¿Í¼­ ¼³Á¤ÇÑ´Ù.
	HYPERLINK* pLink;
	int nLinkCount = pPage->GetHyperLinkCount(); 

	pItem = new LINKITEM;
	strcpy( pItem->string, CHATMGR->GetChatMsg(649) );
	pItem->rgb = RGBA_MAKE( 0, 255, 0, 255 );
	m_pListDlg->cListItem::AddItem( pItem );

	// ÇÑÁÙ ¶ç¿ì±â
	pItem = new LINKITEM;
	strcpy( pItem->string, "" );
	m_pListDlg->cListItem::AddItem( pItem );

	BOOL bCheck = FALSE;
	// ¿©±â¼­ ÇùÇà Á¶°Ç °Ë»ö ÈÄ º¸¿©ÁÙ ¸µÅ©¿¡ ´ëÇÑ Ç¥½Ã
	// ¼öÇà°¡´ÉÇÑ ÇùÇà
	for( j = 0; j < nLinkCount; ++j )
	{
		pLink = pPage->GetHyperText(j);
		if( pLink == NULL ) break;

		CQuest* pQuest = QUESTMGR->GetQuest( pLink->dwData );
		if( pQuest )
		{
			if( pQuest->IsQuestState( m_dwCurNpc ) != 1 )
				continue;
		}
		else
			continue;

		if( !bCheck )
		{
			pItem = new LINKITEM;
			strcpy( pItem->string, CHATMGR->GetChatMsg(653) );
			pItem->rgb = RGBA_MAKE( 0, 255, 0, 255 );
			m_pListDlg->cListItem::AddItem( pItem );
			bCheck = TRUE;
		}

		temp = pHyper->GetHyperText( pLink->wLinkId );
		if( temp )
		{
			// Ãß°¡
			pItem = new LINKITEM;
			strcpy( pItem->string, temp->str );
			pItem->dwType = pLink->wLinkType;
			
			m_pListDlg->cListItem::AddItem( pItem );

			// Data
			m_sHyper[m_nHyperCount].bUse = TRUE;
			m_sHyper[m_nHyperCount].dwListItemIdx = m_pListDlg->GetItemCount()-1;
			m_sHyper[m_nHyperCount].sHyper = *pLink;
			++m_nHyperCount;
		}
	}

	if( bCheck )
	{
		// ÇÑÁÙ ¶ç¿ì±â
		pItem = new LINKITEM;
		strcpy( pItem->string, "" );
		m_pListDlg->cListItem::AddItem( pItem );
	}

	bCheck = FALSE;	
	// ¼öÇàÁßÀÎ ÇùÇà
	for( j = 0; j < nLinkCount; ++j )
	{
		pLink = pPage->GetHyperText(j);
		if( pLink == NULL ) break;

		CQuest* pQuest = QUESTMGR->GetQuest( pLink->dwData );
		if( pQuest )
		{
			if( pQuest->IsQuestState( m_dwCurNpc ) != 2 )
				continue;
		}
		else
			continue;

		if( !bCheck )
		{
			pItem = new LINKITEM;
			strcpy( pItem->string, CHATMGR->GetChatMsg(654) );
			pItem->rgb = RGBA_MAKE( 0, 255, 0, 255 );
			m_pListDlg->cListItem::AddItem( pItem );
			bCheck = TRUE;
		}

		temp = pHyper->GetHyperText( pLink->wLinkId );
		if( temp )
		{
			// Ãß°¡
			pItem = new LINKITEM;
			strcpy( pItem->string, temp->str );
			pItem->dwType = pLink->wLinkType;
			
			m_pListDlg->cListItem::AddItem( pItem );

			// Data
			m_sHyper[m_nHyperCount].bUse = TRUE;
			m_sHyper[m_nHyperCount].dwListItemIdx = m_pListDlg->GetItemCount()-1;
			m_sHyper[m_nHyperCount].sHyper = *pLink;
			++m_nHyperCount;
		}
	}
}
