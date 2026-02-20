// ============================================
// D3D12MeshObject.h
// DirectX 12 静态网格对象类
// 第 5 阶段 - MeshObject 系统实现
// ============================================

#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <vector>
#include <string>
#include "../4DyuchiGRX_common/typedef.h"
#include "I4DyuchiGXRenderer.h"



namespace DirectX { using namespace DirectX; }

// 前向声明
class CoD3D12Device;

// ========== 网格子网格结构 ==========
struct MeshSubset
{
    UINT indexStart;         // 起始索引
    UINT indexCount;         // 索引数量
    UINT vertexStart;        // 起始顶点
    UINT vertexCount;        // 顶点数量
    void* pMaterialHandle;   // 材质句柄
    DWORD dwTextureID;       // 纹理 ID
};

// ========== 网格包围盒结构 ==========
struct MeshBounds
{
    DirectX::XMFLOAT3 center;        // 中心点
    DirectX::XMFLOAT3 extents;       // 尺寸
    DirectX::XMFLOAT3 min;           // 最小点
    DirectX::XMFLOAT3 max;           // 最大点
    float radius;                    // 包围球半径

    MeshBounds() : center{0.0f, 0.0f, 0.0f}, extents{0.0f, 0.0f, 0.0f}, min{0.0f, 0.0f, 0.0f}, max{0.0f, 0.0f, 0.0f}, radius(0.0f) {}
};

// ========== 标准 3D 顶点格式 ==========
struct MeshVertex
{
    DirectX::XMFLOAT3 position;     // 位置 (x, y, z)
    DirectX::XMFLOAT3 normal;       // 法线 (nx, ny, nz)
    DirectX::XMFLOAT2 texCoord;     // 纹理坐标 (u, v)
    DirectX::XMFLOAT3 tangent;      // 切线 (tx, ty, tz)

    MeshVertex() : position(0, 0, 0), normal(0, 0, 0), texCoord(0, 0), tangent(0, 0, 0) {}
};

// ========== 网格常量缓冲区（256 字节对齐）==========
struct MeshConstants
{
    DirectX::XMFLOAT4X4 world;           // 世界矩阵 (64 字节)
    DirectX::XMFLOAT4X4 worldInverse;    // 世界逆矩阵（用于法线变换）(64 字节)
    DirectX::XMFLOAT4X4 prevWorld;       // 上一帧世界矩阵（用于运动模糊）(64 字节)
    DWORD dwMeshFlags;                   // 网格标志 (4 字节)
    DWORD dwPadding[11];                 // 填充到 256 字节 (44 字节)

    MeshConstants()
    {
        world = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        worldInverse = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        prevWorld = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        dwMeshFlags = 0;
        memset(dwPadding, 0, sizeof(dwPadding));
    }
};

// ============================================
// D3D12MeshObject 类
// 静态 3D 网格对象（持久化顶点数据）
// ============================================

class D3D12MeshObject : public IDIMeshObject
{
public:
    D3D12MeshObject();
    virtual ~D3D12MeshObject();

    // ========== IUnknown 接口 ==========
    STDMETHODIMP QueryInterface(REFIID riid, PPVOID ppv) override;
    STDMETHODIMP_(ULONG) AddRef(void) override;
    STDMETHODIMP_(ULONG) Release(void) override;

    // ========== IDIMeshObject 接口 ==========

    // 开始初始化网格
    BOOL __stdcall StartInitialize(MESH_DESC* pDesc, IGeometryController* pControl, IGeometryControllerStatic* pControlStatic) override;

    // 结束初始化
    void __stdcall EndInitialize() override;

    // 插入面组
    BOOL __stdcall InsertFaceGroup(FACE_DESC* pDesc) override;

    // 渲染网格
    BOOL __stdcall Render(
        DWORD dwRefIndex, DWORD dwAlpha,
        LIGHT_INDEX_DESC* pDynamicLightIndexList, DWORD dwLightNum,
        LIGHT_INDEX_DESC* pSpotLightIndexList, DWORD dwSpotLightNum,
        DWORD dwMtlSetIndex,
        DWORD dwEffectIndex,
        DWORD dwFlag
    ) override;

