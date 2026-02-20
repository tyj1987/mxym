// ============================================
// D3D12MeshCache.cpp
// DirectX 12 网格缓存系统实现
// 第 5 阶段 - MeshObject 系统实现
// ============================================

#include "D3D12MeshCache.h"
#include "D3D12MeshObject.h"
#include "CoD3D12Device.h"
#include <algorithm>
#include <iostream>

// ========== 构造函数 ==========
#include "D3D12PCH.h"

D3D12MeshCache::D3D12MeshCache()
    : m_pDevice(nullptr)
    , m_nMaxCacheSize(DEFAULT_MESH_CACHE_SIZE)
    , m_nCurrentMemoryUsage(0)
    , m_nCurrentFrame(0)
    , m_bInitialized(FALSE)
{
}

// ========== 析构函数 ==========
D3D12MeshCache::~D3D12MeshCache()
{
    Release();
}

// ========== 初始化 ==========
HRESULT D3D12MeshCache::Initialize(CoD3D12Device* pDevice, UINT nMaxCacheSize)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;
    m_nMaxCacheSize = nMaxCacheSize;

    // 限制缓存大小在合理范围内
    if (m_nMaxCacheSize < MIN_MESH_CACHE_SIZE)
        m_nMaxCacheSize = MIN_MESH_CACHE_SIZE;
    if (m_nMaxCacheSize > MAX_MESH_CACHE_SIZE)
        m_nMaxCacheSize = MAX_MESH_CACHE_SIZE;

    // 重置统计信息
    m_stats = MeshCacheStats();
    m_nCurrentMemoryUsage = 0;
    m_nCurrentFrame = 0;

    m_bInitialized = TRUE;

    return S_OK;
}

// ========== 释放 ==========
void D3D12MeshCache::Release()
{
    // 清空缓存
    ClearCache();

    m_bInitialized = FALSE;
}

