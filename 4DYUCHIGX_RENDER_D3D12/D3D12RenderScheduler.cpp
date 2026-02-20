// ============================================
// D3D12RenderScheduler.cpp
// DirectX 12 渲染调度器实现
// 统一管理所有渲染阶段
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"
#include "D3D12RenderScheduler.h"
#include "CoD3D12Device.h"
#include "D3D12MeshObject.h"
#include "D3D12HeightField.h"
#include "D3D12SpriteObject.h"
#include "D3D12FontObject.h"
#include "D3D12ShadowManager.h"
#include "D3D12PostProcessManager.h"
#include "D3D12TerrainManager.h"
#include <algorithm>
#include <cmath>

// 辅助宏

#define DX12_THROW_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        throw std::exception("DirectX 12 error in D3D12RenderScheduler"); \
    }

// ========== 构造函数 ==========
D3D12RenderScheduler::D3D12RenderScheduler()
    : m_pDevice(nullptr)
    , m_pShadowManager(nullptr)
    , m_pPostProcessManager(nullptr)
    , m_pTerrainManager(nullptr)
    , m_bShadowEnabled(TRUE)
    , m_bPostProcessEnabled(TRUE)
    , m_bInitialized(FALSE)
    , m_bInFrame(FALSE)
{
}

// ========== 析构函数 ==========
D3D12RenderScheduler::~D3D12RenderScheduler()
{
    Release();
}

// ========== 初始化 ==========
HRESULT D3D12RenderScheduler::Initialize(CoD3D12Device* pDevice)
{
    if (!pDevice)
        return E_INVALIDARG;

    m_pDevice = pDevice;

    // 获取现有管理器（由设备创建）
    m_pShadowManager = pDevice->GetShadowManager();
    m_pPostProcessManager = pDevice->GetPostProcessManager();
    m_pTerrainManager = pDevice->GetTerrainManager();

    m_bInitialized = TRUE;

    return S_OK;
}

// ========== 释放 ==========
void D3D12RenderScheduler::Release()
{
    ClearQueues();

    m_pTerrainManager = nullptr;
    m_pPostProcessManager = nullptr;
    m_pShadowManager = nullptr;
    m_pDevice = nullptr;

    m_bInitialized = FALSE;
}

// ========== 开始帧 ==========
HRESULT D3D12RenderScheduler::BeginFrame(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_bInitialized)
        return E_INVALIDARG;

    m_bInFrame = TRUE;

    // 重置统计
    m_stats.Reset();

    // 清空队列
    ClearQueues();

    return S_OK;
}

// ========== 结束帧 ==========
HRESULT D3D12RenderScheduler::EndFrame(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_bInFrame)
        return E_INVALIDARG;

    m_bInFrame = FALSE;

    return S_OK;
}

// ========== 添加渲染项 ==========
void D3D12RenderScheduler::AddRenderItem(const RenderItem& item, RenderPhase phase)
{
    if (!m_bInFrame || phase >= RenderPhase::Count)
        return;

    m_renderQueues[(UINT)phase].vItems.push_back(item);
    m_renderQueues[(UINT)phase].bSorted = FALSE;
}

// ========== 添加网格对象 ==========
void D3D12RenderScheduler::AddMeshObject(D3D12MeshObject* pMesh, RenderPhase phase)
{
    if (!pMesh || !m_bInFrame)
        return;

    RenderItem item;
    item.type = RenderItemType::Mesh;
    item.pObject = pMesh;
    item.bTransparent = FALSE;
    item.nPriority = 0;

    // 计算距离（如果有包围盒信息）
    // TODO: 从Mesh对象获取中心点位置
    item.fDistanceToCamera = 0.0f;

    AddRenderItem(item, phase);
}

// ========== 添加地形对象 ==========
void D3D12RenderScheduler::AddTerrain(D3D12HeightField* pTerrain, RenderPhase phase)
{
    if (!pTerrain || !m_bInFrame)
        return;

    RenderItem item;
    item.type = RenderItemType::Terrain;
    item.pObject = pTerrain;
    item.bTransparent = FALSE;
    item.nPriority = 0;
    item.fDistanceToCamera = 0.0f;

    AddRenderItem(item, phase);
}

