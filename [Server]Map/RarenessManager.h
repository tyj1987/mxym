// RarenessManager.h: interface for the RarenessManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RARENESSMANAGER_H__98F09FB6_6D0C_45D2_A95A_192AA6ECDEDE__INCLUDED_)
#define AFX_RARENESSMANAGER_H__98F09FB6_6D0C_45D2_A95A_192AA6ECDEDE__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Player.h"

#define RAREITEMMGR	USINGTON(RarenessManager)
#define BOUNDARY_INDEX_FOR_SHOPITEM	55101	//"itemlist.bin" ȹ  ٴ  ε.
#define RARE_VALUE_PROB_LIST_MAX	100

//enum WeaponKind{ GUM, DO, CHANG, AMGI, GOONG, WeaponKindMAX = 5 };
//enum ProtectorKind{ DRESS, HAT, SHOES, ProtectorkindMAX = 3 };
//enum AccessaryKind{ RING, CAPE, NECKLACE, ARMLET, BELT, AccessaryKindMAX = 5 };



//struct 
class RarenessManager  
{
	CYHHashTable<sRareItemInfo>	m_RareItemInfoTable;

	sRareOptionInfo m_RereItemInfo[RareItemKindMAX];

	DWORD	m_dwRareValueProb[RARE_VALUE_PROB_LIST_MAX];
	//RereItem's Additional Opt Info
//	sRareOptionWeaponInfo		m_Weapon[WeaponKindMAX];
//	sRareOptionProtectorInfo	m_Protector[ProtectorkindMAX];
//	sRareOptionBase				m_Accessary[AccessaryKindMAX];
		
public:
	RarenessManager();
	virtual ~RarenessManager();

	void Init();
	void Release();

	bool LoadRareItemOptionInfo();	// ɼ  //Լ ̸     !
	bool LoadRareItemInfo();	// ε()  Ȯ
	bool LoadRareValueProbList();
	void ReleaseRareItemInfo();	// ̺ 
	// Ȯ Ǻ( Ǵ )
	bool GetRare(WORD ObtainItemIdx, ITEM_RARE_OPTION_INFO* pRareOptionInfo, CPlayer* pPlayer, BOOL bRare=FALSE);
	//(̸)ش   ´   
//	void MakeRareWeapon(ITEM_RARE_OPTION_INFO* pRareInfo);
//	void MakeRareProtector(ITEM_RARE_OPTION_INFO* pRareInfo);
//	void MakeRareAccessary(ITEM_RARE_OPTION_INFO* pRareInfo);
	DWORD GetRareItemValue(DWORD RndMin, DWORD RndMax);
	//DB   

	//
	BOOL IsRareItemAble( DWORD ItemIdx )
	{
		if( m_RareItemInfoTable.GetData(ItemIdx) )			return TRUE;
		return FALSE;
	}

	//asist
};

EXTERNGLOBALTON(RarenessManager)

#endif // !defined(AFX_RARENESSMANAGER_H__98F09FB6_6D0C_45D2_A95A_192AA6ECDEDE__INCLUDED_)
