// ============================================
// D3D12RootSignature.h
// DirectX 12 根签名类
// ============================================

#pragma once

#include "D3D12PCH.h"

class D3D12RootSignature
{
public:
    D3D12RootSignature();
    ~D3D12RootSignature();

    // 创建根签名
    HRESULT Create(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc);
    HRESULT Create(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC1& desc);

    // 创建标准根签名（默认配置）
    HRESULT Create(ID3D12Device* device);

    // 创建自定义根签名（带标志）
    HRESULT Create(
        ID3D12Device* device,
        const D3D12_ROOT_SIGNATURE_DESC1* desc,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
    );

    // 获取根签名指针
    ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature.Get(); }

    // 获取根签名大小
    UINT GetByteSize() const { return m_byteSize; }

    // 释放资源
    void Release();

private:
    ComPtr<ID3D12RootSignature> m_pRootSignature;
    UINT m_byteSize = 0;
};
