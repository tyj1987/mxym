// ============================================
// D3D12MeshManager.h
// DirectX 12 网格管理器
// 第 5 阶段 - MeshObject 系统实现
// ============================================

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <map>
#include <vector>
#include <string>



// 前向声明
class D3D12MeshObject;
class D3D12ImmMeshObject;
class D3D12MeshCache;
class CoD3D12Device;

// ============================================
// D3D12MeshManager 类
// 管理所有网格对象的创建、销毁和渲染
// ============================================

class D3D12MeshManager
{
public:
    D3D12MeshManager();
    ~D3D12MeshManager();

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice);
    void Release();

    // ========== MeshObject 创建 ==========
    D3D12MeshObject* CreateMeshObject();
    D3D12ImmMeshObject* CreateImmMeshObject(UINT maxVertices, UINT maxIndices);

    // ========== MeshObject 管理 ==========
    void* AddMesh(D3D12MeshObject* pMesh);
    void* AddMesh(D3D12ImmMeshObject* pMesh);
    void RemoveMesh(void* pHandle);
    D3D12MeshObject* GetMesh(void* pHandle);
    D3D12MeshObject* GetStaticMesh(void* pHandle);
    D3D12ImmMeshObject* GetImmMesh(void* pHandle);

    // ========== 批量渲染 ==========
    void RenderAllMeshes(ID3D12GraphicsCommandList* pCommandList);
    void RenderMeshesByMaterial(ID3D12GraphicsCommandList* pCommandList, void* pMaterialHandle);

    // ========== 缓存管理 ==========
    D3D12MeshCache* GetCache();
    HRESULT AddToCache(const std::string& strKey, D3D12MeshObject* pMesh);
    D3D12MeshObject* GetFromCache(const std::string& strKey);

    // ========== 统计 ==========
    UINT GetMeshCount() const;
    UINT GetImmMeshCount() const;

    // ========== 清理 ==========
    void ClearAll();
    void ClearUnusedMeshes();

private:
    // ========== 内部方法 ==========
    void GenerateHandle();
    void Cleanup();

private:
    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

    // ========== 网格映射表 ==========
    std::map<void*, D3D12MeshObject*> m_mStaticMeshMap;
    std::map<void*, D3D12ImmMeshObject*> m_mImmMeshMap;

    // ========== 缓存系统 ==========
    D3D12MeshCache* m_pCache;

    // ========== Handle 生成 ==========
    UINT_PTR m_nNextHandle;

    // ========== 状态标志 ==========
    BOOL m_bInitialized;
};

// ============================================
// 辅助宏
// ============================================

#define SAFE_DELETE_MESH(p) \
    if ((p) != nullptr) { \
        delete (p); \
        (p) = nullptr; \
    }
