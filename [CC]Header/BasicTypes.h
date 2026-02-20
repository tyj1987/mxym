#ifndef __BASICTYPES_H__
#define __BASICTYPES_H__

// 基本Windows类型定义
// 此文件会自动包含windows.h，确保类型定义的正确性

// 如果windows.h还未被包含，先包含它
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#endif

// 现在提供额外的类型定义，只在Windows SDK未定义时提供
// 这样可以避免类型冲突

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef LONG
typedef long LONG;
#endif

// Windows句柄和类型
#ifndef COLORREF
typedef DWORD COLORREF;
#endif

#ifndef LPCSTR
typedef const char* LPCSTR;
#endif

#ifndef LPSTR
typedef char* LPSTR;
#endif

#ifndef PCHAR
typedef char* PCHAR;
#endif

// LPCTSTR depends on UNICODE definition
#ifdef UNICODE
#ifndef LPCWSTR
typedef const wchar_t* LPCWSTR;
#endif
#define LPCTSTR LPCWSTR
#else
#define LPCTSTR LPCSTR
#endif

// BOOL, TRUE, FALSE, NULL 的定义
// 完全依赖Windows SDK的定义，不再提供自定义定义

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

// RGB宏定义
#ifndef RGB
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)((BYTE)(b)))<<16)))
#endif

#endif // __BASICTYPES_H__
