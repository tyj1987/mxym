//////////////////////////////////////////////////////////////////////////////
/// \file MiniDumper.cpp
/// \author excel96
/// \date 2003.11.18
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MiniDumper.h"
//#include "DebugRoutine.h"
#include "..\[CC]Header\ServerSystem.h"
#include <dbghelp.h>

#define chDIMOF(Array) (sizeof(Array) / sizeof(Array[0]))


// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
	HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

MiniDumper::DumpLevel MiniDumper::s_DumpLevel                = MiniDumper::DUMP_LEVEL_0;
bool                  MiniDumper::s_bAddTimeStamp            = true;
TCHAR                 MiniDumper::s_szAppName[_MAX_PATH]     = {0,};
TCHAR                 MiniDumper::s_szFaultReason[2048]      = {0,};

//////////////////////////////////////////////////////////////////////////////
/// \brief ������
/// \param DL ���� ����
/// \param bAddTimeStamp ���� ���� �̸����ٰ� ���� ������ ������ ��¥��
/// ����ִ°��� ����.
//////////////////////////////////////////////////////////////////////////////
MiniDumper::MiniDumper(DumpLevel DL, bool bAddTimeStamp)
{
//	Assert(s_szAppName[0] == 0);
//	Assert(DL >= 0);
//	Assert(DL <= DUMP_LEVEL_2);

	s_DumpLevel     = DL;
	s_bAddTimeStamp = bAddTimeStamp;

	// ��� ��θ� �˾Ƴ���.
	TCHAR szFilename[_MAX_PATH];
	::GetModuleFileName(NULL, szFilename, _MAX_PATH);

	// Ȯ���ڸ� ������ ��� ��θ� �غ��صΰ�...
	TCHAR* dot = strrchr(szFilename, '.');
	::lstrcpyn(s_szAppName, szFilename, (int)(dot - szFilename + 1));

	dot = strrchr(s_szAppName, '\\');
	::lstrcpyn(s_szAppName, &dot[1], strlen(s_szAppName) );
	

	// ���� ó�� �ڵ鷯�� �����Ѵ�.
	::SetUnhandledExceptionFilter(TopLevelFilter);
}

//////////////////////////////////////////////////////////////////////////////
/// \brief �Ҹ���
//////////////////////////////////////////////////////////////////////////////
MiniDumper::~MiniDumper()
{
}

