#pragma once
#include "..\[CC]Header\CommonStruct.h"

#include "MHFile.h"

#define PET_SPEECHMGR	USINGTON(CPetSpeechManager)

enum Pet_SpeechState{
	ePet_Speech_Stand,	//  
	ePet_Speech_Move,	//̵  
	ePet_Speech_Substand,	//  
	ePet_Speech_Kgong,	// 
	ePet_Speech_Ungi,	//  ̺Ʈ  
	ePet_Speech_Master_Skill,	// ų  
	ePet_Speech_Master_Die,		//  
	ePet_Speech_Pet_Skill,		// ų  
	ePet_Speech_Pet_Die,	//  
	ePet_Speech_Pet_StaminaZero,	// ¹̳ 0  
	ePet_Speech_MAX,
};

enum SPEECH_TYPE{BALLOON,BALLOON_CHAT,CHANEL_SHOUT,};

struct SpeechList
{
	SpeechList() : Prob(0),SpeechIdx(0){}
	DWORD	Prob;
	DWORD	SpeechIdx;
};

struct PetSpeech
{
	PetSpeech() {memset(this,0,sizeof(PetSpeech));}

	DWORD	PetIdx;
	DWORD	PetSpeechType;
	DWORD	dwProbNoSpeech[ePet_Speech_MAX];
	DWORD	dwSpeechTotalRate[ePet_Speech_MAX];
	DWORD	dwSpchLIstCount[ePet_Speech_MAX];
	SpeechList* pSpchList[ePet_Speech_MAX];
};

struct PetSpeechInfo
{
	PetSpeechInfo() : SpeechType(0), SpeechIndex(0){}
	DWORD SpeechType;
	DWORD SpeechIndex;
};

class CPetSpeechManager
{
	CYHHashTable<PetSpeech>		m_PetSpeechInfoTable;	//   ̺

public:
	CPetSpeechManager(void);
	virtual ~CPetSpeechManager(void);

	void Init();
	void Release();

	BOOL LoadPetSpeechInfoList();
	void ReleasePetSpeechInfoList();

	BOOL GetPetCurStateSpeechIndex(DWORD PetKind, DWORD StateKind, DWORD PetGrade, PetSpeechInfo* pRtSpeechInfo);

	void SetPetSpeechInfo(PetSpeech* pPetSpeech, DWORD StateKind, CMHFile* fp);

};

EXTERNGLOBALTON(CPetSpeechManager)