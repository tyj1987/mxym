// 必须首先包含预编译头
// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12PCH.h"
#include "D3D12VertexBuffer.h"
#include "D3D12Fixes.h"
#include "d3dx12.h"
#include <stdexcept>
#include <cassert>


D3D12VertexBuffer::D3D12VertexBuffer()
{
    m_stride = 0;
    m_size = 0;
    m_isDynamic = FALSE;
}

D3D12VertexBuffer::~D3D12VertexBuffer()
{
    Release();
}

HRESULT D3D12VertexBuffer::Create(
    ID3D12Device* device,
    const void* data,
    UINT size,
    UINT stride,
    D3D12_RESOURCE_FLAGS flags)
{
    if (device == nullptr || data == nullptr)
        return E_INVALIDARG;

    if (size == 0 || stride == 0)
        return E_INVALIDARG;

    m_size = size;
    m_stride = stride;

    HRESULT hr = S_OK;

    try {
        // 1. 创建默认堆资源（GPU 端）
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Alignment = 0;
        bufferDesc.Width = size;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = flags;

        // 上传堆属性
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)
        );

        if (FAILED(hr))
            return hr;

        // 2. 创建上传堆资源
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

        hr = device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadBuffer)
        );

        if (FAILED(hr))
        {
            m_vertexBuffer.Reset();
            return hr;
        }

        // 3. 将数据复制到上传堆
        void* pData = nullptr;
        hr = m_uploadBuffer->Map(0, nullptr, &pData);
        if (FAILED(hr))
        {
            m_uploadBuffer.Reset();
            m_vertexBuffer.Reset();
            return hr;
        }

        memcpy(pData, data, size);
        m_uploadBuffer->Unmap(0, nullptr);

        // 4. 创建缓冲区视图
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.SizeInBytes = size;
        m_vertexBufferView.StrideInBytes = stride;

    }
    catch (const std::exception& e)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT D3D12VertexBuffer::CreateDynamic(
    ID3D12Device* device,
    UINT size,
    UINT stride)
{
    if (device == nullptr)
        return E_INVALIDARG;

    if (size == 0 || stride == 0)
        return E_INVALIDARG;

    m_size = size;
    m_stride = stride;
    m_isDynamic = TRUE;

    HRESULT hr = S_OK;

    try {
        // 创建上传堆资源（用于动态数据）
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Alignment = 0;
        bufferDesc.Width = size;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

        hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)
        );

        if (FAILED(hr))
            return hr;

        m_uploadBuffer = m_vertexBuffer; // 上传堆和缓冲区是同一个

        // 创建缓冲区视图
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.SizeInBytes = size;
        m_vertexBufferView.StrideInBytes = stride;
    }
    catch (const std::exception& e)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT D3D12VertexBuffer::UpdateData(const void* data, UINT size, UINT offset)
{
    if (data == nullptr)
        return E_INVALIDARG;

    if (!m_isDynamic)
    {
        // 静态缓冲区不支持直接更新
        // 需要使用 UpdateSubresources
        // TODO: 实现静态缓冲区更新
        return E_NOTIMPL;
    }

    if (offset + size > m_size)
        return E_INVALIDARG;

    // 映射并更新数据
    void* pData = nullptr;
    HRESULT hr = m_vertexBuffer->Map(0, nullptr, &pData);
    if (FAILED(hr))
        return hr;

    memcpy(static_cast<char*>(pData) + offset, data, size);
    m_vertexBuffer->Unmap(0, nullptr);

    return S_OK;
}

void D3D12VertexBuffer::Release()
{
    m_vertexBuffer.Reset();
    m_uploadBuffer.Reset();
    m_size = 0;
    m_stride = 0;
    m_isDynamic = FALSE;
}

HRESULT D3D12VertexBuffer::Map(void** ppData)
{
    if (ppData == nullptr)
        return E_INVALIDARG;

    if (!m_vertexBuffer)
        return E_FAIL;

    // 对于动态缓冲区，直接映射
    if (m_isDynamic)
    {
        return m_vertexBuffer->Map(0, nullptr, ppData);
    }
    else
    {
        // 对于静态缓冲区，映射上传堆
        return m_uploadBuffer->Map(0, nullptr, ppData);
    }
}

void D3D12VertexBuffer::Unmap()
{
    if (m_isDynamic)
    {
        m_vertexBuffer->Unmap(0, nullptr);
    }
    else
    {
        m_uploadBuffer->Unmap(0, nullptr);

        // 如果需要，将数据复制到 GPU 端缓冲区
        // TODO: 实现 UpdateSubresources
    }
}

// 顶点缓冲区管理器实现
D3D12VertexBufferManager::D3D12VertexBufferManager(ID3D12Device* device)
    : m_device(device)
{
    if (device == nullptr)
        throw std::invalid_argument("device cannot be nullptr");
}

D3D12VertexBufferManager::~D3D12VertexBufferManager()
{
    ReleaseAll();
}

D3D12VertexBuffer* D3D12VertexBufferManager::GetVertexBuffer(UINT stride, UINT size)
{
    BufferKey key = { stride, size };

    // 查找缓存
    auto it = m_bufferCache.find(key);
    if (it != m_bufferCache.end())
    {
        return it->second;
    }

    // 创建新的顶点缓冲区
    D3D12VertexBuffer* vb = new D3D12VertexBuffer();
    if (vb == nullptr)
        return nullptr;

    // 初始化为零（稍后填充数据）
    if (FAILED(vb->CreateDynamic(m_device, size, stride)))
    {
        delete vb;
        return nullptr;
    }

    m_bufferCache[key] = vb;
    return vb;
}

void D3D12VertexBufferManager::ReleaseAll()
{
    for (auto& pair : m_bufferCache)
    {
        if (pair.second)
        {
            pair.second->Release();
            delete pair.second;
        }
    }
    m_bufferCache.clear();
}