// ========== 添加到缓存（内部）==========
HRESULT D3D12MeshCache::AddToCacheInternal(const std::string& strKey, D3D12MeshObject* pMesh)
{
    if (!pMesh || !m_bInitialized)
        return E_INVALIDARG;

    // 检查是否已存在
    if (m_mCache.find(strKey) != m_mCache.end())
    {
        return S_FALSE; // 已存在
    }

    // 计算内存使用
    UINT nMeshMemory = CalculateMeshMemory(pMesh);

    // 检查是否超过缓存限制
    if (m_nCurrentMemoryUsage + nMeshMemory > m_nMaxCacheSize)
    {
        // 尝试淘汰旧网格
        EvictLRU();

        // 再次检查
        if (m_nCurrentMemoryUsage + nMeshMemory > m_nMaxCacheSize)
        {
            // 仍然超过限制，拒绝添加
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    // 创建缓存条目
    MeshCacheEntry entry;
    entry.strKey = strKey;
    entry.pMesh = pMesh;
    entry.nLastUsedFrame = m_nCurrentFrame;
    entry.nAccessCount = 1;
    entry.nMemoryUsage = nMeshMemory;

    // 添加到缓存
    m_mCache[strKey] = entry;

    // 添加到 LRU 队列
    m_vLRUQueue.push_back(strKey);

    // 更新内存使用
    m_nCurrentMemoryUsage += nMeshMemory;

    // 更新统计
    UpdateStats();

    return S_OK;
}

// ========== 添加到缓存 ==========
HRESULT D3D12MeshCache::AddToCache(const std::string& strKey, D3D12MeshObject* pMesh)
{
    if (!strKey.empty())
    {
        return AddToCacheInternal(strKey, pMesh);
    }

    return E_INVALIDARG;
}

// ========== 从缓存获取 ==========
D3D12MeshObject* D3D12MeshCache::GetFromCache(const std::string& strKey)
{
    if (!m_bInitialized)
        return nullptr;

    auto it = m_mCache.find(strKey);
    if (it != m_mCache.end())
    {
        // 缓存命中
        m_stats.nCacheHits++;

        // 更新访问信息
        it->second.nAccessCount++;
        it->second.nLastUsedFrame = m_nCurrentFrame;

        // 更新 LRU 队列
        UpdateAccess(strKey, m_nCurrentFrame);

        return it->second.pMesh;
    }

    // 缓存未命中
    m_stats.nCacheMisses++;

    return nullptr;
}

// ========== 从缓存移除 ==========
void D3D12MeshCache::RemoveFromCache(const std::string& strKey)
{
    if (!m_bInitialized)
        return;

    auto it = m_mCache.find(strKey);
    if (it != m_mCache.end())
    {
        // 更新内存使用
        m_nCurrentMemoryUsage -= it->second.nMemoryUsage;

        // 从 LRU 队列中移除
        auto lruIt = std::find(m_vLRUQueue.begin(), m_vLRUQueue.end(), strKey);
        if (lruIt != m_vLRUQueue.end())
        {
            m_vLRUQueue.erase(lruIt);
        }

        // 释放网格（不删除对象，只移除引用）
        // if (it->second.pMesh)
        // {
        //     it->second.pMesh->Release();
        // }

        // 从缓存中移除
        m_mCache.erase(it);

        // 更新统计
        UpdateStats();
    }
}

// ========== 清空缓存 ==========
void D3D12MeshCache::ClearCache()
{
    if (!m_bInitialized)
        return;

    // 释放所有网格
    for (auto& pair : m_mCache)
    {
        // 注意：这里不删除网格对象，因为它们可能被外部引用
        // pair.second.pMesh->Release();
    }

    // 清空映射表和队列
    m_mCache.clear();
    m_vLRUQueue.clear();

    // 重置内存使用
    m_nCurrentMemoryUsage = 0;

    // 重置统计
    m_stats = MeshCacheStats();
}

// ========== 更新访问（LRU）==========
void D3D12MeshCache::UpdateAccess(const std::string& strKey, UINT nCurrentFrame)
{
    // 从队列中移除
    auto it = std::find(m_vLRUQueue.begin(), m_vLRUQueue.end(), strKey);
    if (it != m_vLRUQueue.end())
    {
        m_vLRUQueue.erase(it);
    }

    // 添加到队尾（最近使用）
    m_vLRUQueue.push_back(strKey);
}

// ========== 淘汰最少使用的网格 ==========
void D3D12MeshCache::EvictLRU()
{
    if (m_vLRUQueue.empty())
        return;

    // 获取最久未使用的网格（队首）
    std::string strKey = m_vLRUQueue.front();

    // 从缓存中移除
    RemoveFromCache(strKey);
}

// ========== 设置最大缓存大小 ==========
void D3D12MeshCache::SetMaxCacheSize(UINT nMaxSize)
{
    m_nMaxCacheSize = nMaxSize;

    // 限制在合理范围内
    if (m_nMaxCacheSize < MIN_MESH_CACHE_SIZE)
        m_nMaxCacheSize = MIN_MESH_CACHE_SIZE;
    if (m_nMaxCacheSize > MAX_MESH_CACHE_SIZE)
        m_nMaxCacheSize = MAX_MESH_CACHE_SIZE;

    // 如果当前使用超过新限制，淘汰网格
    while (m_nCurrentMemoryUsage > m_nMaxCacheSize && !m_vLRUQueue.empty())
    {
        EvictLRU();
    }
}

// ========== 获取最大缓存大小 ==========
UINT D3D12MeshCache::GetMaxCacheSize() const
{
    return m_nMaxCacheSize;
}

// ========== 获取统计信息 ==========
const MeshCacheStats& D3D12MeshCache::GetStats() const
{
    return m_stats;
}

// ========== 重置统计 ==========
void D3D12MeshCache::ResetStats()
{
    m_stats.nCacheHits = 0;
    m_stats.nCacheMisses = 0;
}

// ========== 获取缓存大小 ==========
UINT D3D12MeshCache::GetCacheSize() const
{
    return static_cast<UINT>(m_mCache.size());
}

// ========== 获取缓存内存使用 ==========
UINT D3D12MeshCache::GetCacheMemoryUsage() const
{
    return m_nCurrentMemoryUsage;
}

// ========== 获取缓存命中率 ==========
float D3D12MeshCache::GetCacheHitRate() const
{
    UINT nTotalAccesses = m_stats.nCacheHits + m_stats.nCacheMisses;
    if (nTotalAccesses == 0)
        return 0.0f;

    return static_cast<float>(m_stats.nCacheHits) / static_cast<float>(nTotalAccesses);
}

// ========== 打印统计 ==========
void D3D12MeshCache::PrintStats() const
{
    std::cout << "========== Mesh Cache Statistics ==========" << std::endl;
    std::cout << "Total Meshes: " << m_stats.nTotalMeshes << std::endl;
    std::cout << "Total Vertices: " << m_stats.nTotalVertices << std::endl;
    std::cout << "Total Indices: " << m_stats.nTotalIndices << std::endl;
    std::cout << "Memory Usage: " << (m_stats.nMemoryUsage / 1024 / 1024) << " MB" << std::endl;
    std::cout << "Cache Hits: " << m_stats.nCacheHits << std::endl;
    std::cout << "Cache Misses: " << m_stats.nCacheMisses << std::endl;
    std::cout << "Hit Rate: " << (GetCacheHitRate() * 100.0f) << "%" << std::endl;
    std::cout << "===========================================" << std::endl;
}

// ========== 获取所有条目 ==========
std::vector<MeshCacheEntry> D3D12MeshCache::GetAllEntries() const
{
    std::vector<MeshCacheEntry> vEntries;
    vEntries.reserve(m_mCache.size());

    for (const auto& pair : m_mCache)
    {
        vEntries.push_back(pair.second);
    }

    return vEntries;
}

// ========== 预加载网格 ==========
HRESULT D3D12MeshCache::PreloadMeshes(const std::vector<std::string>& vKeys)
{
    // TODO: 第 12 周实现
    // 这个方法需要实际的网格加载功能
    // 当前只做预留

    return S_OK;
}

// ========== 更新统计信息 ==========
void D3D12MeshCache::UpdateStats()
{
    m_stats.nTotalMeshes = static_cast<UINT>(m_mCache.size());
    m_stats.nMemoryUsage = m_nCurrentMemoryUsage;

    // 计算总顶点和索引数
    m_stats.nTotalVertices = 0;
    m_stats.nTotalIndices = 0;

    for (const auto& pair : m_mCache)
    {
        if (pair.second.pMesh)
        {
            m_stats.nTotalVertices += pair.second.pMesh->GetVertexCount();
            m_stats.nTotalIndices += pair.second.pMesh->GetIndexCount();
        }
    }
}

// ========== 计算网格内存 ==========
UINT D3D12MeshCache::CalculateMeshMemory(D3D12MeshObject* pMesh) const
{
    if (!pMesh)
        return 0;

    // 计算顶点缓冲区大小
    UINT nVertexMemory = pMesh->GetVertexCount() * pMesh->GetVertexStride();

    // 计算索引缓冲区大小
    UINT nIndexMemory = pMesh->GetIndexCount() * sizeof(DWORD);

    // 加上一些额外的开销
    UINT nOverhead = 1024; // 1KB 开销

    return nVertexMemory + nIndexMemory + nOverhead;
}
