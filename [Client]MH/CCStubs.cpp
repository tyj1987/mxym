// stdafx.h must be first
#include "stdafx.h"

// MHClient Stub Implementations for Missing CC Module Methods
// This file provides minimal implementations for methods that are NOT implemented
// in the client source files being compiled directly into MHClient

#ifdef _MHCLIENT_

// Include client-side headers that have the method declarations
#include "../[CC]Skill/SkillManager_client.h"
#include "../[CC]Skill/SkillInfo.h"
#include "../[CC]Skill/SkillAreaManager.h"
#include "../[CC]Suryun/SuryunManager_client.h"
#include "../[CC]BattleSystem/Battle.h"
#include "../[CC]BattleSystem/BattleSystem_Client.h"
#include "../[CC]BattleSystem/SiegeWar/Battle_SiegeWar.h"
#include "../[CC]BattleSystem/SiegeWar/BattleFactory_SiegeWar.h"
#include "../[CC]BattleSystem/SiegeWar/BattleTeam_SiegeWar.h"
#include "../[CC]BattleSystem/GTournament/Battle_GTournament.h"
#include "../[CC]BattleSystem/GTournament/BattleFactory_GTournament.h"
#include "../[CC]BattleSystem/GTournament/BattleTeam_GTournament.h"
#include "../[CC]BattleSystem/BattleTeam.h"
#include "../[CC]Ability/AbilityUpdater.h"
#include "../[CC]Ability/AbilityUpdater_Interface.h"
#include "../[CC]Ability/AbilityUpdater_ApplyData.h"
#include "../[CC]Ability/Ability.h"
#include "../[CC]Ability/AbilityManager.h"
#include "../[CC]Ability/AbilityGroup.h"
#include "../[CC]Ability/AbilityIcon.h"
#include "../[CC]Ability/AbilityUse.h"
#include "../[CC]Ability/AbilityUse_Society.h"
#include "../[CC]Skill/DelayGroup.h"
#include "../[CC]Skill/SpecialState.h"
#include "../[CC]Skill/StunState.h"
#include "../[CC]Skill/AmplifiedPower.h"
#include "../[CC]Quest/SubQuestInfo.h"
#include "../[CC]Quest/QuestScriptLoader.h"
#include "../[CC]Quest/QuestString.h"
#include "../[CC]Quest/QuestInfo.h"
#include "MHFile.h"
#include "Hero.h"
#include "Object.h"
#include "ItemManager.h"
#include "Player.h"

///////////////////////////////////////////////////////////////////////////////
// CMHFile Stub Implementations
///////////////////////////////////////////////////////////////////////////////

void CMHFile::GetString(char* str)
{
	if (str) str[0] = '\0';
}

int CMHFile::GetLevel()
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CSkillManager Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CSkillManager::CSkillManager()
{
}

CSkillManager::~CSkillManager()
{
}

void CSkillManager::Init()
{
}

CSkillInfo* CSkillManager::GetSkillInfo(WORD SkillInfoIdx)
{
	return m_SkillInfoTable.GetData(SkillInfoIdx);
}

CSkillObject* CSkillManager::GetSkillObject(DWORD SkillObjID)
{
	return m_SkillObjectTable.GetData(SkillObjID);
}

void CSkillManager::NetworkMsgParse(BYTE Protocol, void* pMsg)
{
}

BOOL CSkillManager::OnSkillCommand(CHero* pHero, CActionTarget* pTarget, BOOL bMugong)
{
	return TRUE;
}

void CSkillManager::OnComboTurningPoint(CHero* pHero)
{
}

void CSkillManager::UpdateSkillObjectTargetList(CObject* pObject)
{
}

void CSkillManager::RemoveTarget(CObject* pObject, BYTE bTargetKind)
{
}

void CSkillManager::DeleteTempSkill()
{
}

SKILL_CHANGE_INFO* CSkillManager::GetSkillChangeInfo(WORD wMugongIdx)
{
	return NULL;
}

BOOL CSkillManager::IsDeadlyMugong(WORD wMugongIdx)
{
	return FALSE;
}

BOOL CSkillManager::ExcuteSkillSYN(CHero* pHero, CActionTarget* pTarget, BOOL bMugong)
{
	return TRUE;
}

BOOL CSkillManager::ExcuteSkillSYN(CHero* pHero, CActionTarget* pTarget, CSkillInfo* pSkillInfo)
{
	return TRUE;
}

