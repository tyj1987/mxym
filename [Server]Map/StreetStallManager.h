#ifndef __STREETSTALL_MANAGER__
#define __STREETSTALL_MANAGER__
#include "..\[CC]Header\CommonStruct.h"

 
//#define STREETSTALLMGR cStreetStallManager::GetInstance()
#define STREETSTALLMGR USINGTON(cStreetStallManager)


class cStreetStall;
class CPlayer;
struct sCELLINFO;

enum StreetStallFindState
{
	eFind_Sell,
	eFind_Buy,
	eFind_Max,
};

enum StreetStallPriceState
{
	ePrice_Min,
	ePrice_Max,
};

enum StreetStallDelayState
{
	eDelay_StallSearch,
	eDelay_ItemView,
	eDelay_Max,
};

#define STALL_SEARCH_DELAY_TIME	3000
#define	ITEM_VIEW_DELAY_TIME	1000

class cStreetStallManager 
{
private:
	BOOL CanBuyItem(CPlayer* pOwner, CPlayer *pGuest, sCELLINFO* pItemInfo, WORD* EmptyCellPos, STREETSTALL_BUYINFO* pBuyInfo, ITEMBASE* pStallItemBase, DWORD dwBuyNum );
	BOOL CanSellItem(CPlayer* pOwner, CPlayer *pGuest, sCELLINFO* pItemInfo, WORD* EmptyCellPos, STREETSTALL_BUYINFO* pBuyInfo, ITEMBASE* pStallItemBase, DWORD dwBuyNum, WORD& result );
	void AssertBuyErr(WORD Err);
	void AssertSellErr(WORD Err);
protected:
	CMemoryPoolTempl<cStreetStall>*		m_mpStreetStall;
	CYHHashTable<cStreetStall>			m_StallTable;

	// magi82(47)
	DWORD	m_dwSearchDelayTime[eDelay_Max];	//  ˻ 
	CMemoryPoolTempl<STALL_DEALITEM_INFO>*		m_pDealItemPool[eFind_Max];
	CYHHashTable<STALL_DEALITEM_INFO>				m_pDealItemTable[eFind_Max];
	CMemoryPoolTempl<STALL_DEALER_INFO>*		m_pDealerPool[eFind_Max];

public:
	cStreetStallManager();
	virtual ~cStreetStallManager();

	//MAKESINGLETON( cStreetStallManager );

	void StreetStallMode( CPlayer* pPlayer, WORD StallKind, char* title, BOOL bMode );

	cStreetStall* CreateStreetStall( CPlayer* pOwner, WORD StallKind, char* title );
	void DeleteStreetStall( CPlayer* pOwner );
	cStreetStall* FindStreetStall( CPlayer* pOwner );
	
//	BOOL GuestIn( CPlayer* pOwner, CPlayer* pGuest );
//	void GuestOut( DWORD OwnerId, CPlayer* pGuest );
	void UserLogOut( CPlayer* pPlayer );

	BOOL BuyItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pBuyInfo );
	BOOL BuyDupItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pBuyInfo );
	BOOL SellItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pSellInfo );
	BOOL SellDupItem( CPlayer* pOwner, cStreetStall* pStall, CPlayer* pGuest, STREETSTALL_BUYINFO* pSellInfo );

	void CreateDupItem( DWORD dwObjectId, ITEMBASE* pItemBase, DWORD FromChrID );

	BOOL IsExist( cStreetStall* pStall );

	void NetworkMsgParse( BYTE Protocol, void* pMsg );
	void SendNackMsg(CPlayer* pPlayer, BYTE Protocol);

	// magi82(47)
	void	InitStallData();		//  ˻ ޸Ǯ, ̺ ʱȭ
	void	ReleaseStallData();		//  ˻ ޸Ǯ, ̺ 
	void	ResetStallData( DWORD dwOwnerIdx, StreetStallFindState State );	//  ˻  
	void	InsertStallData(  DWORD dwOwnerIdx, DWORD dwItemIdx, StreetStallFindState State, DWORD dwMoney );	//  ˻  ߰
	void	DeleteStallData( DWORD dwOwnerIdx, DWORD dwItemIdx, StreetStallFindState State );	//  ˻  
	DWORD	GetPrice( DWORD dwOwnerIdx, DWORD dwItemIdx, StreetStallPriceState State );	// ش  ְ,  
	BOOL	CheckDelayTime( DWORD dwTime, StreetStallDelayState State );		//  ˻  üũ
};
EXTERNGLOBALTON(cStreetStallManager);

#endif // __STREETSTALL_MANAGER__
