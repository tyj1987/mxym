#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <map>
#include <vector>

#include "../4DyuchiGRX_common/typedef.h"



// 前向声明
class CoD3D12Device;
class D3D12FontObject;

// DirectX 12 Font 管理器
class D3D12FontManager
{
public:
    D3D12FontManager();
    virtual ~D3D12FontManager();

    // ========== 初始化 ==========

    // 初始化管理器
    HRESULT Initialize(CoD3D12Device* pDevice);

    // 释放所有资源
    void Release();

    // ========== Font 管理 ==========

    // 添加 Font
    void* AddFont(D3D12FontObject* pFont);

    // 移除 Font
    void RemoveFont(void* pHandle);

    // 获取 Font
    D3D12FontObject* GetFont(void* pHandle);

    // 检查 Font 是否存在
    BOOL HasFont(void* pHandle) const;

    // ========== 文本渲染 ==========

    // 渲染文本
    BOOL RenderFont(D3D12FontObject* pFont, TCHAR* str, DWORD dwLen,
                     RECT* pRect, DWORD dwColor, CHAR_CODE_TYPE type,
                     int iZOrder, DWORD dwFlag);

    // ========== 统计 ==========

    // 获取 Font 数量
    size_t GetFontCount() const { return m_mFontMap.size(); }

private:
    // ========== 辅助方法 ==========

    // 创建文本顶点缓冲区
    HRESULT CreateTextVertexBuffer();

    // 创建文本索引缓冲区
    HRESULT CreateTextIndexBuffer();

    // 创建根签名
    HRESULT CreateRootSignature();

    // 创建 PSO
    HRESULT CreatePipelineState();

    // ========== Font 映射 ==========

    // Font 映射（句柄 -> Font）
    std::map<void*, D3D12FontObject*> m_mFontMap;

    // ========== 顶点缓冲区 ==========

    ComPtr<ID3D12Resource> m_pTextVertexBuffer;
    ComPtr<ID3D12Resource> m_pUploadTextVertexBuffer;  // 上传堆
    UINT m_nTextVertexBufferSize;

    ComPtr<ID3D12Resource> m_pTextIndexBuffer;
    ComPtr<ID3D12Resource> m_pUploadTextIndexBuffer;  // 上传堆
    UINT m_nTextIndexBufferSize;

    // ========== 配置 ==========

    UINT m_nMaxCharacters;  // 最大字符数量

    // ========== 渲染管线 ==========

    ComPtr<ID3D12RootSignature> m_pRootSignature;
    ComPtr<ID3D12PipelineState> m_pTextPSO;

    // ========== 设备指针 ==========

    CoD3D12Device* m_pDevice;
};
