// SuryunRegenGroupInfo.h: interface for the CSuryunRegenGroupInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SURYUNREGENGROUPINFO_H__9FE85A75_39F6_4BDB_98C8_5B43FBEA90F1__INCLUDED_)
#define AFX_SURYUNREGENGROUPINFO_H__9FE85A75_39F6_4BDB_98C8_5B43FBEA90F1__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CSuryunRegenGroup;

struct SEQINFO
{
	int GroupID;
	float fRate;
	char GroupName[17];
};

class CSuryunRegenGroupInfo  
{
	int m_GroupID;
	char m_GroupName[17];
	int m_MonsterKind;	// 0~4  0:ְ~4:־
	int m_Count; //    
	float m_x; //ǥ
	float m_z;
	float m_Range; 
	DWORD m_RegenTime; // m_RegenTimeĿ ´.

	SEQINFO m_SeqArray[5];
	
	BOOL ChkRegen(CSuryunRegenGroup* pRegenGroup, DWORD elpsTime);
	void DoRegen(CPlayer* pPlayer, WORD MonsterKind, CSuryunRegenGroup* pRegenGroup);
	WORD GetMonsterIdx(WORD* MonsterIdxAry, int MonsterKind);
public:
	CSuryunRegenGroupInfo();
	~CSuryunRegenGroupInfo();

	void TimeRegen(CSuryunRegenGroup* pRegenGroup, DWORD elpsTime, CPlayer* pPlayer, WORD* MonsterKind);
	void fromStr(char* pBuf);

	int ChkRatioRegen(int SeqNum,int CurMonsterNum);
	void RatioRegen(CSuryunRegenGroup* pRegenGroup, CPlayer* pPlayer, WORD* MonsterKind);
	int GetMonsterCount() { return m_Count; }
};

#endif // !defined(AFX_SURYUNREGENGROUPINFO_H__9FE85A75_39F6_4BDB_98C8_5B43FBEA90F1__INCLUDED_)
