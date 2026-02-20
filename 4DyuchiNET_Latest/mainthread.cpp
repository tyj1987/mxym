// �� ���� ���α׷��� ũ�� �� �ٱ�� �� �� �ִ�.���� io�� ó���ϴ� worker������ ���ܰ� 
// ���ŵ� �޽����� ó���ϰ� �ɵ������� ������ �ؾ��� ���� �����ϴ� ���� �������̴�.
// �̰��� ���� ������� �̿� ���õ� �۷ι� �Լ��� �ҽ��̴�.

#include "net_define.h"
#include "cpio.h"
#include "mainthread.h"
#include "que.h"
#include "network.h"
#include "quemanager.h"
#include "switch_que.h"
#include "net_global.h"
#include "log.h"
#include <process.h>
#include "pre_connect.h"
#include "typedef.h"

OnIntialFunc 					g_pOnInitialFunc = NULL;			// ���� �����尡 �������ڸ��� ������ �Լ�
HANDLE	g_hMainThreadEvent[MAX_MAIN_THREAD_EVENT_NUM];				// ���� �����尡 ����� �̺�Ʈ ��ü �ڵ� �迭
HANDLE	g_hMainThread			=	NULL;							// ���� ������ �ڵ�.
DWORD	g_dwMainThreadEventNum	=	MIM_MAIN_THREAD_EVENT_NUM;		// ���� �����尡 üũ�� �̺�Ʈ�� ����.
HANDLE	g_hCheckBreakEvent		=	NULL;

//typedef void (*VOIDFUNC)(void);
EVENTCALLBACK pEventFunc[MAX_MAIN_THREAD_USER_DEFINE_EVENT_NUM];	// ���� ���� �̺�Ʈ�� �۵��� �Լ� ������ �迭.
																	// Modified by chan78 at 2001/10/17

struct EVENT_DESC
{
	DWORD		dwEventIndex;
	HANDLE		hEvent;
};

// 2007/12/19 by yuchi
// ���� �̺�Ʈ�� �ε����� �̺�Ʈ �ڵ��� ���� ���̺�
DWORD		g_dwUserEventNum = 0;
EVENT_DESC g_userEventDescList[MAX_MAIN_THREAD_USER_DEFINE_EVENT_NUM];

UINT WINAPI ProcessMsgThread(LPVOID lpVoid);	

void SetOnInitialFunc(OnIntialFunc pFunc)
{
	g_pOnInitialFunc = pFunc;
}

// ����� ���� �̺�Ʈ�� ��������� �̺�Ʈ �߻��� ������ �Լ��� �������ش�.
// ����� ���� �̺�Ʈ�� ���鶩 �ݵ�� ��������� �Ѵ�.
// Modified by chan78 at 2001/10/17
void SetMainThreadUserDefineEventFunc(DWORD index,EVENTCALLBACK pFunc)
{
	pEventFunc[index] = pFunc;
}

// ���� �����带 ��� ����� �Ҷ� ����Ѵ�.
// npc�� �����������...��Ÿ ���
void BreakMainThread()
{
	SetEvent(g_hMainThreadEvent[EVENT_INDEX_BREAK]);
	WaitForSingleObject(g_hCheckBreakEvent,INFINITE);
}

// ����� ���� �����带 �ٽ� ������ ����Ѵ�.
void ResumeMainThread()
{
	SetEvent(g_hMainThreadEvent[EVENT_INDEX_RESUME]);
}
// ���� �����尡 �޽����� ó���ؾ����� worker�����尡 �˸��� set�� �̺�Ʈ ��ü�� �ڵ��� ����.
HANDLE	GetMainThreadMsgEvent()
{
	return g_hMainThreadEvent[EVENT_INDEX_MSG_EVENT];
}
// ����� ���� �̺�Ʈ�� �ڵ��� ����.


