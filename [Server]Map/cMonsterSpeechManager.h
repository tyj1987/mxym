// cMonsterSpeechManager.h: interface for the cMonsterSpeechManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMONSTERSPEECHMANAGER_H__980D146A_30B9_4F0C_9A69_4E74D859217A__INCLUDED_)
#define AFX_CMONSTERSPEECHMANAGER_H__980D146A_30B9_4F0C_9A69_4E74D859217A__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "CommonGameDefine.h"
#include "MHFile.h"

#define MON_SPEECHMGR USINGTON(cMonsterSpeechManager)

//#define MONSTERLIST_MAXNUM	190		͸Ʈ ε  ִ  .
#define MAX_SPEECHLIST_NUM	50

enum Mon_SpeechState{
		eMon_Speech_ForeAtk,	//  
		eMon_Speech_ForgivePursuit,	//  
		eMon_Speech_Help,	// û 
		eMon_Speech_AboutHelp,	// û   
		eMon_Speech_Death,	// 
		eMon_Speech_Stand,	// 
		eMon_Speech_WalkAround,	//̵ 
		eMon_Speech_KeepStand,
		eMon_Speech_KeepWalkAround,
//		eMon_Speech_Rest,	//޽  .
		eMon_Speech_MAX,
};

//enum Monster_Type{ eMonType_General, eMonType_M_Boss, eMonType_H_Boss };
enum Monster_SpeechTpye{eMSpch_Balloon, eMSpch_BallonChat, eMSpch_SmallShout,};	//MonsterType

struct MonSpeechList
{
	MonSpeechList() : Prob(0),SpeechIdx(0){}
	DWORD	Prob;
	DWORD	SpeechIdx;
};

struct MonSpeechInfo
{
	MonSpeechInfo() : SpeechType(0), SpeechIndex(0){}
	DWORD SpeechType;
	DWORD SpeechIndex;
};

struct MonSpeech
{
	MonSpeech() {memset(this,0,sizeof(MonSpeech));}
	DWORD	MonsterIdx;
	DWORD	MonsterType;
	DWORD	dwProbNoSpeech[eMon_Speech_MAX];
	DWORD	dwSpeechTotalRate[eMon_Speech_MAX];
	DWORD	dwSpchListCount[eMon_Speech_MAX];
	MonSpeechList*	pSpchList[eMon_Speech_MAX];
};

class cMonsterSpeechManager  
{
	CYHHashTable<MonSpeech>		m_MonSpeechInfoTable;	//   ̺
	MonSpeechInfo				m_SpeechInfo;

public:
	cMonsterSpeechManager();
	virtual ~cMonsterSpeechManager();

	void Init();
	void Release();
	bool LoadMonSpeechInfoList();

	void ReleaseSpeechInfoList();

	MonSpeechInfo* GetCurStateSpeechIndex(DWORD MonKind, DWORD StateKind);
	void SetSpeechInfo(MonSpeech* pMonSpeech, DWORD StateKind, CMHFile* fp);
};

EXTERNGLOBALTON(cMonsterSpeechManager)
#endif // !defined(AFX_CMONSTERSPEECHMANAGER_H__980D146A_30B9_4F0C_9A69_4E74D859217A__INCLUDED_)
