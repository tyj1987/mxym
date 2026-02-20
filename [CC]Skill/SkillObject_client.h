// SkillObject.h: interface for the CSkillObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKILLOBJECT_H__DEE12E2A_038A_410E_ADBB_BA63FCADA5AA__INCLUDED_)
#define AFX_SKILLOBJECT_H__DEE12E2A_038A_410E_ADBB_BA63FCADA5AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _MAPSERVER_
#include "..\[Server]Map\Object.h"
#else
// 客户端：包含客户端的Object.h
#include "..\\[Client]MH\\Object.h"
#endif

#include "SkillObjectSingleUnit.h"
//#include "cLinkedList.h"
#include "SkillInfo.h"

// Forward declare HEFFPROC type (defined in EffectManager.h)
#ifndef HEFFPROC
typedef DWORD HEFFPROC;
#endif

class CSkillInfo;
class CActionTarget;
class CSkillObjectTerminator;
class CSkillObjectTargetList;
class CSkillObjectFirstUnit;
class CSkillObjectStateUnit;

#define SO_DESTROYOBJECT	-1
#define SO_OK				0

class CSkillObject  : public CObject
{
protected:
	CSkillInfo* m_pSkillInfo;
	CSkillObjectTerminator* m_pTerminator;
	CSkillObjectTargetList* m_pTargetList;
	CSkillObjectFirstUnit* m_pFirstUnit;
	cPtrList m_SingleUnitList;
	cPtrList m_StateUnitList;
	
	WORD m_PositiveTargetType;
	WORD m_NegativeTargetType;
	WORD m_SkillObjectMoveType;

	CObject* m_pOperator;

	BOOL m_IsHeroSkill;
	BOOL m_bWasTempObject;
	BOOL m_bInited;
	BOOL m_bDieFlag;

	SKILLOBJECT_INFO m_SkillObjectInfo;

	HEFFPROC m_hObjectEff;

	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2  - ̿
	//  ȯ ߰
	WORD m_OptionIndex;
	//////////////////////////////////////////////////////////////////////////
	
	void AddSingleUnit(CSkillObjectSingleUnit* pSingleUnit);
	void AddStateUnit(CSkillObjectStateUnit* pStateUnit);

	void Release();			// CObject::Release ȣ Ǵ   

	void SetOperatorState();
	void EndOperatorState();

	void StartObjectEffect(CTargetList* pTList,MAINTARGET* pMainTarget);
	void EndObjectEffect();

	void Damage(CObject* pAttacker,BYTE DamageKind,DWORD Damage,BOOL bCritical, BOOL bDecisive, DWORD titanObserbDamage);
public:
	CSkillObject(CSkillInfo* pSkillInfo,
				CSkillObjectTerminator* pTerminator,
				CSkillObjectTargetList* pTargetList,
				CSkillObjectFirstUnit* pFirstUnit);
	virtual ~CSkillObject();

	virtual void Init(SKILLOBJECT_INFO* pInfo,CTargetList* pTList);
	void ReleaseSkillObject();
	
	void SetMissed();

	virtual DWORD Update();

	virtual void UpdateTargetList(CObject* pObject);
	void RemoveTarget(CObject* pObject,BYTE bTargetKind);

	void AddTargetObject(CObject* pObject,BYTE bTargetKind);
	void RemoveTargetObject(DWORD ObjectID);

	friend class CSkillObjectFactory;

	void OnReceiveSkillSingleResult(MSG_SKILL_SINGLE_RESULT* pmsg);

	WORD GetSkillIdx()	{	return m_SkillObjectInfo.SkillIdx;	}
	WORD GetComboNum();
	
	virtual void SetPosition(VECTOR3* pPos);
	virtual void GetPosition(VECTOR3* pPos);
	virtual void SetAngle(float AngleRad);
	virtual float GetAngle();
	virtual DIRINDEX GetDirectionIndex();

	BOOL IsNegativeTarget(CObject* pObject);
	BOOL IsPositiveTarget(CObject* pObject);
	BOOL TargetTypeCheck(WORD TargetType,CObject* pObject);

	CObject* GetOperator()	{ return m_pOperator;	}
	CSkillInfo* GetSkillInfo() { return m_pSkillInfo;	}
	SKILLOBJECT_INFO* GetSkillObjectInfo() { return &m_SkillObjectInfo;	}
	
	virtual BOOL Operate(CObject* pRequestor,MAINTARGET* pMainTarget,CTargetList* pTList)	{	return FALSE; }

	virtual void SetMotionInState(BYTE State);
	
	inline BOOL IsLifeObject()	{	return m_pSkillInfo->IsLifeSkill();	}
};

#endif // !defined(AFX_SKILLOBJECT_H__DEE12E2A_038A_410E_ADBB_BA63FCADA5AA__INCLUDED_)
