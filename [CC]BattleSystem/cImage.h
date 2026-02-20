#ifndef _CIMAGE_STUB_H
#define _CIMAGE_STUB_H

// Forward declaration for MHClient compilation
#ifdef _MHCLIENT_
class cImage;
#endif

// Stub definition for client library compilation
#ifdef _MHCLIENT_LIBRARY_
#include "../[CC]Header/CommonStruct.h"
class cImage
{
public:
	cImage() : m_bVisible(TRUE) {}
	virtual ~cImage() {}
	void Render() {}
	void RenderSprite(VECTOR2* pos, VECTOR2* scale, DWORD color = 0xFFFFFFFF) {}
	void SetPosition(float x, float y) {}
	void SetScale(float x, float y) {}
	void Show() { m_bVisible = TRUE; }
	void Hide() { m_bVisible = FALSE; }
	BOOL IsVisible() { return m_bVisible; }
private:
	BOOL m_bVisible;
};
#endif

#endif
