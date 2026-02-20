// ============================================
// D3D12ImmMeshObject.cpp
// DirectX 12 立即模式网格对象类实现
// 第 5 阶段 - MeshObject 系统实现
// ============================================

#include "D3D12PCH.h"
#include "D3D12ImmMeshObject.h"
#include <cmath>
#include "CoD3D12Device.h"
#include <cstring>

// ========== 构造函数 ==========
#include "D3D12PCH.h"

D3D12ImmMeshObject::D3D12ImmMeshObject()
    : m_dwRefCount(1)
    , m_nMaxVertices(0)
    , m_nCurrentVertexCount(0)
    , m_nMaxIndices(0)
    , m_nCurrentIndexCount(0)
    , m_pMaterial(nullptr)
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
D3D12ImmMeshObject::~D3D12ImmMeshObject()
{
    // 释放资源由 ComPtr 自动管理
}

// ========== IUnknown::QueryInterface ==========
STDMETHODIMP D3D12ImmMeshObject::QueryInterface(REFIID riid, PPVOID ppv)
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
STDMETHODIMP_(ULONG) D3D12ImmMeshObject::AddRef(void)
{
    return InterlockedIncrement(&m_dwRefCount);
}

// ========== IUnknown::Release ==========
STDMETHODIMP_(ULONG) D3D12ImmMeshObject::Release(void)
{
    ULONG newRefCount = InterlockedDecrement(&m_dwRefCount);
    if (newRefCount == 0)
    {
        delete this;
    }
    return newRefCount;
}

// ========== 初始化 ==========
HRESULT D3D12ImmMeshObject::Initialize(CoD3D12Device* pDevice, UINT maxVertices, UINT maxIndices)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;
    m_nMaxVertices = maxVertices;
    m_nMaxIndices = maxIndices;

    // 创建顶点缓冲区
    HRESULT hr = CreateVertexBuffer(maxVertices * sizeof(IVERTEX), sizeof(IVERTEX));
    if (FAILED(hr))
        return hr;

    // 创建索引缓冲区（如果需要）
    if (maxIndices > 0)
    {
        hr = CreateIndexBuffer(maxIndices * sizeof(DWORD));
        if (FAILED(hr))
            return hr;
    }

    // 创建常量缓冲区
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

    hr = m_pDevice->GetD3D12Device()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pConstantBuffer)
    );

    if (FAILED(hr))
        return hr;

    // 初始化常量
    UpdateConstants();

    return S_OK;
}

// ========== 创建顶点缓冲区 ==========
HRESULT D3D12ImmMeshObject::CreateVertexBuffer(UINT size, UINT stride)
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = size;
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

    ComPtr<ID3D12Resource> uploadBuffer;
    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );

    if (FAILED(hr))
        return hr;

    m_pVertexBuffer = uploadBuffer;

    m_vertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = size;
    m_vertexBufferView.StrideInBytes = stride;

    return S_OK;
}

// ========== 创建索引缓冲区 ==========
HRESULT D3D12ImmMeshObject::CreateIndexBuffer(UINT size)
{
    ID3D12Device* pDevice = m_pDevice->GetD3D12Device();

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = size;
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

    ComPtr<ID3D12Resource> uploadBuffer;
    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );

    if (FAILED(hr))
        return hr;

    m_pIndexBuffer = uploadBuffer;

    m_indexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = size;
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    return S_OK;
}

// ========== 更新顶点数据（IVERTEX）==========
HRESULT D3D12ImmMeshObject::UpdateVertices(const IVERTEX* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return E_INVALIDARG;

    if (count > m_nMaxVertices)
        return E_INVALIDARG;

    // 映射缓冲区
    void* pMappedData = nullptr;
    D3D12_RANGE range = { 0, count * sizeof(IVERTEX) };
    HRESULT hr = m_pVertexBuffer->Map(0, &range, &pMappedData);
    if (FAILED(hr))
        return hr;

    // 复制顶点数据
    memcpy(pMappedData, pVertices, count * sizeof(IVERTEX));

    // 取消映射
    m_pVertexBuffer->Unmap(0, nullptr);

    m_nCurrentVertexCount = count;

    return S_OK;
}

