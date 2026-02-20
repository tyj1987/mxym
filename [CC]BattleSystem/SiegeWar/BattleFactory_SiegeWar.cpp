// BattleFactory_SiegeWar.cpp: implementation of the CBattleFactory_SiegeWar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../[CC]Header/ServerSystem.h"
#include "BattleFactory_SiegeWar.h"
#include "Battle_SiegeWar.h"
#include "BattleTeam_SiegeWar.h"

#ifdef _MAPSERVER_
#include "BattleSystem_server.h"
#include "../[Client]MH/GridSystem.h"
#include "../[Client]MH/TileManager.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "../[CC]Header/CommonStruct.h"
#include "../[CC]Header/CommonGameFunc.h"
#include "../[CC]Header/CommonGameDefine.h"
CBattleFactory_SiegeWar::CBattleFactory_SiegeWar()
{
#ifdef  _MAPSERVER_
	m_BattlePool.Init( 5, 1, "CBattle_SiegeWar" );
#endif
}

CBattleFactory_SiegeWar::~CBattleFactory_SiegeWar()
{
#ifdef _MAPSERVER_
	m_BattlePool.Release();
#endif
}


CBattle* CBattleFactory_SiegeWar::CreateBattle(BATTLE_INFO_BASE* pCreateInfo, MAPTYPE MapNum)
{
	BATTLE_INFO_SIEGEWAR* pInfo = (BATTLE_INFO_SIEGEWAR*)pCreateInfo;
	
#ifdef _MAPSERVER_
	pInfo->BattleID = BATTLESYSTEM->MakeNewBattleID();
	//׸ 
	g_pServerSystem->GetGridSystem()->CreateGridTable(pInfo->BattleID, eGridType_General, 512,512,MapNum);	
	//Ÿ 
	if(g_pServerSystem->GetMap()->GetTileManager()->CreateTileGroup(pInfo->BattleID, MapNum, 0, 0, 51200, 51200) == FALSE)
	{
		return NULL;
	}	
	CBattle_SiegeWar* pBattle = m_BattlePool.Alloc();
#else
	CBattle_SiegeWar* pBattle = new CBattle_SiegeWar;
#endif

	// 
	CBattleTeam_SiegeWar* pBTeam1 = new CBattleTeam_SiegeWar;	
	pBTeam1->Init( pInfo->DefenceCount, &pInfo->GuildList[0] );
	// 
	CBattleTeam_SiegeWar* pBTeam2 = new CBattleTeam_SiegeWar;
	pBTeam2->Init( pInfo->AttackCount, &pInfo->GuildList[pInfo->DefenceCount] );

	pBattle->Initialize( pInfo, pBTeam1, pBTeam2 );

	return pBattle;
}

void CBattleFactory_SiegeWar::DeleteBattle(CBattle* pBattle)
{
#ifdef _MAPSERVER_
	g_pServerSystem->GetGridSystem()->DeleteGridTable( pBattle->GetBattleID() );
	g_pServerSystem->GetMap()->GetTileManager()->DeleteTileGroup( pBattle->GetBattleID() );

	BATTLESYSTEM->ReleaseBattleID( pBattle->GetBattleID() );
	m_BattlePool.Free( (CBattle_SiegeWar*)pBattle );
#else
	delete pBattle;	
#endif
}
