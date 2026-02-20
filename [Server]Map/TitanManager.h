// TitanManager.h: interface for the CTitanManager class.
//////////////////////////////////////////////////////////////////////

#if !defined(TITANMANAGER_H)
#define TITANMANAGER_H
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum	EnduranceCalcPoint{eWhenTitanAttack, eWhenTitanDefense};

enum	GetOffKindofReason{eNormal, eFromUser, eMasterLifeRate, eExhaustFuel, eExhaustSpell,};

// magi82(33)
enum	EnduranceException	{	eExceptionNone, eExceptionInven, eExceptionPyoguk	};

//  
enum	{AtAtk, AtDef};

#define MAX_TITANGRADE	3
#define TITAN_EQUIPITEM_ENDURANCE_MAX	10000000	// õ
class CTitan;

//enum	eTitanEquips{TE_HELMET, TE_ARMOR, TE_GLOVES, TE_LEGS, TE_CLOAK, TE_SHIELD, TE_WEAPON, TE_MAX};	//eTitanWearedItem_Max

struct TitanWearedInfo
{
	WORD TitanEquipItemIdx;
	DWORD TitanEquipItemDBIdx;
};

class CTitanManager  
{
	//static	DWORD	m_gTitanSpellDecrease;
	CMemoryPoolTempl<CPlayer>* PlayerPool;

	CTitan*		m_pCurRidingTitan;
	CPlayer*	m_pMaster;
	titan_calc_stats	m_titanStats;
	titan_calc_stats	m_titanItemStats;
	DWORD		m_dwCurRegistTitanCallItemDBIdx;	// ϻ Ÿź  DBIDX
	DWORD		m_dwRecallCheckTime;
	WORD		TitanScaleForNewOne;	// 100 => Ŭ̾Ʈ 1.0f.
	TitanWearedInfo		m_TitanWearedInfo[eTitanWearedItem_Max];
	BOOL		m_bAvaliableEndurance;	//  ȿ ˻縦  º.
	TITAN_SHOPITEM_OPTION m_TitanShopitemOption;	// magi82(26)

	CYHHashTable<TITAN_TOTALINFO>				m_TitanInfoList;
	CYHHashTable<TITAN_ENDURANCE_ITEMINFO>		m_ItemEnduranceList;
	CYHHashTable<TITAN_ENDURANCE_ITEMINFO>		m_ItemUsingEnduranceList;
	CMemoryPoolTempl<TITAN_ENDURANCE_ITEMINFO>	m_ItemEndurancePool;

	// 2007. 9. 14. CBH - Ÿź ȯ ð   ߰
	DWORD m_dwTitanRecallProcessTime;	
	DWORD m_dwCurrentTime;
	BOOL m_bTitanRecall;
	BOOL m_bTitanRecallClient;
	//////////////////////////////////////////////////////

	// Ÿź   ð
	DWORD m_dwTitanEPTime;

	// magi82(23)
	DWORD m_dwTitanMaintainTime;

public:
	CTitanManager();
	virtual ~CTitanManager();

	CTitan*		GetCurRidingTitan()		{	return m_pCurRidingTitan;	}

	void	Init(CPlayer* pPlayer);
	void	Release();

	void	SetRegistTitanCallItemDBIdx(DWORD itemDBIdx)	{	m_dwCurRegistTitanCallItemDBIdx = itemDBIdx;	}
	DWORD	GetRegistTitanCallItemDBIdx()	{	return m_dwCurRegistTitanCallItemDBIdx;	}
	void	RemoveTitan();
	// Ÿź  
	void	AddTitanTotalInfo(TITAN_TOTALINFO* pTitanInfo, int flagSendMsgTo = eServerOnly);
	BOOL	AddTitanTotalInfoList(TITAN_TOTALINFO* pTitanInfo);
	TITAN_TOTALINFO*	GetTitanTotalInfo(DWORD callItemDBIdx);
	void	RemoveTitanTotalInfo(DWORD callItemDBIdx);	// ȯ Ʈ 󿡼 
	void	DeleteTitanTotalInfo(DWORD callItemDBIdx);	// ȯۻ, α׾ƿ.
	WORD	GetTitanInfoList(TITAN_TOTALINFO* RtInfo);