// ========== 更新顶点数据（VECTOR3）==========
HRESULT D3D12ImmMeshObject::UpdateVertices(const VECTOR3* pPositions, UINT count)
{
    if (!pPositions || count == 0)
        return E_INVALIDARG;

    if (count > m_nMaxVertices)
        return E_INVALIDARG;

    // 映射缓冲区
    void* pMappedData = nullptr;
    D3D12_RANGE range = { 0, count * sizeof(IVERTEX) };
    HRESULT hr = m_pVertexBuffer->Map(0, &range, &pMappedData);
    if (FAILED(hr))
        return hr;

    // 转换并复制顶点数据
    IVERTEX* pVertices = static_cast<IVERTEX*>(pMappedData);
    for (UINT i = 0; i < count; ++i)
    {
        pVertices[i].x = pPositions[i].x;
        pVertices[i].y = pPositions[i].y;
        pVertices[i].z = pPositions[i].z;
        pVertices[i].u1 = 0.0f;
        pVertices[i].v1 = 0.0f;
    }

    // 取消映射
    m_pVertexBuffer->Unmap(0, nullptr);

    m_nCurrentVertexCount = count;

    return S_OK;
}

// ========== 更新顶点数据（MeshVertex）==========
HRESULT D3D12ImmMeshObject::UpdateVertices(const MeshVertex* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return E_INVALIDARG;

    if (count > m_nMaxVertices)
        return E_INVALIDARG;

    // 映射缓冲区
    void* pMappedData = nullptr;
    D3D12_RANGE range = { 0, count * sizeof(IVERTEX) };
    HRESULT hr = m_pVertexBuffer->Map(0, &range, &pMappedData);
    if (FAILED(hr))
        return hr;

    // 转换并复制顶点数据
    IVERTEX* pDestVertices = static_cast<IVERTEX*>(pMappedData);
    for (UINT i = 0; i < count; ++i)
    {
        pDestVertices[i].x = pVertices[i].position.x;
        pDestVertices[i].y = pVertices[i].position.y;
        pDestVertices[i].z = pVertices[i].position.z;
        pDestVertices[i].u1 = pVertices[i].texCoord.x;
        pDestVertices[i].v1 = pVertices[i].texCoord.y;
    }

    // 取消映射
    m_pVertexBuffer->Unmap(0, nullptr);

    m_nCurrentVertexCount = count;

    return S_OK;
}

// ========== 更新顶点数据（D3D12_IMM_VERTEX）==========
HRESULT D3D12ImmMeshObject::UpdateVertices(const D3D12_IMM_VERTEX* pVertices, UINT count)
{
    if (!pVertices || count == 0)
        return E_INVALIDARG;

    if (count > m_nMaxVertices)
        return E_INVALIDARG;

    // 映射缓冲区
    void* pMappedData = nullptr;
    D3D12_RANGE range = { 0, count * sizeof(IVERTEX) };
    HRESULT hr = m_pVertexBuffer->Map(0, &range, &pMappedData);
    if (FAILED(hr))
        return hr;

    // 转换并复制顶点数据
    IVERTEX* pDestVertices = static_cast<IVERTEX*>(pMappedData);
    for (UINT i = 0; i < count; ++i)
    {
        pDestVertices[i].x = pVertices[i].v3Pos.x;
        pDestVertices[i].y = pVertices[i].v3Pos.y;
        pDestVertices[i].z = pVertices[i].v3Pos.z;
        pDestVertices[i].u1 = pVertices[i].fTu;
        pDestVertices[i].v1 = pVertices[i].fTv;
    }

    // 取消映射
    m_pVertexBuffer->Unmap(0, nullptr);

    m_nCurrentVertexCount = count;

    return S_OK;
}

// ========== 更新索引数据（DWORD）==========
HRESULT D3D12ImmMeshObject::UpdateIndices(const DWORD* pIndices, UINT count)
{
    if (!pIndices || count == 0)
        return E_INVALIDARG;

    if (count > m_nMaxIndices)
        return E_INVALIDARG;

    if (!m_pIndexBuffer)
        return E_FAIL;

    // 映射缓冲区
    void* pMappedData = nullptr;
    D3D12_RANGE range = { 0, count * sizeof(DWORD) };
    HRESULT hr = m_pIndexBuffer->Map(0, &range, &pMappedData);
    if (FAILED(hr))
        return hr;

    // 复制索引数据
    memcpy(pMappedData, pIndices, count * sizeof(DWORD));

    // 取消映射
    m_pIndexBuffer->Unmap(0, nullptr);

    m_nCurrentIndexCount = count;

    return S_OK;
}

