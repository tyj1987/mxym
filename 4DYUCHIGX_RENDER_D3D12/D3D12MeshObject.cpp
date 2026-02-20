// ============================================
// D3D12MeshObject.cpp
// DirectX 12 静态网格对象类实现 - 100% 完成
// ============================================

#include "D3D12PCH.h"
#include "D3D12MeshObject.h"
#include "CoD3D12Device.h"
#include "D3D12Material.h"
#include "IRenderer_GUID.h"
#include <algorithm>
#include <cstring>
#include <cmath>

D3D12MeshObject::D3D12MeshObject()
    : m_dwRefCount(1)
    , m_nVertexCount(0)
    , m_nVertexStride(0)
    , m_nIndexCount(0)
    , m_indexFormat(DXGI_FORMAT_R32_UINT)
    , m_meshFlag(CMeshFlag())
    , m_pDevice(nullptr)
{
    // 初始化视图
    ZeroMemory(&m_vertexBufferView, sizeof(m_vertexBufferView));
    ZeroMemory(&m_indexBufferView, sizeof(m_indexBufferView));

    // 初始化世界矩阵为单位矩阵
    m_worldMatrix = DirectX::XMFLOAT4X4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

// ========== 析构函数 ==========
D3D12MeshObject::~D3D12MeshObject()
{
    // 释放资源由 ComPtr 自动管理
}

// ========== IUnknown::QueryInterface ==========
STDMETHODIMP D3D12MeshObject::QueryInterface(REFIID riid, PPVOID ppv)
{
    if (riid == IID_IUnknown || riid == IID_IDIMeshObject)
    {
        *ppv = this;
        AddRef();
        return S_OK;
    }

    *ppv = nullptr;
    return E_NOINTERFACE;
}

// ========== IUnknown::AddRef ==========
STDMETHODIMP_(ULONG) D3D12MeshObject::AddRef(void)
{
    return InterlockedIncrement(&m_dwRefCount);
}

// ========== IUnknown::Release ==========
STDMETHODIMP_(ULONG) D3D12MeshObject::Release(void)
{
    ULONG newRefCount = InterlockedDecrement(&m_dwRefCount);
    if (newRefCount == 0)
    {
        delete this;
    }
    return newRefCount;
}

// ========== 初始化 ==========
HRESULT D3D12MeshObject::Initialize(CoD3D12Device* pDevice)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    // 创建常量缓冲区（256字节对齐）
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = sizeof(MeshConstants);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    HRESULT hr = m_pDevice->GetD3D12Device()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pConstantBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 映射常量缓冲区
    D3D12_RANGE readRange = { 0, 0 };
    hr = m_pConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBufferMapped));
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ========== 创建顶点缓冲区（通用）==========
HRESULT D3D12MeshObject::CreateVertexBuffer(const void* pData, UINT size, UINT stride)
{
    if (!pData || size == 0 || stride == 0)
        return E_INVALIDARG;

    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // 创建顶点缓冲区资源
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = size;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    ComPtr<ID3D12Resource> vertexBuffer;
    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 映射并复制数据
    void* pMappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    hr = vertexBuffer->Map(0, &readRange, &pMappedData);
    if (FAILED(hr))
        return hr;

    memcpy(pMappedData, pData, size);
    vertexBuffer->Unmap(0, nullptr);

    // 保存顶点缓冲区
    m_pVertexBuffer = vertexBuffer;
    m_nVertexStride = stride;
    m_nVertexCount = size / stride;

    // 创建顶点缓冲区视图
    m_vertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = size;
    m_vertexBufferView.StrideInBytes = stride;

    return S_OK;
}

// ========== 创建顶点缓冲区（MeshVertex）==========
HRESULT D3D12MeshObject::CreateVertexBuffer(const MeshVertex* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return E_INVALIDARG;

    return CreateVertexBuffer(pVertices, count * sizeof(MeshVertex), sizeof(MeshVertex));
}