BOOL CSkillManager::RealExcuteSkillSYN(CHero* pHero, CActionTarget* pTarget, CSkillInfo* pSkillInfo)
{
	return TRUE;
}

BOOL CSkillManager::ExcuteTacticSkillSYN(CHero* pHero, TACTIC_TOTALINFO* pTInfo, BYTE OperNum)
{
	return TRUE;
}

BOOL CSkillManager::CheckSkillKind(WORD wSkillKind)
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// CSuryunManager Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CSuryunManager::CSuryunManager()
{
}

CSuryunManager::~CSuryunManager()
{
}

DWORD CSuryunManager::GetSuryunFee(SURYUNINFO* pInfo)
{
	return 0;
}

void CSuryunManager::GoSuryunMapSyn()
{
}

void CSuryunManager::NetworkMsgParse(BYTE Protocol, void* pMsg)
{
}

///////////////////////////////////////////////////////////////////////////////
// CSkillInfo Stub Implementations
///////////////////////////////////////////////////////////////////////////////

int CSkillInfo::IsInSkillRange(CHero* pHero, CActionTarget* pTarget, SKILLOPTION* pOption)
{
	return TRUE;
}

int CSkillInfo::IsExcutableSkillState(CHero* pHero, int option, SKILLOPTION* pOption)
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CSkillArea Stub Implementations
///////////////////////////////////////////////////////////////////////////////

int CSkillArea::IsInDamageTile(VECTOR3* pObjPos)
{
	return FALSE;
}

CSkillArea::CSkillArea(CSkillAreaData* pData)
{
}

CSkillArea::~CSkillArea()
{
}

///////////////////////////////////////////////////////////////////////////////
// CSkillAreaManager Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CSkillAreaManager::CSkillAreaManager()
{
}

CSkillAreaManager::~CSkillAreaManager()
{
}

///////////////////////////////////////////////////////////////////////////////
// CBattle Abstract Base Class Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattle::CBattle()
{
}

CBattle::~CBattle()
{
}

void CBattle::Initialize(BATTLE_INFO_BASE* pInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2)
{
}

void CBattle::OnCreate(BATTLE_INFO_BASE* pInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2)
{
}

void CBattle::OnDestroy()
{
}

void CBattle::OnFightStart()
{
}

void CBattle::OnTeamMemberAdd(int idx, DWORD dwCharID, char* name)
{
}

int CBattle::OnTeamMemberDelete(int idx, DWORD dwCharID, char* name)
{
	return 0;
}

int CBattle::OnTeamMemberDie(int idx, DWORD dwCharID, DWORD dwKillerID)
{
	return 0;
}

void CBattle::OnTeamMemberRevive(int idx, CObject* pObj)
{
}

void CBattle::OnTeamObjectCreate(int idx, CBattleObject* pObj)
{
}

int CBattle::IsEnemy(CObject* pObj1, CObject* pObj2)
{
	return FALSE;
}

int CBattle::IsFriend(CObject* pObj1, CObject* pObj2)
{
	return TRUE;
}

void CBattle::StartBattle()
{
}

int CBattle::Judge()
{
	return 0;
}

DWORD CBattle::GetBattleTeamID(CObject* pObj)
{
	return 0;
}

void CBattle::Draw()
{
}

void CBattle::OnMonsterDistribute(CMonster* pMonster, CObject* pObj)
{
}

void CBattle::OnTeamObjectDestroy(int idx, CBattleObject* pObj)
{
}

void CBattle::OnTick()
{
}

void CBattle::Render()
{
}

void CBattle::Victory(int team1, int team2)
{
}

///////////////////////////////////////////////////////////////////////////////
// CBattle_SiegeWar Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattle_SiegeWar::CBattle_SiegeWar()
{
}

CBattle_SiegeWar::~CBattle_SiegeWar()
{
}

void CBattle_SiegeWar::Initialize(BATTLE_INFO_BASE* pInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2)
{
}

void CBattle_SiegeWar::OnCreate(BATTLE_INFO_BASE* pInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2)
{
}

void CBattle_SiegeWar::OnDestroy()
{
}

void CBattle_SiegeWar::OnTeamMemberAdd(int idx, DWORD dwCharID, char* name)
{
}

int CBattle_SiegeWar::OnTeamMemberDie(int idx, DWORD dwCharID, DWORD dwKillerID)
{
	return 0;
}

void CBattle_SiegeWar::Render()
{
}

void CBattle_SiegeWar::OnFightStart()
{
}

