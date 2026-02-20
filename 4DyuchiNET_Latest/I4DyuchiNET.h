#ifndef _I4DYUCHINET_H
#define _I4DYUCHINET_H

#include "stdafx.h"
#include "I4DyuchiNET_GUID.h"
#include "typedef.h"

// INET_BUF结构体定义（与WSABUF兼容）
struct INET_BUF
{
	ULONG len;		// 缓冲区长度
	char* buf;		// 缓冲区指针
};

// 自定义事件描述结构体
struct CUSTOM_EVENT_DESC
{
	EVENTCALLBACK pEventFunc;		// 事件回调函数
	DWORD dwPeriodicTime;			// 定时器周期时间
};

// DESC_NETWORK结构体定义
struct DESC_NETWORK
{
	// 基本配置
	DWORD dwMaxUserNum;						// 最大用户数量
	DWORD dwMaxServerNum;					// 最大服务器数量
	DWORD dwUserBufferSizePerConnection;		// 每个用户连接的缓冲区大小
	DWORD dwServerBufferSizePerConnection;	// 每个服务器连接的缓冲区大小
	DWORD dwUserMaxTransferSize;				// 用户最大传输大小
	DWORD dwServerMaxTransferSize;			// 服务器最大传输大小
	DWORD dwConnectNumAtSameTime;			// 同时连接数量
	DWORD dwCustomDefineEventNum;			// 自定义事件数量
	DWORD dwMainMsgQueMaxBufferSize;		// 主线程消息队列最大缓冲区大小
	DWORD dwFlag;							// 标志位

	// 自定义事件数组
	CUSTOM_EVENT_DESC* pEvent;				// 指向自定义事件数组的指针

	// 用户侧回调函数
	ACCEPTFUNC OnAcceptUser;					// 用户接受连接回调
	DISCONNECTFUNC OnDisconnectUser;			// 用户断开连接回调
	RECVFUNC OnRecvFromUserTCP;				// 用户TCP接收回调

	// 服务器侧回调函数
	ACCEPTFUNC OnAcceptServer;				// 服务器接受连接回调
	DISCONNECTFUNC OnDisconnectServer;		// 服务器断开连接回调
	RECVFUNC OnRecvFromServerTCP;			// 服务器TCP接收回调
};

// I4DyuchiNET接口定义
interface I4DyuchiNET : public IUnknown
{
public:
	// 创建网络
	virtual BOOL __stdcall CreateNetwork(DESC_NETWORK* desc, DWORD dwUserAcceptInterval, DWORD dwServerAcceptInterval, OnIntialFunc pFunc) = 0;

	// 主线程控制
	virtual void __stdcall BreakMainThread() = 0;
	virtual void __stdcall ResumeMainThread() = 0;

	// 用户信息管理
	virtual void __stdcall SetUserInfo(DWORD dwConnectionIndex, void* user) = 0;
	virtual void* __stdcall GetUserInfo(DWORD dwConnectionIndex) = 0;

	// 服务器信息管理
	virtual void __stdcall SetServerInfo(DWORD dwConnectionIndex, void* server) = 0;
	virtual void* __stdcall GetServerInfo(DWORD dwConnectionIndex) = 0;

	// 地址获取
	virtual sockaddr_in* __stdcall GetServerAddress(DWORD dwConnectionIndex) = 0;
	virtual sockaddr_in* __stdcall GetUserAddress(DWORD dwConnectionIndex) = 0;
	virtual BOOL __stdcall GetServerAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) = 0;
	virtual BOOL __stdcall GetUserAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) = 0;

	// 发送数据
	virtual BOOL __stdcall SendToServer(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) = 0;
	virtual BOOL __stdcall SendToUser(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) = 0;
	virtual BOOL __stdcall SendToServer(DWORD dwConnectionIndex, INET_BUF* pBuf, DWORD dwNum, DWORD flag) = 0;
	virtual BOOL __stdcall SendToUser(DWORD dwConnectionIndex, INET_BUF* pBuf, DWORD dwNum, DWORD flag) = 0;
	virtual BOOL __stdcall SendToServer(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) = 0;
	virtual BOOL __stdcall SendToUser(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) = 0;

	// 强制断开连接
	virtual void __stdcall CompulsiveDisconnectServer(DWORD dwConnectionIndex) = 0;
	virtual void __stdcall CompulsiveDisconnectUser(DWORD dwConnectionIndex) = 0;

	// 获取传输大小
	virtual int __stdcall GetServerMaxTransferRecvSize() = 0;
	virtual int __stdcall GetServerMaxTransferSendSize() = 0;
	virtual int __stdcall GetUserMaxTransferRecvSize() = 0;
	virtual int __stdcall GetUserMaxTransferSendSize() = 0;

	// 广播
	virtual void __stdcall BroadcastServer(char* pMsg, DWORD len, DWORD flag) = 0;
	virtual void __stdcall BroadcastUser(char* pMsg, DWORD len, DWORD flag) = 0;

	// 获取连接数量
	virtual DWORD __stdcall GetConnectedServerNum() = 0;
	virtual DWORD __stdcall GetConnectedUserNum() = 0;

	// 获取绑定端口
	virtual WORD __stdcall GetBindedPortServerSide() = 0;
	virtual WORD __stdcall GetBindedPortUserSide() = 0;

	// 连接到服务器
	virtual BOOL __stdcall ConnectToServerWithUserSide(char* szIP, WORD port, CONNECTSUCCESSFUNC, CONNECTFAILFUNC, void* pExt) = 0;
	virtual BOOL __stdcall ConnectToServerWithServerSide(char* szIP, WORD port, CONNECTSUCCESSFUNC, CONNECTFAILFUNC, void* pExt) = 0;

	// 启动服务器
	virtual BOOL __stdcall StartServerWithUserSide(char* ip, WORD port) = 0;
	virtual BOOL __stdcall StartServerWithServerSide(char* ip, WORD port) = 0;

	// 自定义事件
	virtual HANDLE __stdcall GetCustomEventHandle(DWORD index) = 0;
};

#endif