HANDLE	GetMainThreadUserDefineEvent(DWORD index)
{
	// 2007/12/19 �Լ� ���� ���� by yuchi
	//return g_hMainThreadEvent[index+EVENT_INDEX_USER_DEFINE_0];

	HANDLE		hEvent = NULL;

	for (DWORD i=0; i<g_dwUserEventNum; i++)
	{
		
		if (g_userEventDescList[i].dwEventIndex == index)
		{
			hEvent = g_userEventDescList[i].hEvent;
			break;
		}
	}
	return hEvent;
}
void ProcessPreConnectedItem()
{
	SetEvent(g_hMainThreadEvent[EVENT_INDEX_PRE_CONNECT]);

}
// ���� ������ ����.
// Modified by chan78 at 2002/07/22
BOOL StartMainThread(DWORD dwFlag)
{
	UINT ThreadID = 0;
	
	// ����� ���� �̺�Ʈ�� ������ �⺻���� �̺�Ʈ ��ü ����.
	g_hMainThreadEvent[EVENT_INDEX_MSG_EVENT]			= CreateEvent(NULL,FALSE,FALSE,NULL);
	g_hMainThreadEvent[EVENT_INDEX_DESTROY]			= CreateEvent(NULL,FALSE,FALSE,NULL);
	g_hMainThreadEvent[EVENT_INDEX_BREAK]				= CreateEvent(NULL,FALSE,FALSE,NULL);
	g_hMainThreadEvent[EVENT_INDEX_RESUME]			= CreateEvent(NULL,FALSE,FALSE,NULL);
	g_hMainThreadEvent[EVENT_INDEX_PRE_CONNECT]		= CreateEvent(NULL,FALSE,FALSE,NULL);
	g_hCheckBreakEvent								= CreateEvent(NULL,FALSE,FALSE,NULL);

	// ������ ����.��ŸƮ 
	g_hMainThread = (HANDLE)_beginthreadex(NULL,NULL,ProcessMsgThread,NULL,NULL,&ThreadID);
	

	if (0xffffffff == (DWORD)g_hMainThread)
	{
		MessageBox(NULL,"Fail to Create MainThread","Error",MB_OK);
		return FALSE;
	}

	return FALSE;

}
// ����� ���� �̺�Ʈ�� ����ϱ� ���� �� ������ �̸� ��������� �Ѵ�.
void SetMainThreadUserDefineEventNum(DWORD num)
{
	// ����� ����� ���� �̺�Ʈ ������ŭ �̺�Ʈ ��ü�� �����.

	g_dwMainThreadEventNum = MIM_MAIN_THREAD_EVENT_NUM + num;
	memset(pEventFunc,0,sizeof(VOIDFUNC)*MAX_MAIN_THREAD_USER_DEFINE_EVENT_NUM);

	memset(g_userEventDescList,0,sizeof(g_userEventDescList));

	g_dwUserEventNum = num;

	for (DWORD i=0; i<num; i++)
	{
		g_hMainThreadEvent[EVENT_INDEX_USER_DEFINE_0+i]	= CreateEvent(NULL,FALSE,FALSE,NULL);

		// 2007/12/19 yuchi
		g_userEventDescList[i].dwEventIndex = i;
		g_userEventDescList[i].hEvent = g_hMainThreadEvent[EVENT_INDEX_USER_DEFINE_0+i];

	}	
	
}

// ���� �����带 �����Ѵ�.
void EndMainThread()
{
	ResumeMainThread();

	// ���� �����尡 ������ �� �ֵ��� �̺�Ʈ�� �������ְ� ������ �����Ҷ����� ��ٸ���.
	SetEvent(g_hMainThreadEvent[EVENT_INDEX_DESTROY]);
	WaitForSingleObject(g_hMainThread,INFINITE);

	// �̺�Ʈ ��ü���� �ڵ��� �ݴ´�.
	for (DWORD i=0; i<g_dwMainThreadEventNum; i++)
	{
		CloseHandle(g_hMainThreadEvent[i]);
	}
	CloseHandle(g_hMainThread);
	CloseHandle(g_hCheckBreakEvent);
}


