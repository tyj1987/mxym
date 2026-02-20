// ============================================
// D3D12RenderScheduler.h
// DirectX 12 渲染调度器
// 统一管理所有渲染阶段
// ============================================

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>
#include <map>
#include <queue>
#include <functional>


using namespace DirectX;

// 前向声明
class CoD3D12Device;
class D3D12MeshObject;
class D3D12HeightField;
class D3D12SpriteObject;
class D3D12FontObject;
class D3D12ShadowManager;
class D3D12PostProcessManager;
class D3D12TerrainManager;

// ========== 渲染阶段枚举 ==========
enum class RenderPhase
{
    Shadow = 0,                 // 阴影贴图渲染
    OpaqueGeometry,             // 不透明几何体
    TransparentGeometry,        // 透明几何体
    PostProcess,                // 后期处理
    UI,                         // UI渲染
    Count
};

// ========== 渲染项类型 ==========
enum class RenderItemType
{
    Mesh,                       // 网格对象
    Terrain,                    // 地形对象
    Sprite,                     // 精灵对象
    Font                        // 字体对象
};

// ========== 渲染项 ==========
struct RenderItem
{
    RenderItemType type;         // 渲染对象类型
    void* pObject;               // 渲染对象指针
    float fDistanceToCamera;     // 到相机的距离（用于排序）
    DWORD dwSortKey;             // 材质/状态排序键
    BOOL bTransparent;           // 是否透明
    UINT nPriority;              // 优先级（0=最高）

    RenderItem()
        : type(RenderItemType::Mesh)
        , pObject(nullptr)
        , fDistanceToCamera(0.0f)
        , dwSortKey(0)
        , bTransparent(FALSE)
        , nPriority(0)
    {}
};

// ========== 渲染队列 ==========
struct RenderQueue
{
    std::vector<RenderItem> vItems;
    BOOL bSorted;

    RenderQueue() : bSorted(FALSE) {}

    void Clear()
    {
        vItems.clear();
        bSorted = FALSE;
    }

    UINT GetSize() const
    {
        return static_cast<UINT>(vItems.size());
    }
};

// ========== 渲染统计 ==========
struct RenderStats
{
    UINT nShadowPassDrawCalls;       // 阴影通道绘制调用
    UINT nGeometryPassDrawCalls;     // 几何通道绘制调用
    UINT nTransparentPassDrawCalls;  // 透明通道绘制调用
    UINT nPostProcessPassDrawCalls;  // 后期处理通道绘制调用
    UINT nUIPassDrawCalls;           // UI通道绘制调用
    UINT nTotalDrawCalls;            // 总绘制调用
    float fShadowPassTime;           // 阴影通道耗时(ms)
    float fGeometryPassTime;         // 几何通道耗时(ms)
    float fTransparentPassTime;      // 透明通道耗时(ms)
    float fPostProcessPassTime;      // 后期处理耗时(ms)
    float fTotalFrameTime;           // 总帧时间(ms)

    RenderStats()
        : nShadowPassDrawCalls(0)
        , nGeometryPassDrawCalls(0)
        , nTransparentPassDrawCalls(0)
        , nPostProcessPassDrawCalls(0)
        , nUIPassDrawCalls(0)
        , nTotalDrawCalls(0)
        , fShadowPassTime(0.0f)
        , fGeometryPassTime(0.0f)
        , fTransparentPassTime(0.0f)
        , fPostProcessPassTime(0.0f)
        , fTotalFrameTime(0.0f)
    {}

    void Reset()
    {
        nShadowPassDrawCalls = 0;
        nGeometryPassDrawCalls = 0;
        nTransparentPassDrawCalls = 0;
        nPostProcessPassDrawCalls = 0;
        nUIPassDrawCalls = 0;
        nTotalDrawCalls = 0;
        fShadowPassTime = 0.0f;
        fGeometryPassTime = 0.0f;
        fTransparentPassTime = 0.0f;
        fPostProcessPassTime = 0.0f;
        fTotalFrameTime = 0.0f;
    }
};

// ========== 相机参数 ==========
struct CameraParams
{
    XMFLOAT3 v3Position;            // 相机位置
    XMFLOAT3 v3LookAt;              // 相机朝向
    XMFLOAT3 v3Up;                  // 相机上方向
    XMFLOAT4X4 matView;             // 视图矩阵
    XMFLOAT4X4 matProjection;       // 投影矩阵
    float fNear;                    // 近平面距离
    float fFar;                     // 远平面距离
    float fFOV;                     // 视野角度
    float fAspectRatio;            // 宽高比

    CameraParams()
        : v3Position(0, 0, 0)
        , v3LookAt(0, 0, 1)
        , v3Up(0, 1, 0)
        , fNear(0.1f)
        , fFar(1000.0f)
        , fFOV(XM_PI / 4.0f)
        , fAspectRatio(16.0f / 9.0f)
    {
        matView = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        matProjection = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    }
};