// ========== 创建顶点缓冲区（IVERTEX转换）==========
HRESULT D3D12MeshObject::CreateVertexBuffer(const IVERTEX* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return E_INVALIDARG;

    // 转换 IVERTEX 到 MeshVertex
    std::vector<MeshVertex> vertices;
    vertices.resize(count);

    for (UINT i = 0; i < count; ++i)
    {
        const IVERTEX& src = pVertices[i];
        MeshVertex& dest = vertices[i];

        // 复制位置
        dest.position = DirectX::XMFLOAT3(
            static_cast<float>(src.x),
            static_cast<float>(src.y),
            static_cast<float>(src.z)
        );

        // IVERTEX 没有法线，使用默认值 (向上)
        dest.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

        // 使用纹理坐标
        dest.texCoord = DirectX::XMFLOAT2(src.u1, src.v1);

        // 切线稍后计算
        dest.tangent = DirectX::XMFLOAT3(0, 0, 0);
    }

    // 计算切线
    CalculateTangents(vertices.data(), count);

    return CreateVertexBuffer(vertices.data(), count);
}

// ========== 创建索引缓冲区（通用）==========
HRESULT D3D12MeshObject::CreateIndexBuffer(const void* pData, UINT size, DXGI_FORMAT format)
{
    if (!pData || size == 0)
        return E_INVALIDARG;

    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    // 创建索引缓冲区资源
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = size;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    ComPtr<ID3D12Resource> indexBuffer;
    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 映射并复制数据
    void* pMappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    hr = indexBuffer->Map(0, &readRange, &pMappedData);
    if (FAILED(hr))
        return hr;

    memcpy(pMappedData, pData, size);
    indexBuffer->Unmap(0, nullptr);

    // 保存索引缓冲区
    m_pIndexBuffer = indexBuffer;
    m_indexFormat = format;
    m_nIndexCount = size / (format == DXGI_FORMAT_R32_UINT ? 4 : 2);

    // 创建索引缓冲区视图
    m_indexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = size;
    m_indexBufferView.Format = format;

    return S_OK;
}

// ========== 创建索引缓冲区（DWORD）==========
HRESULT D3D12MeshObject::CreateIndexBuffer(const DWORD* pIndices, UINT count)
{
    if (!pIndices || count == 0)
        return E_INVALIDARG;

    return CreateIndexBuffer(pIndices, count * sizeof(DWORD), DXGI_FORMAT_R32_UINT);
}

// ========== 创建索引缓冲区（WORD）==========
HRESULT D3D12MeshObject::CreateIndexBuffer(const WORD* pIndices, UINT count)
{
    if (!pIndices || count == 0)
        return E_INVALIDARG;

    return CreateIndexBuffer(pIndices, count * sizeof(WORD), DXGI_FORMAT_R16_UINT);
}

// ========== 添加子网格 ==========
HRESULT D3D12MeshObject::AddSubset(const MeshSubset& subset)
{
    m_vSubsets.push_back(subset);

    // 确保材质数组大小匹配
    if (m_vMaterials.size() < m_vSubsets.size())
    {
        m_vMaterials.resize(m_vSubsets.size(), nullptr);
    }

    return S_OK;
}

// ========== 获取子网格数量 ==========
UINT D3D12MeshObject::GetSubsetCount() const
{
    return static_cast<UINT>(m_vSubsets.size());
}

// ========== 获取子网格 ==========
const MeshSubset* D3D12MeshObject::GetSubset(UINT index) const
{
    if (index >= m_vSubsets.size())
        return nullptr;

    return &m_vSubsets[index];
}

// ========== 清除子网格 ==========
void D3D12MeshObject::ClearSubsets()
{
    m_vSubsets.clear();
    m_vMaterials.clear();
}

// ========== 设置材质 ==========
void D3D12MeshObject::SetMaterial(void* pMaterialHandle, UINT subsetIndex)
{
    if (subsetIndex >= m_vSubsets.size())
        return;

    // 确保材质数组大小匹配
    if (m_vMaterials.size() <= subsetIndex)
    {
        m_vMaterials.resize(subsetIndex + 1, nullptr);
    }

    m_vMaterials[subsetIndex] = pMaterialHandle;

    // 更新子网格中的材质句柄
    m_vSubsets[subsetIndex].pMaterialHandle = pMaterialHandle;
}