//
// �̰��� �ٷ� ���� �������� ��ü�̴�.
//
UINT WINAPI ProcessMsgThread(LPVOID lpVoid)
{	


	DWORD dwEventIndex	= 0;		// �̺�Ʈ�� � �������� �Ǻ��� �� �ִ� �ε����� �޾ƿ� ����.
	CMsgQue* pQ			= NULL;		// readable ť�� �����͸� �޾ƿ� ������ ����.
	MSG_CONTAINER*	pmc = NULL;		// �޽����� �ܾ�� �޽��� �����̳��� ������ ����.
	CConnection*	con = NULL;		// ��� ���ؼǿ��� ���ƿԴ����� �޾ƿ� CConnection ������ ����.


	if (g_pOnInitialFunc)
		g_pOnInitialFunc(NULL);

	while (TRUE)
	{
		if ((dwEventIndex = 
			WaitForMultipleObjects(
			g_dwMainThreadEventNum,
			g_hMainThreadEvent,
			FALSE,
			INFINITE)
			) == WAIT_FAILED)
		{
			// ���⼭ ������ �߻��ߴٸ� �̺�Ʈ ��ü�� ��������� �ʾҰų� ������� �̺�Ʈ ��ü�� �������� ��û�� ������ �������̴�.
			MessageBox(GetActiveWindow(),"Fail WaitForMultipleObject","Error",MB_OK);
		}

		if (dwEventIndex == EVENT_INDEX_DESTROY)	// ������ ����
			goto seg_destroy;							// �����ڵ�� ����
		

		if (dwEventIndex == EVENT_INDEX_BREAK)		// ���� �����忡 �극��ũ�� �ɾ���.
		{
			// �����Ҷ����� ���.
			SetEvent(g_hCheckBreakEvent);
			WaitForSingleObject(g_hMainThreadEvent[EVENT_INDEX_RESUME],INFINITE);
			continue;
		}
		if (dwEventIndex == EVENT_INDEX_PRE_CONNECT)
		{
			g_pPreConnectPool->ProcessPreConnect();
			continue;

		}

		if (dwEventIndex >= EVENT_INDEX_USER_DEFINE_0)
		{
			// ����� ���� �̺�Ʈ�� �߻��ߴ�.����� ���� �̺�Ʈ�� �ش��ϴ� �Լ��� �����Ѵ�.
			// Modified by chan78 at 2002/05/09

			// 2007/12/19 by yuchi
			DWORD		dwUserEventIndex = -1;
			
			for (DWORD i=0; i<g_dwUserEventNum; i++)
			{
				
				if (g_userEventDescList[i].hEvent == g_hMainThreadEvent[dwEventIndex])
				{
					
					
					dwUserEventIndex = g_userEventDescList[i].dwEventIndex;
					
					pEventFunc[dwUserEventIndex](dwUserEventIndex);

					
					// ��������� �����ϱ� ���ؼ�  �̹��� ��Ʈ�� �̺�Ʈ�� ���� �ڷ� ��ġ��Ų��.
					DWORD		dwNum = g_dwMainThreadEventNum - dwEventIndex - 1;
					// �̹��� ��Ʈ�� �̺�Ʈ�� �� �ڿ� ��ġ�ϴ� ���̶�� �ٲ� �ʿ����.
					if (dwNum)
					{
						
						HANDLE	hCurEvent = g_hMainThreadEvent[dwEventIndex];
						memcpy(g_hMainThreadEvent+dwEventIndex,g_hMainThreadEvent+dwEventIndex+1,sizeof(HANDLE)*dwNum);
						g_hMainThreadEvent[g_dwMainThreadEventNum-1] = hCurEvent;
					}
					
					break;
				}

			}
			//pEventFunc[(dwEventIndex-EVENT_INDEX_USER_DEFINE_0)]((dwEventIndex-EVENT_INDEX_USER_DEFINE_0));
			continue;
		}
	
		// ���� ����ġ �Ϸ� �̺�Ʈ�� ���󼭸� �޽����� ó���Ѵ�.
		pQ = g_pQueManager->GetReadableQue();	// ���� readable queue�� �����͸� ���´�.
				
		// �޽��� ť�� ����ִ� ��ŭ �Ѳ����� �� ó���Ѵ�.
		for (DWORD i=0; i<pQ->GetMsgNum(); i++)
		{
			// �޽��� �����̳ʸ� ���� �޽��� ������ �Ǻ��ϰ� �׿� �´� ó���� �Ѵ�.
			pmc = GetMsgContainer(pQ);	
			if (pmc->GetActionType() == ACTION_TYPE_DISCONNECT)
			{
				// disconnect�̺�Ʈ�̴�.�޽��� ���·� ���ƿ����� �̰� � ���� ������ ������̴�.
				if (pmc->GetActorType() == ACTOR_TYPE_USER)
				{
					// ���� ����� �������̶�� ������ ��Ʈ��ũ Ŭ�������� �ش� ���ؼ��� �����Ѵ�.
					// �����ϱ� ���� ������ ���涧 ����� �Լ��� ����������.
					g_pNetworkForUser->GetConnection(&con,pmc->GetConnectionIndex());
					if (con)
					{
						g_pNetworkForUser->OnDisconnect(pmc->GetConnectionIndex());
						g_pNetworkForUser->CloseConnection(pmc->GetConnectionIndex());
					}
				}
				else if (pmc->GetActorType() == ACTOR_TYPE_SERVER)
				{
					// ���� ����� �������̶�� ������ ��Ʈ��ũ Ŭ�������� �ش� ���ؼ��� �����Ѵ�.
					// �����ϱ� ���� ������ ���涧 ����� �Լ��� ����������.
					g_pNetworkForServer->GetConnection(&con,pmc->GetConnectionIndex());
					if (con)
					{
						g_pNetworkForServer->OnDisconnect(pmc->GetConnectionIndex());
						g_pNetworkForServer->CloseConnection(pmc->GetConnectionIndex());
					}
				}
			}
			else if (pmc->GetActionType() == ACTION_TYPE_TCP)
			{
				// TCP��Ŷ�� ������ ����̴�.
				if (pmc->GetActorType() == ACTOR_TYPE_USER)
				{
					// ���������κ��� TCP��Ŷ�� �޾Ҵٸ�...��� connection���� ���´��� ã�Ƽ� 
					// tcp��Ŷ�� ó���ϴ� �Լ��� �ѱ��.
					g_pNetworkForUser->GetConnection(&con,pmc->GetConnectionIndex());
					if (con)
						g_pNetworkForUser->OnRecvTCP(pmc->GetConnectionIndex(),pmc->GetMsg(),pmc->GetMsgLength());
				}
				else if (pmc->GetActorType() == ACTOR_TYPE_SERVER)
				{
					// ���������κ��� TCP��Ŷ�� �޾Ҵٸ� ���� ��� ����connection���� ã�Ƽ� 
					// tcp��Ŷ�� ó���ϴ� �Լ��� �ѱ��.
					g_pNetworkForServer->GetConnection(&con,pmc->GetConnectionIndex());
					if (con)
						g_pNetworkForServer->OnRecvTCP(pmc->GetConnectionIndex(),pmc->GetMsg(),pmc->GetMsgLength());
				}
			}
		}
		pQ->Clear();
		SetEvent(g_hProcessComplete);
	}
seg_destroy:
	SetEvent(g_hProcessComplete);
	_endthreadex(0);
	return 0;
}