// ========== 添加精灵对象 ==========
void D3D12RenderScheduler::AddSprite(D3D12SpriteObject* pSprite, float fZOrder)
{
    if (!pSprite || !m_bInFrame)
        return;

    RenderItem item;
    item.type = RenderItemType::Sprite;
    item.pObject = pSprite;
    item.bTransparent = TRUE;
    item.fDistanceToCamera = fZOrder;
    item.nPriority = 1;

    AddRenderItem(item, RenderPhase::TransparentGeometry);
}

// ========== 添加字体对象 ==========
void D3D12RenderScheduler::AddFont(D3D12FontObject* pFont, float fZOrder)
{
    if (!pFont || !m_bInFrame)
        return;

    RenderItem item;
    item.type = RenderItemType::Font;
    item.pObject = pFont;
    item.bTransparent = TRUE;
    item.fDistanceToCamera = fZOrder;
    item.nPriority = 2;

    AddRenderItem(item, RenderPhase::TransparentGeometry);
}

// ========== 清空队列 ==========
void D3D12RenderScheduler::ClearQueues()
{
    for (UINT i = 0; i < (UINT)RenderPhase::Count; ++i)
    {
        m_renderQueues[i].Clear();
    }
}

// ========== 排序所有队列 ==========
void D3D12RenderScheduler::SortQueues()
{
    for (UINT i = 0; i < (UINT)RenderPhase::Count; ++i)
    {
        SortQueue((RenderPhase)i);
    }
}

// ========== 排序单个队列 ==========
void D3D12RenderScheduler::SortQueue(RenderPhase phase)
{
    if (phase >= RenderPhase::Count)
        return;

    RenderQueue& queue = m_renderQueues[(UINT)phase];

    if (queue.bSorted)
        return;

    switch (phase)
    {
    case RenderPhase::OpaqueGeometry:
        // 不透明对象按材质排序
        std::sort(queue.vItems.begin(), queue.vItems.end(),
            [](const RenderItem& a, const RenderItem& b) {
                if (a.nPriority != b.nPriority)
                    return a.nPriority < b.nPriority;
                return a.dwSortKey < b.dwSortKey;
            });
        break;

    case RenderPhase::TransparentGeometry:
        // 透明对象按深度排序（从远到近）
        std::sort(queue.vItems.begin(), queue.vItems.end(),
            [](const RenderItem& a, const RenderItem& b) {
                if (a.nPriority != b.nPriority)
                    return a.nPriority < b.nPriority;
                return a.fDistanceToCamera > b.fDistanceToCamera;
            });
        break;

    default:
        // 其他队列按优先级排序
        std::sort(queue.vItems.begin(), queue.vItems.end(),
            [](const RenderItem& a, const RenderItem& b) {
                return a.nPriority < b.nPriority;
            });
        break;
    }

    queue.bSorted = TRUE;
}

// ========== 渲染阴影阶段 ==========
HRESULT D3D12RenderScheduler::RenderShadowPhase(ID3D12GraphicsCommandList* pCommandList, const CameraParams& camera)
{
    if (!pCommandList || !m_bShadowEnabled || !m_pShadowManager)
        return S_OK;

    // TODO: 计算光源方向
    XMFLOAT3 lightDir = XMFLOAT3(0.5f, -1.0f, 0.3f);

    // 渲染阴影贴图
    m_pShadowManager->RenderShadowMap(pCommandList, lightDir, camera.v3Position, camera.matView, camera.matProjection);

    // 统计
    m_stats.nShadowPassDrawCalls++; // TODO: 实际统计

    return S_OK;
}

// ========== 渲染几何阶段 ==========
HRESULT D3D12RenderScheduler::RenderGeometryPhase(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList)
        return E_INVALIDARG;

    RenderQueue& queue = m_renderQueues[(UINT)RenderPhase::OpaqueGeometry];

    // 确保已排序
    if (!queue.bSorted)
        SortQueue(RenderPhase::OpaqueGeometry);

    // 渲染所有不透明几何体
    for (const RenderItem& item : queue.vItems)
    {
        if (item.type == RenderItemType::Mesh)
        {
            D3D12MeshObject* pMesh = static_cast<D3D12MeshObject*>(item.pObject);
            if (pMesh)
            {
                RenderMeshObject(pCommandList, pMesh);
                m_stats.nGeometryPassDrawCalls++;
            }
        }
        else if (item.type == RenderItemType::Terrain)
        {
            D3D12HeightField* pTerrain = static_cast<D3D12HeightField*>(item.pObject);
            if (pTerrain)
            {
                RenderTerrainObject(pCommandList, pTerrain);
                m_stats.nGeometryPassDrawCalls++;
            }
        }
    }

    return S_OK;
}