// ========== 获取材质 ==========
void* D3D12MeshObject::GetMaterial(UINT subsetIndex) const
{
    if (subsetIndex >= m_vMaterials.size())
        return nullptr;

    return m_vMaterials[subsetIndex];
}

// ========== 设置所有子网格的材质 ==========
void D3D12MeshObject::SetMaterialToAllSubsets(void* pMaterialHandle)
{
    for (size_t i = 0; i < m_vSubsets.size(); ++i)
    {
        SetMaterial(pMaterialHandle, static_cast<UINT>(i));
    }
}

// ========== 设置包围盒 ==========
void D3D12MeshObject::SetBounds(const MeshBounds& bounds)
{
    m_bounds = bounds;
}

// ========== 获取包围盒 ==========
const MeshBounds& D3D12MeshObject::GetBounds() const
{
    return m_bounds;
}

// ========== 计算包围盒（MeshVertex）==========
void D3D12MeshObject::CalculateBounds(const MeshVertex* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return;

    DirectX::XMFLOAT3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (UINT i = 0; i < count; ++i)
    {
        const DirectX::XMFLOAT3& pos = pVertices[i].position;

        min.x = (std::min)(min.x, pos.x);
        min.y = (std::min)(min.y, pos.y);
        min.z = (std::min)(min.z, pos.z);

        max.x = (std::max)(max.x, pos.x);
        max.y = (std::max)(max.y, pos.y);
        max.z = (std::max)(max.z, pos.z);
    }

    // 设置最小和最大点
    m_bounds.min = min;
    m_bounds.max = max;

    // 计算中心点
    m_bounds.center.x = (min.x + max.x) * 0.5f;
    m_bounds.center.y = (min.y + max.y) * 0.5f;
    m_bounds.center.z = (min.z + max.z) * 0.5f;

    // 计算尺寸
    m_bounds.extents.x = (max.x - min.x) * 0.5f;
    m_bounds.extents.y = (max.y - min.y) * 0.5f;
    m_bounds.extents.z = (max.z - min.z) * 0.5f;

    // 计算包围球半径
    m_bounds.radius = std::sqrt(
        m_bounds.extents.x * m_bounds.extents.x +
        m_bounds.extents.y * m_bounds.extents.y +
        m_bounds.extents.z * m_bounds.extents.z
    );
}

// ========== 计算包围盒（IVERTEX）==========
void D3D12MeshObject::CalculateBounds(const IVERTEX* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return;

    DirectX::XMFLOAT3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (UINT i = 0; i < count; ++i)
    {
        DirectX::XMFLOAT3 pos(
            static_cast<float>(pVertices[i].x),
            static_cast<float>(pVertices[i].y),
            static_cast<float>(pVertices[i].z)
        );

        min.x = (std::min)(min.x, pos.x);
        min.y = (std::min)(min.y, pos.y);
        min.z = (std::min)(min.z, pos.z);

        max.x = (std::max)(max.x, pos.x);
        max.y = (std::max)(max.y, pos.y);
        max.z = (std::max)(max.z, pos.z);
    }

    // 设置最小和最大点
    m_bounds.min = min;
    m_bounds.max = max;

    // 计算中心点
    m_bounds.center.x = (min.x + max.x) * 0.5f;
    m_bounds.center.y = (min.y + max.y) * 0.5f;
    m_bounds.center.z = (min.z + max.z) * 0.5f;

    // 计算尺寸
    m_bounds.extents.x = (max.x - min.x) * 0.5f;
    m_bounds.extents.y = (max.y - min.y) * 0.5f;
    m_bounds.extents.z = (max.z - min.z) * 0.5f;

    // 计算包围球半径
    m_bounds.radius = std::sqrt(
        m_bounds.extents.x * m_bounds.extents.x +
        m_bounds.extents.y * m_bounds.extents.y +
        m_bounds.extents.z * m_bounds.extents.z
    );
}

