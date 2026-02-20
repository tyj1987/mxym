// BattleFactory_Default.cpp: implementation of the CBattleFactory_Default class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BattleFactory_Default.h"
#include "Battle.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBattleFactory_Default::CBattleFactory_Default()
{
}

CBattleFactory_Default::~CBattleFactory_Default()
{
}

CBattle* CBattleFactory_Default::CreateBattle(BATTLE_INFO_BASE* pCreateInfo, MAPTYPE MapNum)
{
	// Default battle implementation
	return new CBattle();
}

void CBattleFactory_Default::DeleteBattle(CBattle* pBattle)
{
	if(pBattle)
		delete pBattle;
}