int CBattle_SiegeWar::IsEnemy(CObject* pObj1, CObject* pObj2)
{
	return FALSE;
}

int CBattle_SiegeWar::IsFriend(CObject* pObj1, CObject* pObj2)
{
	return TRUE;
}

void CBattle_SiegeWar::SetBattleInfo(SEND_SW_BTGUILDLIST* pList)
{
}

void CBattle_SiegeWar::SetSiegeWarName(CObject* pObject, DWORD dwGuildID1, DWORD dwGuildID2)
{
}

DWORD CBattle_SiegeWar::GetBattleTeamID(CObject* pObj)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CBattleTeam_SiegeWar Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattleTeam_SiegeWar::CBattleTeam_SiegeWar()
{
}

CBattleTeam_SiegeWar::~CBattleTeam_SiegeWar()
{
}

///////////////////////////////////////////////////////////////////////////////
// CBattleFactory_SiegeWar Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattleFactory_SiegeWar::CBattleFactory_SiegeWar()
{
}

///////////////////////////////////////////////////////////////////////////////
// CBattle_GTournament Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattle_GTournament::CBattle_GTournament()
{
}

CBattle_GTournament::~CBattle_GTournament()
{
}

void CBattle_GTournament::Initialize(BATTLE_INFO_BASE* pInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2)
{
}

void CBattle_GTournament::OnCreate(BATTLE_INFO_BASE* pInfo, CBattleTeam* pTeam1, CBattleTeam* pTeam2)
{
}

void CBattle_GTournament::OnDestroy()
{
}

void CBattle_GTournament::OnTeamMemberAdd(int idx, DWORD dwCharID, char* name)
{
}

int CBattle_GTournament::OnTeamMemberDie(int idx, DWORD dwCharID, DWORD dwKillerID)
{
	return 0;
}

void CBattle_GTournament::OnTick()
{
}

int CBattle_GTournament::IsEnemy(CObject* pObj1, CObject* pObj2)
{
	return FALSE;
}

int CBattle_GTournament::IsFriend(CObject* pObj1, CObject* pObj2)
{
	return TRUE;
}

int CBattle_GTournament::Judge()
{
	return 0;
}

void CBattle_GTournament::OnFightStart()
{
}

void CBattle_GTournament::Victory(int team1, int team2)
{
}

void CBattle_GTournament::Draw()
{
}

void CBattle_GTournament::Render()
{
}

DWORD CBattle_GTournament::GetBattleTeamID(CObject* pObj)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CBattleTeam_GTournament Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattleTeam_GTournament::CBattleTeam_GTournament()
{
}

CBattleTeam_GTournament::~CBattleTeam_GTournament()
{
}

