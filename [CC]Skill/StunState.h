// StunState.h: interface for the CStunState class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STUNSTATE_H__F806FA5B_BBC7_4CB8_AB35_E1A7DEFFB02A__INCLUDED_)
#define AFX_STUNSTATE_H__F806FA5B_BBC7_4CB8_AB35_E1A7DEFFB02A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SpecialState.h"

#ifdef _MHCLIENT_
#ifndef _MHCLIENT_LIBRARY_
#include "..\Effect\EffectManager.h"
#endif
#endif

class CObject;

class CStunState  : public CSpecialState
{
public:
	CStunState(); // Default constructor
	CStunState(CObject* pParent);
	~CStunState();
};

#endif // !defined(AFX_STUNSTATE_H__F806FA5B_BBC7_4CB8_AB35_E1A7DEFFB02A__INCLUDED_)