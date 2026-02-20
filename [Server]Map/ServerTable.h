// ServerTable.h: interface for the CServerTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERTABLE_H__C42B519F_767C_4291_B2DA_5F077251C100__INCLUDED_)
#define AFX_SERVERTABLE_H__C42B519F_767C_4291_B2DA_5F077251C100__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//--------------------------------------------------------------------------
// ���� ������ ������ �ִ� Ŭ����
// ������ �ε����� �ʴ´�.
// �������ý�ŸƮ�� ���� �ʴ´�.
//--------------------------------------------------------------------------
#include "..\[CC]Header\ServerGameStruct.h"
#include "D:\mxym\[Lib]YHLibrary\hashtable.h"


enum eSERVER_KIND
{
	eSK_NONE,
	eSK_DIST	= 1,
	eSK_AGENT	= 2,
	eSK_MAP		= 3,
};


class CServerTable  : public CYHHashTable<SERVERINFO>
{
/*
#ifdef __AGENTSERVER__
	WORD				m_MapIDMapNum[MAX_MAP_NUM];		// MapID����ϴ� ����.
#endif //__AGENTSERVER__*/


	//DWORD		m_dwMaxBucketNum;
	DWORD			m_MaxServerConnectionIndex;
	
public:
	CServerTable();
	virtual ~CServerTable();
	void Init(DWORD dwBucketNum);
	void Release();


	//---- ���� ó�� ���(�ؽ��� ó��)
	SERVERINFO * FindServer(WORD Port);
	SERVERINFO* FindServerForConnectionIndex(DWORD dwConnectionIndex);
	void AddServer(SERVERINFO * info, WORD Port);
	SERVERINFO * RemoveServer(DWORD dwConnectionIndex);			// key�� �ƴ� & ���ϵǴ� SERVERINFO(�� �޸�)������ ��
	SERVERINFO * RemoveServer(WORD wKey);
	void RemoveServerForKind(WORD wSrvKind);					// ���� �������� ����
	void RemoveAllServer();										// HASH�� �����ִ� SERVERINFO(���޸�)���� �� ����

	SERVERINFO* GetNextServer();
	SERVERINFO* GetNextDistServer();
	SERVERINFO* GetNextMapServer();
	SERVERINFO* GetNextAgentServer();
	
	void AddSelfServer(SERVERINFO * info);
	void AddMSServer(SERVERINFO * info);
	//---- ���� ��� ���
	SERVERINFO * GetSelfServer(){ return m_pSelfServerInfo; }
	SERVERINFO * GetMSServer(){ return m_pMSServerInfo; }

	WORD GetServerPort(WORD ServerKind, WORD ServerNum);
	WORD GetServerNum(WORD ServerPort);
/*
#ifdef __AGENTSERVER__
	void SetMapRegist(BYTE MapNum, WORD port)
	{
		m_MapIDMapNum[MapNum] = port; 
	}
	WORD GetMapRegistPort(BYTE MapNum)
	{
		return m_MapIDMapNum[MapNum]; 
	}
	BYTE GetMapRegistMapNum(WORD Port)
	{
		for(int i=0; i<MAX_MAP_NUM; i++)
		{
			if(m_MapIDMapNum[i] == Port)
				return i;
		}
		return 0;
	}
#endif //__AGENTSERVER__*/

	DWORD GetMaxServerConnectionIndex(){return m_MaxServerConnectionIndex;}
	void SetMaxServerConnectionIndex(DWORD val){ m_MaxServerConnectionIndex=val; }	
	
	DWORD GetFastServer(WORD ServerKind);
	BOOL GetFastServer(WORD ServerKind, char* pOutIP,WORD* pOutPort);

protected:
	SERVERINFO * m_pSelfServerInfo;
	SERVERINFO * m_pMSServerInfo;
};

//extern char * SERVER_KIND_STR[];
extern CServerTable * g_pServerTable;

#endif // !defined(AFX_SERVERTABLE_H__C42B519F_767C_4291_B2DA_5F077251C100__INCLUDED_)
