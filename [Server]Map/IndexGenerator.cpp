// IndexGenerator.cpp - 简单的索引生成器实现（替代ar archive格式的IndexGenerator.lib）

// 不使用预编译头，避免包含冲突

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <map>

using namespace std;

// 类型定义
typedef void* INDEXCR_HANDLE;

// 索引生成器实现类
class CIndexGeneratorImpl
{
public:
	CIndexGeneratorImpl(DWORD dwMaxIndex) : m_dwNextIndex(1), m_dwMaxIndex(dwMaxIndex)
	{
		InitializeCriticalSection(&m_cs);
	}

	~CIndexGeneratorImpl()
	{
		DeleteCriticalSection(&m_cs);
	}

	DWORD AllocIndex()
	{
		EnterCriticalSection(&m_cs);
		DWORD dwIndex = m_dwNextIndex++;
		if (dwIndex >= m_dwMaxIndex)
		{
			// 索引用完，从头开始（简单的循环策略）
			m_dwNextIndex = 1;
		}
		LeaveCriticalSection(&m_cs);
		return dwIndex;
	}

	void FreeIndex(DWORD dwIndex)
	{
		// 简单实现：不需要做任何事情
	}

private:
	DWORD m_dwNextIndex;
	DWORD m_dwMaxIndex;
	CRITICAL_SECTION m_cs;
};

// 全局生成器映射
static map<DWORD, CIndexGeneratorImpl*> g_Generators;
static DWORD g_NextHandle = 1;
static CRITICAL_SECTION g_GeneratorsCS;

// 静态初始化
class CStaticInit
{
public:
	CStaticInit()
	{
		InitializeCriticalSection(&g_GeneratorsCS);
	}
	~CStaticInit()
	{
		// 清理所有剩余的生成器
		for (auto it = g_Generators.begin(); it != g_Generators.end(); ++it)
		{
			delete it->second;
		}
		g_Generators.clear();
		DeleteCriticalSection(&g_GeneratorsCS);
	}
};
static CStaticInit s_StaticInit;

// 导出函数 - 不使用__declspec，直接使用标准extern "C"
extern "C"
{
	INDEXCR_HANDLE __stdcall ICCreate()
	{
		CIndexGeneratorImpl* pGenerator = new CIndexGeneratorImpl(1000);

		EnterCriticalSection(&g_GeneratorsCS);
		DWORD dwHandle = g_NextHandle++;
		g_Generators[dwHandle] = pGenerator;
		LeaveCriticalSection(&g_GeneratorsCS);

		return (INDEXCR_HANDLE)dwHandle;
	}

	void __stdcall ICRelease(INDEXCR_HANDLE pIC)
	{
		DWORD dwHandle = (DWORD)pIC;

		EnterCriticalSection(&g_GeneratorsCS);
		auto it = g_Generators.find(dwHandle);
		if (it != g_Generators.end())
		{
			delete it->second;
			g_Generators.erase(it);
		}
		LeaveCriticalSection(&g_GeneratorsCS);
	}

	void __stdcall ICInitialize(INDEXCR_HANDLE pIC, DWORD dwMaxIndex)
	{
		// 简单实现：忽略
	}

	DWORD __stdcall ICAllocIndex(INDEXCR_HANDLE pIC)
	{
		DWORD dwHandle = (DWORD)pIC;

		EnterCriticalSection(&g_GeneratorsCS);
		auto it = g_Generators.find(dwHandle);
		LeaveCriticalSection(&g_GeneratorsCS);

		if (it != g_Generators.end())
		{
			return it->second->AllocIndex();
		}
		return 0;
	}

	void __stdcall ICFreeIndex(INDEXCR_HANDLE pIC, DWORD dwIndex)
	{
		DWORD dwHandle = (DWORD)pIC;

		EnterCriticalSection(&g_GeneratorsCS);
		auto it = g_Generators.find(dwHandle);
		LeaveCriticalSection(&g_GeneratorsCS);

		if (it != g_Generators.end())
		{
			it->second->FreeIndex(dwIndex);
		}
	}
}
