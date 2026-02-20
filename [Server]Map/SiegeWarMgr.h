// SiegeWarMgr.h: interface for the CSiegeWarMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIEGEWARMGR_H__0620B52A_B018_4B66_9EF9_6A9D6703763D__INCLUDED_)
#define AFX_SIEGEWARMGR_H__0620B52A_B018_4B66_9EF9_6A9D6703763D__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "..\[CC]BattleSystem\SiegeWar\Battle_SiegeWar.h"

#define SIEGEWARMGR CSiegeWarMgr::GetInstance()

// ¡ Ÿüũ.. .
enum
{
	NAKYANG_SYMBOL,
	MAX_SYMBOL
};


class CGuild;
class CPlayer;



enum eSW_Error
{
	eSWError_NoError=0,
	eSWError_Error,
	eSWError_NoGuildInfo,					// Don't find guildinfo
	eSWError_NoGuildMaster,					// Not a Guild Master
	eSWError_NoCastleGuild,					// Not a Castle Guild
	eSWError_FailedRegistSiegeWarTime,		// Failed Regist siegewar-time
	eSWError_AlreadyDefenceProposal,		// Already Defence-proposal
	eSWError_AlreadyAttackRegist,			// Registed Attack guild
	eSWError_DontDefenceProposal,			// Don't proposal to defence
	eSWError_NoProposalGuild,				// Not a proposal guild
	eSWError_MyUnionIsCastleUnion,			// Proposal Failed
	eSWError_MyUnionIsOtherTeam,			//	
	eSWError_OverGuildCount,				// Over the guild count at siegewar
	eSWError_RegistTime,					// Regist error : siegewar start-time
	eSWError_NoProposalTime,	
	eSWError_CastleGuild,					// Engrave
	eSWError_NoPerfectGuild,				// Not a Perfect Guild Level(5)
	eSWError_OtherPlayerEngrave,			// Already engrave by the other player
	eSWError_NoBaseVillage,					// Not a Guild-basetown
	eSWError_NoSiegeWarTime,				// Not a siegewar-time
	eSWError_Observer,						//
	eSWError_NoAcceptTime,
};

enum eSW_State
{
	eSWState_Before=0,
	eSWState_Proclamation,
	eSWState_Acceptance,
	eSWState_BeforeSiegeWar,
	eSWState_SiegeWar,
	eSWState_EndSiegeWar,
};

enum eSW_GuildType
{
	eSWGuildState_None=0,
	eSWGuildState_CastleGuild,				// 
	eSWGuildState_DefenceGuild,				// 
	eSWGuildState_DefenceProposalGuild,		// û 
	eSWGuildState_AttackGuild,				// û 
	eSWGuildState_CastleUnionGuild,			// ͱ
};

enum eSW_GateKind
{
	eSWGateKind_OuterGate1 = 1,
	eSWGateKind_OuterGate2 = 2,
	eSWGateKind_InnerGate1 = 3,
	eSWGateKind_InnerGate2 = 4,
};



class CSiegeWarMgr  
{
	CYHHashTable<CGuild>		m_DefenceProposalList;		// û ⹮
	CYHHashTable<CGuild>		m_DefenceAcceptList;		//  
	CYHHashTable<CGuild>		m_AttackGuildList;			//  
	CYHHashTable<CGuild>		m_AttackUnionList;			//   Ʈ -  ,  ʵ ϱ
	
	CBattle_SiegeWar*			m_pBattle;	

	DWORD			m_SiegeWarIdx;
	DWORD			m_SiegeWarState;			// Process State
	DWORD			m_CastleUnionIdx;			//  Idx
	DWORD			m_CastleGuildIdx;			//  Idx
	stTIME			m_SiegeWarTime[2];			// 0-, 1-

	// ʰ  ε		0-, 1-, 2...->޴ ʹȣ
	DWORD			m_SiegeWarMapNum[SIEGEWAR_MAX_SIEGEMAP][SIEGEWAR_MAX_AFFECTED_MAP];	
	DWORD			m_SiegeMapCount;			// ü  

	DWORD			m_TaxRate;					// ݺ
	DWORD			m_EngraveIdx;				//  ĳIdx
	DWORD			m_EngraveTimer;				//  ð
	DWORD			m_EngraveGuildIdx;			// ο  GuildIdx

	DWORD			m_SiegeWarSuccessTimer;		//   Ÿ̸
	DWORD			m_SiegeWarEndTimer;			//    ð Ÿ̸