// ============================================
// D3D12RenderScheduler 类
// 统一管理所有渲染阶段的调度器
// ============================================

class D3D12RenderScheduler
{
public:
    D3D12RenderScheduler();
    ~D3D12RenderScheduler();

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice);
    void Release();

    // ========== 帧控制 ==========
    HRESULT BeginFrame(ID3D12GraphicsCommandList* pCommandList);
    HRESULT EndFrame(ID3D12GraphicsCommandList* pCommandList);

    // ========== 渲染队列管理 ==========
    void AddRenderItem(const RenderItem& item, RenderPhase phase);
    void AddMeshObject(D3D12MeshObject* pMesh, RenderPhase phase = RenderPhase::OpaqueGeometry);
    void AddTerrain(D3D12HeightField* pTerrain, RenderPhase phase = RenderPhase::OpaqueGeometry);
    void AddSprite(D3D12SpriteObject* pSprite, float fZOrder);
    void AddFont(D3D12FontObject* pFont, float fZOrder);

    void ClearQueues();
    void SortQueues();
    void SortQueue(RenderPhase phase);

    // ========== 渲染阶段执行 ==========
    HRESULT RenderShadowPhase(ID3D12GraphicsCommandList* pCommandList, const CameraParams& camera);
    HRESULT RenderGeometryPhase(ID3D12GraphicsCommandList* pCommandList);
    HRESULT RenderTransparentPhase(ID3D12GraphicsCommandList* pCommandList);
    HRESULT RenderPostProcessPhase(ID3D12GraphicsCommandList* pCommandList);
    HRESULT RenderUIPhase(ID3D12GraphicsCommandList* pCommandList);

    // ========== 完整渲染流程 ==========
    HRESULT RenderFrame(ID3D12GraphicsCommandList* pCommandList, const CameraParams& camera);

    // ========== 相机管理 ==========
    void SetCamera(const CameraParams& camera);
    const CameraParams& GetCamera() const;

    // ========== 统计 ==========
    UINT GetQueueSize(RenderPhase phase) const;
    const RenderStats& GetStats() const;
    void ResetStats();

    // ========== 配置 ==========
    void SetShadowEnabled(BOOL bEnabled);
    void SetPostProcessEnabled(BOOL bEnabled);
    BOOL IsShadowEnabled() const;
    BOOL IsPostProcessEnabled() const;

    // ========== 获取管理器 ==========
    D3D12ShadowManager* GetShadowManager() const;
    D3D12PostProcessManager* GetPostProcessManager() const;
    D3D12TerrainManager* GetTerrainManager() const;

private:
    // ========== 内部方法 ==========

    // 按材质排序（不透明对象）
    void SortOpaqueByMaterial();

    // 按深度排序（透明对象）
    void SortTransparentByDepth();

    // 渲染单个网格对象
    void RenderMeshObject(ID3D12GraphicsCommandList* pCommandList, D3D12MeshObject* pMesh);

    // 渲染单个地形对象
    void RenderTerrainObject(ID3D12GraphicsCommandList* pCommandList, D3D12HeightField* pTerrain);

    // 渲染精灵对象
    void RenderSpriteObject(ID3D12GraphicsCommandList* pCommandList, D3D12SpriteObject* pSprite);

    // 渲染字体对象
    void RenderFontObject(ID3D12GraphicsCommandList* pCommandList, D3D12FontObject* pFont);

    // 计算对象到相机的距离
    float CalculateDistanceToCamera(const XMFLOAT3& objectPos) const;

    // 设置资源屏障
    void SetResourceBarrier(ID3D12GraphicsCommandList* pCommandList,
                           ID3D12Resource* pResource,
                           D3D12_RESOURCE_STATES before,
                           D3D12_RESOURCE_STATES after);

private:
    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

    // ========== 管理器指针 ==========
    D3D12ShadowManager* m_pShadowManager;
    D3D12PostProcessManager* m_pPostProcessManager;
    D3D12TerrainManager* m_pTerrainManager;

    // ========== 渲染队列 ==========
    RenderQueue m_renderQueues[(UINT)RenderPhase::Count];

    // ========== 相机参数 ==========
    CameraParams m_camera;

    // ========== 配置 ==========
    BOOL m_bShadowEnabled;
    BOOL m_bPostProcessEnabled;

    // ========== 统计 ==========
    RenderStats m_stats;

    // ========== 状态标志 ==========
    BOOL m_bInitialized;
    BOOL m_bInFrame;
};

// ============================================
// 辅助宏
// ============================================

#define MAX_RENDER_ITEMS 10000
#define DEFAULT_SHADOW_CASCADE_COUNT 4
#define DEFAULT_SHADOW_RESOLUTION 2048