// ========== 更新索引数据（WORD）==========
HRESULT D3D12ImmMeshObject::UpdateIndices(const WORD* pIndices, UINT count)
{
    if (!pIndices || count == 0)
        return E_INVALIDARG;

    if (count > m_nMaxIndices)
        return E_INVALIDARG;

    if (!m_pIndexBuffer)
        return E_FAIL;

    // 映射缓冲区
    void* pMappedData = nullptr;
    D3D12_RANGE range = { 0, count * sizeof(DWORD) };
    HRESULT hr = m_pIndexBuffer->Map(0, &range, &pMappedData);
    if (FAILED(hr))
        return hr;

    // 转换并复制索引数据
    DWORD* pDestIndices = static_cast<DWORD*>(pMappedData);
    for (UINT i = 0; i < count; ++i)
    {
        pDestIndices[i] = static_cast<DWORD>(pIndices[i]);
    }

    // 取消映射
    m_pIndexBuffer->Unmap(0, nullptr);

    m_nCurrentIndexCount = count;

    return S_OK;
}

// ========== 混合更新（顶点+索引）==========
HRESULT D3D12ImmMeshObject::UpdateMesh(const IVERTEX* pVertices, UINT vertexCount, const DWORD* pIndices, UINT indexCount)
{
    HRESULT hr = UpdateVertices(pVertices, vertexCount);
    if (FAILED(hr))
        return hr;

    if (pIndices && indexCount > 0)
    {
        hr = UpdateIndices(pIndices, indexCount);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

// ========== 立即渲染 ==========
void D3D12ImmMeshObject::RenderImm(ID3D12GraphicsCommandList* pCommandList, void* pMaterialHandle)
{
    if (!pCommandList || m_nCurrentVertexCount == 0)
        return;

    // 设置顶点缓冲区
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // 设置索引缓冲区（如果有）
    if (m_nCurrentIndexCount > 0 && m_pIndexBuffer)
    {
        pCommandList->IASetIndexBuffer(&m_indexBufferView);
    }

    // 设置拓扑类型
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // TODO: 第 11 周设置材质
    // TODO: 设置常量缓冲区
    // TODO: 绑定纹理

    // 更新常量
    UpdateConstants();

    // 绘制
    if (m_nCurrentIndexCount > 0 && m_pIndexBuffer)
    {
        pCommandList->DrawIndexedInstanced(m_nCurrentIndexCount, 1, 0, 0, 0);
    }
    else
    {
        pCommandList->DrawInstanced(m_nCurrentVertexCount, 1, 0, 0);
    }
}

// ========== 清空 ==========
void D3D12ImmMeshObject::Clear()
{
    m_nCurrentVertexCount = 0;
    m_nCurrentIndexCount = 0;
}

// ========== 重置 ==========
void D3D12ImmMeshObject::Reset()
{
    Clear();
    m_pMaterial = nullptr;

    // 重置世界矩阵
    m_worldMatrix = DirectX::XMFLOAT4X4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    UpdateConstants();
}

// ========== 设置材质 ==========
void D3D12ImmMeshObject::SetMaterial(void* pMaterialHandle)
{
    m_pMaterial = pMaterialHandle;
}

// ========== 获取材质 ==========
void* D3D12ImmMeshObject::GetMaterial() const
{
    return m_pMaterial;
}

// ========== 设置世界矩阵 ==========
void D3D12ImmMeshObject::SetWorldMatrix(const DirectX::XMFLOAT4X4& matrix)
{
    m_worldMatrix = matrix;
}

// ========== 获取世界矩阵 ==========
const DirectX::XMFLOAT4X4& D3D12ImmMeshObject::GetWorldMatrix() const
{
    return m_worldMatrix;
}

// ========== 更新常量缓冲区 ==========
void D3D12ImmMeshObject::UpdateConstants()
{
    if (!m_pConstantBuffer)
        return;

    // 更新常量
    m_constants.world = m_worldMatrix;

    // 计算逆矩阵（用于法线变换）
    DirectX::XMMATRIX world = XMLoadFloat4x4(&m_worldMatrix);
    DirectX::XMMATRIX worldInverse = XMMatrixInverse(nullptr, world);
    XMStoreFloat4x4(&m_constants.worldInverse, worldInverse);

    // 映射并上传常量
    void* pMappedData = nullptr;
    if (SUCCEEDED(m_pConstantBuffer->Map(0, nullptr, &pMappedData)))
    {
        memcpy(pMappedData, &m_constants, sizeof(MeshConstants));
        m_pConstantBuffer->Unmap(0, nullptr);
    }
}

// ========== 获取顶点缓冲区视图 ==========
const D3D12_VERTEX_BUFFER_VIEW* D3D12ImmMeshObject::GetVertexBufferView() const
{
    return &m_vertexBufferView;
}

// ========== 获取索引缓冲区视图 ==========
const D3D12_INDEX_BUFFER_VIEW* D3D12ImmMeshObject::GetIndexBufferView() const
{
    return &m_indexBufferView;
}

// ========== 获取最大顶点数 ==========
UINT D3D12ImmMeshObject::GetMaxVertices() const
{
    return m_nMaxVertices;
}

// ========== 获取最大索引数 ==========
UINT D3D12ImmMeshObject::GetMaxIndices() const
{
    return m_nMaxIndices;
}

// ========== 获取当前顶点数 ==========
UINT D3D12ImmMeshObject::GetCurrentVertexCount() const
{
    return m_nCurrentVertexCount;
}

// ========== 获取当前索引数 ==========
UINT D3D12ImmMeshObject::GetCurrentIndexCount() const
{
    return m_nCurrentIndexCount;
}

// ========== 是否为空 ==========
BOOL D3D12ImmMeshObject::IsEmpty() const
{
    return m_nCurrentVertexCount == 0;
}

// ============================================
// IDIMeshObject 接口实现
// ============================================

// ========== 开始初始化网格 ==========
BOOL __stdcall D3D12ImmMeshObject::StartInitialize(MESH_DESC* pDesc, IGeometryController* pControl, IGeometryControllerStatic* pControlStatic)
{
    if (!pDesc)
        return FALSE;

    // TODO: 实现完整的网格初始化
    // ImmMeshObject通常不需要预先初始化，而是动态更新
    return TRUE;
}

// ========== 结束初始化 ==========
void __stdcall D3D12ImmMeshObject::EndInitialize()
{
    // ImmMeshObject不需要初始化结束处理
}

// ========== 插入面组 ==========
BOOL __stdcall D3D12ImmMeshObject::InsertFaceGroup(FACE_DESC* pDesc)
{
    if (!pDesc)
        return FALSE;

    // TODO: 实现面组插入
    // ImmMeshObject通常直接使用UpdateMesh方法
    return TRUE;
}

// ========== 渲染网格（DX8兼容接口） ==========
BOOL __stdcall D3D12ImmMeshObject::Render(
    DWORD dwRefIndex, DWORD dwAlpha,
    LIGHT_INDEX_DESC* pDynamicLightIndexList, DWORD dwLightNum,
    LIGHT_INDEX_DESC* pSpotLightIndexList, DWORD dwSpotLightNum,
    DWORD dwMtlSetIndex,
    DWORD dwEffectIndex,
    DWORD dwFlag)
{
    // TODO: 实现完整的渲染
    // 需要获取命令列表并执行渲染
    return TRUE;
}

// ========== 渲染投影 ==========
BOOL __stdcall D3D12ImmMeshObject::RenderProjection(
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
BOOL __stdcall D3D12ImmMeshObject::Update(DWORD dwFlag)
{
    // 更新常量缓冲区
    UpdateConstants();
    return TRUE;
}

// ========== 禁用更新 ==========
void __stdcall D3D12ImmMeshObject::DisableUpdate()
{
    // TODO: 实现更新禁用标志
}
