// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2E18BDB6_4FBE_49A6_B6B5_A64DE5290F40__INCLUDED_)
#define AFX_STDAFX_H__2E18BDB6_4FBE_49A6_B6B5_A64DE5290F40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <winsock2.h>   // 改为winsock2.h，避免与主项目的winsock2.h冲突
#include <stdlib.h>     // for rand, srand
#include <time.h>       // for time
#include <crtdbg.h>     // for _ASSERTE

// TODO: reference additional headers your program requires here

#include "YHLibrary.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2E18BDB6_4FBE_49A6_B6B5_A64DE5290F40__INCLUDED_)
