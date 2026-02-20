// afxres.h - 简化版MFC资源头文件
// 用于替代缺失的标准afxres.h

#ifndef _AFXRES_H_
#define _AFXRES_H_

#ifdef RC_INVOKED

// 资源编译器所需的定义
#ifndef _INC_WINDOWS
#include <winresrc.h>
#endif

// 确保必要的资源类型已定义
#ifndef VS_VERSION_INFO
#define VS_VERSION_INFO 1
#endif

#ifndef RT_MANIFEST
#define RT_MANIFEST 24
#endif

#else // !RC_INVOKED

// C++编译时的定义
#include <windows.h>

#endif // RC_INVOKED

#endif // _AFXRES_H_
