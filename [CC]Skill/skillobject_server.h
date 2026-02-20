// SkillObject.h: interface for the CSkillObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKILLOBJECT_H__DEE12E2A_038A_410E_ADBB_BA63FCADA5AA__INCLUDED_)
#define AFX_SKILLOBJECT_H__DEE12E2A_038A_410E_ADBB_BA63FCADA5AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\[Server]Map\Object.h"
#include "SkillObjectSingleUnit.h"
#include "D:\mxym\[Lib]YHLibrary\PtrList.h"

class CSkillInfo;
class CActionTarget;
class CSkillObjectTerminator;
class CSkillObjectTargetList;
class CSkillObjectFirstUnit;
class CSkillObjectStateUnit;

#define SO_DESTROYOBJECT	-1
#define SO_OK				0

#define MUGONG_USE_EXP		100

class CSkillObject  : public CObject
{
protected:
	CSkillInfo* m_pSkillInfo;
	CSkillObject(CSkillInfo* pSkillInfo,
				CSkillObjectTerminator* pTerminator,
				CSkillObjectTargetList* pTargetList,
				CSkillObjectFirstUnit* pFirstUnit);

	CSkillObjectTerminator* m_pTerminator;
	CSkillObjectTargetList* m_pTargetList;
	CSkillObjectFirstUnit* m_pFirstUnit;
	cPtrList m_SingleUnitList;
	cPtrList m_StateUnitList;
	
	
	WORD m_PositiveTargetType;
	WORD m_NegativeTargetType;

	CObject* m_pOperator;
	
	SKILLOBJECT_INFO m_SkillObjectInfo;
	
	void AddSingleUnit(CSkillObjectSingleUnit* pSingleUnit);
	void AddStateUnit(CSkillObjectStateUnit* pStateUnit);

	BOOL m_bDieFlag;

	DWORD m_MaxLife;
	DWORD m_CurLife;
	
	float m_SkillTreeAmp;

	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2  - ̿
	//  ȯ ߰
	WORD m_OptionIndex;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 06. 07. ų - ̿
	BOOL IsSame(CSkillObject* pSkill);
	//////////////////////////////////////////////////////////////////////////
public:
	//YHӽ
	int releaseKind;

	virtual ~CSkillObject();

	virtual void Init(SKILLOBJECT_INFO* pInfo,CTargetList* pTList,float skillTreeAmp = 1,CObject* pMainTarget = NULL);
	void Release();

	virtual DWORD Update();

	virtual void UpdateTargetList(CObject* pObject);

	void AddTargetObject(CObject* pObject);
	void RemoveTargetObject(DWORD ObjectID);

	WORD GetSkillIdx()		{	return m_SkillObjectInfo.SkillIdx;	}

	friend class CSkillObjectFactory;

	
	void SetAddMsg(char* pAddMsg,WORD* pMsgLen,DWORD dwReceiverID,BOOL bLogin);
	void SetRemoveMsg(char* pRemoveMsg,WORD* pMsgLen,DWORD dwReceiverID);
	
	void GetPosition(VECTOR3* pPos);
	DIRINDEX GetDirectionIndex()	{	return m_SkillObjectInfo.Direction;	}


	BOOL IsNegativeTarget(CObject* pObject);
	BOOL IsPositiveTarget(CObject* pObject);
	BOOL TargetTypeCheck(WORD TargetType,CObject* pObject);

	CObject* GetOperator();
	CSkillInfo* GetSkillInfo() { return m_pSkillInfo;	}	
	SKILLOBJECT_INFO* GetSkillObjectInfo() { return &m_SkillObjectInfo;	}
	virtual BOOL Operate(CObject* pRequestor,MAINTARGET* pMainTarget,CTargetList* pTList)	{	return FALSE;	}

	//// 2007. 7. 10. CBH -   Լ ߰
	void SkillObjectFirstUnitResult();	
	///////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// overriding Լ
	virtual void DoDie(CObject* pAttacker);
	virtual DWORD GetLife();
	virtual void SetLife(DWORD Life,BOOL bSendMsg = TRUE);
	virtual DWORD DoGetMaxLife();
};

#endif // !defined(AFX_SKILLOBJECT_H__DEE12E2A_038A_410E_ADBB_BA63FCADA5AA__INCLUDED_)
