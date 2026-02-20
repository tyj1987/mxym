#pragma once
#include "..\[CC]Header\CommonStruct.h"

#include "FortWarWareSlot.h"


#pragma pack(push,1)

enum eFortWarState
{
	eFortWarState_None = 0,
	eFortWarState_Before10Min,
	eFortWarState_Ing,
};

enum eFortWarDay
{
	eFortWarDay_Sun = 0,
	eFortWarDay_Mon,
	eFortWarDay_Tue,
	eFortWarDay_Wed,
	eFortWarDay_Thur,
	eFortWarDay_Fri,
	eFortWarDay_Sat,
	eFortWarDay_Max,
};

struct sFortWarTimeInfo
{
	BOOL		bOn;
	SYSTEMTIME	FortWarStartTime;
	DWORD		dwWarTime;
};

struct sFortWarInfo
{
	int		nRelationFortWarIDX;
	DWORD	dwCharacterIdx;
	DWORD	dwGuildIdx;
	char	CharacterName[MAX_NAME_LENGTH+1];
	DWORD	dwProfitMoney;
};

#pragma pack(pop)

#define FORTWARMGR	USINGTON(CFortWarManager)

class CPlayer;
class CMonster;

class CFortWarManager
{
protected:
	BOOL			m_bLoadDataFromDB;					// DB   ޾Ҵ?
	BOOL			m_bFortWarMap;						//  
	DWORD			m_dwChannelNum;						//   ä ȣ
	DWORD			m_dwWarState;						//  
	DWORD			m_dwWarTime;						
    sFortWarTimeInfo	m_FortWarTimeInfo[eFortWarDay_Max];
	STATIC_NPCINFO*	m_pNpcInfo;							// npc

	CPlayer*		m_pEngravePlayer;					//  ĳ
	DWORD			m_dwEngraveTime;					//  ð(60)

//	char			m_EngraveSuccessPlayerName[MAX_NAME_LENGTH+1];	// ο  ÷̾ ̸

	CYHHashTable<sFortWarInfo>	m_FortWarInfoTable;			//   
	WORD						m_wRelationFortWarMapNum;	//  ʹȣ
	int							m_nRelationFortWarIDX;		//   ε(2100000001,2100000002,2100000003)

	// â    â õ ʸ ...
	CFortWarWareSlot	m_FortWarWareSlot01;				//   â01
	CFortWarWareSlot	m_FortWarWareSlot02;				//   â02
	CFortWarWareSlot	m_FortWarWareSlot03;				//   â03
	CFortWarWareSlot	m_SiegeWarWareSlot;					// â
	int					m_nRelationSiegeWarIDX;				//  ε(2100000000)
	BOOL				m_bSiegeWarWareUsableMap;			// â ̿  ִ ?

	CYHHashTable<CPlayer>	m_FortWarWarePlayerTable01;		// âִ ĳ
	CYHHashTable<CPlayer>	m_FortWarWarePlayerTable02;		// âִ ĳ
	CYHHashTable<CPlayer>	m_FortWarWarePlayerTable03;		// âִ ĳ
	CYHHashTable<CPlayer>	m_SiegeWarWarePlayerTable;		// âִ ĳ

	DWORD		m_dwFortWarProfitMoney;				// â 
	int			m_nFortWarTexRate;					// ɼ(1/10000)
	int			m_nFortWarWareRate;					// â̿(1/10000)
	DWORD		m_dwUpdateTime;

protected:
    int GetSecondInterval( SYSTEMTIME curTime, SYSTEMTIME basicTime );			// ϰ ̸ , ̸ ...

public:
	CFortWarManager(void);
	~CFortWarManager(void);

	void Init();
	void Release();
	
	//  
	void ProcessFortWar();
	BOOL LoadFortWarInfo();
	void AddPlayer( CPlayer* pPlayer );
	void RemovePlayer( CPlayer* pPlayer );	
	void DiePlayer( CPlayer* pPlayer );
	void SetTotalFortWarInfoFromDB( sFortWarInfo info );
	void UpdateFortWarInfo( int nWarIDX, DWORD dwCharacterIdx, char* pCharacterName, DWORD dwGuildIdx );
	void CompleteLoadDataFromDB()	{ m_bLoadDataFromDB = TRUE; }
	DWORD GetMasterID();
	char* GetMasterName();
	DWORD GetMasterGuildID();

	// ̵
	void ProcessDBUpdateProfitMoney();
	DWORD AddProfitMoneyFromItemSell( DWORD dwMoney );
	DWORD AddProfitMoneyFromItemBuy( DWORD dwMoney );
	DWORD AddProfitMoneyFromMonster( DWORD dwMoney );
	void SetFortWarProfitMoneyFromDB( int nWarIDX, DWORD dwMoney );

	// â
	void RemoveWarePlayerTable( CPlayer* pPlayer );
	void TotalFortWarItemLoad();
	void SetTotalFortWarItemInfoFromDB( int nWarIDX, ITEMBASE* pItem );
	BOOL AddProfitItemFromMonster( CPlayer* pPlayer, DWORD dwItemIdx, WORD wItemNum );
	void AddProfitItemFromDBResult( int nWarIDX, ITEMBASE* pItem );
	CFortWarWareSlot* GetFortWarWareSlot( int nWarIDX );
	void SendItemInfoToFortWarePlayer( int nWarIDX, MSGBASE* pMsg, int nSize );
	void SendItemInfoToFortWarePlayerExceptOne( int nWarIDX, DWORD dwPlayerID, MSGBASE* pMsg, int nSize );
	void SendItemInfoToSeigeWarePlayer( MSGBASE* pMsg, int nSize );
	void SendItemInfoToSeigeWarePlayerExceptOne( DWORD dwPlayerID, MSGBASE* pMsg, int nSize );
	void SetSiegeWarWareUsable( BOOL bUse )		{ m_bSiegeWarWareUsableMap = bUse; }

	void NetworkMsgParse( DWORD dwConnectionIndex, BYTE Protocol, void* pMsg );

protected:
	// msg óԼ...
	void Msg_MP_FORTWAR_ENGRAVE_START_SYN( void* pMsg );
	void Msg_MP_FORTWAR_ENGRAVE_CANCEL_SYN( void* pMsg );

    void Msg_MP_FORTWAR_WAREHOUSE_INFO_SYN( void* pMsg );
	void Msg_MP_FORTWAR_SIEGEWAREHOUSE_INFO_SYN( void* pMsg );
	void Msg_MP_FORTWAR_WAREHOUSE_LEAVE( void* pMsg );
	void Msg_MP_FORTWAR_WAREHOUSE_ITEM_MOVE_SYN( void* pMsg );
	void Msg_MP_FORTWAR_SIEGEWAREHOUSE_ITEM_MOVE_SYN( void* pMsg );

	void Msg_MP_FORTWAR_WAREHOUSE_ITEM_INSERT_TO_MAP( void* pMsg );
	void Msg_MP_FORTWAR_WAREHOUSE_ITEM_MOVE_TO_INVEN_TO_MAP( void* pMsg );
	void Msg_MP_FORTWAR_WAREHOUSE_ITEM_MOVE_TO_SIEGEHOUSE_TO_MAP( void* pMsg );

	void Msg_MP_FORTWAR_WAREHOUSE_PUTOUT_MONEY_SYN( void* pMsg );
	void Msg_MP_FORTWAR_WAREHOUSE_MONEY_TO_MAP( void* pMsg );
	void Msg_MP_FORTWAR_SIEGEWAREHOUSE_PUTOUT_MONEY_SYN( void* pMsg );
};

EXTERNGLOBALTON(CFortWarManager)
