// cJackpotManager.h: interface for the cJackpotManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CJACKPOTMANAGER_H__8F994787_2618_43B5_B144_32070D18B413__INCLUDED_)
#define AFX_CJACKPOTMANAGER_H__8F994787_2618_43B5_B144_32070D18B413__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_PROBABILITY_LIMIT	100000000 //Ȯִ 1

#define JACKPOTMGR	USINGTON(cJackpotManager)

enum ePrizeKind{ NO_PRIZE = -1, FST_PRIZE, SND_PRIZE, THR_PRIZE, FTH_PRIZE, NUM_PRIZE };	//÷ 
enum {EXCP_MAP_COUNT = 8};

struct stPrize
{
	stPrize():m_dwProbability(0),m_dwPercent(0),AbsMoney(0){};
	DWORD	m_dwProbability;	//ȭ Ȯ
	DWORD	m_dwPercent;	// ۼƮ ((%) == m_dwPercent /100)
	DWORD	AbsMoney;
};

class cJackpotManager  
{
	DWORD	m_dwTotalJPMoney;	//  Fr_DB
	DWORD	m_dwMapJPMoney;	// 
	DWORD	m_dwPrizeMoney;	//÷ ݾ(Fr_DB)
	DWORD	m_dwPercentage;

	CPlayer*	m_pPlayer;
	CMonster*	m_pMonster;

	int		m_nPrizeKind;	//÷ (NO_PRIZE = -1)
	
	DWORD	m_dwChipPerMon;	// (Fr_Script)
	DWORD	m_dwUpDateTimeLen;	// ݾ Update ð (Fr_Script)
	DWORD	m_dwLastDBUpdateTime;	// DBUpdate ð
	stPrize	m_stPrize[NUM_PRIZE];	//÷ (Fr_Script)
	
//	bool	m_bActive;		//JPMgr active
	BOOL	m_bDoPrize;		//JPMgr active
	BOOL	m_bIsAppMon;	// 
	BOOL	m_bIsAppMap;	// 

public:
	cJackpotManager();
	virtual ~cJackpotManager();

//	void	SetActive(bool A)	{m_bActive = A;}	// No useful yet
	BOOL	IsAppMap(WORD MapNum);
	BOOL	IsAppMon(CPlayer* pPlayer, CMonster* pMonster);
	BOOL	IsUserIn();
	BOOL	GetIsAppMap() {return m_bIsAppMap;}
	void	SetTotalMoney(DWORD TotalMoney) {m_dwTotalJPMoney = TotalMoney;}
	void	SetPrizeOnOff(BOOL bOn) {m_bDoPrize = bOn;}
	void	SetChipPerMon(DWORD Chip) {m_dwChipPerMon = Chip;}
	void	SetPrizeProb(DWORD PrizeKind, DWORD Prob, DWORD Percent)
	{
		m_stPrize[PrizeKind].m_dwProbability = Prob;
		m_stPrize[PrizeKind].m_dwPercent = Percent;
	}
	void	SetDBUpdateTimeLen(DWORD TimeLength) {m_dwUpDateTimeLen = TimeLength;}

	BOOL	LoadJackpotInfo();
	void	Process();
	DWORD	SetProbToDword(DWORD* atom, DWORD* denomi);
	DWORD	MakeRndValue();
	int		CheckPrize(int kind);
	int		CheckPrize();
//	void	DoJackpotTest(CPlayer* pPlayer, CMonster* pMonster);
	void	DoJackpot(CPlayer* pPlayer, CMonster* pMonster);
	void	SendMsgDBMapMoney();
	void	SendMsgDBPrizeInfo(CPlayer* pPlayer, int PrizeKind);
//	void	SendMsgAllUserPrizeNotify();	//!ӽ
	void	SendMsgAllUserPrizeNotify(DWORD TotalMoney, DWORD PrizeMoney, DWORD PlayerID, DWORD PrizeKind );
//	void	SendMsgMapUserTotalMoney();	//!ӽ
//	void	SendMsgMapUserTotalMoney(DWORD TotalMoney);
//	void	SendMsgAddUserTotalMoney(CObject* pObject);
//	void	SendMsgPrizeEffect(DWORD PlayerID);
	void	SetPrizeInfo(DWORD TotalMoney, DWORD PrizeMoney, DWORD PlayerID, DWORD PrizeKind);
	DWORD	CalcAddProb(int kind);

	void	Init(WORD Mapnum);
	void	Release();
};

EXTERNGLOBALTON(cJackpotManager)

#endif // !defined(AFX_CJACKPOTMANAGER_H__8F994787_2618_43B5_B144_32070D18B413__INCLUDED_)

/*
 *	    ǿ    Ӵϸ ϸ
 *	ð  DB Ʈ ϸ ѱݾ  ޴´
 *	  ѱݾ AG CL  ȴ
 *	 Ӵ  ÷̾  ¸  ̿θ ϸ
 *	÷ ÷  MSG DB  ÷ݾ׼  ޴´
 *	AG  CL ޵ȴ
 *	 ð Ʈ/ 
 *	÷ Ʈ/ 
 */

/*
 *	ʱȭ		
 *		  Ȯ	IsAppMap(WORD MapNum)
 *		 ʱ  ε	LoadJackpotInfo()
 *
 *
 *	ΰ ƾ ִ.
 *	1.
 *	2.DB UPDATE
 *
 *	1.MONSTER:OBJECT :: DoDie()  
 *		DoJackpot()
 *		ʰ   ´ Ȯϰ	GetIsAppMap()/	IsAppMon()
 *		ʿ  װ
 *		÷θ ȮѴ	CheckPrize()
 *			 ÷Ȯ			MakeRndValue()/	CalcAddProb(int kind)
 *			÷  ش  ÷  DB Query	G_ JackpotPrizeInfo( MONEYTYPE MapMoney, DWORD PlayerID, DWORD PrizeKind, DWORD PrizePercentage)
 *				޾ƿ 	G_ RJackpotPrizeInfo( LPQUERY pData, LPDBMESSAGE pMessage )
 *				ʿ ϰ	SetPrizeInfo(DWORD TotalMoney, DWORD PrizeMoney, DWORD PlayerID, DWORD PrizeKind)
 *				ü  Ʈ  ޽ 	SendMsgAllUserPrizeNotify(DWORD TotalMoney, DWORD PrizeMoney, DWORD PlayerID, DWORD PrizeKind )/	SendMsgPrizeEffect(DWORD PlayerID)
 *		
 *	2.Process()
 *		DB Ʈ ð    Ȯ	IsUserIn()
 *		  	SendMsgDBMapMoney():JackpotAddTotalMoney( MONEYTYPE MapMoney )
 *		ջ  ݾ ޾ƿ´	RJackpotAddTotalMoney( LPQUERY pData, LPDBMESSAGE pMessage )
 *		  鿡 	SendMsgMapUserTotalMoney(DWORD TotalMoney)
 *		
 *		
 *	ETC
 *		ʿ  鿡   		void	SendMsgAddUserTotalMoney(CObject* pObject)
 *		
 *		
 */