	DWORD			m_FightTime;				//  ð.

	stCASTLEGATELEVEL		m_GateInfo;
	cPtrList				m_CastleGateList;	// 
	CASTLEGATE_BASEINFO		m_CastleGateInfoList[MAX_CASTLEGATE_NUM];
	int						m_CastleGateCount;

	DWORD			m_SymbolIndex[MAX_SYMBOL];

	//SW061205  NPC
	WORD			m_wCurDay;
	DWORD			m_dwCurFlag;

public:
	MAKESINGLETON( CSiegeWarMgr );
	CSiegeWarMgr();
	virtual ~CSiegeWarMgr();

	//
	void Init();
	void LoadCastleGateInfo();
	void CreateSiegeBattle();
	void SetSiegeWarInfo( DWORD SiegeWarIdx, DWORD TaxRate, DWORD RegistTime, DWORD SiegeWarTime, DWORD Level );
	void SetSiegeGuildInfo( SIEGEWAR_GUILDDBINFO* pGuildList, DWORD Count );
	void CreateCastleGate();
	void AddCastleGate();
	void DeleteCastleGate( DWORD GateID );
	void SetFirstSiegeWarTime();

	
	DWORD AddProposalGuildList( DWORD GuildIdx );
	DWORD AddAttackGuildList( DWORD GuildIdx );
	BOOL IsRegistTime( DWORD dwTime );
	void BreakUpGuild( DWORD GuildIdx );
	BOOL IsPossibleUnion( DWORD GuildIdx1, DWORD GuildIdx2 );
	void CreateUnionCheck( DWORD UnionIdx );
	void AddAcceptGuild( DWORD GuildIdx );
 	void DeleteAcceptGuild( DWORD GuildIdx );
 	void DestoryCastleUnion();
 	void SetCastleUnionIdx( DWORD UnionIdx )				{	m_CastleUnionIdx = UnionIdx;	}


	BOOL IsAcceptGuild( DWORD GuildIdx );
	BOOL IsAttackGuild( DWORD GuildIdx );
	BOOL IsAttackUnion( DWORD UnionIdx );
	void AddPlayer( CPlayer* pPlayer );

	void SetGateLevel( DWORD GateID, DWORD Level )			{	m_GateInfo.SetLevel( GateID, Level );	}
	DWORD GetGateLevel( DWORD GateID )						{	m_GateInfo.GetLevel( GateID );	}

	void Process();
	void StartSiegeWar();
	void EndSiegeWar();	
	void SetBattleInfo();
	void SuccessSiegeWar( DWORD GuildIdx );	
	void SendBattleInfoToVillageMap();
	int AddEngraveSyn( CPlayer* pPlayer, DWORD GuildIdx );

	//
	DWORD GetSiegeWarIdx()					{	return m_SiegeWarIdx;	}
	DWORD GetGuildTeamIdx( DWORD GuildIdx );
	DWORD IsAbleOrganizeUnion( DWORD GuildIdx1, DWORD GuildIdx2 );
	void SendMsgToSeigeGuild( MSGBASE* pMsg, DWORD dwLength );
	DWORD GetEngraveIdx()					{	return m_EngraveIdx;	}
	void CancelEngraveIdx();

	DWORD GetVillageMapNum();
	DWORD GetSiegeMapNum();
	BOOL IsNeedLoadSiegeInfo();
	BOOL IsProcessSiegewar()
	{
		if( m_SiegeWarState == eSWState_SiegeWar )	return TRUE;
		else										return FALSE;
	}
	BOOL IsInSiegeWar( DWORD GuildIdx );
	void	GetSiegeWarTime( DWORD* pTime0, DWORD* pTime1 );
	DWORD GetCastleUnionIdx()				{	return m_CastleUnionIdx;	}
 	DWORD GetCastleGuildIdx()				{	return m_CastleGuildIdx;	}

	void UserLogOut( CPlayer* pPlayer );
	void NetworkMsgParse( DWORD dwConnectionIndex, BYTE Protocol,void* pMsg );	

	//SW061205  NPC
	void	CheckDateforFlagNPC();
	DWORD	GetSWState()	{	return m_SiegeWarState;	}
};

#endif // !defined(AFX_SIEGEWARMGR_H__0620B52A_B018_4B66_9EF9_6A9D6703763D__INCLUDED_)
