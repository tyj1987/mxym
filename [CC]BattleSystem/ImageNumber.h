#ifndef _IMAGENUMBER_CLIENT_H
#define _IMAGENUMBER_CLIENT_H

// Forward declaration for MHClient compilation
#ifdef _MHCLIENT_
class CImageNumber;
#endif

// Stub definition for client library compilation
#ifdef _MHCLIENT_LIBRARY_
#include "../[CC]Header/CommonStruct.h"
class CImageNumber
{
protected:
	VECTOR2 m_vPos;
	VECTOR2 m_vScale;
	DWORD	m_dwNumber;
	DWORD	m_dwLimitCipher;
	BOOL	m_bFillZero;
	int		m_nNumWidth;
	int		m_nGap;
	BOOL	m_bNumberChange;
	DWORD	m_dwFadeOutStartTime;
	DWORD	m_dwFadeOutTime;
	BOOL	m_bFadeOut;

public:
	CImageNumber() { memset(this, 0, sizeof(*this)); }
	virtual ~CImageNumber() {}
	void Init(int nNumWidth, int nGap) {}
	void SetNumber(DWORD dwNum) { m_dwNumber = dwNum; }
	void SetLimitCipher(DWORD dwCipher) { m_dwLimitCipher = dwCipher; }
	void SetFillZero(BOOL bFillZero) { m_bFillZero = bFillZero; }
	void SetPosition(int x, int y) { m_vPos.x = (float)x; m_vPos.y = (float)y; }
	void Render() {}
	void SetScale(float x, float y) { m_vScale.x = x; m_vScale.y = y; }
	void SetFadeOut(DWORD dwTime) {}
	BOOL IsNumberChanged() { return m_bNumberChange; }
};
#endif

#endif
