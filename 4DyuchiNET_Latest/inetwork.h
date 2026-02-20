#ifndef _INETWORK_H
#define _INETWORK_H

#include "typedef.h"

// INetwork接口定义
interface INetwork
{
public:
	// 初始化
	virtual BOOL Initialize(OnIntialFunc pOnInitialFunc,
							OnAcceptFunc pOnAcceptFunc,
							OnConnectFunc pOnConnectFunc,
							OnDisconnectFunc pOnDisconnectFunc,
							OnRecvFunc pOnRecvFunc,
							OnErrorFunc pOnErrorFunc) = 0;

	// 启动/停止
	virtual BOOL Start(DWORD dwPort, DWORD dwMaxUserNum, DWORD dwMaxBufferSize, DWORD dwThreadNum) = 0;
	virtual void Stop() = 0;

	// 连接管理
	virtual DWORD Connect(char* pIP, DWORD dwPort, DWORD dwBufferSize) = 0;
	virtual void Disconnect(DWORD dwConnectID) = 0;

	// 发送数据
	virtual BOOL Send(DWORD dwConnectID, char* pMsg, DWORD dwSize) = 0;

	// 状态查询
	virtual BOOL IsConnected(DWORD dwConnectID) = 0;
	virtual DWORD GetIP(DWORD dwConnectID) = 0;

	// 释放
	virtual void Release() = 0;
};

#endif