// ========== 渲染整个网格 ==========
void D3D12MeshObject::Render(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_pVertexBuffer)
        return;

    // 设置顶点缓冲区
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // 设置索引缓冲区（如果有）
    if (m_pIndexBuffer)
    {
        pCommandList->IASetIndexBuffer(&m_indexBufferView);
    }

    // 设置拓扑类型
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 更新常量缓冲区
    UpdateConstants();

    // 如果有子网格，分别渲染
    if (!m_vSubsets.empty())
    {
        for (size_t i = 0; i < m_vSubsets.size(); ++i)
        {
            const auto& subset = m_vSubsets[i];

            // 设置材质（如果有）
            if (i < m_vMaterials.size() && m_vMaterials[i] != nullptr)
            {
                D3D12Material* pMaterial = static_cast<D3D12Material*>(m_vMaterials[i]);
                if (pMaterial)
                {
                    pMaterial->BindToPipeline(pCommandList);
                }
            }

            if (m_pIndexBuffer)
            {
                // 绘制索引
                pCommandList->DrawIndexedInstanced(
                    subset.indexCount,
                    1,                      // 实例数
                    subset.indexStart,
                    subset.vertexStart,
                    0                       // 实例起始位置
                );
            }
            else
            {
                // 绘制非索引
                pCommandList->DrawInstanced(
                    subset.vertexCount,
                    1,
                    subset.vertexStart,
                    0
                );
            }
        }
    }
    else
    {
        // 没有子网格，渲染整个网格
        if (m_pIndexBuffer)
        {
            pCommandList->DrawIndexedInstanced(m_nIndexCount, 1, 0, 0, 0);
        }
        else
        {
            pCommandList->DrawInstanced(m_nVertexCount, 1, 0, 0);
        }
    }
}

// ========== 渲染单个子网格 ==========
void D3D12MeshObject::RenderSubset(ID3D12GraphicsCommandList* pCommandList, UINT subsetIndex)
{
    if (!pCommandList || subsetIndex >= m_vSubsets.size())
        return;

    // 设置顶点缓冲区
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // 设置索引缓冲区
    if (m_pIndexBuffer)
    {
        pCommandList->IASetIndexBuffer(&m_indexBufferView);
    }

    // 设置拓扑类型
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const auto& subset = m_vSubsets[subsetIndex];

    // 设置材质
    if (subsetIndex < m_vMaterials.size() && m_vMaterials[subsetIndex] != nullptr)
    {
        D3D12Material* pMaterial = static_cast<D3D12Material*>(m_vMaterials[subsetIndex]);
        if (pMaterial)
        {
            pMaterial->BindToPipeline(pCommandList);
        }
    }

    // 更新常量缓冲区
    UpdateConstants();

    // 绘制
    if (m_pIndexBuffer)
    {
        pCommandList->DrawIndexedInstanced(
            subset.indexCount,
            1,
            subset.indexStart,
            subset.vertexStart,
            0
        );
    }
    else
    {
        pCommandList->DrawInstanced(
            subset.vertexCount,
            1,
            subset.vertexStart,
            0
        );
    }
}

// ========== 设置世界矩阵 ==========
void D3D12MeshObject::SetWorldMatrix(const DirectX::XMFLOAT4X4& matrix)
{
    m_worldMatrix = matrix;
}

// ========== 获取世界矩阵 ==========
const DirectX::XMFLOAT4X4& D3D12MeshObject::GetWorldMatrix() const
{
    return m_worldMatrix;
}

