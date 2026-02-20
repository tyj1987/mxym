// TargetData.cpp: implementation of the CTargetData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TargetData.h"

#include "..\..\[CC]Header\CommonGameFunc.h"  // for WriteDebugFile

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTargetData::CTargetData()
{

}

CTargetData::~CTargetData()
{

}

void CTargetData::SetTarget(DWORD id,WORD flag,BYTE kind)
{
	TargetID = id;
	TargetFlag = flag;
	TargetKind = kind;
}

void CTargetData::SetTargetData(int n,DWORD data)
{
	if( n > MAX_TARGET_DATA )
	{
		char str[256] ={0,};
		sprintf( str, "CTargetData::SetTargetData()�� ���ڰ��� �߸��Ǿ����ϴ�. (%d)", n );
		WRITEDEBUGFILE( str );
	}
	TargetData[n] = data;
}

DWORD CTargetData::GetTargetData( int n )
{
	if( n > MAX_TARGET_DATA )
	{
		char str[256] ={0,};
		sprintf( str, "CTargetData::GetTargetData()�� ���ڰ��� �߸��Ǿ����ϴ�. (%d)", n );
		WRITEDEBUGFILE( str );
	}
	return TargetData[n];
}