// ========== 渲染透明阶段 ==========
HRESULT D3D12RenderScheduler::RenderTransparentPhase(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList)
        return E_INVALIDARG;

    RenderQueue& queue = m_renderQueues[(UINT)RenderPhase::TransparentGeometry];

    // 确保已排序
    if (!queue.bSorted)
        SortQueue(RenderPhase::TransparentGeometry);

    // 渲染所有透明对象
    for (const RenderItem& item : queue.vItems)
    {
        if (item.type == RenderItemType::Sprite)
        {
            D3D12SpriteObject* pSprite = static_cast<D3D12SpriteObject*>(item.pObject);
            if (pSprite)
            {
                RenderSpriteObject(pCommandList, pSprite);
                m_stats.nTransparentPassDrawCalls++;
            }
        }
        else if (item.type == RenderItemType::Font)
        {
            D3D12FontObject* pFont = static_cast<D3D12FontObject*>(item.pObject);
            if (pFont)
            {
                RenderFontObject(pCommandList, pFont);
                m_stats.nTransparentPassDrawCalls++;
            }
        }
    }

    return S_OK;
}

// ========== 渲染后期处理阶段 ==========
HRESULT D3D12RenderScheduler::RenderPostProcessPhase(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList || !m_bPostProcessEnabled || !m_pPostProcessManager)
        return S_OK;

    // 执行后期处理
    HRESULT hr = m_pPostProcessManager->Process(pCommandList);

    if (SUCCEEDED(hr))
    {
        m_stats.nPostProcessPassDrawCalls++; // TODO: 实际统计
    }

    return hr;
}

// ========== 渲染UI阶段 ==========
HRESULT D3D12RenderScheduler::RenderUIPhase(ID3D12GraphicsCommandList* pCommandList)
{
    if (!pCommandList)
        return E_INVALIDARG;

    // TODO: 实现UI渲染

    return S_OK;
}

// ========== 渲染完整帧 ==========
HRESULT D3D12RenderScheduler::RenderFrame(ID3D12GraphicsCommandList* pCommandList, const CameraParams& camera)
{
    if (!pCommandList || !m_bInitialized)
        return E_INVALIDARG;

    // 1. 开始帧
    HRESULT hr = BeginFrame(pCommandList);
    if (FAILED(hr))
        return hr;

    // 2. 排序队列
    SortQueues();

    // 3. 渲染阴影阶段
    hr = RenderShadowPhase(pCommandList, camera);
    if (FAILED(hr))
        return hr;

    // 4. 渲染几何阶段
    hr = RenderGeometryPhase(pCommandList);
    if (FAILED(hr))
        return hr;

    // 5. 渲染透明阶段
    hr = RenderTransparentPhase(pCommandList);
    if (FAILED(hr))
        return hr;

    // 6. 渲染后期处理阶段
    hr = RenderPostProcessPhase(pCommandList);
    if (FAILED(hr))
        return hr;

    // 7. 渲染UI阶段
    hr = RenderUIPhase(pCommandList);
    if (FAILED(hr))
        return hr;

    // 8. 结束帧
    hr = EndFrame(pCommandList);
    if (FAILED(hr))
        return hr;

    // 9. 更新总统计
    m_stats.nTotalDrawCalls = m_stats.nShadowPassDrawCalls +
                              m_stats.nGeometryPassDrawCalls +
                              m_stats.nTransparentPassDrawCalls +
                              m_stats.nPostProcessPassDrawCalls +
                              m_stats.nUIPassDrawCalls;

    return S_OK;
}

// ========== 设置相机 ==========
void D3D12RenderScheduler::SetCamera(const CameraParams& camera)
{
    m_camera = camera;
}

// ========== 获取相机 ==========
const CameraParams& D3D12RenderScheduler::GetCamera() const
{
    return m_camera;
}

