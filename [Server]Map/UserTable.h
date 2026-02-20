// UserTable.h: interface for the CUserTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERTABLE_H__5BBB3509_10AF_4E68_9B3A_C07E9F23D2A9__INCLUDED_)
#define AFX_USERTABLE_H__5BBB3509_10AF_4E68_9B3A_C07E9F23D2A9__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#include "ServerSystem.h"
//#include <HashTable.h>



class CSkillArea;

#ifdef __MAPSERVER__

class CUserTable : public CYHHashTable<CObject>
{
	DWORD m_dwUserCount;
	DWORD m_dwMonsterCount;
	DWORD m_dwNpcCount;
	DWORD m_dwExtraCount;
public:
	CUserTable();
	virtual ~CUserTable();

	void Init(DWORD dwBucket);
	CObject * FindUser(DWORD dwKey);
	CObject * FindUserForName( char* strCharacterName );
	BOOL AddUser(CObject* pObject,DWORD dwKey);
	CObject * RemoveUser(DWORD dwKey);
	void RemoveAllUser();
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// 06. 08. 2  - ̿
	///  GetTargetInRange Լ  ڷ Ÿ ߰
	void GetTargetInRange(VECTOR3* pPivotPos,float Radius,CTargetList* pTList,float SafeRange = 0);
	///////////////////////////////////////////////////////////////////////////////////////////////////
	void GetTargetInArea(CSkillArea* pSkillArea,CTargetList* pTList);
	void GetTargetInDonutRangeInChannel(DWORD MonGridID, VECTOR3* pPivotPos,float MaxRadius, float MinRadius, CObject* rtpObject);

	
	DWORD GetUserCount(){ return m_dwUserCount; }
	DWORD GetMonsterCount(){ return m_dwMonsterCount; }
	DWORD GetNpcCount(){ return m_dwNpcCount; }
	DWORD GetExtraCount(){ return m_dwExtraCount; }

	void SetPositionUserHead() {  CYHHashTable<CObject>::SetPositionHead(); }
	CObject * GetUserData() { return CYHHashTable<CObject>::GetData(); }

};
#else

BOOL DisconnectUser(DWORD dwConnectionIndex);
BOOL DisconnectUser( DWORD dwConnectionIndex, int flag );

#include "..\[Server]Agent\Crypt.h"

struct aGAMEOPTION
{
	BYTE bNoFriend;
	BYTE bNoWhisper;

	aGAMEOPTION():bNoWhisper(0), bNoFriend(0) {}
};

typedef struct tagUSERINFO
{
	DWORD dwConnectionIndex;
	DWORD dwCharacterID;
	DWORD dwUserID;
	BYTE UserLevel;
	DWORD dwMapServerConnectionIndex;
	WORD  wUserMapNum;
	DWORD dwCharacterIDS[4];	//   ĳȣ ϱ
	BYTE bLoginMapNum[4];

#ifdef _AGENTSERVER
	DWORD DistAuthKey;			// Distribute   AuthKey 
#else
	DWORD State;
	BOOL bRecvDistOut;
#endif
	
#ifdef _NETWORKTEST
	DWORD dwStartTrafficTime;
	DWORD dwTrafficForUserSend[MP_MAX];
	DWORD dwTrafficForServerSend[MP_MAX];
	DWORD dwPacketForUserSend[MP_MAX];
	DWORD dwPacketForServerSend[MP_MAX];
	void AddPacketForUser(DWORD c, DWORD size)
	{
		dwTrafficForUserSend[c] += size;
		++dwPacketForUserSend[c];
	}
	void AddPacketForServer(DWORD c, DWORD size)
	{
		dwTrafficForServerSend[c] += size;
		++dwPacketForServerSend[c];
	}
#endif

	DWORD dwUniqueConnectIdx;	//    ȣ
								// dwConnectionIdxʹ ٸ.  

	CCrypt crypto;
	BYTE CheckSum;

	aGAMEOPTION GameOption;

	BOOL CheckCorrectCheckSum(BYTE CS)
	{
		if(CS != CheckSum)
			return FALSE;
		++CheckSum;
		return TRUE;
	}

	DWORD dwLastConnectionCheckTime;
	BOOL m_ConnectionCheckFailed;
}USERINFO;

class CUserTable : public CYHHashTable<USERINFO>
{

	DWORD m_MaxUserCount;

	DWORD m_dwUserCount;


	DWORD m_addCount;
	DWORD m_removeCount;

public:
	CUserTable();
	virtual ~CUserTable();
	void SetPositionUserHead() {  CYHHashTable<USERINFO>::SetPositionHead(); }
	USERINFO * GetUserData() { return CYHHashTable<USERINFO>::GetData(); }
	void Init(DWORD dwBucket);
	USERINFO * FindUser(DWORD dwKey);
	BOOL AddUser(USERINFO* pObject,DWORD dwKey);
	USERINFO * RemoveUser(DWORD dwKey);
	void RemoveAllUser();

	DWORD GetUserCount(){ return m_dwUserCount; }

	DWORD GetUserMaxCount(){return m_MaxUserCount;}
	void SetCalcMaxCount(DWORD CurCount);
	
	BOOL SendToUser(DWORD dwKey,DWORD dwUniqueConnectIdx,MSGBASE* pmsg,DWORD size);
	BOOL OnDisconnectUser(DWORD dwKey);

	DWORD GetAddCount() { return m_addCount; }
	DWORD GetRemoveCount() { return m_removeCount; }
};
#endif
  


extern CUserTable * g_pUserTable;							// key : connectionidx
#ifdef __AGENTSERVER__
extern CUserTable * g_pUserTableForUserID;					// key : UserID
extern CUserTable * g_pUserTableForObjectID;				// key : dwObjectID
#endif
#endif // !defined(AFX_USERTABLE_H__5BBB3509_10AF_4E68_9B3A_C07E9F23D2A9__INCLUDED_)