///////////////////////////////////////////////////////////////////////////////
// CBattleFactory_GTournament Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattleFactory_GTournament::CBattleFactory_GTournament()
{
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityUpdater Base Class Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityUpdater::CAbilityUpdater()
{
}

CAbilityUpdater::~CAbilityUpdater()
{
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityUpdater_Interface Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityUpdater_Interface::CAbilityUpdater_Interface()
{
}

CAbilityUpdater_Interface::~CAbilityUpdater_Interface()
{
}

void CAbilityUpdater_Interface::Update(DWORD& Flag, DWORD Param, CAbilityGroup* pGroup, CYHHashTable<CAbilityInfo>* pInfoTable)
{
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityUpdater_ApplyData Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityUpdater_ApplyData::CAbilityUpdater_ApplyData()
{
}

CAbilityUpdater_ApplyData::~CAbilityUpdater_ApplyData()
{
}

void CAbilityUpdater_ApplyData::Update(DWORD& UpdateCmd, DWORD Param, CAbilityGroup* pGroup, CYHHashTable<CAbilityInfo>* pInfoTable)
{
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityUse Base Class Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityUse::CAbilityUse()
{
}

CAbilityUse::~CAbilityUse()
{
}

///////////////////////////////////////////////////////////////////////////////
// CAbility Stub Implementations
///////////////////////////////////////////////////////////////////////////////

BYTE CAbility::GetLevel()
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CBattleTeam Base Class Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattleTeam::CBattleTeam()
{
}

CBattleTeam::~CBattleTeam()
{
}

int CBattleTeam::IsTeamMember(CObject* pObj)
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// CBattleTeam_SiegeWar Additional Methods
///////////////////////////////////////////////////////////////////////////////

int CBattleTeam_SiegeWar::IsAddableTeamMember(CObject* pObj)
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// CBattleTeam_GTournament Additional Methods
///////////////////////////////////////////////////////////////////////////////

void CBattleTeam_GTournament::DoAddTeamMember(CObject* pObj)
{
}

void CBattleTeam_GTournament::DoDeleteTeamMember(CObject* pObj)
{
}

///////////////////////////////////////////////////////////////////////////////
// CBattleFactory_SiegeWar Additional Methods
///////////////////////////////////////////////////////////////////////////////

CBattleFactory_SiegeWar::~CBattleFactory_SiegeWar()
{
}

CBattle* CBattleFactory_SiegeWar::CreateBattle(BATTLE_INFO_BASE* pInfo, WORD wIdx)
{
	return NULL;
}

void CBattleFactory_SiegeWar::DeleteBattle(CBattle* pBattle)
{
}

///////////////////////////////////////////////////////////////////////////////
// CBattleFactory_GTournament Additional Methods
///////////////////////////////////////////////////////////////////////////////

CBattleFactory_GTournament::~CBattleFactory_GTournament()
{
}

CBattle* CBattleFactory_GTournament::CreateBattle(BATTLE_INFO_BASE* pInfo, WORD wIdx)
{
	return NULL;
}

void CBattleFactory_GTournament::DeleteBattle(CBattle* pBattle)
{
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityManager Additional Methods
///////////////////////////////////////////////////////////////////////////////

int CAbilityManager::SetAbilityQuickPosition(WORD wOldPos, WORD wNewPos, CAbilityGroup* pGroup)
{
	return 0;
}

int CAbilityManager::CanUpgrade(CAbilityInfo* pInfo, CAbilityGroup* pGroup)
{
	return 1;
}

int CAbilityManager::CanDowngradeSkPoint(CAbilityInfo* pInfo, CAbilityGroup* pGroup)
{
	return 1;
}

int CAbilityManager::CheckCanResetPoint(CPlayer* pPlayer, CAbilityInfo* pInfo)
{
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Quest Related Stub Implementations
///////////////////////////////////////////////////////////////////////////////

DWORD CSubQuestInfo::GetNpcMarkType(DWORD dwNpcID)
{
	return 0;
}

int CSubQuestInfo::IsQuestState(DWORD dwState)
{
	return 0;
}

int CSubQuestInfo::CheckLimitCondition(DWORD dwParam)
{
	return 1;
}

CQuestString::CQuestString()
{
}

CQuestString* CQuestScriptLoader::LoadQuestString(CMHFile* pFile)
{
	return NULL;
}

CQuestInfo* CQuestScriptLoader::LoadQuestInfo(CMHFile* pFile, DWORD dwQuestID)
{
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityManager Full Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityManager g_CAbilityManager;

CAbilityManager::CAbilityManager()
{
}

CAbilityManager::~CAbilityManager()
{
}

void CAbilityManager::Init()
{
}

void CAbilityManager::AddUpdater(CAbilityUpdater* pUpdater)
{
}

CAbilityInfo* CAbilityManager::GetAbilityInfo(WORD wIdx)
{
	return NULL;
}

BYTE CAbilityManager::GetAbilityLevel(WORD wIdx, CAbilityGroup* pGroup)
{
	return 0;
}

int CAbilityManager::UpgradeAbility(WORD wIdx, CAbilityGroup* pGroup)
{
	return 1;
}

int CAbilityManager::UpgradeAbilitySkPoint(WORD wIdx, CAbilityGroup* pGroup)
{
	return 1;
}

int CAbilityManager::DowngradeAbilitySkPoint(WORD wIdx, CAbilityGroup* pGroup)
{
	return 1;
}

int CAbilityManager::ResetAbilitySkPoint(WORD wIdx, CAbilityGroup* pGroup)
{
	return 1;
}

void CAbilityManager::UpdateAbilityState(DWORD dwFlag, DWORD Param, CAbilityGroup* pGroup)
{
}

int CAbilityManager::CheckAbilityLevel(WORD wIdx, CAbilityGroup* pGroup, BYTE level)
{
	return 1;
}

float CAbilityManager::GetAbilityKyungGongSpeed(WORD wIdx)
{
	return 1.0f;
}

void CAbilityManager::InitAbility(CPlayer* pPlayer, ABILITY_TOTALINFO* pInfo, CAbilityGroup* pGroup)
{
}

void CAbilityManager::InitAbilityExp(CAbilityGroup* pGroup, DWORD dwExp)
{
}

int CAbilityManager::CanUpgradeSkPoint(CAbilityInfo* pInfo, CAbilityGroup* pGroup)
{
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityGroup Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityGroup::CAbilityGroup()
{
}

CAbilityGroup::~CAbilityGroup()
{
}

void CAbilityGroup::SetAbilityExp(DWORD dwExp)
{
}

CAbility* CAbilityGroup::GetAbility(WORD wKind)
{
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// CAbility Additional Method Stubs
///////////////////////////////////////////////////////////////////////////////

WORD CAbility::GetKind()
{
	return 0;
}

void CAbility::Use()
{
}

///////////////////////////////////////////////////////////////////////////////
// CDelayGroup Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CDelayGroup::CDelayGroup()
{
}

CDelayGroup::~CDelayGroup()
{
}

void CDelayGroup::Init()
{
}

void CDelayGroup::AddDelay(WORD wType1, WORD wType2, DWORD dwDelay, DWORD dwParam)
{
}

float CDelayGroup::CheckTitanPotionDelay(DWORD* pdwResult)
{
	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
// CBattleSystem Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CBattleSystem::CBattleSystem()
{
}

CBattleSystem::~CBattleSystem()
{
}

CBattle* CBattleSystem::GetBattle(DWORD dwID)
{
	return NULL;
}

CBattle* CBattleSystem::GetBattle(CHero* pHero)
{
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// CBattleFactory Base Class Stubs
///////////////////////////////////////////////////////////////////////////////

CBattleFactory::CBattleFactory()
{
}

CBattleFactory::~CBattleFactory()
{
}

///////////////////////////////////////////////////////////////////////////////
// CSpecialState Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CSpecialState::~CSpecialState()
{
}

void CSpecialState::Init()
{
}

void CSpecialState::Process(DWORD dwTick)
{
}

///////////////////////////////////////////////////////////////////////////////
// CStunState Stub Implementations
///////////////////////////////////////////////////////////////////////////////

// 添加默认构造函数
CStunState::CStunState()
    : CSpecialState(NULL)
{
}

CStunState::CStunState(CObject* pObj)
{
}

///////////////////////////////////////////////////////////////////////////////
// CAmplifiedPowerPhy Stub Implementations
///////////////////////////////////////////////////////////////////////////////

// 添加默认构造函数
CAmplifiedPowerPhy::CAmplifiedPowerPhy()
    : CSpecialState(NULL)
{
}

CAmplifiedPowerPhy::CAmplifiedPowerPhy(CObject* pObj)
{
}

///////////////////////////////////////////////////////////////////////////////
// CAmplifiedPowerAtt Stub Implementations
///////////////////////////////////////////////////////////////////////////////

// 添加默认构造函数
CAmplifiedPowerAtt::CAmplifiedPowerAtt()
    : CSpecialState(NULL)
{
}

CAmplifiedPowerAtt::CAmplifiedPowerAtt(CObject* pObj)
{
}

///////////////////////////////////////////////////////////////////////////////
// CSkillArea Additional Stubs
///////////////////////////////////////////////////////////////////////////////

// 注意：IsInDamageTile已在SkillArea.cpp中实现，此处移除重复定义

///////////////////////////////////////////////////////////////////////////////
// CAbilityIcon Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityIcon::CAbilityIcon()
{
}

void CAbilityIcon::InitAbilityIcon(CAbilityInfo* pInfo)
{
}

void CAbilityIcon::UpdateState(int x, int y)
{
}

void CAbilityIcon::RenderInfo(VECTOR2* pPos)
{
}

///////////////////////////////////////////////////////////////////////////////
// CAbilityUse_Society Stub Implementations
///////////////////////////////////////////////////////////////////////////////

CAbilityUse_Society::CAbilityUse_Society()
{
}

CAbilityUse_Society::~CAbilityUse_Society()
{
}

void CAbilityUse_Society::Act(CObject* pObj, CAbilityInfo* pInfo)
{
}

///////////////////////////////////////////////////////////////////////////////
// CSubQuestInfo Additional Stubs
///////////////////////////////////////////////////////////////////////////////

int CSubQuestInfo::IsNpcRelationQuest(DWORD dwNpcID)
{
	return 0;
}

DWORD CSubQuestInfo::GetNpcScriptPage(DWORD dwNpcID)
{
	return 0;
}

#endif // _MHCLIENT_