// ========== 获取队列大小 ==========
UINT D3D12RenderScheduler::GetQueueSize(RenderPhase phase) const
{
    if (phase >= RenderPhase::Count)
        return 0;

    return m_renderQueues[(UINT)phase].GetSize();
}

// ========== 获取统计 ==========
const RenderStats& D3D12RenderScheduler::GetStats() const
{
    return m_stats;
}

// ========== 重置统计 ==========
void D3D12RenderScheduler::ResetStats()
{
    m_stats.Reset();
}

// ========== 设置阴影启用 ==========
void D3D12RenderScheduler::SetShadowEnabled(BOOL bEnabled)
{
    m_bShadowEnabled = bEnabled;
}

// ========== 设置后期处理启用 ==========
void D3D12RenderScheduler::SetPostProcessEnabled(BOOL bEnabled)
{
    m_bPostProcessEnabled = bEnabled;
}

// ========== 是否启用阴影 ==========
BOOL D3D12RenderScheduler::IsShadowEnabled() const
{
    return m_bShadowEnabled;
}

// ========== 是否启用后期处理 ==========
BOOL D3D12RenderScheduler::IsPostProcessEnabled() const
{
    return m_bPostProcessEnabled;
}

// ========== 获取阴影管理器 ==========
D3D12ShadowManager* D3D12RenderScheduler::GetShadowManager() const
{
    return m_pShadowManager;
}

// ========== 获取后期处理管理器 ==========
D3D12PostProcessManager* D3D12RenderScheduler::GetPostProcessManager() const
{
    return m_pPostProcessManager;
}

// ========== 获取地形管理器 ==========
D3D12TerrainManager* D3D12RenderScheduler::GetTerrainManager() const
{
    return m_pTerrainManager;
}

// ========== 渲染网格对象 ==========
void D3D12RenderScheduler::RenderMeshObject(ID3D12GraphicsCommandList* pCommandList, D3D12MeshObject* pMesh)
{
    if (!pCommandList || !pMesh)
        return;

    // 渲染网格
    pMesh->Render(pCommandList);
}

// ========== 渲染地形对象 ==========
void D3D12RenderScheduler::RenderTerrainObject(ID3D12GraphicsCommandList* pCommandList, D3D12HeightField* pTerrain)
{
    if (!pCommandList || !pTerrain)
        return;

    // 渲染地形
    pTerrain->Render(pCommandList);
}

// ========== 渲染精灵对象 ==========
void D3D12RenderScheduler::RenderSpriteObject(ID3D12GraphicsCommandList* pCommandList, D3D12SpriteObject* pSprite)
{
    if (!pCommandList || !pSprite)
        return;

    // TODO: 实现精灵渲染
    // pSprite->Render(pCommandList);
}

// ========== 渲染字体对象 ==========
void D3D12RenderScheduler::RenderFontObject(ID3D12GraphicsCommandList* pCommandList, D3D12FontObject* pFont)
{
    if (!pCommandList || !pFont)
        return;

    // TODO: 实现字体渲染
    // pFont->Render(pCommandList);
}

// ========== 计算到相机的距离 ==========
float D3D12RenderScheduler::CalculateDistanceToCamera(const XMFLOAT3& objectPos) const
{
    XMFLOAT3 diff;
    diff.x = objectPos.x - m_camera.v3Position.x;
    diff.y = objectPos.y - m_camera.v3Position.y;
    diff.z = objectPos.z - m_camera.v3Position.z;

    return std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

// ========== 设置资源屏障 ==========
void D3D12RenderScheduler::SetResourceBarrier(ID3D12GraphicsCommandList* pCommandList,
                                             ID3D12Resource* pResource,
                                             D3D12_RESOURCE_STATES before,
                                             D3D12_RESOURCE_STATES after)
{
    if (!pCommandList || !pResource)
        return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = pResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;

    pCommandList->ResourceBarrier(1, &barrier);
}

// ========== 按材质排序（不透明对象）==========
void D3D12RenderScheduler::SortOpaqueByMaterial()
{
    SortQueue(RenderPhase::OpaqueGeometry);
}

// ========== 按深度排序（透明对象）==========
void D3D12RenderScheduler::SortTransparentByDepth()
{
    SortQueue(RenderPhase::TransparentGeometry);
}
