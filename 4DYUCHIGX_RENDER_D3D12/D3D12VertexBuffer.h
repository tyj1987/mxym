#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <map>
#include <DirectXMath.h>



// 顶点格式定义
struct VertexPosition
{
    DirectX::XMFLOAT3 position;
};

struct VertexPositionColor
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

struct VertexPositionNormal
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
};

struct VertexPositionNormalTexture
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
};

struct VertexPositionNormalTextureColor
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
    DirectX::XMFLOAT4 color;
};

// 顶点缓冲区类
class D3D12VertexBuffer
{
public:
    D3D12VertexBuffer();
    virtual ~D3D12VertexBuffer();

    // 创建顶点缓冲区
    // data: 顶点数据指针
    // size: 数据大小（字节）
    // stride: 每个顶点的大小（字节）
    HRESULT Create(
        ID3D12Device* device,
        const void* data,
        UINT size,
        UINT stride,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
    );

    // 从系统内存创建（用于频繁更新的动态数据）
    HRESULT CreateDynamic(
        ID3D12Device* device,
        UINT size,
        UINT stride
    );

    // 更新动态缓冲区数据
    HRESULT UpdateData(const void* data, UINT size, UINT offset = 0);

    // 释放资源
    void Release();

    // 获取缓冲区视图
    ID3D12Resource* GetBuffer() const { return m_vertexBuffer.Get(); }
    D3D12_VERTEX_BUFFER_VIEW GetView() const { return m_vertexBufferView; }

    // 获取缓冲区信息
    UINT GetStride() const { return m_stride; }
    UINT GetSize() const { return m_size; }
    UINT GetVertexCount() const { return m_size / m_stride; }
    ID3D12Resource* GetUploadResource() const { return m_uploadBuffer.Get(); }

    // 映射缓冲区（用于动态更新）
    HRESULT Map(void** ppData);
    void Unmap();

private:
    ComPtr<ID3D12Resource> m_vertexBuffer;      // GPU 端缓冲区
    ComPtr<ID3D12Resource> m_uploadBuffer;       // 上传堆缓冲区

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    UINT m_stride = 0;
    UINT m_size = 0;
    BOOL m_isDynamic = FALSE;
};

// 顶点缓冲区管理器（用于批处理和缓存）
class D3D12VertexBufferManager
{
public:
    D3D12VertexBufferManager(ID3D12Device* device);
    ~D3D12VertexBufferManager();

    // 获取或创建顶点缓冲区
    D3D12VertexBuffer* GetVertexBuffer(UINT stride, UINT size);

    // 释放所有缓冲区
    void ReleaseAll();

private:
    ID3D12Device* m_device;

    struct BufferKey
    {
        UINT stride;
        UINT size;

        bool operator<(const BufferKey& other) const
        {
            if (stride != other.stride)
                return stride < other.stride;
            return size < other.size;
        }

        bool operator==(const BufferKey& other) const
        {
            return stride == other.stride && size == other.size;
        }
    };

    std::map<BufferKey, D3D12VertexBuffer*> m_bufferCache;
};
