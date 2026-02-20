#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <map>



// 索引缓冲区类
class D3D12IndexBuffer
{
public:
    D3D12IndexBuffer();
    virtual ~D3D12IndexBuffer();

    // 创建索引缓冲区
    // data: 索引数据指针
    // count: 索引数量
    // format: 索引格式 (DXGI_FORMAT_R32_UINT 或 DXGI_FORMAT_R16_UINT)
    HRESULT Create(
        ID3D12Device* device,
        const void* data,
        UINT count,
        DXGI_FORMAT format = DXGI_FORMAT_R32_UINT
    );

    // 创建动态索引缓冲区
    HRESULT CreateDynamic(
        ID3D12Device* device,
        UINT count,
        DXGI_FORMAT format = DXGI_FORMAT_R32_UINT
    );

    // 更新索引数据
    HRESULT UpdateData(const void* data, UINT count, UINT offset = 0);

    // 释放资源
    void Release();

    // 获取缓冲区视图
    ID3D12Resource* GetBuffer() const { return m_indexBuffer.Get(); }
    D3D12_INDEX_BUFFER_VIEW GetView() const { return m_indexBufferView; }

    // 获取缓冲区信息
    DXGI_FORMAT GetFormat() const { return m_format; }
    UINT GetIndexCount() const { return m_indexCount; }
    UINT GetSize() const { return m_size; }
    ID3D12Resource* GetUploadResource() const { return m_uploadBuffer.Get(); }

    // 映射缓冲区
    HRESULT Map(void** ppData);
    void Unmap();

private:
    ComPtr<ID3D12Resource> m_indexBuffer;       // GPU 端缓冲区
    ComPtr<ID3D12Resource> m_uploadBuffer;        // 上传堆缓冲区

    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
    UINT m_indexCount = 0;
    UINT m_size = 0;
    DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
    BOOL m_isDynamic = FALSE;
};

// 索引缓冲区管理器
class D3D12IndexBufferManager
{
public:
    D3D12IndexBufferManager(ID3D12Device* device);
    ~D3D12IndexBufferManager();

    // 获取或创建索引缓冲区
    D3D12IndexBuffer* GetIndexBuffer(UINT count, DXGI_FORMAT format);

    // 释放所有缓冲区
    void ReleaseAll();

private:
    ID3D12Device* m_device;

    struct BufferKey
    {
        UINT count;
        DXGI_FORMAT format;

        bool operator<(const BufferKey& other) const
        {
            if (count != other.count)
                return count < other.count;
            return static_cast<UINT>(format) < static_cast<UINT>(other.format);
        }

        bool operator==(const BufferKey& other) const
        {
            return count == other.count && format == other.format;
        }
    };

    std::map<BufferKey, D3D12IndexBuffer*> m_bufferCache;
};
