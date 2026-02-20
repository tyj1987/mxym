// ============================================
// D3D12TerrainManager.cpp
// DirectX 12 地形管理器实现
// 第 7 阶段 - HeightField 地形系统实现
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12TerrainManager.h"
#include "D3D12HeightField.h"
#include "CoD3D12Device.h"

// ========== 构造函数 ==========

D3D12TerrainManager::D3D12TerrainManager()
    : m_pDevice(nullptr)
    , m_nVisiblePatchCount(0)
    , m_nNextHandle(1)
    , m_bInitialized(FALSE)
{
}

// ========== 析构函数 ==========
D3D12TerrainManager::~D3D12TerrainManager()
{
    Release();
}

// ========== 初始化 ==========
HRESULT D3D12TerrainManager::Initialize(CoD3D12Device* pDevice)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    m_bInitialized = TRUE;

    return S_OK;
}

// ========== 释放 ==========
void D3D12TerrainManager::Release()
{
    // 释放所有地形
    for (auto& pair : m_mTerrainMap)
    {
        if (pair.second)
        {
            pair.second->Release();
        }
    }
    m_mTerrainMap.clear();

    m_bInitialized = FALSE;
}

// ========== 创建地形 ==========
D3D12HeightField* D3D12TerrainManager::CreateTerrain()
{
    if (!m_bInitialized)
        return nullptr;

    D3D12HeightField* pTerrain = new D3D12HeightField();
    if (pTerrain->Initialize(m_pDevice) == S_OK)
    {
        return pTerrain;
    }

    delete pTerrain;
    return nullptr;
}

// ========== 销毁地形 ==========
void D3D12TerrainManager::DestroyTerrain(D3D12HeightField* pTerrain)
{
    if (!pTerrain)
        return;

    pTerrain->Release();
}

// ========== 添加地形 ==========
void* D3D12TerrainManager::AddTerrain(D3D12HeightField* pTerrain)
{
    if (!pTerrain || !m_bInitialized)
        return nullptr;

    // 生成句柄
    void* pHandle = reinterpret_cast<void*>(m_nNextHandle++);

    // 添加到映射表
    m_mTerrainMap[pHandle] = pTerrain;

    // 增加引用计数
    pTerrain->AddRef();

    return pHandle;
}

// ========== 移除地形 ==========
void D3D12TerrainManager::RemoveTerrain(void* pHandle)
{
    if (!pHandle || !m_bInitialized)
        return;

    auto it = m_mTerrainMap.find(pHandle);
    if (it != m_mTerrainMap.end())
    {
        it->second->Release();
        m_mTerrainMap.erase(it);
    }
}

// ========== 获取地形 ==========
D3D12HeightField* D3D12TerrainManager::GetTerrain(void* pHandle)
{
    if (!pHandle || !m_bInitialized)
        return nullptr;

    auto it = m_mTerrainMap.find(pHandle);
    if (it != m_mTerrainMap.end())
    {
        return it->second;
    }

    return nullptr;
}

// ========== 渲染所有地形 ==========
void D3D12TerrainManager::RenderAll(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_bInitialized)
        return;

    // 渲染所有地形
    for (const auto& pair : m_mTerrainMap)
    {
        if (pair.second)
        {
            pair.second->Render(pCommandList);
        }
    }
}

// ========== 更新地形（LOD 和裁剪）==========
void D3D12TerrainManager::Update(const XMFLOAT3& cameraPos)
{
    if (!m_bInitialized)
        return;

    m_nVisiblePatchCount = 0;

    // 更新所有地形
    for (const auto& pair : m_mTerrainMap)
    {
        if (pair.second)
        {
            pair.second->UpdatePatches(cameraPos, m_lodConfig.fLODDistance[0]);

            // 统计可见分块
            m_nVisiblePatchCount += pair.second->GetPatchCount();
            // TODO: 第 16 周准确统计可见分块数量
        }
    }
}

// ========== 更新裁剪 ==========
void D3D12TerrainManager::UpdateCulling(const XMFLOAT3& cameraPos)
{
    // TODO: 第 16 周实现视锥体裁剪
}

// ========== 设置 LOD 配置 ==========
void D3D12TerrainManager::SetLODConfig(const TerrainLODConfig& config)
{
    m_lodConfig = config;
}

// ========== 获取 LOD 配置 ==========
const TerrainLODConfig& D3D12TerrainManager::GetLODConfig() const
{
    return m_lodConfig;
}

// ========== 获取地形数量 ==========
UINT D3D12TerrainManager::GetTerrainCount() const
{
    return static_cast<UINT>(m_mTerrainMap.size());
}

// ========== 获取总分块数量 ==========
UINT D3D12TerrainManager::GetTotalPatchCount() const
{
    UINT nTotalPatches = 0;
    for (const auto& pair : m_mTerrainMap)
    {
        if (pair.second)
        {
            nTotalPatches += pair.second->GetPatchCount();
        }
    }
    return nTotalPatches;
}

// ========== 获取可见分块数量 ==========
UINT D3D12TerrainManager::GetVisiblePatchCount() const
{
    return m_nVisiblePatchCount;
}
