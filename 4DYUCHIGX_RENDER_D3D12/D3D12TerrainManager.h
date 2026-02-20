// ============================================
// D3D12TerrainManager.h
// DirectX 12 地形管理器
// 第 7 阶段 - HeightField 地形系统实现
// ============================================

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>



namespace DirectX { using namespace DirectX; }

// 前向声明
class D3D12HeightField;
class CoD3D12Device;

// ========== 地形 LOD 配置 ==========
struct TerrainLODConfig
{
    UINT nLODCount;           // LOD 层级数
    float fLODDistance[8];    // 切换距离
    BOOL bEnableGeomorph;      // 几何克隆
    float fMorphSpeed;         // 克隆速度

    TerrainLODConfig()
        : nLODCount(4)
        , bEnableGeomorph(TRUE)
        , fMorphSpeed(2.0f)
    {
        fLODDistance[0] = 50.0f;
        fLODDistance[1] = 100.0f;
        fLODDistance[2] = 200.0f;
        fLODDistance[3] = 400.0f;
    }
};

// ============================================
// D3D12TerrainManager 类
// 管理所有地形对象
// ============================================

class D3D12TerrainManager
{
public:
    D3D12TerrainManager();
    ~D3D12TerrainManager();

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice);
    void Release();

    // ========== 地形创建 ==========
    D3D12HeightField* CreateTerrain();
    void DestroyTerrain(D3D12HeightField* pTerrain);
    void* AddTerrain(D3D12HeightField* pTerrain);
    void RemoveTerrain(void* pHandle);
    D3D12HeightField* GetTerrain(void* pHandle);

    // ========== 渲染 ==========
    void RenderAll(ID3D12GraphicsCommandList* pCommandList);
    void Update(const XMFLOAT3& cameraPos);

    // ========== LOD 管理 ==========
    void SetLODConfig(const TerrainLODConfig& config);
    const TerrainLODConfig& GetLODConfig() const;

    // ========== 统计 ==========
    UINT GetTerrainCount() const;
    UINT GetTotalPatchCount() const;
    UINT GetVisiblePatchCount() const;

private:
    // ========== 内部方法 ==========
    void UpdateCulling(const XMFLOAT3& cameraPos);

private:
    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

    // ========== 地形映射表 ==========
    std::map<void*, D3D12HeightField*> m_mTerrainMap;

    // ========== LOD 配置 ==========
    TerrainLODConfig m_lodConfig;

    // ========== 统计 ==========
    UINT m_nVisiblePatchCount;

    // ========== Handle 生成 ==========
    UINT_PTR m_nNextHandle;

    // ========== 状态标志 ==========
    BOOL m_bInitialized;
};

// ============================================
// 辅助宏
// ============================================

#define MAX_TERRAIN_LODS 8
