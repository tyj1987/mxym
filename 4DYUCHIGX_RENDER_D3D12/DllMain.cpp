// DllMain.cpp
// DirectX 12 渲染器 DLL 入口点

#include "D3D12PCH.h"
#include "CoD3D12Device.h"

// DLL 实例句柄
HINSTANCE g_hInst = NULL;

// DLL 引用计数
long g_dwRefCount = 0;

// ========== DLL 入口点 ==========
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        g_hInst = NULL;
        break;
    }
    return TRUE;
}

// ========== COM 工厂函数 ==========
// 实现 DllCreateInstance 函数，用于创建渲染器实例
HRESULT __stdcall DllCreateInstance(void** ppv)
{
    if (!ppv)
        return E_INVALIDARG;

    // 创建 CoD3D12Device 实例
    CoD3D12Device* pDevice = new CoD3D12Device();
    if (!pDevice)
        return E_OUTOFMEMORY;

    // 添加引用计数
    pDevice->AddRef();
    *ppv = pDevice;

    return S_OK;
}

// 导出函数声明
extern "C" HRESULT __declspec(dllexport) __stdcall DllCreateInstance(void** ppv);
