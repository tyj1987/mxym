// ============================================
// typedef.h
// 通用类型定义
// ============================================

#pragma once

#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <Windows.h>

// 基本类型定义
#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// 字符类型
#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef QWORD
typedef unsigned long long QWORD;
#endif

// 有符号类型
#ifndef INT8
typedef signed char INT8;
#endif

#ifndef INT16
typedef short INT16;
#endif

#ifndef INT32
typedef int INT32;
#endif

#ifndef INT64
typedef long long INT64;
#endif

#ifndef UINT8
typedef unsigned char UINT8;
#endif

#ifndef UINT16
typedef unsigned short UINT16;
#endif

#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef UINT64
typedef unsigned long long UINT64;
#endif

// 浮点类型
#ifndef FLOAT
typedef float FLOAT;
#endif

// 句柄类型 - 注意：HWND, HINSTANCE, HANDLE 等已在 Windows.h 中定义
// 此处不重复定义，避免冲突

// HRESULT 已经在 winerror.h 中定义

// 通用指针 - 注意：LPVOID, LPCVOID 等已在 Windows.h 中定义
// 字符串指针 - 注意：LPSTR, LPCSTR, LPWSTR, LPCWSTR, LPTSTR 等已在 Windows.h 中定义
// 此处不重复定义，避免冲突

// LPCTSTR 也已在 Windows.h 中定义，此处不重复定义

// 宏定义
#ifndef NULL
#define NULL 0
#endif

// 常用宏
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p) { delete (p); (p) = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p) = NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }
#endif

// Min/Max 宏
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

// 交换宏
#ifndef SWAP
#define SWAP(a, b) { (a) ^= (b); (b) ^= (a); (a) ^= (b); }
#endif

#endif // TYPEDEF_H
