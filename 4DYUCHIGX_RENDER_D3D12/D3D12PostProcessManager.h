// ============================================
// D3D12PostProcessManager.h
// DirectX 12 后期处理管理器
// 第 6 阶段 - 高级特性实现
// ============================================

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>



namespace DirectX { using namespace DirectX; }

// 前向声明
class CoD3D12Device;

// ========== Bloom 配置 ==========
struct BloomConfig
{
    float fThreshold;         // 亮度阈值
    float fIntensity;         // 泛光强度
    UINT nBlurPasses;         // 模糊迭代次数
    float fBlurSize;          // 模糊半径
    BOOL bEnabled;            // 是否启用

    BloomConfig()
        : fThreshold(0.8f)
        , fIntensity(1.0f)
        , nBlurPasses(3)
        , fBlurSize(2.0f)
        , bEnabled(FALSE)
    {}
};

// ========== Motion Blur 配置 ==========
struct MotionBlurConfig
{
    float fIntensity;         // 模糊强度
    UINT nSampleCount;        // 采样数量
    BOOL bVelocityBuffer;     // 是否使用速度缓冲区
    BOOL bEnabled;            // 是否启用

    MotionBlurConfig()
        : fIntensity(0.5f)
        , nSampleCount(16)
        , bVelocityBuffer(FALSE)
        , bEnabled(FALSE)
    {}
};

// ========== Depth of Field 配置 ==========
struct DoFConfig
{
    float fFocusDistance;     // 对焦距离
    float fFocusRange;        // 对焦范围
    float fBlurNear;          // 近距离模糊强度
    float fBlurFar;           // 远距离模糊强度
    BOOL bEnabled;            // 是否启用

    DoFConfig()
        : fFocusDistance(10.0f)
        , fFocusRange(5.0f)
        , fBlurNear(1.0f)
        , fBlurFar(1.0f)
        , bEnabled(FALSE)
    {}
};

// ========== Tone Mapping 配置 ==========
struct ToneMappingConfig
{
    UINT nMethod;             // 0=None, 1=ACES, 2=Reinhard, 3=Filmic
    float fExposure;          // 曝光
    float fGamma;             // 伽马
    BOOL bEnabled;            // 是否启用

    ToneMappingConfig()
        : nMethod(1)  // ACES
        , fExposure(1.0f)
        , fGamma(2.2f)
        , bEnabled(TRUE)
    {}
};

// ========== 后期处理缓冲区 ==========
struct PostProcessBuffer
{
    ComPtr<ID3D12Resource> pSceneColor;        // 场景颜色
    ComPtr<ID3D12Resource> pBloomExtract;      // Bloom 亮度提取
    ComPtr<ID3D12Resource> pBloomBlur[2];      // Bloom 模糊（乒乓缓冲）
    ComPtr<ID3D12Resource> pMotionVelocity;    // 运动速度
    ComPtr<ID3D12Resource> pFinalOutput;       // 最终输出

    D3D12_CPU_DESCRIPTOR_HANDLE hRTVScene;
    D3D12_CPU_DESCRIPTOR_HANDLE hRTVBloomExtract;
    D3D12_CPU_DESCRIPTOR_HANDLE hRTVBloomBlur[2];
    D3D12_GPU_DESCRIPTOR_HANDLE hSRVScene;
    D3D12_GPU_DESCRIPTOR_HANDLE hSRVBloomExtract;
    D3D12_GPU_DESCRIPTOR_HANDLE hSRVBloomBlur[2];

    UINT nWidth;
    UINT nHeight;

    PostProcessBuffer()
        : nWidth(0)
        , nHeight(0)
    {
        ZeroMemory(&hRTVScene, sizeof(hRTVScene));
        ZeroMemory(&hRTVBloomExtract, sizeof(hRTVBloomExtract));
        ZeroMemory(&hRTVBloomBlur, sizeof(hRTVBloomBlur));
        ZeroMemory(&hSRVScene, sizeof(hSRVScene));
        ZeroMemory(&hSRVBloomExtract, sizeof(hSRVBloomExtract));
        ZeroMemory(&hSRVBloomBlur, sizeof(hSRVBloomBlur));
    }
};

