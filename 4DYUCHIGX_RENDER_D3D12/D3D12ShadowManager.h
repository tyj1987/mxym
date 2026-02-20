// ============================================
// D3D12ShadowManager.h
// DirectX 12 阴影管理器
// 第 6 阶段 - 高级特性实现
// ============================================

#pragma once

// DirectX Math 配置
#define XM_NO_XMVECTOR_OVERLOADS 1
#define XM_NO_ALIGNMENT

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>


using namespace DirectX;

// 前向声明
class CoD3D12Device;

// ========== 阴影贴图配置 ==========
struct ShadowMapConfig
{
    UINT nResolution;          // 阴影贴图分辨率（1024, 2048, 4096）
    UINT nCascadeCount;        // 级联数量（1-4）
    float fCascadeSplits[4];   // 级联分割点
    float fBias;               // 阴影偏移
    float fNormalBias;         // 法线偏移
    BOOL bSoftShadows;         // 软阴影
    float fSoftness;           // 软阴影柔和度

    ShadowMapConfig()
        : nResolution(2048)
        , nCascadeCount(4)
        , fBias(0.001f)
        , fNormalBias(0.01f)
        , bSoftShadows(FALSE)
        , fSoftness(1.0f)
    {
        fCascadeSplits[0] = 5.0f;
        fCascadeSplits[1] = 15.0f;
        fCascadeSplits[2] = 50.0f;
        fCascadeSplits[3] = 150.0f;
    }
};

// ========== 级联阴影 ==========
struct CascadeShadow
{
    XMFLOAT4X4 matViewProj;         // 视图投影矩阵
    XMFLOAT4X4 matView;             // 视图矩阵
    XMFLOAT4X4 matProj;             // 投影矩阵
    XMFLOAT4X4 shadowMatrix;        // 阴影矩阵（新增）
    XMFLOAT4X4 projectionMatrix;    // 投影矩阵（新增）

    ComPtr<ID3D12Resource> pShadowMap;           // 阴影贴图资源
    ComPtr<ID3D12Resource> pDepthStencil;       // 深度模板

    D3D12_CPU_DESCRIPTOR_HANDLE hDSV;           // DSV 句柄
    D3D12_GPU_DESCRIPTOR_HANDLE hSRV;           // SRV 句柄

    XMFLOAT3 v3Center;              // 级联中心
    float fRadius;                  // 级联半径

    CascadeShadow()
        : fRadius(0.0f)
    {
        XMMATRIX identity = XMMatrixIdentity();
        XMStoreFloat4x4(&matViewProj, identity);
        XMStoreFloat4x4(&matView, identity);
        XMStoreFloat4x4(&matProj, identity);
        XMStoreFloat4x4(&shadowMatrix, identity);
        XMStoreFloat4x4(&projectionMatrix, identity);
        v3Center = XMFLOAT3{0.0f, 0.0f, 0.0f};
        ZeroMemory(&hDSV, sizeof(hDSV));
        ZeroMemory(&hSRV, sizeof(hSRV));
    }
};

// ========== 阴影常量缓冲区 ==========
struct ShadowConstants
{
    XMFLOAT4X4 matShadowViewProj[4];   // 阴影视图投影矩阵
    XMFLOAT4 v4SplitDistances;         // 分割距离
    XMFLOAT4 v4ShadowParams;           // 阴影参数（bias, normalBias, softness, reserved）
    UINT nCascadeCount;                 // 级联数量
    UINT nPadding[3];

    ShadowConstants()
    {
        for (int i = 0; i < 4; ++i)
        {
            matShadowViewProj[i] = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        }
        v4SplitDistances = XMFLOAT4(5.0f, 15.0f, 50.0f, 150.0f);
        v4ShadowParams = XMFLOAT4(0.001f, 0.01f, 1.0f, 0.0f);
        nCascadeCount = 4;
        ZeroMemory(nPadding, sizeof(nPadding));
    }
};

// ============================================
// D3D12ShadowManager 类
// 管理阴影贴图的创建、渲染和应用
// ============================================

