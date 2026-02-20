#ifndef _IOBJECT_H_
#define _IOBJECT_H_

#include "../CommonStruct.h"

// CEngineObject前向声明
class CEngineObject;

// 对象接口 - 定义所有编译模式下通用的接口
// 这个接口只声明方法，不提供实现
// 由具体实现（客户端或服务器）提供

class IObjectInterface
{
public:
	virtual ~IObjectInterface() {}

	// 位置相关
	virtual VECTOR3* GetCurPosition() = 0;
	virtual float GetAngleDeg() = 0;
	virtual int GetDirectionIndex() = 0;

	// 状态相关
	virtual BOOL IsDied() = 0;
	virtual BOOL IsInited() = 0;

	// 引擎对象
	virtual CEngineObject* GetEngineObject() = 0;
};

#endif // _IOBJECT_H_
