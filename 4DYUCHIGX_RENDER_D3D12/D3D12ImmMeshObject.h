// ============================================
// D3D12ImmMeshObject.h
// DirectX 12 立即模式网格对象类
// 第 5 阶段 - MeshObject 系统实现
// ============================================

#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include <vector>

#include "../4DyuchiGRX_common/typedef.h"
#include "I4DyuchiGXRenderer.h"
#include "D3D12MeshObject.h"



namespace DirectX { using namespace DirectX; }

// 前向声明
class CoD3D12Device;

// D3D12_IMM_VERTEX 结构体（用于立即模式渲染）
struct D3D12_IMM_VERTEX
{
    DirectX::XMFLOAT3 v3Pos;
    DirectX::XMFLOAT3 v3Normal;
    float fTu;
    float fTv;
    DWORD dwDiffuse;
};

// 从IVERTEX转换为D3D12_IMM_VERTEX的辅助函数
inline D3D12_IMM_VERTEX ConvertIVertexToD3D12(const IVERTEX& src, const VECTOR3& pos)
{
    D3D12_IMM_VERTEX dest;
    dest.v3Pos = DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
    dest.v3Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);  // 默认法线
    dest.fTu = src.u1;
    dest.fTv = src.v1;
    dest.dwDiffuse = 0xFFFFFFFF;  // 默认白色
    return dest;
}

// ============================================
// D3D12ImmMeshObject 类
// 立即模式 3D 网格对象（动态顶点数据）
// 用途：调试绘制、特效、UI 等频繁变化的内容
// ============================================

class D3D12ImmMeshObject : public IDIMeshObject
{
public:
    D3D12ImmMeshObject();
    virtual ~D3D12ImmMeshObject();

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
    HRESULT Initialize(CoD3D12Device* pDevice, UINT maxVertices, UINT maxIndices);

    // ========== 动态顶点更新 ==========
    HRESULT UpdateVertices(const IVERTEX* pVertices, UINT count);
    HRESULT UpdateVertices(const VECTOR3* pPositions, UINT count);
    HRESULT UpdateVertices(const MeshVertex* pVertices, UINT count);
    HRESULT UpdateVertices(const D3D12_IMM_VERTEX* pVertices, UINT count);

    // ========== 动态索引更新 ==========
    HRESULT UpdateIndices(const DWORD* pIndices, UINT count);
    HRESULT UpdateIndices(const WORD* pIndices, UINT count);

    // ========== 混合更新（顶点+索引）==========
    HRESULT UpdateMesh(const IVERTEX* pVertices, UINT vertexCount, const DWORD* pIndices, UINT indexCount);

    // ========== 立即渲染 ==========
    void RenderImm(ID3D12GraphicsCommandList* pCommandList, void* pMaterialHandle);

    // ========== 清空 ==========
    void Clear();
    void Reset();

    // ========== 材质 ==========
    void SetMaterial(void* pMaterialHandle);
    void* GetMaterial() const;

    // ========== 变换 ==========
    void SetWorldMatrix(const DirectX::XMFLOAT4X4& matrix);
    const DirectX::XMFLOAT4X4& GetWorldMatrix() const;

    // ========== 获取视图 ==========
    const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const;
    const D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() const;

    // ========== 统计 ==========
    UINT GetMaxVertices() const;
    UINT GetMaxIndices() const;
    UINT GetCurrentVertexCount() const;
    UINT GetCurrentIndexCount() const;
    BOOL IsEmpty() const;

private:
    // COM 引用计数
    DWORD m_dwRefCount;

    // ========== 动态顶点缓冲区（上传堆）==========
    ComPtr<ID3D12Resource> m_pVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    UINT m_nMaxVertices;
    UINT m_nCurrentVertexCount;

    // ========== 动态索引缓冲区（上传堆）==========
    ComPtr<ID3D12Resource> m_pIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    UINT m_nMaxIndices;
    UINT m_nCurrentIndexCount;

    // ========== 材质 ==========
    void* m_pMaterial;

    // ========== 世界矩阵 ==========
    DirectX::XMFLOAT4X4 m_worldMatrix;

    // ========== 常量缓冲区 ==========
    ComPtr<ID3D12Resource> m_pConstantBuffer;
    MeshConstants m_constants;

    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

    // ========== 内部辅助方法 ==========
    HRESULT CreateVertexBuffer(UINT size, UINT stride);
    HRESULT CreateIndexBuffer(UINT size);
    void UpdateConstants();
};

// ============================================
// 辅助宏
// ============================================

#define SAFE_RELEASE_IMMSH(p) \
    if ((p) != nullptr) { \
        (p)->Release(); \
        (p) = nullptr; \
    }
