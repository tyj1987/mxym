#include "stdafx.h"
#include "ServerSystem.h"
#include "fieldsubmonster.h"
#include "PackedData.h"
#include "CharMove.h"
#include "../[CC]Header/GameResourceManager.h"

CFieldSubMonster::CFieldSubMonster(void)
{
	m_nRegenPosIndex = 0;
}

CFieldSubMonster::~CFieldSubMonster(void)
{
}

void CFieldSubMonster::DoDie(CObject* pAttacker)
{
	CMonster::DoDie(pAttacker);	

	// CFieldBossMonsterManager�� �׾��ٴ°��� �뺸���ش�
	FIELDBOSSMONMGR->SubDead(this);
}

void CFieldSubMonster::SetLife(DWORD Life,BOOL bSendMsg)
{
	CMonster::SetLife(Life, bSendMsg);	
	
	if(bSendMsg)
	{
		MSG_DWORD3 msg;
		msg.Category = MP_BOSSMONSTER;
		msg.Protocol = MP_FIELD_LIFE_NOTIFY;
		msg.dwData1 = GetLife();
		msg.dwData2 = GetMonsterKind();
		msg.dwData3 = GetID();
		PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));
	}
}

void CFieldSubMonster::SetShield(DWORD Shield,BOOL bSendMsg)
{
	CMonster::SetShield(Shield, bSendMsg);	
	
	if(bSendMsg)
	{
		MSG_DWORD3 msg;
		msg.Category = MP_BOSSMONSTER;
		msg.Protocol = MP_FIELD_SHIELD_NOTIFY;
		msg.dwData1 = GetLife();
		msg.dwData2 = GetMonsterKind();
		msg.dwData3 = GetID();
		PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));
	}
}

BOOL CFieldSubMonster::IsBattle()
{
	// ��ȭ ��� �϶� FALSE ����
	if( m_stateParam.stateCur < eMA_PERSUIT )
		return FALSE;

	// ���� ��� �϶� �Ҹ�ð� ī��Ʈ ����
	m_Info.m_dwCountTime = m_Info.m_dwDistructTime;
	// ȸ���ð��� ����
	m_Info.m_dwPieceTime = gCurTime;

	// TRUE ����
	return TRUE;
}

BOOL CFieldSubMonster::IsDistruct()
{
	BOOL rt = FALSE;
	
	// ������ üũ �ð��� 0�� ��� ���� �ð����� �����ϰ� FALSE ����
	if( m_Info.m_dwLastCheckTime != 0 )
	{
		if( ( gCurTime - m_Info.m_dwLastCheckTime )  <  m_Info.m_dwCountTime )
		{
			m_Info.m_dwCountTime -= ( gCurTime - m_Info.m_dwLastCheckTime );
		}
		else
		{
			m_Info.m_dwCountTime = 0;

			rt = TRUE;
		}
	}

	m_Info.m_dwLastCheckTime = gCurTime;

	return rt;
}

void CFieldSubMonster::Recover()	
{
	if(m_Info.m_dwPieceTime + m_Info.m_dwRecoverStartTime < gCurTime)
	{
		DWORD maxlife = GetMaxLife();
		DWORD curlife = GetLife();
		DWORD maxshield = GetMaxShield();
		DWORD curshield = GetShield();
		
		if(gCurTime - m_LifeRecoverTime.lastCheckTime > m_Info.m_dwRecoverDelayTime)
		{
			if(curlife < maxlife)
			{
				DWORD pluslife = (DWORD)(maxlife * m_Info.m_fLifeRate);
				SetLife(curlife + pluslife, TRUE);
				m_LifeRecoverTime.lastCheckTime = gCurTime;
			}
			if(curshield < maxshield)
			{
				DWORD plusshield = (DWORD)(maxshield * m_Info.m_fShieldRate);
				SetShield(curshield+plusshield, TRUE);
			}
		}
	}	
}

VECTOR3* CFieldSubMonster::GetRegenPos()
{
	int ran=0;
	VECTOR3* pThisPos = CCharMove::GetPosition(this);
	VECTOR3 vTargetPos;

	VECTOR3 domainPoint = FIELDBOSSMONMGR->GetRegenPosition(this->GetRegenPosIndex());

	if(m_pSInfo->DomainRange==0)
		m_pSInfo->DomainRange = 1;

	int dwRange = (int)m_pSInfo->DomainRange;

	//���� ������ġ�� ���õ� �ݰ������ �����ϰ� �����δ�.
	vTargetPos.x = domainPoint.x + ( (rand() % 2) ? -1 : 1 ) * (rand() % dwRange) * 100;
	vTargetPos.z = domainPoint.z + ( (rand() % 2) ? -1 : 1 ) * (rand() % dwRange) * 100;	

	//vTargetPos.x = domainPoint.x + ( (rand() % 2) ? -1 : 1 ) * (rand() % 10) * 100;
	//vTargetPos.z = domainPoint.z + ( (rand() % 2) ? -1 : 1 ) * (rand() % 10) * 100;	

	return &vTargetPos;
}

BOOL CFieldSubMonster::DoWalkAround()
{
	int ran=0;
	VECTOR3 Target;
	VECTOR3 * pThisPos = CCharMove::GetPosition(this);
	
	VECTOR3 domainPoint = *GetRegenPos();
	VECTOR3 decisionPoint;
	AdjacentPointForOriginPoint(pThisPos, &domainPoint, (float)10000, &decisionPoint);	

	float dx=0,dz=0;
	ran = rand();
	if(m_pSInfo->DomainRange==0)
		m_pSInfo->DomainRange = 1;

	//dx = float(ran%m_pSInfo->DomainRange) * (ran%2?1:-1);
	//ran = rand();
	//dz = float(ran%m_pSInfo->DomainRange) * (ran%2?1:-1);
	//Target.x = decisionPoint.x + dx;
	//Target.z = decisionPoint.z + dz;

	Target.x = decisionPoint.x;
	Target.z = decisionPoint.z;

	if(Target.x < 10.0f)
		Target.x = 10.0f;
	else if(Target.x > 51200.0f)
		Target.x = 51200.0f;

	if(Target.z < 10.0f)
		Target.z = 10.0f;
	else if(Target.z > 51200.0f)
		Target.z = 51200.0f;

	//taiyo �ӽ�ó�� 
	VECTOR3 CollisonPos;

	MAPTYPE MapNum = GAMERESRCMNGR->GetLoadMapNum();
	if(g_pServerSystem->GetMap()->CollisionCheck(pThisPos,&Target,&CollisonPos,this) == TRUE)
	{
		Target.x = decisionPoint.x - dx;
		Target.z = decisionPoint.z - dz;
	}
	if(g_pServerSystem->GetMap()->CollisionCheck(pThisPos,&Target,&CollisonPos,this) == TRUE)
	{
		Target.x = decisionPoint.x + dx;
		Target.z = decisionPoint.z - dz;
	}
	if(g_pServerSystem->GetMap()->CollisionCheck(pThisPos,&Target,&CollisonPos,this) == TRUE)
	{
		Target.x = decisionPoint.x - dx;
		Target.z = decisionPoint.z + dz;
	}

	//YH2DO
	if(Target.x < 10)
		Target.x = 10;
	else if(Target.x > 51200)
		Target.x = 51200;
	if(Target.z < 10)
		Target.z = 10;
	else if(Target.z > 51200)
		Target.z = 51200;

	OnMove(&Target);

	return TRUE;
}
