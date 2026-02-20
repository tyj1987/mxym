// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12IndexBuffer.h"
#include "D3D12Fixes.h"
#include "d3dx12.h"
#include <stdexcept>


D3D12IndexBuffer::D3D12IndexBuffer()
{
    m_indexCount = 0;
    m_size = 0;
    m_format = DXGI_FORMAT_UNKNOWN;
    m_isDynamic = FALSE;
}

D3D12IndexBuffer::~D3D12IndexBuffer()
{
    Release();
}

HRESULT D3D12IndexBuffer::Create(
    ID3D12Device* device,
    const void* data,
    UINT count,
    DXGI_FORMAT format)
{
    if (device == nullptr || data == nullptr)
        return E_INVALIDARG;

    if (count == 0)
        return E_INVALIDARG;

    m_indexCount = count;
    m_format = format;
    m_size = count * (format == DXGI_FORMAT_R32_UINT ? 4 : 2);

    HRESULT hr = S_OK;

    try {
        // 确定格式大小
        UINT stride = (format == DXGI_FORMAT_R32_UINT) ? 4 : 2;

        // 1. 创建默认堆资源
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Alignment = 0;
        bufferDesc.Width = m_size;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)
        );

        if (FAILED(hr))
            return hr;

        // 2. 创建上传堆资源
        D3D12_RESOURCE_DESC uploadBufferDesc = {};
        uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadBufferDesc.Alignment = 0;
        uploadBufferDesc.Width = m_size;
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
            m_indexBuffer.Reset();
            return hr;
        }

        // 3. 将数据复制到上传堆
        void* pData = nullptr;
        hr = m_uploadBuffer->Map(0, nullptr, &pData);
        if (FAILED(hr))
        {
            m_uploadBuffer.Reset();
            m_indexBuffer.Reset();
            return hr;
        }

        memcpy(pData, data, m_size);
        m_uploadBuffer->Unmap(0, nullptr);

        // 4. 创建索引缓冲区视图
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = format;
        m_indexBufferView.SizeInBytes = m_size;
    }
    catch (const std::exception& e)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT D3D12IndexBuffer::CreateDynamic(
    ID3D12Device* device,
    UINT count,
    DXGI_FORMAT format)
{
    if (device == nullptr)
        return E_INVALIDARG;

    if (count == 0)
        return E_INVALIDARG;

    m_indexCount = count;
    m_format = format;
    m_size = count * (format == DXGI_FORMAT_R32_UINT ? 4 : 2);
    m_isDynamic = TRUE;

    HRESULT hr = S_OK;

    try {
        // 创建上传堆资源
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Alignment = 0;
        bufferDesc.Width = m_size;
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
            IID_PPV_ARGS(&m_indexBuffer)
        );

        if (FAILED(hr))
            return hr;

        m_uploadBuffer = m_indexBuffer;

        // 创建索引缓冲区视图
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = format;
        m_indexBufferView.SizeInBytes = m_size;
    }
    catch (const std::exception& e)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT D3D12IndexBuffer::UpdateData(const void* data, UINT count, UINT offset)
{
    if (data == nullptr)
        return E_INVALIDARG;

    if (!m_isDynamic)
        return E_NOTIMPL;

    if (offset + count * (m_format == DXGI_FORMAT_R32_UINT ? 4 : 2) > m_size)
        return E_INVALIDARG;

    UINT stride = (m_format == DXGI_FORMAT_R32_UINT) ? 4 : 2;
    void* pData = nullptr;
    HRESULT hr = m_indexBuffer->Map(0, nullptr, &pData);
    if (FAILED(hr))
        return hr;

    memcpy(static_cast<char*>(pData) + offset, data, count * stride);
    m_indexBuffer->Unmap(0, nullptr);

    return S_OK;
}

void D3D12IndexBuffer::Release()
{
    m_indexBuffer.Reset();
    m_uploadBuffer.Reset();
    m_indexCount = 0;
    m_size = 0;
    m_format = DXGI_FORMAT_UNKNOWN;
    m_isDynamic = FALSE;
}

HRESULT D3D12IndexBuffer::Map(void** ppData)
{
    if (ppData == nullptr)
        return E_INVALIDARG;

    if (!m_indexBuffer)
        return E_FAIL;

    if (m_isDynamic)
    {
        return m_indexBuffer->Map(0, nullptr, ppData);
    }
    else
    {
        return m_uploadBuffer->Map(0, nullptr, ppData);
    }
}

void D3D12IndexBuffer::Unmap()
{
    if (m_isDynamic)
    {
        m_indexBuffer->Unmap(0, nullptr);
    }
    else
    {
        m_uploadBuffer->Unmap(0, nullptr);
    }
}

// 索引缓冲区管理器实现
D3D12IndexBufferManager::D3D12IndexBufferManager(ID3D12Device* device)
    : m_device(device)
{
    if (device == nullptr)
        throw std::invalid_argument("device cannot be nullptr");
}

D3D12IndexBufferManager::~D3D12IndexBufferManager()
{
    ReleaseAll();
}

D3D12IndexBuffer* D3D12IndexBufferManager::GetIndexBuffer(UINT count, DXGI_FORMAT format)
{
    BufferKey key = { count, format };

    auto it = m_bufferCache.find(key);
    if (it != m_bufferCache.end())
    {
        return it->second;
    }

    D3D12IndexBuffer* ib = new D3D12IndexBuffer();
    if (ib == nullptr)
        return nullptr;

    if (FAILED(ib->CreateDynamic(m_device, count, format)))
    {
        delete ib;
        return nullptr;
    }

    m_bufferCache[key] = ib;
    return ib;
}

void D3D12IndexBufferManager::ReleaseAll()
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