	void	DeleteTitan(DWORD callItemDBIdx);

	void	CreateNewTitan(CPlayer* pMaster, DWORD dwItemIdx, DWORD dwCallItemDBIdx, WORD wTitanKind, WORD wTitanGrade = 1);
	void	CreateUpgradeTitan(CPlayer* pMaster, DWORD dwItemIdx, DWORD dwCallItemDBIdx);

	void	SetTitanScale(WORD wScale)	{	TitanScaleForNewOne = wScale;	}	// Ÿź   
	DWORD	GetTitanScale()	{	return TitanScaleForNewOne;	}

	// Ÿź μ
	// °ó
	DWORD	GetTitanSpellDecrease();
	void	ReduceTitanSpell(DWORD spell);

	//   ó
	void	TitanProcess();

	void	UpDateCurTitanAndEquipItemInfo();

	void	UpdateCurTitanInfo();

	// Ÿź ž
	BOOL	RideInTitan(DWORD callItemDBIdx, BOOL bSummonFromUser = TRUE);

	void	CheckRidingTitan();
	void	SetTitanRiding(BOOL bVal);
	// Ÿź ž  ˻
	BOOL	CheckBeforeRideInTitan(DWORD callItemDBIdx);
	void	SetRecallCheckTime(DWORD checkTime = 0);

	BOOL	CheckTimeRecallAvailable();

	// Ÿź ž/ų 밡 . ȿ  3̻ 
	BOOL	CheckEquipState();

	// Ÿź ž
	BOOL	GetOffTitan(int reason = eNormal);

	// Ÿź ȯ
	void	ExchangeTitan(DWORD callItemDBIdx, CPlayer* pNewMaster);

	// Ÿź ޾
	//  ó(޺  ٸ. ȯ .  ȯ ϱ)
	// Ÿź  ó(DB)
	void	UpgradeTitan(DWORD callItemDBIdx, DWORD newCallItemDBIdx);

	// Ÿź 
	// Ÿź ó(DB)
	BOOL	RegistTitan(DWORD	callItemDBIdx);
	
	// Ÿź 
	// Ÿź ó(DB)
	BOOL	CancleTitanRegister(DWORD callItemDBIdx);

	// Ÿź / 
	int		ApplyYoungYakItemToCurTitan(ITEM_INFO* pItemInfo);
	void	AddCurTitanFuel(WORD amount);
	void	AddCurTitanSpell(WORD amount);
	void	AddCurTitanFuelAsRate(float recoverRate);
	void	AddCurTitanSpellAsRate(float recoverRate);

	void	SetTitanStats();
	titan_calc_stats*	GetTitanStats()		{	return &m_titanStats;		}
	titan_calc_stats*	GetTitanItemStats()	{	return &m_titanItemStats;	}
///////////////////////////////////////////////////////////////////////////////
	//    
	void	MakeNewEnduranceInfo(CPlayer* pOwner, ITEMBASE* pItemInfo, EnduranceException eException = eExceptionNone);	// magi82(33)
	void	DeleteTitanEquip(DWORD itemDBIdx);

