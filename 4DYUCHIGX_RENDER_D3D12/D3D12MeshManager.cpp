// ============================================
// D3D12MeshManager.cpp
// DirectX 12 网格管理器实现
// 第 5 阶段 - MeshObject 系统实现
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12MeshManager.h"
#include "D3D12MeshObject.h"
#include "D3D12ImmMeshObject.h"
#include "D3D12MeshCache.h"
#include "CoD3D12Device.h"

// ========== 构造函数 ==========

D3D12MeshManager::D3D12MeshManager()
    : m_pDevice(nullptr)
    , m_pCache(nullptr)
    , m_nNextHandle(1)
    , m_bInitialized(FALSE)
{
}

// ========== 析构函数 ==========
D3D12MeshManager::~D3D12MeshManager()
{
    Release();
}

// ========== 初始化 ==========
HRESULT D3D12MeshManager::Initialize(CoD3D12Device* pDevice)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    // 创建缓存系统
    m_pCache = new D3D12MeshCache();
    HRESULT hr = m_pCache->Initialize(pDevice);
    if (FAILED(hr))
    {
        SAFE_DELETE_MESH(m_pCache);
        return hr;
    }

    m_bInitialized = TRUE;

    return S_OK;
}

// ========== 释放 ==========
void D3D12MeshManager::Release()
{
    // 清空所有网格
    ClearAll();

    // 释放缓存
    if (m_pCache)
    {
        m_pCache->Release();
        SAFE_DELETE_MESH(m_pCache);
    }

    m_bInitialized = FALSE;
}

// ========== 创建静态网格 ==========
D3D12MeshObject* D3D12MeshManager::CreateMeshObject()
{
    if (!m_bInitialized)
        return nullptr;

    D3D12MeshObject* pMesh = new D3D12MeshObject();
    if (pMesh->Initialize(m_pDevice) == S_OK)
    {
        return pMesh;
    }

    SAFE_DELETE_MESH(pMesh);
    return nullptr;
}

// ========== 创建立即网格 ==========
D3D12ImmMeshObject* D3D12MeshManager::CreateImmMeshObject(UINT maxVertices, UINT maxIndices)
{
    if (!m_bInitialized)
        return nullptr;

    D3D12ImmMeshObject* pImmMesh = new D3D12ImmMeshObject();
    if (pImmMesh->Initialize(m_pDevice, maxVertices, maxIndices) == S_OK)
    {
        return pImmMesh;
    }

    SAFE_DELETE_MESH(pImmMesh);
    return nullptr;
}

// ========== 添加静态网格 ==========
void* D3D12MeshManager::AddMesh(D3D12MeshObject* pMesh)
{
    if (!pMesh || !m_bInitialized)
        return nullptr;

    // 生成句柄
    void* pHandle = reinterpret_cast<void*>(m_nNextHandle++);

    // 添加到映射表
    m_mStaticMeshMap[pHandle] = pMesh;

    // 增加引用计数
    pMesh->AddRef();

    return pHandle;
}

// ========== 添加立即网格 ==========
void* D3D12MeshManager::AddMesh(D3D12ImmMeshObject* pMesh)
{
    if (!pMesh || !m_bInitialized)
        return nullptr;

    // 生成句柄
    void* pHandle = reinterpret_cast<void*>(m_nNextHandle++);

    // 添加到映射表
    m_mImmMeshMap[pHandle] = pMesh;

    // 增加引用计数
    pMesh->AddRef();

    return pHandle;
}

// ========== 移除网格 ==========
void D3D12MeshManager::RemoveMesh(void* pHandle)
{
    if (!pHandle || !m_bInitialized)
        return;

    // 查找静态网格
    auto it = m_mStaticMeshMap.find(pHandle);
    if (it != m_mStaticMeshMap.end())
    {
        it->second->Release();
        m_mStaticMeshMap.erase(it);
        return;
    }

    // 查找立即网格
    auto itImm = m_mImmMeshMap.find(pHandle);
    if (itImm != m_mImmMeshMap.end())
    {
        itImm->second->Release();
        m_mImmMeshMap.erase(itImm);
        return;
    }
}

