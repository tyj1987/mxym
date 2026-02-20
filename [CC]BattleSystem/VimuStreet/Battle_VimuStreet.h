#ifndef _BATTLE_VIMUSTREET_H
#define _BATTLE_VIMUSTREET_H

#include "battle.h"
#include "BattleTeam_VimuStreet.h"

#ifdef _MHCLIENT_LIBRARY_
#include "ImageNumber_Client.h"
#include "cImage.h"
#elif defined(_MHCLIENT_)
#include "../[Client]MH/ImageNumber.h"
#include "cImage.h"
#endif

#define BATTLE_VIMUSTREET_READYTIME		6000	// Ŭ̾Ʈ ⿣ 5..
#define BATTLE_VIMUSTREET_FIGHTTIME		60000
#define BATTLE_VIMUSTREET_RESULTTIME	10000

enum eVIMUSTREET_TEAM
{
	VIMUTEAM_BLUE,
	VIMUTEAM_RED,
};


struct BATTLE_INFO_VIMUSTREET : public BATTLE_INFO_BASE
{
	DWORD		Character[eBattleTeam_Max];
	VECTOR3		vStgPos;
};

class CBattle_VimuStreet : public CBattle
{	
	BATTLE_INFO_VIMUSTREET m_VimuStreetInfo;

	CBattleTeam_VimuStreet m_Team[2];
	
//	BOOL JudgeOneTeamWinsOtherTeam(int TheTeam,int OtherTeam);
#ifdef _MHCLIENT_LIBRARY_
	CImageNumber	m_ImageNumber;

//	cImage			m_ImageReady;
	cImage			m_ImageStart;
	cImage			m_ImageWin;
	cImage			m_ImageLose;
	cImage			m_ImageDraw;
	cImage*			m_pCurShowImage;

	VECTOR2			m_vTitlePos;
	VECTOR2			m_vTitleScale;

////ȿ ӽ
	DWORD	m_dwFadeOutStartTime;
	BOOL	m_bFadeOut;


#elif defined(_MHCLIENT_)
	// MHClient编译模式：这些成员由MHClient的实现定义
	CImageNumber	m_ImageNumber;
	cImage			m_ImageStart;
	cImage			m_ImageWin;
	cImage			m_ImageLose;
	cImage			m_ImageDraw;
	cImage*			m_pCurShowImage;

	VECTOR2			m_vTitlePos;
	VECTOR2			m_vTitleScale;

	DWORD			m_dwFadeOutStartTime;
	BOOL			m_bFadeOut;

#else
	/////////////////////////////////////////////////////////////////////////////////
	// 06. 06. ÿ  2 Player   ó ȵǴ  ذ - ̿
	//  ϳۿ  ι° ɸ ó ҰϿ
	//  Player  ó ϵ 
	BOOL			m_bDieByOp[2];	//濡 ׾?(  FALSE)

#endif
	
#include "../[CC]Header/CommonStruct.h"
#include "../[CC]Header/CommonGameFunc.h"
#include "../[CC]Header/CommonGameDefine.h"
public:
	CBattle_VimuStreet();
	virtual ~CBattle_VimuStreet();

	void Initialize(BATTLE_INFO_BASE* pCreateInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2);
	// Battle  
#ifdef _MAPSERVER_
	virtual void GetBattleInfo(char* pInfo,WORD* size);
//	virtual void GetBattleInfo(BATTLE_INFO_BASE*& pInfo,int& size);
#endif

	// ,Ʊ 
	virtual BOOL IsEnemy(CObject* pOperator,CObject* pTarget);
	virtual BOOL IsFriend(CObject* pOperator,CObject* pTarget);
	
	// event func
	virtual void OnCreate(BATTLE_INFO_BASE* pCreateInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2);
	virtual void OnFightStart();
	virtual void OnDestroy();
	virtual void OnTeamMemberAdd( int Team, DWORD MemberID, char* Name );
	virtual BOOL OnTeamMemberDie(int Team,DWORD VictimMemberID,DWORD KillerID);
	virtual BOOL OnTeamMemberDelete(int Team,DWORD MemberID,char* Name);

	virtual void OnTeamObjectCreate(int Team,CBattleObject* pBattleObject);
	virtual void OnTeamObjectDestroy(int Team,CBattleObject* pBattleObject);


	virtual void OnTick();

	//  
	virtual BOOL Judge();
	BOOL JudgeOneTeamWinsOtherTeam(int TheTeam,int OtherTeam);
	virtual void Victory(int WinnerTeamNum,int LoserTeamNum);
	virtual void Draw();

	

	

#ifdef _MHCLIENT_
	// Render		(Ŭ̾Ʈ )
	virtual void Render();
#endif	
};


#endif