//////////////////////////////////////////////////////////////////////////////
/// \brief ���ܿ� ���� ������ �޾Ƽ�, �̴� ���� ������ �����Ѵ�. 
/// 
/// SetUnhandledExceptionFilter() API�� ���ؼ� �����ǰ�, ���μ��� ���ο��� 
/// Unhandled Exception�� �߻��� ���, ȣ��ǰ� �ȴ�. �� ����Ű� �پ��ִ� 
/// ���, Unhandled Exception Filter�� ȣ����� �ʴ´�. �� ���� �� �Լ� 
/// ���θ� ������� ���� ���ٴ� ���̴�. �� �Լ� ���θ� ������ϱ� ���ؼ��� 
/// �޽��� �ڽ� �Ǵ� ������ �̿��ؾ��Ѵ�.
/// 
/// \param pExceptionInfo ���� ����
/// \return LONG �� �Լ��� �����ϰ� �� ����, ���� �ൿ��. �ڼ��� ���� SEH
/// ������ �����ϵ���.
//////////////////////////////////////////////////////////////////////////////
LONG WINAPI MiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS* pExPtr)
{
	LONG    retval                   = EXCEPTION_CONTINUE_SEARCH;
	HMODULE hDLL                     = NULL;
	TCHAR   szDbgHelpPath[_MAX_PATH] = {0, };
	TCHAR   szDumpPath[MAX_PATH * 2] = {0,};

	// ���� ���� ������ �ִ� ���丮���� DBGHELP.DLL�� �ε��� ����.
	// Windows 2000 �� System32 ���丮�� �ִ� DBGHELP.DLL ������ ������ 
	// ������ ���� �� �ֱ� �����̴�. (�ּ� 5.1.2600.0 �̻��̾�� �Ѵ�.)
	if (::GetModuleFileName(NULL, szDbgHelpPath, _MAX_PATH))
	{
		LPTSTR pSlash = ::strrchr(szDbgHelpPath, '\\');
		if (pSlash)
		{
			::lstrcpy(pSlash + 1, "DBGHELP.DLL");
			hDLL = ::LoadLibrary(szDbgHelpPath);
		}
	}

	// ���� ���丮�� ���ٸ�, �ƹ� �����̳� �ε��Ѵ�.
	if (hDLL == NULL) hDLL = ::LoadLibrary("DBGHELP.DLL");

	// DBGHELP.DLL�� ã�� �� ���ٸ� �� �̻� ������ �� ����.
	if (hDLL == NULL)
	{
//		filelog(NULL, "DBGHELP.DLL not found");
		return retval;
	}

	// DLL ���ο��� MiniDumpWriteDump API�� ã�´�.
	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = 
		(MINIDUMPWRITEDUMP)::GetProcAddress(hDLL, "MiniDumpWriteDump");

	// �̴ϴ��� �Լ��� ã�� �� ���ٸ� �� �̻� ������ �� ����.
	if (pfnMiniDumpWriteDump == NULL)
	{
//		filelog(NULL, "DBGHELP.DLL too old");
		return retval;
	}

	if (s_bAddTimeStamp)
	{
		// ���� �ð��� ���´�.
		SYSTEMTIME t;
		::GetLocalTime(&t);

		// �ð� ���ڿ��� �غ��Ѵ�.
		TCHAR szTail[_MAX_PATH];
#ifdef _MAPSERVER_		
		_snprintf(szTail, chDIMOF(szTail)-1, 
			"_[%d]_%04d_%02d_%02d_%02d_%02d_%02d",
			g_pServerSystem->GetMapNum(),
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
#elif _AGENTSERVER
		_snprintf(szTail, chDIMOF(szTail)-1, 
			"_[%d]_%04d_%02d_%02d_%02d_%02d_%02d",
			g_pServerSystem->GetServerNum(),
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
#elif _DISTRIBUTESERVER_
		_snprintf(szTail, chDIMOF(szTail)-1, 
			"_%04d_%02d_%02d_%02d_%02d_%02d",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

#endif	//

		// ���� ���� �̸� += �ð� ���ڿ�
		::lstrcat(szDumpPath, "./Log/");
		::lstrcat(szDumpPath, s_szAppName);
		::lstrcat(szDumpPath, szTail);
	}

	// ���� ���� �̸� += Ȯ����
	::lstrcat(szDumpPath, ".dmp");

	// ������ �����Ѵ�.
	HANDLE hFile = ::CreateFile(
		szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// ������ ������ �� ���ٸ� �� �̻� ������ �� ����.
	if (hFile == INVALID_HANDLE_VALUE)
	{
//		filelog(NULL, "Failed to create dump file '%s' (error %s)", 
//			szDumpPath, GetLastErrorString().c_str());
		return retval;
	}

	MINIDUMP_EXCEPTION_INFORMATION ExceptionParam;

	ExceptionParam.ThreadId = ::GetCurrentThreadId();
	ExceptionParam.ExceptionPointers = pExPtr;
	ExceptionParam.ClientPointers = FALSE;

	// �ɼǿ� ���� ���� ������ �����Ѵ�. 
	BOOL bResult = FALSE;
	switch (s_DumpLevel)
	{
	case DUMP_LEVEL_0: // MiniDumpNormal
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpNormal, &ExceptionParam, NULL, NULL);
		break;
	case DUMP_LEVEL_1: // MiniDumpWithDataSegs 
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpWithDataSegs, &ExceptionParam, NULL, NULL);
		break;
	case DUMP_LEVEL_2: // MiniDumpWithFullMemory 
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpWithFullMemory, &ExceptionParam, NULL, NULL);
		break;
	default:
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpNormal, &ExceptionParam, NULL, NULL);
		break;
	}

	// ���� ���� ���� ����� �α� ���Ͽ��� ����Ѵ�.
	if (bResult)
	{
		TCHAR szMessage[8192] = {0,};
		lstrcat(szMessage, "Saved dump file to '");
		lstrcat(szMessage, szDumpPath);
		lstrcat(szMessage, "'.\nFault Reason : ");
		lstrcat(szMessage, GetFaultReason(pExPtr));

//		filelog(NULL, szMessage);
//		retval = EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
//		filelog(NULL, "Failed to save dump file to '%s' (error %d,%s)", 
//			szDumpPath, ::GetLastError(), GetLastErrorString().c_str());

//		Assert(false);
	}

	::CloseHandle(hFile);

	return retval;
}

//////////////////////////////////////////////////////////////////////////////
/// \brief 
/// 
/// \param pExPtrs 
/// \return LPCTSTR
//////////////////////////////////////////////////////////////////////////////
LPCTSTR MiniDumper::GetFaultReason(struct _EXCEPTION_POINTERS* pExPtrs)
{
	if (::IsBadReadPtr(pExPtrs, sizeof(EXCEPTION_POINTERS))) 
		return "BAD EXCEPTION POINTERS";

	// ������ ���� �ڵ��� �׳� ��ȯ�� �� �ִ�.
	switch (pExPtrs->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
	case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
	case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
	case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
	case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
	case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
	case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
	case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
	case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
	case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
	case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
	case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
	case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
	case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
	case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
	case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
	case EXCEPTION_GUARD_PAGE:               return "EXCEPTION_GUARD_PAGE";
	case EXCEPTION_INVALID_HANDLE:           return "EXCEPTION_INVALID_HANDLE";
	case 0xE06D7363:                         return "Microsoft C++ Exception";
	default:
		break;
	}

	// ���� �� �� ������ �������...
	lstrcpy(s_szFaultReason, "Unknown"); 
	::FormatMessage(
		FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
		::GetModuleHandle("NTDLL.DLL"),
		pExPtrs->ExceptionRecord->ExceptionCode, 
		0,
		s_szFaultReason,
		0,
		NULL);

	return s_szFaultReason;
}