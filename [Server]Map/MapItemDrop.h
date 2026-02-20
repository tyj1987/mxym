//----------------------------------------------------------------------------------------------------
//  MapItemDrop   version:  1.0     date: 05/22/2008
//
//  Copyright (C) 2008 - All Rights Reserved
//----------------------------------------------------------------------------------------------------
///	Index	Stiner(MapDropItem)
///	@file	MapItemDrop.h
///	@author	̼
///	@brief	   Ŭ
//----------------------------------------------------------------------------------------------------
#pragma once
#include "..\[CC]Header\CommonStruct.h"

#define	MAX_DROP_MAPITEM_PERCENT	1000000

#define MAPITEMDROP_OBJ	CMapItemDrop::GetInstance()

struct MAP_ITEM
{
#ifdef _DEBUG
	char	Name[MAX_ITEMNAME_LENGTH+1];	//  ̸
#endif
	WORD	wItemIdx;						//  ε
	DWORD	dwDropPercent;					//  Ȯ
	WORD	wDropCount;						//   Ƚ
	WORD	wMaxDropCount;					//   Ƚ
};

struct MAP_DROP_ITEM
{
	DWORD		wChannel;							// äιȣ
	WORD		wMaxDropNum;						// ִ  
	WORD		wNumDropItem;						// Ǵ   
	DWORD		dwTotalSumDropPercent;				//   
	MAP_ITEM	arrMapItems[MAX_DROPITEM_NUM];		// Ǵ  
};

class CMapItemDrop
{
	BOOL	m_bLoadSucceed;	// ũƮ ε  
	BOOL	m_bInited;		// ְ ʱȭ 

	// ʱ ¥
	WORD	m_wInitDay;		// Sun = 0, Mon, Tue, Wed, Thu, Fri, Sat
	WORD	m_wInitHour;

	CYHHashTable<MAP_DROP_ITEM>	m_MapItemDropTable;

	//  ʱȭ ߴ ¥
	SYSTEMTIME	m_PrevInitedTime;

protected:
	BOOL	LoadMapDropItemList();
	void	ClearMapDropItemList();
	BOOL	LoadInitedTimeFile();

	MAP_DROP_ITEM*	GetMapDropItem(DWORD wChannel);

	MAP_ITEM*	GetMapItem(WORD wChannel, WORD wItemIdx);

	void	DropItem(CPlayer* pPlayer, MAP_ITEM* pMapItem, WORD MonsterKind);


	void	SetPrevInitedTime(SYSTEMTIME* sTime);
	SYSTEMTIME*	GetPrevInitedTime();

	void	InitItemDropInfo();

	void	InitItemDropInfoToDB();
	void	UpdateItemDropInfoToDB( DWORD dwCharacterIdx, WORD wMapNum, WORD wChannel, WORD wItemIDX, WORD wDropCount, WORD wMaxDropCount );
	void	LogMapDropItem(WORD wUserIDX, WORD wCharacterIDX, WORD wItemIDX, WORD wMap, WORD wChannel, WORD wDropCount, WORD wMaxDropCount);
	void	GetDropItemInfoFromDB();

public:
	GETINSTANCE(CMapItemDrop);

	CMapItemDrop(void);
	~CMapItemDrop(void);

	void	Init();
	void	Process();

	void	CalculateDropRate(CPlayer* pPlayer, WORD MonsterKind);

	void	SetItemDropInfoByDB(WORD wMapNum, WORD wChannel, WORD wItemIDX, WORD wDropCount, WORD wMaxDropCount);
};
