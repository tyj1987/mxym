// MonitoringServerStubs.cpp
// Monitoring Server 存根实现 - 提供其他模块需要的符号

#include "stdafx.h"

// 全局变量定义
DWORD gCurTime = 0;
HWND g_hWnd = NULL;

// CNetwork 类存根 - Monitoring Server 使用自己的 MonitorNetwork
// 这里提供最小化的实现以满足链接器
class CNetwork
{
public:
	CNetwork() {}
	~CNetwork() {}
	void Release() {}
	void Init(void* desc) {}
	BOOL StartServerServer(char* szIP, int port) { return TRUE; }
	BOOL StartUserServer(char* szIP, int port) { return TRUE; }
	BOOL ConnectToServer(char* szIP, int port, void* pParam) { return TRUE; }
	void Send2Server(DWORD dwConnectionIndex, char* msg, DWORD size) {}
	void Send2AgentServer(char* msg, DWORD size) {}
	void Send2User(DWORD dwConnectionIndex, char* msg, DWORD size) {}
	void Send2User(class MSGBASE* msg, DWORD size) {}
	void Broadcast2Server(char* msg, DWORD size) {}
	void Broadcast2User(class MSGBASE* msg, DWORD size) {}
	void Broadcast2MapServer(char* msg, DWORD size) {}
	void Broadcast2MapServerExceptOne(DWORD dwConnectionIndex, char* msg, DWORD size) {}
	void Broadcast2AgentServer(char* msg, DWORD size) {}
	void Broadcast2AgentServerExceptSelf(char* msg, DWORD size) {}
	void Broadcast2AgentServerExceptOne(DWORD dwConnectionIndex, char* msg, DWORD size) {}
	void GetUserAddress(DWORD dwConnectionIndex, char* ip, WORD* port) {}
	void DisconnectUser(DWORD dwConnectionIndex) {}
	void EnCrypt(void* userInfo, char* msg, DWORD size) {}
};

// CNetwork 全局实例
CNetwork g_Network;