// ========== 获取网格（通用）==========
D3D12MeshObject* D3D12MeshManager::GetMesh(void* pHandle)
{
    if (!pHandle || !m_bInitialized)
        return nullptr;

    // 首先查找静态网格
    auto it = m_mStaticMeshMap.find(pHandle);
    if (it != m_mStaticMeshMap.end())
    {
        return it->second;
    }

    // 没有找到，返回 nullptr（立即网格需要用 GetImmMesh 获取）
    return nullptr;
}

// ========== 获取静态网格 ==========
D3D12MeshObject* D3D12MeshManager::GetStaticMesh(void* pHandle)
{
    if (!pHandle || !m_bInitialized)
        return nullptr;

    auto it = m_mStaticMeshMap.find(pHandle);
    if (it != m_mStaticMeshMap.end())
    {
        return it->second;
    }

    return nullptr;
}

// ========== 获取立即网格 ==========
D3D12ImmMeshObject* D3D12MeshManager::GetImmMesh(void* pHandle)
{
    if (!pHandle || !m_bInitialized)
        return nullptr;

    auto it = m_mImmMeshMap.find(pHandle);
    if (it != m_mImmMeshMap.end())
    {
        return it->second;
    }

    return nullptr;
}

// ========== 渲染所有网格 ==========
void D3D12MeshManager::RenderAllMeshes(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_bInitialized)
        return;

    // 渲染所有静态网格
    for (const auto& pair : m_mStaticMeshMap)
    {
        if (pair.second)
        {
            pair.second->Render(pCommandList);
        }
    }

    // TODO: 第 11 周优化 - 按材质分组渲染
}

// ========== 按材质渲染网格 ==========
void D3D12MeshManager::RenderMeshesByMaterial(ID3D12GraphicsCommandList* pCommandList, void* pMaterialHandle)
{
    if (!pCommandList || !m_bInitialized)
        return;

    // 渲染使用指定材质的静态网格
    for (const auto& pair : m_mStaticMeshMap)
    {
        if (pair.second)
        {
            // TODO: 第 11 周检查材质
            // if (pair.second->GetMaterial() == pMaterialHandle)
            {
                pair.second->Render(pCommandList);
            }
        }
    }
}

// ========== 获取缓存 ==========
D3D12MeshCache* D3D12MeshManager::GetCache()
{
    return m_pCache;
}

// ========== 添加到缓存 ==========
HRESULT D3D12MeshManager::AddToCache(const std::string& strKey, D3D12MeshObject* pMesh)
{
    if (!m_pCache || !m_bInitialized)
        return E_FAIL;

    return m_pCache->AddToCache(strKey, pMesh);
}

// ========== 从缓存获取 ==========
D3D12MeshObject* D3D12MeshManager::GetFromCache(const std::string& strKey)
{
    if (!m_pCache || !m_bInitialized)
        return nullptr;

    return m_pCache->GetFromCache(strKey);
}

// ========== 获取静态网格数量 ==========
UINT D3D12MeshManager::GetMeshCount() const
{
    return static_cast<UINT>(m_mStaticMeshMap.size());
}

// ========== 获取立即网格数量 ==========
UINT D3D12MeshManager::GetImmMeshCount() const
{
    return static_cast<UINT>(m_mImmMeshMap.size());
}

// ========== 清空所有网格 ==========
void D3D12MeshManager::ClearAll()
{
    // 释放所有静态网格
    for (auto& pair : m_mStaticMeshMap)
    {
        if (pair.second)
        {
            pair.second->Release();
        }
    }
    m_mStaticMeshMap.clear();

    // 释放所有立即网格
    for (auto& pair : m_mImmMeshMap)
    {
        if (pair.second)
        {
            pair.second->Release();
        }
    }
    m_mImmMeshMap.clear();

    // 清空缓存
    if (m_pCache)
    {
        m_pCache->ClearCache();
    }
}

// ========== 清空未使用的网格 ==========
void D3D12MeshManager::ClearUnusedMeshes()
{
    // TODO: 第 12 周实现
    // 需要跟踪每个网格的引用计数
    // 释放引用计数为 1（只有管理器引用）的网格
}

// ========== 生成句柄 ==========
void D3D12MeshManager::GenerateHandle()
{
    m_nNextHandle++;
}

// ========== 清理 ==========
void D3D12MeshManager::Cleanup()
{
    // TODO: 第 12 周实现
    // 定期清理未使用的资源
}
