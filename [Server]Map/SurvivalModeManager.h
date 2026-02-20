#pragma once
#include "..\[CC]Header\CommonStruct.h"

#define SVVMODEMGR USINGTON(CSurvivalModeManager)

enum eSurvivalModeState
{
	eSVVMode_None,
	eSVVMode_Ready,
	eSVVMode_Fight,
	eSVVMode_End,
};

enum eSurvivalModeStateTime
{
	eSVVMD_TIME_READY = 15000,
	eSVVMD_TIME_END = 10000,
};

class CSurvivalModeManager
{
	WORD	m_wModeState;
	DWORD	m_dwStateRemainTime;

	DWORD	m_dwUsingCountLimit;

	int		m_nUserAlive;

	CYHHashTable<CObject>	m_SVModeUserTable;	//GM 
	CYHHashTable<DWORD>		m_SVItemUsingCounter;	//SVModeUser  Ƚ .
	cPtrList				m_SVModeAliveUserList;	//GM 

public:
	CSurvivalModeManager(void);
	~CSurvivalModeManager(void);

	void	Init();
	void	Release();
	void	Process();
	void	NetworkMsgParse(DWORD dwConnectionIndex, BYTE Protocol, void* pMsg);

	void	SendMsgToAllSVModeUser(MSGBASE* pMsg, int msgsize);
	void	SendAliveUserCount();
	void	SendNackMsg(CPlayer* pGM, BYTE Protocol, BYTE errstate);

	BOOL	CheckRemainTime();
	//SW061129 ȫ߰û۾ - 밹
	void	SetUsingCountLimit(DWORD limit);
	BOOL	AddItemUsingCount(CPlayer* pPlayer);	// īƮ ȳѾ TRUE ȯ

	void	ChangeStateTo(WORD nextState);
	//̹    츮  Ǯ.
	void	ReadyToSurvivalMode();
	void	ReturnToMap();

	void	AddSVModeUser(CObject* pObject);
	void	RemoveSVModeUser(CObject* pObject);
	//int		AddAliveUserCount(BOOL bPlus);
	void	AddAliveUser(CObject* pObject);
	void	RemoveAliveUser(CObject* pObject);

	void	SetCurModeState(WORD state);
	WORD	GetCurModeState()	{	return m_wModeState;	}
};

EXTERNGLOBALTON(CSurvivalModeManager)
