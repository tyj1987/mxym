#ifndef __WINDOWS_TYPES_H__
#define __WINDOWS_TYPES_H__

// Windows类型统一定义
// 确保在所有其他头文件之前包含此文件

// 必须首先包含Windows SDK
#include <windows.h>
#include <winsock2.h>

// 然后提供向后兼容的类型别名
// 如果Windows SDK没有定义某些类型，我们才定义

// 基本类型（Windows SDK已定义，这里只为文档说明）
// typedef unsigned short WORD;  // Windows SDK定义
// typedef unsigned long DWORD;   // Windows SDK定义
// typedef unsigned char BYTE;    // Windows SDK定义
// typedef long LONG;             // Windows SDK定义

// BOOL类型完全依赖Windows SDK
// Windows SDK的minwindef.h定义：typedef int BOOL;

// 字符串类型
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

// COLORREF
#ifndef COLORREF
typedef DWORD COLORREF;
#endif

// 宏定义
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

// RGB宏
#ifndef RGB
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)((BYTE)(b)))<<16)))
#endif

#endif // __WINDOWS_TYPES_H__
