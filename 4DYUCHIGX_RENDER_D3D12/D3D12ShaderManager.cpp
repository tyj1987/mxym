// 必须首先包含预编译头
#include "D3D12PCH.h"

#include "D3D12ShaderManager.h"
#include "D3D12Fixes.h"
#include "D3D12PipelineState.h"  // 使用独立的 D3D12PipelineState
#include <fstream>
#include <sstream>
#include <algorithm>

const D3D12_INPUT_ELEMENT_DESC D3D12InputLayout::s_positionLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 }
};

const D3D12_INPUT_ELEMENT_DESC D3D12InputLayout::s_positionNormalLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 }
};

const D3D12_INPUT_ELEMENT_DESC D3D12InputLayout::s_positionNormalTextureLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 }
};

const D3D12_INPUT_ELEMENT_DESC D3D12InputLayout::s_positionColorLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 }
};

const D3D12_INPUT_ELEMENT_DESC D3D12InputLayout::s_positionNormalTextureColorLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_VERTEX, 0 }
};

D3D12_INPUT_LAYOUT_DESC D3D12InputLayout::GetPositionLayout()
{
    D3D12_INPUT_LAYOUT_DESC desc = {};
    desc.pInputElementDescs = s_positionLayout;
    desc.NumElements = sizeof(s_positionLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    return desc;
}

D3D12_INPUT_LAYOUT_DESC D3D12InputLayout::GetPositionNormalLayout()
{
    D3D12_INPUT_LAYOUT_DESC desc = {};
    desc.pInputElementDescs = s_positionNormalLayout;
    desc.NumElements = sizeof(s_positionNormalLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    return desc;
}

D3D12_INPUT_LAYOUT_DESC D3D12InputLayout::GetPositionNormalTextureLayout()
{
    D3D12_INPUT_LAYOUT_DESC desc = {};
    desc.pInputElementDescs = s_positionNormalTextureLayout;
    desc.NumElements = sizeof(s_positionNormalTextureLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    return desc;
}

D3D12_INPUT_LAYOUT_DESC D3D12InputLayout::GetPositionColorLayout()
{
    D3D12_INPUT_LAYOUT_DESC desc = {};
    desc.pInputElementDescs = s_positionColorLayout;
    desc.NumElements = sizeof(s_positionColorLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    return desc;
}

D3D12_INPUT_LAYOUT_DESC D3D12InputLayout::GetPositionNormalTextureColorLayout()
{
    D3D12_INPUT_LAYOUT_DESC desc = {};
    desc.pInputElementDescs = s_positionNormalTextureColorLayout;
    desc.NumElements = sizeof(s_positionNormalTextureColorLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    return desc;
}

// D3D12ShaderCompiler 实现
HRESULT D3D12ShaderCompiler::CompileFromFile(
    const wchar_t* filename,
    const char* entryPoint,
    const char* target,
    ID3DBlob** blob)
{
    // 读取文件
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return E_FAIL;

    file.seekg(0, std::ios::end);
    SIZE_T fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> fileData(fileSize);
    file.read(fileData.data(), fileSize);
    file.close();

    return CompileFromString(
        fileData.data(),
        fileSize,
        entryPoint,
        target,
        blob
    );
}

HRESULT D3D12ShaderCompiler::CompileFromString(
    const char* source,
    SIZE_T sourceSize,
    const char* entryPoint,
    const char* target,
    ID3DBlob** blob)
{
    return CompileInternal(source, sourceSize, entryPoint, target, blob);
}

HRESULT D3D12ShaderCompiler::CompileVertexShader(
    const wchar_t* filename,
    ID3DBlob** blob)
{
    return CompileFromFile(filename, "main", "vs_5_0", blob);
}

HRESULT D3D12ShaderCompiler::CompilePixelShader(
    const wchar_t* filename,
    ID3DBlob** blob)
{
    return CompileFromFile(filename, "main", "ps_5_0", blob);
}

HRESULT D3D12ShaderCompiler::CompileInternal(
    const void* sourceCode,
    SIZE_T sourceSize,
    const char* entryPoint,
    const char* target,
    ID3DBlob** blob)
{
    if (sourceCode == nullptr || sourceSize == 0)
        return E_INVALIDARG;

    UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    return D3DCompile(
        sourceCode,
        sourceSize,
        nullptr,              // pFileName
        nullptr,              // pDefines
        nullptr,              // pInclude
        entryPoint,
        target,
        flags,
        0,                   // Flags2
        blob,
        nullptr              // ppErrorMsgs
    );
}

// D3D12RootSignature 实现已移至 D3D12RootSignature.cpp
// D3D12PipelineState 已移至独立的 D3D12PipelineState.h/cpp 文件
// 此处保留 D3D12Utils 实现

// D3D12Utils 实现
HRESULT D3D12Utils::UploadDataToGPU(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    ID3D12Resource* destination,
    const void* data,
    UINT64 size)
{
    if (device == nullptr || commandList == nullptr || destination == nullptr || data == nullptr)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    try
    {
        // 创建上传堆资源
        D3D12_RESOURCE_DESC uploadBufferDesc = {};
        uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadBufferDesc.Alignment = 0;
        uploadBufferDesc.Width = size;
        uploadBufferDesc.Height = 1;
        uploadBufferDesc.DepthOrArraySize = 1;
        uploadBufferDesc.MipLevels = 1;
        uploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        uploadBufferDesc.SampleDesc.Count = 1;
        uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        uploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);

        ComPtr<ID3D12Resource> uploadBuffer;
        hr = device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer)
        );

        if (FAILED(hr))
            return hr;

        // 映射并复制数据
        void* pData = nullptr;
        hr = uploadBuffer->Map(0, nullptr, &pData);
        if (FAILED(hr))
        {
            uploadBuffer.Reset();
            return hr;
        }

        memcpy(pData, data, size);
        uploadBuffer->Unmap(0, nullptr);

        // 创建资源屏障
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = destination;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

        commandList->ResourceBarrier(1, &barrier);

        // 复制数据
        commandList->CopyBufferRegion(
            destination,             // pDstBuffer
            0,                        // DstOffset
            uploadBuffer.Get(),       // pSrcBuffer
            0,                        // SrcOffset
            size                      // NumBytes
        );

        // 等待完成
        D3D12_RESOURCE_BARRIER endBarrier = {};
        endBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        endBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        endBarrier.UAV.pResource = nullptr;

        commandList->ResourceBarrier(1, &endBarrier);

        // 执行命令列表并等待
        // 注意：调用者需要执行和等待命令列表
    }
    catch (...)
    {
        return E_FAIL;
    }

    return S_OK;
}

void D3D12Utils::TransitionResource(
    ID3D12GraphicsCommandList* commandList,
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES stateBefore,
    D3D12_RESOURCE_STATES stateAfter,
    UINT subresource)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.Subresource = subresource;
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter = stateAfter;

    commandList->ResourceBarrier(1, &barrier);
}

void D3D12Utils::ResourceBarrier(
    ID3D12GraphicsCommandList* commandList,
    D3D12_RESOURCE_BARRIER* barriers,
    UINT numBarriers)
{
    commandList->ResourceBarrier(numBarriers, barriers);
}