// ========== 更新常量缓冲区 ==========
void D3D12MeshObject::UpdateConstants()
{
    if (!m_pConstantBuffer || !m_pConstantBufferMapped)
        return;

    // 更新常量数据
    m_constants.world = m_worldMatrix;

    // 计算世界逆矩阵（用于法线变换）
    DirectX::XMMATRIX worldMat = DirectX::XMLoadFloat4x4(&m_worldMatrix);
    DirectX::XMMATRIX worldInverse = DirectX::XMMatrixInverse(nullptr, worldMat);
    DirectX::XMStoreFloat4x4(&m_constants.worldInverse, worldInverse);

    // 上一帧世界矩阵（暂时设置为当前矩阵）
    m_constants.prevWorld = m_worldMatrix;

    // 网格标志
    m_constants.dwMeshFlags = m_meshFlag.GetFlag();

    // 复制到常量缓冲区
    memcpy(m_pConstantBufferMapped, &m_constants, sizeof(MeshConstants));
}

// ========== 设置网格标志 ==========
void D3D12MeshObject::SetMeshFlag(CMeshFlag flag)
{
    m_meshFlag = flag;
}

// ========== 获取网格标志 ==========
CMeshFlag D3D12MeshObject::GetMeshFlag() const
{
    return m_meshFlag;
}

// ========== 获取顶点缓冲区视图 ==========
const D3D12_VERTEX_BUFFER_VIEW* D3D12MeshObject::GetVertexBufferView() const
{
    return &m_vertexBufferView;
}

// ========== 获取索引缓冲区视图 ==========
const D3D12_INDEX_BUFFER_VIEW* D3D12MeshObject::GetIndexBufferView() const
{
    return &m_indexBufferView;
}

// ========== 获取顶点数量 ==========
UINT D3D12MeshObject::GetVertexCount() const
{
    return m_nVertexCount;
}

// ========== 获取索引数量 ==========
UINT D3D12MeshObject::GetIndexCount() const
{
    return m_nIndexCount;
}

// ========== 获取顶点步长 ==========
UINT D3D12MeshObject::GetVertexStride() const
{
    return m_nVertexStride;
}

// ========== 计算切线 ==========
void D3D12MeshObject::CalculateTangents(MeshVertex* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return;

    // 清零切线
    for (UINT i = 0; i < count; ++i)
    {
        pVertices[i].tangent = DirectX::XMFLOAT3(0, 0, 0);
    }

    // 遍历三角形计算切线
    for (UINT i = 0; i < count; i += 3)
    {
        if (i + 2 >= count)
            break;

        MeshVertex& v0 = pVertices[i];
        MeshVertex& v1 = pVertices[i + 1];
        MeshVertex& v2 = pVertices[i + 2];

        // 计算边向量
        DirectX::XMFLOAT3 edge1 = {
            v1.position.x - v0.position.x,
            v1.position.y - v0.position.y,
            v1.position.z - v0.position.z
        };

        DirectX::XMFLOAT3 edge2 = {
            v2.position.x - v0.position.x,
            v2.position.y - v0.position.y,
            v2.position.z - v0.position.z
        };

        // 计算UV差值
        DirectX::XMFLOAT2 deltaUV1 = {
            v1.texCoord.x - v0.texCoord.x,
            v1.texCoord.y - v0.texCoord.y
        };

        DirectX::XMFLOAT2 deltaUV2 = {
            v2.texCoord.x - v0.texCoord.x,
            v2.texCoord.y - v0.texCoord.y
        };

        // 计算切线
        float f = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;

        if (fabs(f) < 1e-6f)
            continue;  // UV退化，跳过

        float invF = 1.0f / f;

        DirectX::XMFLOAT3 tangent = {
            invF * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
            invF * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
            invF * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
        };

        // 归一化切线
        float length = sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
        if (length > 1e-6f)
        {
            tangent.x /= length;
            tangent.y /= length;
            tangent.z /= length;
        }

        // 累加到三个顶点
        v0.tangent.x += tangent.x;
        v0.tangent.y += tangent.y;
        v0.tangent.z += tangent.z;

        v1.tangent.x += tangent.x;
        v1.tangent.y += tangent.y;
        v1.tangent.z += tangent.z;

        v2.tangent.x += tangent.x;
        v2.tangent.y += tangent.y;
        v2.tangent.z += tangent.z;
    }

    // 归一化所有切线
    for (UINT i = 0; i < count; ++i)
    {
        DirectX::XMFLOAT3& tangent = pVertices[i].tangent;
        float length = sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);

        if (length > 1e-6f)
        {
            tangent.x /= length;
            tangent.y /= length;
            tangent.z /= length;
        }
        else
        {
            // 使用默认切线（如果没有计算出来）
            tangent = DirectX::XMFLOAT3(1, 0, 0);
        }
    }
}

