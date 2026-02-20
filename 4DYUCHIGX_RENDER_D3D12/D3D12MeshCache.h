// ============================================
// D3D12MeshCache.h
// DirectX 12 网格缓存系统
// 第 5 阶段 - MeshObject 系统实现
// ============================================

#pragma once

#include "D3D12PCH.h"
#include <string>
#include <map>
#include <vector>
// 前向声明
class D3D12MeshObject;
class CoD3D12Device;

// ========== 缓存统计信息 ==========
struct MeshCacheStats
{
    UINT nTotalMeshes;          // 缓存中的网格总数
    UINT nTotalVertices;        // 总顶点数
    UINT nTotalIndices;         // 总索引数
    UINT nMemoryUsage;          // 内存使用量（字节）
    UINT nCacheHits;            // 缓存命中次数
    UINT nCacheMisses;          // 缓存未命中次数

    MeshCacheStats()
        : nTotalMeshes(0)
        , nTotalVertices(0)
        , nTotalIndices(0)
        , nMemoryUsage(0)
        , nCacheHits(0)
        , nCacheMisses(0)
    {}
};

// ========== 缓存条目信息 ==========
struct MeshCacheEntry
{
    std::string strKey;                 // 缓存键
    D3D12MeshObject* pMesh;             // 网格对象
    UINT nLastUsedFrame;                // 最后使用的帧
    UINT nAccessCount;                  // 访问次数
    UINT nMemoryUsage;                  // 内存使用量

    MeshCacheEntry()
        : pMesh(nullptr)
        , nLastUsedFrame(0)
        , nAccessCount(0)
        , nMemoryUsage(0)
    {}
};

// ============================================
// D3D12MeshCache 类
// 网格缓存系统，用于优化网格加载和渲染
// ============================================

class D3D12MeshCache
{
public:
    D3D12MeshCache();
    ~D3D12MeshCache();

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice, UINT nMaxCacheSize = 512 * 1024 * 1024); // 默认 512MB
    void Release();

    // ========== 缓存管理 ==========
    HRESULT AddToCache(const std::string& strKey, D3D12MeshObject* pMesh);
    D3D12MeshObject* GetFromCache(const std::string& strKey);
    void RemoveFromCache(const std::string& strKey);
    void ClearCache();

    // ========== LRU 管理 ==========
    void UpdateAccess(const std::string& strKey, UINT nCurrentFrame);
    void EvictLRU(); // 淘汰最少使用的网格
    void SetMaxCacheSize(UINT nMaxSize);
    UINT GetMaxCacheSize() const;

    // ========== 统计 ==========
    const MeshCacheStats& GetStats() const;
    void ResetStats();
    UINT GetCacheSize() const;
    UINT GetCacheMemoryUsage() const;
    float GetCacheHitRate() const;

    // ========== 调试 ==========
    void PrintStats() const;
    std::vector<MeshCacheEntry> GetAllEntries() const;

    // ========== 预加载 ==========
    HRESULT PreloadMeshes(const std::vector<std::string>& vKeys);

private:
    // ========== 内部方法 ==========
    HRESULT AddToCacheInternal(const std::string& strKey, D3D12MeshObject* pMesh);
    void UpdateStats();
    UINT CalculateMeshMemory(D3D12MeshObject* pMesh) const;

private:
    // ========== 缓存映射表 ==========
    std::map<std::string, MeshCacheEntry> m_mCache;

    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

    // ========== 缓存配置 ==========
    UINT m_nMaxCacheSize;        // 最大缓存大小（字节）
    UINT m_nCurrentMemoryUsage;  // 当前内存使用量
    UINT m_nCurrentFrame;        // 当前帧数

    // ========== 统计信息 ==========
    MeshCacheStats m_stats;

    // ========== LRU 队列 ==========
    std::vector<std::string> m_vLRUQueue; // 最近使用队列（队首 = 最久未使用）

    // ========== 状态标志 ==========
    BOOL m_bInitialized;
};

// ============================================
// 辅助宏
// ============================================

#define DEFAULT_MESH_CACHE_SIZE (512 * 1024 * 1024)  // 512 MB
#define MIN_MESH_CACHE_SIZE (16 * 1024 * 1024)        // 16 MB
#define MAX_MESH_CACHE_SIZE (4 * 1024 * 1024 * 1024)  // 4 GB
