// DelayGroup.cpp: implementation of the CDelayGroup class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DelayGroup.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryPoolTempl<CDelayGroup::CDelay>* CDelayGroup::m_pDelayPool = NULL;
DWORD CDelayGroup::m_PoolRefCount = 0;

DWORD CDelayGroup::MakeKey(WORD Kind,WORD Idx)
{
	return (Kind << 16) | Idx;
}

CDelayGroup::CDelayGroup()
{
	if(m_pDelayPool == NULL)
	{
		m_pDelayPool = new CMemoryPoolTempl<CDelay>;
#ifdef _MAPSERVER_
		m_pDelayPool->Init(1024,128,"CDelayGroup");
#else
		m_pDelayPool->Init(16,16,"CDelayGroup");
#endif
#include "..\[CC]Header\CommonStruct.h"
#include "..\[CC]Header\CommonGameFunc.h"
#include "..\[CC]Header\CommonGameDefine.h"
	}

	++m_PoolRefCount;

	m_DelayedTable.Initialize(16);

	m_pTitanPotionDelay = NULL;
}

CDelayGroup::~CDelayGroup()
{
	Release();

	if(m_PoolRefCount == 0)
	{
		static BOOL bMsgBox = TRUE;
		if(bMsgBox)
			MessageBox(0,"DelayGroup  ̻ϴ!!",0,0);

		return;
	}

	--m_PoolRefCount;
	if(m_PoolRefCount == 0)
	{
		m_pDelayPool->Release();
		delete m_pDelayPool;
		m_pDelayPool = NULL;
	}
}

void CDelayGroup::Init()
{
	m_pTitanPotionDelay = NULL;
	Release();
}

void CDelayGroup::Release()
{
	m_DelayedTable.SetPositionHead();
	while(CDelay* pDelay = m_DelayedTable.GetData())
	{
		m_pDelayPool->Free(pDelay);
	}
	m_DelayedTable.RemoveAll();
}

float CDelayGroup::CheckDelay(WORD Kind,WORD Idx, DWORD* pRemainTime)
{
	// magi82 - Titan(070910) Ÿź Ʈ
	//if(Idx >= SKILLNUM_TO_TITAN)
	//	Idx -= SKILLNUM_TO_TITAN;

	DWORD key = MakeKey(Kind,Idx);
	CDelay* pDelay = m_DelayedTable.GetData(key);
	if(pDelay == NULL)
		return 0;

	DWORD Elapsed = gCurTime - pDelay->StartTime;
#ifdef _MAPSERVER_
	//2007. 11. 15. CBH -     
	//Ŭ̾Ʈ ̰ Ǯ ٷ      ̰  ʾƼ
	//  װ ־ 0.3  ֱ 300 ؼ .
	float rate = 1 - ( (Elapsed+300)/(float)pDelay->DelayTime);
#else
	float rate = 1 - (Elapsed/(float)pDelay->DelayTime);
#endif
	
	if( pRemainTime )
	{
		if( pDelay->DelayTime > Elapsed )
			*pRemainTime = pDelay->DelayTime - Elapsed;
		else
			*pRemainTime = 0;
	}

	// ̰  ̸ .
	if(rate <= 0)
	{
		if( pRemainTime )
			*pRemainTime = 0;
		m_pDelayPool->Free(pDelay);
		m_DelayedTable.Remove(key);
		return 0;
	}

	return rate;
}
void CDelayGroup::AddDelay(WORD Kind,WORD Idx,DWORD DelayTime,DWORD ElapsedTime )
{
	DWORD key = MakeKey(Kind,Idx);
	CDelay* pDelay = m_DelayedTable.GetData(key);

	//  ̰   Ѵ.
	if(pDelay)
	{
		pDelay->StartTime = gCurTime;
		pDelay->DelayTime = DelayTime;
		return;
	}

	//   ߰Ѵ.
	pDelay = m_pDelayPool->Alloc();
	pDelay->Kind = Kind;
	pDelay->Idx = Idx;
	
	DWORD dwStartTime = gCurTime-ElapsedTime;
	if( gCurTime < ElapsedTime )
	{
		dwStartTime = gCurTime;

		if( DelayTime > ElapsedTime )
			DelayTime -= ElapsedTime;
		else
			DelayTime = 0;
	}
		
	pDelay->StartTime = dwStartTime;
	pDelay->DelayTime = DelayTime;

	m_DelayedTable.Add(pDelay,key);
}

float CDelayGroup::CheckTitanPotionDelay( DWORD* pRemainTime )
{
	if( m_pTitanPotionDelay == NULL )
		return 0;

	DWORD Elapsed = gCurTime - m_pTitanPotionDelay->StartTime;
	float rate = 1 - (Elapsed/(float)m_pTitanPotionDelay->DelayTime);

	if( pRemainTime )
	{
		if( m_pTitanPotionDelay->DelayTime > Elapsed )
			*pRemainTime = m_pTitanPotionDelay->DelayTime - Elapsed;
		else
			*pRemainTime = 0;
	}

	// ̰  ̸ .
	if(rate <= 0)
	{
		if( pRemainTime )
			*pRemainTime = 0;
		m_pDelayPool->Free(m_pTitanPotionDelay);
		m_pTitanPotionDelay = NULL;
		return 0;
	}

	return rate;
}
void CDelayGroup::AddTitanPotionDelay( DWORD DelayTime, DWORD ElapsedTime )
{
	//  ̰   Ѵ.
	if( m_pTitanPotionDelay )
	{
		m_pTitanPotionDelay->StartTime = gCurTime;
		m_pTitanPotionDelay->DelayTime = DelayTime;
		return;
	}

	//   ߰Ѵ.
	m_pTitanPotionDelay = m_pDelayPool->Alloc();

	DWORD dwStartTime = gCurTime-ElapsedTime;
	if( gCurTime < ElapsedTime )
	{
		dwStartTime = gCurTime;

		if( DelayTime > ElapsedTime )
			DelayTime -= ElapsedTime;
		else
			DelayTime = 0;
	}

	m_pTitanPotionDelay->StartTime = dwStartTime;
	m_pTitanPotionDelay->DelayTime = DelayTime;
}
