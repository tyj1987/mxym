// ============================================
// D3D12RootSignature.cpp
// DirectX 12 根签名实现
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"

#include "D3D12RootSignature.h"
#include "D3D12Fixes.h"
#include "d3dx12.h"
#include <stdexcept>

D3D12RootSignature::D3D12RootSignature()
{
}

D3D12RootSignature::~D3D12RootSignature()
{
    Release();
}

HRESULT D3D12RootSignature::Create(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
{
    if (!pDevice)
        return E_INVALIDARG;

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeRootSignature(
        &desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error);

    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        return hr;
    }

    hr = pDevice->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_pRootSignature));

    return hr;
}

HRESULT D3D12RootSignature::Create(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC1& desc)
{
    if (!pDevice)
        return E_INVALIDARG;

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    HRESULT hr = D3DX12SerializeVersionedRootSignature(
        &desc,
        &signature,
        &error);

    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        return hr;
    }

    hr = pDevice->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_pRootSignature));

    return hr;
}

void D3D12RootSignature::Release()
{
    m_pRootSignature.Reset();
    m_byteSize = 0;
}

HRESULT D3D12RootSignature::Create(ID3D12Device* device)
{
    if (device == nullptr)
        return E_INVALIDARG;

    // 创建标准根签名
    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    ranges[0].NumDescriptors = 1;
    ranges[0].BaseShaderRegister = 0;
    ranges[0].RegisterSpace = 0;
    ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    CD3DX12_ROOT_PARAMETER1 rootParameters[3] = {};

    // CBV (b0)
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].Descriptor.RegisterSpace = 0;

    // SRV (t0)
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges = ranges;

    // Sampler (s0)
    CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
    samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    samplerRange.NumDescriptors = 1;
    samplerRange.BaseShaderRegister = 0;
    samplerRange.RegisterSpace = 0;
    samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &samplerRange;

    D3D12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 3;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_DESCRIPTOR_LAYOUT;

    // 创建根签名
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3DX12SerializeVersionedRootSignature(
        &rootSignatureDesc,
        &signature,
        &error
    );

    if (SUCCEEDED(hr))
    {
        hr = device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_pRootSignature)
        );
    }

    if (SUCCEEDED(hr))
    {
        // 默认使用 CBV/SRV/UAV 描述符堆的大小
        m_byteSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    return hr;
}

HRESULT D3D12RootSignature::Create(
    ID3D12Device* device,
    const D3D12_ROOT_SIGNATURE_DESC1* desc,
    D3D12_ROOT_SIGNATURE_FLAGS flags)
{
    if (device == nullptr || desc == nullptr)
        return E_INVALIDARG;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3DX12SerializeVersionedRootSignature(
        desc,
        &signature,
        &error
    );

    if (SUCCEEDED(hr))
    {
        hr = device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_pRootSignature)
        );
    }

    if (SUCCEEDED(hr))
    {
        m_byteSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    return hr;
}