	// Ÿź    
	void	AddTitanEquipItemEnduranceInfo(TITAN_ENDURANCE_ITEMINFO* pEnduranceInfo, int flagSendMsgTo = eServerOnly);
	BOOL	AddTitanEquipItemEnduranceInfoList(TITAN_ENDURANCE_ITEMINFO* pEnduranceInfo);
	TITAN_ENDURANCE_ITEMINFO*	GetTitanItemEnduranceInfo(DWORD equipItemDBIdx);
	void	RemoveTitanEquipItemInfo(DWORD equipItemDBIdx);	// ȯ Ʈ 󿡼 
	void	DeleteTitanEquipItemInfo(DWORD equipItemDBIdx);	//  ı, Ǵ .
	WORD	GetTitanEquipItemInfoList(TITAN_ENDURANCE_ITEMINFO* RtInfo);
	void	DiscardTitanEquipItem(POSTYPE whatPos, WORD whatItemIdx, DWORD titanEquipItemDBIdx);

	// Ÿź     (for DBUpdate)
	void	AddTitanUsingEquipItemList(TITAN_ENDURANCE_ITEMINFO* pEnduranceInfo);
	void	RemoveTitanUsingEquipItemList(DWORD equipItemDBIdx);
	void	UpdateUsingEquipItemEnduranceInfo();
	void	GetAppearanceInfo(TITAN_APPEARANCEINFO* RtInfo);

	BOOL	CheckUsingEquipItemNum();
	BOOL	CheckUsingEquipItemEndurance();
	BOOL	IsAvaliableEndurance()	{	return m_bAvaliableEndurance;	}

	//  
	void	DoRandomDecrease(int flgPoint);
	DWORD	GetDecreaseEnduranceFromItemIdx(DWORD itemIdx);
	void	PlusItemEndurance(DWORD titanEquipItemDBIdx, DWORD increaseAmount);
	void	MinusItemEndurance(DWORD titanEquipItemDBIdx, DWORD decreaseAmount);
	void	SendItemEnduranceInfo(TITAN_ENDURANCE_ITEMINFO* pInfo);

	void	SetWearedInfo(WORD wEquipItemKind, ITEMBASE* pItemBase, BOOL bIn);
	//void	SetWearedInfo(WORD wEquipItemKind, DWORD itemIdx, DWORD itemDBIdx);
	TitanWearedInfo*	GetWearedInfo(int equipKind);	//   ޾Ƽ  .

	void	ExchangeTitanEquipItem(DWORD dwItemDBIdx, CPlayer* pNewOwner);

	void	SendTitanStats();
	void	SendTitanPartsChange();

	// magi82 - Titan(070515)
	void	SetTitanRepairEquipItem( CPlayer* pPlayer, MSG_DWORD2* pmsg );
	void	SetTitanRepairTotalEquipItem( CPlayer* pPlayer, MSG_TITAN_REPAIR_TOTAL_EQUIPITEM_SYN* pmsg );
	//////////////////////////////////////////////////////////////////////////

	///// 2007. 9. 13. CBH
	BOOL	TitanRecallStartSyn( DWORD callItemDBIdx, BOOL bSummonFromUser = TRUE );	//Ÿź ȯ 
	void	TitanRecallProcess();														//Ÿź ð ó Լ
	void	InitTitanRecall();															//Ÿź ȯ  ʱȭ Լ
	void	StartTitanRecall();															//Ÿź ȯ 
	BOOL	RideInTitan();																//Ÿź ž
	BOOL	IsTitanRecall();															//Ÿź ȯ  ȯ
	void	SetRecallProcessTime(DWORD dwRecallProcessTime);							//Ÿź ȯ ĳ ð 
	///////////////////////////////////////////////////////////////////////////

	void	MoveTitanEquipItemUpdateToDB(ITEMBASE* pFromItem, ITEMBASE* pToItem, POSTYPE FromPos, POSTYPE ToPos);

	void NetworkMsgParse( DWORD dwConnectionIndex, BYTE Protocol, void* pMsg );

	// magi82(26)
	void	SetTitanShopitemOption(DWORD dwItemIdx, BOOL bAdd);
	TITAN_SHOPITEM_OPTION* GetTitanShopitemOption()	{	return &m_TitanShopitemOption;	};
};


#endif // !defined(TITANMANAGER_H)