class D3D12ShadowManager
{
public:
    D3D12ShadowManager();
    ~D3D12ShadowManager();

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice);
    void Release();

    // ========== 阴影贴图管理 ==========
    HRESULT CreateShadowMap(UINT nResolution, UINT nCascadeCount);
    void ReleaseShadowMap();

    // ========== 渲染阴影 ==========
    HRESULT BeginShadowMap(ID3D12GraphicsCommandList* pCommandList, UINT cascadeIndex);
    HRESULT EndShadowMap(ID3D12GraphicsCommandList* pCommandList, UINT cascadeIndex);
    void RenderShadowMap(ID3D12GraphicsCommandList* pCommandList, const XMFLOAT3& lightDir, const XMFLOAT3& cameraPos, const XMFLOAT4X4& cameraView);

    // ========== 配置 ==========
    void SetShadowConfig(const ShadowMapConfig& config);
    const ShadowMapConfig& GetShadowConfig() const;

    // ========== 更新常量 ==========
    void UpdateShadowConstants(ID3D12GraphicsCommandList* pCommandList);

    // ========== 获取阴影贴图 ==========
    D3D12_GPU_DESCRIPTOR_HANDLE GetShadowMapSRV(UINT cascadeIndex) const;
    UINT GetCascadeCount() const;
    const CascadeShadow* GetCascade(UINT index) const;

    // ========== 辅助方法 ==========
    void CalculateCascadeSplits(float fNear, float fFar);
    void UpdateCascadeMatrices(const XMFLOAT3& lightDir, const XMFLOAT3& cameraPos, const XMFLOAT4X4& cameraView, const XMFLOAT4X4& cameraProj, UINT cascadeIndex);
    void RenderShadowMap(ID3D12GraphicsCommandList* pCommandList, const XMFLOAT3& lightDir, const XMFLOAT3& cameraPos, const XMFLOAT4X4& cameraView, const XMFLOAT4X4& cameraProj);

private:
    // ========== 内部方法 ==========
    HRESULT CreateShadowMapResources(UINT nResolution, UINT nCascadeCount);
    HRESULT CreateShadowRootSignature();
    HRESULT CreateShadowPSO();
    HRESULT CreateShadowMapView(UINT cascadeIndex);
    HRESULT CreateShadowConstantBuffer();
    void UpdateShadowConstantBuffer();
    void RenderSceneToShadowMap(ID3D12GraphicsCommandList* pCommandList);
    void CalculateCascadeAABB(const XMFLOAT3& cameraPos, const XMFLOAT4X4& cameraView, const XMFLOAT4X4& cameraProj, float fNear, float fFar, XMFLOAT3& vCenter, XMFLOAT3& vExtents);

private:
    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

    // ========== 配置 ==========
    ShadowMapConfig m_config;

    // ========== 级联阴影 ==========
    std::vector<CascadeShadow> m_vCascades;

    // ========== 描述符堆 ==========
    ComPtr<ID3D12DescriptorHeap> m_pShadowMapDSVHeap;
    ComPtr<ID3D12DescriptorHeap> m_pShadowMapSRVHeap;

    // ========== 常量缓冲区 ==========
    ComPtr<ID3D12Resource> m_pShadowConstantBuffer;
    ShadowConstants m_shadowConstants;

    // ========== 根签名和 PSO ==========
    ComPtr<ID3D12RootSignature> m_pShadowRootSignature;
    ComPtr<ID3D12PipelineState> m_pShadowPSO;

    // ========== 级联分割 ==========
    float m_fCascadeSplits[5];  // 级联分割距离（4个级联 + 1个远平面）

    // ========== 状态标志 ==========
    BOOL m_bInitialized;
    UINT m_nCurrentCascadeIndex;  // 当前级联索引
};

// ============================================
// 辅助宏
// ============================================

#define MAX_CASCADE_COUNT 4
#define DEFAULT_SHADOW_RESOLUTION 2048
#define MIN_SHADOW_RESOLUTION 512
#define MAX_SHADOW_RESOLUTION 8192
