#ifndef _CLIENTOBJECTADAPTER_H_
#define _CLIENTOBJECTADAPTER_H_

#ifdef _MHCLIENT_

#include "../Interface/IObject.h"
#include "../../[Client]MH/Object.h"

// 客户端对象适配器
// 将客户端CObject的方法适配到IObjectInterface接口
// 用于解决Skill模块编译时的类型识别问题

namespace ClientObjectAdapter
{
	// CObject方法适配
	inline VECTOR3* GetCurPosition(CObject* pObj)
	{
		return &pObj->GetCurPosition();
	}

	inline float GetAngleDeg(CObject* pObj)
	{
		return pObj->GetAngleDeg();
	}

	inline BOOL IsDied(CObject* pObj)
	{
		return pObj->IsDied();
	}

	inline BOOL IsInited(CObject* pObj)
	{
		return pObj->IsInited();
	}

	inline int GetDirectionIndex(CObject* pObj)
	{
		return pObj->GetDirectionIndex();
	}

	inline CEngineObject* GetEngineObject(CObject* pObj)
	{
		return pObj->GetEngineObject();
	}
}

#endif // _MHCLIENT_

#endif // _CLIENTOBJECTADAPTER_H_