// ============================================
// D3D12PostProcessManager 类
// 管理所有后期特效
// ============================================

class D3D12PostProcessManager
{
public:
    D3D12PostProcessManager();
    ~D3D12PostProcessManager();

    // ========== 初始化 ==========
    HRESULT Initialize(CoD3D12Device* pDevice, UINT nWidth, UINT nHeight);
    void Release();

    // ========== 缓冲区管理 ==========
    HRESULT CreateBuffers(UINT nWidth, UINT nHeight);
    void ReleaseBuffers();
    void ResizeBuffers(UINT nWidth, UINT nHeight);

    // ========== Bloom ==========
    HRESULT ApplyBloom(ID3D12GraphicsCommandList* pCommandList);
    HRESULT ExtractBrightness(ID3D12GraphicsCommandList* pCommandList);
    HRESULT BlurBloom(ID3D12GraphicsCommandList* pCommandList, UINT nPasses);
    HRESULT CombineBloom(ID3D12GraphicsCommandList* pCommandList);

    // ========== Motion Blur ==========
    HRESULT ApplyMotionBlur(ID3D12GraphicsCommandList* pCommandList);

    // ========== Depth of Field ==========
    HRESULT ApplyDepthOfField(ID3D12GraphicsCommandList* pCommandList);

    // ========== Tone Mapping ==========
    HRESULT ApplyToneMapping(ID3D12GraphicsCommandList* pCommandList);

    // ========== 完整流程 ==========
    HRESULT Process(ID3D12GraphicsCommandList* pCommandList);

    // ========== 配置 ==========
    void SetBloomConfig(const BloomConfig& config);
    void SetMotionBlurConfig(const MotionBlurConfig& config);
    void SetDoFConfig(const DoFConfig& config);
    void SetToneMappingConfig(const ToneMappingConfig& config);

    const BloomConfig& GetBloomConfig() const;
    const MotionBlurConfig& GetMotionBlurConfig() const;
    const DoFConfig& GetDoFConfig() const;
    const ToneMappingConfig& GetToneMappingConfig() const;

    // ========== 获取资源 ==========
    ID3D12Resource* GetSceneColor() const;
    ID3D12Resource* GetFinalOutput() const;

private:
    // ========== 内部方法 ==========
    HRESULT CreateBloomResources();
    HRESULT CreateBloomPSOs();
    HRESULT BlurBloomPass(ID3D12GraphicsCommandList* pCommandList, UINT nSrcIndex, UINT nDestIndex);

private:
    // ========== 设备指针 ==========
    CoD3D12Device* m_pDevice;

    // ========== 配置 ==========
    BloomConfig m_bloomConfig;
    MotionBlurConfig m_motionBlurConfig;
    DoFConfig m_dofConfig;
    ToneMappingConfig m_toneMappingConfig;

    // ========== 缓冲区 ==========
    PostProcessBuffer m_buffers;

    // ========== 描述符堆 ==========
    ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
    ComPtr<ID3D12DescriptorHeap> m_pSRVHeap;

    // ========== 管线状态对象 ==========
    ComPtr<ID3D12PipelineState> m_pBloomExtractPSO;
    ComPtr<ID3D12PipelineState> m_pBloomBlurPSO;
    ComPtr<ID3D12PipelineState> m_pBloomCombinePSO;
    ComPtr<ID3D12PipelineState> m_pToneMappingPSO;

    // ========== 根签名 ==========
    ComPtr<ID3D12RootSignature> m_pPostProcessRootSignature;

    // ========== 状态标志 ==========
    BOOL m_bInitialized;
};

// ============================================
// 辅助宏
// ============================================

#define DEFAULT_BLOOM_THRESHOLD 0.8f
#define DEFAULT_BLOOM_INTENSITY 1.0f
#define DEFAULT_MOTION_BLUR_INTENSITY 0.5f
#define DEFAULT_DOF_FOCUS_DISTANCE 10.0f
