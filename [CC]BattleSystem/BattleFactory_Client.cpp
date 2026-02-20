// BattleFactory_Client.cpp: 客户端版本的战斗工厂实现
//
// 这个文件为客户端提供 CBattleFactory 类的桩实现
// 因为服务器端的 BattleFactory.cpp 使用 #ifdef _MAPSERVER_
// 在客户端编译时会产生空的对象文件

#include "stdafx.h"

#ifdef _MHCLIENT_

// 包含头文件
#include "BattleFactory.h"
#include "Battle.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBattleFactory::CBattleFactory()
{
	// 客户端不需要实际的工厂实现
}

CBattleFactory::~CBattleFactory()
{
	// 客户端不需要实际的工厂实现
}

CBattle* CBattleFactory::CreateBattle(BATTLE_INFO_BASE* pCreateInfo, MAPTYPE MapNum)
{
	// 客户端不需要创建战斗对象
	return NULL;
}

#endif // _MHCLIENT_
