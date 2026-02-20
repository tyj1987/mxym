// BattleFactory_Default.h: interface for the CBattleFactory_Default class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BATTLEFACTORY_DEFAULT_H__B72B2B60A_9A86_47AF_BEEC_7420B7C57B14__INCLUDED_)
#define AFX_BATTLEFACTORY_DEFAULT_H__B72B2B60A_9A86_47AF_BEEC_7420B7C57B14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "../[CC]Header/CommonStruct.h"
#include "../[CC]Header/CommonGameFunc.h"
#include "../[CC]Header/CommonGameDefine.h"
#endif // _MSC_VER > 1000

#include "BattleFactory.h"

class CBattleFactory_Default : public CBattleFactory
{
public:
	CBattleFactory_Default();
	virtual ~CBattleFactory_Default();

	virtual CBattle* CreateBattle(BATTLE_INFO_BASE* pCreateInfo, MAPTYPE MapNum);
	virtual void DeleteBattle(CBattle* pBattle);
};

#endif // !defined(AFX_BATTLEFACTORY_DEFAULT_H__B72B2B60A_9A86_47AF_BEEC_7420B7C57B14__INCLUDED_)
