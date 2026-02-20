// TitanItemManager.h: interface for the CTitanItemManager class.
//////////////////////////////////////////////////////////////////////

#if !defined(TITANITEMMANAGER_H)
#define TITANITEMMANAGER_H
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define TITANITEMMGR CTitanItemManager::GetInstance()

#include "..\[CC]Header\GameResourceStruct.h"

class ITEM_MIX_INFO;
class CMHFile;

class CTitanItemManager  
{
	CYHHashTable<ITEM_MIX_INFO> m_TPMInfoList;

	CMemoryPoolTempl<TITAN_PARTS_KIND>*	m_TitanPartsKindPool;
	CYHHashTable<TITAN_PARTS_KIND>		m_TitanPartsKindTable;

	CYHHashTable<TITAN_UPGRADEINFO>		m_TitanUpgradeInfoTable;

	CYHHashTable<TITAN_BREAKINFO>		m_TitanBreakInfoTable;


	DWORD m_TitanBreakRate;	// Ÿź  Ȯ

public:
	CTitanItemManager();
	virtual ~CTitanItemManager();
	GETINSTANCE(CTitanItemManager);

	void LoadTitanMixList();
	void LoadTitanMixResultItemInfo(CMHFile * fp, ITEM_MIX_RES * pResInfo);
	ITEM_MIX_INFO * GetTitanMixItemInfo(WORD wItemIdx);

	//  Ӽ ̺  Լ
	BOOL LoadTitanPartsKindList();
	TITAN_PARTS_KIND* GetTitanPartsKind(DWORD partsIdx);

	// Ÿź ׷̵ ̺  Լ
	void	LoadTitanUpgradeInfoList();
	void	LoadTitanUpgradeInfoResultItemInfo(CMHFile * fp, TITAN_UPGRADE_MATERIAL * pMaterialInfo);
	TITAN_UPGRADEINFO * GetTitanUpgradeInfoItemInfo(DWORD titanIdx);

	// Ÿź  ̺  Լ
	void	LoadTitanBreakInfoMaterialInfo(CMHFile * fp, TITAN_BREAK_MATERIAL * pMaterialInfo);
	void	LoadTitanBreakInfoList();
	TITAN_BREAKINFO * GetTitanBreakInfoItemInfo(DWORD dwIdx);

	// Ÿź Ʈũ ޼  Լ..
	// Ÿź  
	void TitanMixAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg );
	void TitanMix( CPlayer* pPlayer, MSG_TITANMIX* pmsg );

	// Ÿź ׷̵ 
	void TitanupgradeAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg );
	void Titanupgrade( CPlayer* pPlayer, MSG_TITAN_UPGRADE_SYN* pmsg );

	// Ÿź ü 
	void TitanBreakAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg );
	void TitanBreak( CPlayer* pPlayer, MSG_DWORD2* pmsg );

	// magi82 - Titan(070611) Ÿź ȯ ּó
	// Ÿź ռ 
	//void TitanMugongChange( CPlayer* pPlayer, MSG_WORD3* pmsg );

	// Ÿź / 
	void TitanRegisterAdditem( CPlayer* pPlayer, MSG_ITEM* pmsg );
	void TitanDissolutionAdditem( CPlayer* pPlayer, MSG_ITEM* pmsg );

	// Ÿź   
	void TitanPartsMakeAddItem( CPlayer* pPlayer, MSG_ITEM* pmsg );
	void TitanPartsMake( CPlayer* pPlayer, MSG_ITEM_MIX_SYN* pmsg );

	// Ÿź  
	void TitanRegister( CPlayer* pPlayer, MSG_DWORD2* pmsg );

	// Ÿź  
	void TitanCancellation( CPlayer* pPlayer, MSG_DWORD2* pmsg );
};


#endif // !defined(TITANITEMMANAGER_H)