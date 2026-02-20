#pragma once 
#include "..\[CC]Header\CommonStruct.h"
#define PARTYMGR CPartyManager::GetInstance()


class CPlayer;
class CParty;
struct PARTYMEMBER;


class CPartyManager {
	CYHHashTable<CParty> m_partyRoomHashTable;	

	CPartyManager();

public:
	GETINSTANCE(CPartyManager);

	virtual ~CPartyManager();

	CParty* GetParty(DWORD partyIDx); 
	
	CParty* RegistParty(DWORD PartyIDX);
	// 2008. 5. 21. CBH - Ƽ  ߰ ɼ   ////
	//void CreatePartyQuery(CPlayer * pMaster, BYTE bOption);
	//void CreatePartyResult(CPlayer * pMaster, DWORD PartyIDX, BYTE Option);
	void CreatePartyQuery(CPlayer * pMaster, PARTY_ADDOPTION* pAddOption);
	void CreatePartyResult(CPlayer * pMaster, PARTY_ADDOPTION* pAddOption);
	//////////////////////////////////////////////////////////

	void AddMember(DWORD PartyID, DWORD TargetPlayerID);
	void AddMemberResult(CParty* pParty, CPlayer* pNewMember);
	void AddMemberNack(DWORD PartyID, DWORD TargetPlayerID);
	void DelMember(DWORD PlayerID,DWORD PartyID);
	void DelMemberSyn(DWORD PlayerID, DWORD PartyID);
	void RemoveParty(DWORD DeletedPlayerID, DWORD PartyID);
	void BanParty(DWORD PartyID,DWORD PlayerID,DWORD TargetPlayerID);
	void ChangeMasterParty(DWORD PartyID,DWORD PlayerID,DWORD TargetPlayerID);
	void DoChangeMasterParty(CParty* pParty, DWORD TargetID);
	void BreakupParty(DWORD PartyID,DWORD PlayerID);

	void UserLogIn(CPlayer* pPlayer,BOOL bNotifyUserLogin);
	void UserLogOut(CPlayer* pPlayer);

	void AddPartyInvite(DWORD MasterID, DWORD TargetID);

	//  α  Ƽ  DB Ƽ  ȣϴ Լ
	//2008. 5. 22. CBH -  ߰ ɼ  
	//void UserLogInAfterDBQuery(DWORD CharacterID,DWORD PartyIdx,PARTYMEMBER* PartyInfo, BYTE Option);
	void UserLogInAfterDBQuery(DWORD CharacterID,DWORD PartyIdx,PARTYMEMBER* PartyInfo, PARTY_ADDOPTION AddOption);

	void NotifyChangesOtherMapServer(DWORD TargetPlayerID,CParty* pParty, BYTE Protocol, DWORD PartyIDX, LEVELTYPE Level=0);

	//void NotifyCreateParty(DWORD PartyIDX);
	void NotifyCreateParty(PARTY_INFO* pPartyInfo);

	void NotifyAddParty(SEND_CHANGE_PARTY_MEMBER_INFO* pmsg);
	void NotifyBanParty(SEND_CHANGE_PARTY_MEMBER_INFO* pmsg);
	void NotifyChangeMasterParty(SEND_CHANGE_PARTY_MEMBER_INFO* pmsg);
	void NotifyBreakupParty(SEND_CHANGE_PARTY_MEMBER_INFO* pmsg);
	void NotifyDelParty(SEND_CHANGE_PARTY_MEMBER_INFO* pmsg);
	void NotifyDelSyn(MSG_DWORD2* pmsg);
	void NotifyUserLogIn(SEND_CHANGE_PARTY_MEMBER_INFO* pmsg);
	void NotifyUserLogOut(SEND_CHANGE_PARTY_MEMBER_INFO* pmsg);
	void NotifyMemberLoginMsg(DWORD PartyID, DWORD PlayerID);
	
	void SendErrMsg(DWORD PlayerID, int ErrKind, BYTE Protocol);
	BOOL SendPartyMsg(DWORD PartyID, MSGBASE* pmsg, int msglen);
	void SetTacticObjectID(DWORD PartyID,DWORD TacticObjectID);
	
	void MemberLevelUp(DWORD PartyIDX, DWORD PlayerIDX, LEVELTYPE lvl);
	void DoMemberLevelUp(DWORD PartyIDX, DWORD PlayerIDX, LEVELTYPE lvl);
	
	void SendObtainItemMsg(CPlayer* pPlayer, WORD ObtainIdx);

	void MasterToPartyRequest(CPlayer* pPlayer, MSG_NAME_DWORD2* pMsg);	// 2008. 5. 23. CBH - 忡  û ////
	void MasterToPartyRequestErr(DWORD dwPlayerID, DWORD dwErrIndex);	//2008. 5. 23. CBH -  û 	
	void NotifyPartyInfoSyn(DWORD dwPartyID);		//2008. 5. 27. CBH - ش    ʼ  Լ
	void NotifyPartyInfo(PARTY_INFO* pPartyInfo);		//2008. 5. 27. CBH - Ƽ  Ѵ.
	void PartyMatchingInfo(CPlayer* pPlayer, MSG_WORD* pMsg);		//2008. 5. 28. CBH - ĸĪ Ʈ 
	void Process();		//2008. 6. 4. CBH - Ľû īƮ ó
};