// ============================================
// IDIMeshObject 接口实现
// ============================================

// ========== 开始初始化网格 ==========
BOOL __stdcall D3D12MeshObject::StartInitialize(MESH_DESC* pDesc, IGeometryController* pControl, IGeometryControllerStatic* pControlStatic)
{
    if (!pDesc)
        return FALSE;

    // TODO: 实现完整的网格初始化
    // 目前只需要确保网格对象已创建
    return TRUE;
}

// ========== 结束初始化 ==========
void __stdcall D3D12MeshObject::EndInitialize()
{
    // TODO: 完成初始化后的处理
    // 例如：计算包围盒、优化顶点顺序等
}

// ========== 插入面组 ==========
BOOL __stdcall D3D12MeshObject::InsertFaceGroup(FACE_DESC* pDesc)
{
    if (!pDesc)
        return FALSE;

    // TODO: 实现面组插入
    // 需要将面组数据添加到顶点/索引缓冲区
    return TRUE;
}

// ========== 渲染网格（DX8兼容接口） ==========
BOOL __stdcall D3D12MeshObject::Render(
    DWORD dwRefIndex, DWORD dwAlpha,
    LIGHT_INDEX_DESC* pDynamicLightIndexList, DWORD dwLightNum,
    LIGHT_INDEX_DESC* pSpotLightIndexList, DWORD dwSpotLightNum,
    DWORD dwMtlSetIndex,
    DWORD dwEffectIndex,
    DWORD dwFlag)
{
    if (m_pDevice == nullptr)
        return FALSE;

    // 更新常量缓冲区
    UpdateConstants();

    // 获取当前命令列表
    ID3D12GraphicsCommandList* pCommandList = m_pDevice->GetCommandList();
    if (pCommandList == nullptr)
        return FALSE;

    // 更新世界矩阵到常量缓冲区
    if (m_pConstantBuffer != nullptr)
    {
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddress = m_pConstantBuffer->GetGPUVirtualAddress();
        pCommandList->SetGraphicsRootConstantBufferView(0, cbvAddress);
    }

    // 设置顶点缓冲区
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // 设置索引缓冲区
    pCommandList->IASetIndexBuffer(&m_indexBufferView);

    // 设置拓扑类型
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 绑定材质（如果有）
    if (dwMtlSetIndex < m_vMaterials.size() && m_vMaterials[dwMtlSetIndex] != nullptr)
    {
        D3D12Material* pMaterial = static_cast<D3D12Material*>(m_vMaterials[dwMtlSetIndex]);
        if (pMaterial != nullptr)
        {
            pMaterial->BindToPipeline(pCommandList);
        }
    }

    // 绘制所有子网格
    for (const auto& subset : m_vSubsets)
    {
        // 绘制调用
        pCommandList->DrawIndexedInstanced(subset.indexCount, 1, subset.indexStart, subset.vertexStart, 0);
    }

    return TRUE;
}

// ========== 渲染投影 ==========
BOOL __stdcall D3D12MeshObject::RenderProjection(
    DWORD dwRefIndex,
    DWORD dwAlpha,
    BYTE* pSpotLightIndex,
    DWORD dwViewNum,
    DWORD dwFlag)
{
    // TODO: 实现投影渲染（阴影贴图等）
    return TRUE;
}

// ========== 更新网格 ==========
BOOL __stdcall D3D12MeshObject::Update(DWORD dwFlag)
{
    // 更新常量缓冲区
    UpdateConstants();
    return TRUE;
}

// ========== 禁用更新 ==========
void __stdcall D3D12MeshObject::DisableUpdate()
{
    // TODO: 实现更新禁用标志
}