    // 渲染投影
    BOOL __stdcall RenderProjection(
        DWORD dwRefIndex,
        DWORD dwAlpha,
        BYTE* pSpotLightIndex,
        DWORD dwViewNum,
        DWORD dwFlag
    ) override;

    // 更新网格
    BOOL __stdcall Update(DWORD dwFlag) override;

    // 禁用更新
    void __stdcall DisableUpdate() override;

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice);

    // ========== 顶点缓冲区管理 ==========
    HRESULT CreateVertexBuffer(const void* pData, UINT size, UINT stride);
    HRESULT CreateVertexBuffer(const MeshVertex* pVertices, UINT count);
    HRESULT CreateVertexBuffer(const IVERTEX* pVertices, UINT count);

    // ========== 索引缓冲区管理 ==========
    HRESULT CreateIndexBuffer(const void* pData, UINT size, DXGI_FORMAT format);
    HRESULT CreateIndexBuffer(const DWORD* pIndices, UINT count);
    HRESULT CreateIndexBuffer(const WORD* pIndices, UINT count);

    // ========== 子网格管理 ==========
    HRESULT AddSubset(const MeshSubset& subset);
    UINT GetSubsetCount() const;
    const MeshSubset* GetSubset(UINT index) const;
    void ClearSubsets();

    // ========== 材质管理 ==========
    void SetMaterial(void* pMaterialHandle, UINT subsetIndex = 0);
    void* GetMaterial(UINT subsetIndex = 0) const;
    void SetMaterialToAllSubsets(void* pMaterialHandle);

    // ========== 包围盒 ==========
    void SetBounds(const MeshBounds& bounds);
    const MeshBounds& GetBounds() const;
    void CalculateBounds(const MeshVertex* pVertices, UINT count);
    void CalculateBounds(const IVERTEX* pVertices, UINT count);

    // ========== 渲染 ==========
    void Render(ID3D12GraphicsCommandList* pCommandList);
    void RenderSubset(ID3D12GraphicsCommandList* pCommandList, UINT subsetIndex);

    // ========== 变换 ==========
    void SetWorldMatrix(const DirectX::XMFLOAT4X4& matrix);
    const DirectX::XMFLOAT4X4& GetWorldMatrix() const;
    void UpdateConstants();

    // ========== 标志 ==========
    void SetMeshFlag(CMeshFlag flag);
    CMeshFlag GetMeshFlag() const;

    // ========== 获取视图 ==========
    const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const;
    const D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() const;

    // ========== 统计 ==========
    UINT GetVertexCount() const;
    UINT GetIndexCount() const;
    UINT GetVertexStride() const;

private:
    // COM 引用计数
    DWORD m_dwRefCount;

    // ========== 顶点缓冲区 ==========
    ComPtr<ID3D12Resource> m_pVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    UINT m_nVertexCount;
    UINT m_nVertexStride;

    // ========== 索引缓冲区 ==========
    ComPtr<ID3D12Resource> m_pIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_nIndexCount;
    DXGI_FORMAT m_indexFormat;

    // ========== 子网格 ==========
    std::vector<MeshSubset> m_vSubsets;

    // ========== 材质 ==========
    std::vector<void*> m_vMaterials;

    // ========== 包围盒 ==========
    MeshBounds m_bounds;

    // ========== 网格标志 ==========
    CMeshFlag m_meshFlag;

    // ========== 世界矩阵 ==========
    DirectX::XMFLOAT4X4 m_worldMatrix;

    // ========== 常量缓冲区 ==========
    ComPtr<ID3D12Resource> m_pConstantBuffer;
    MeshConstants m_constants;
    BYTE* m_pConstantBufferMapped;  // 映射的常量缓冲区指针

    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

private:
    // ========== 内部辅助方法 ==========

    // 计算切线
    void CalculateTangents(MeshVertex* pVertices, UINT count);
};

// ============================================
// 辅助宏
// ============================================

#define SAFE_RELEASE_MESH(p) \
    if ((p) != nullptr) { \
        (p)->Release(); \
        (p) = nullptr; \
    }
