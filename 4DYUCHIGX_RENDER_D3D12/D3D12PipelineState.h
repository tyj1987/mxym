// ============================================
// D3D12PipelineState.h
// DirectX 12 管线状态对象类
// ============================================

#pragma once

#include "D3D12PCH.h"
#include <DirectXMath.h>
#include "D3D12Fixes.h"

// 渲染状态配置
struct BlendConfig
{
    BOOL bAlphaToCoverageEnable;
    BOOL bIndependentBlendEnable;
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc[8];

    BlendConfig()
        : bAlphaToCoverageEnable(FALSE)
        , bIndependentBlendEnable(FALSE)
    {
        for (int i = 0; i < 8; ++i)
        {
            renderTargetBlendDesc[i].BlendEnable = FALSE;
            renderTargetBlendDesc[i].LogicOpEnable = FALSE;
            renderTargetBlendDesc[i].SrcBlend = D3D12_BLEND_ONE;
            renderTargetBlendDesc[i].DestBlend = D3D12_BLEND_ZERO;
            renderTargetBlendDesc[i].BlendOp = D3D12_BLEND_OP_ADD;
            renderTargetBlendDesc[i].SrcBlendAlpha = D3D12_BLEND_ONE;
            renderTargetBlendDesc[i].DestBlendAlpha = D3D12_BLEND_ZERO;
            renderTargetBlendDesc[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            renderTargetBlendDesc[i].LogicOp = D3D12_LOGIC_OP_NOOP;
            renderTargetBlendDesc[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }
    }
};

struct RasterizerConfig
{
    D3D12_FILL_MODE fillMode;
    D3D12_CULL_MODE cullMode;
    BOOL bFrontCounterClockwise;
    INT depthBias;
    FLOAT depthBiasClamp;
    FLOAT slopedDepthBias;
    BOOL bDepthClipEnable;
    BOOL bMultisampleEnable;
    BOOL bAntialiasedLineEnable;
    UINT forcedSampleCount;
    D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster;

    RasterizerConfig()
        : fillMode(D3D12_FILL_MODE_SOLID)
        , cullMode(D3D12_CULL_MODE_BACK)
        , bFrontCounterClockwise(FALSE)
        , depthBias(0)
        , depthBiasClamp(0.0f)
        , slopedDepthBias(0.0f)
        , bDepthClipEnable(TRUE)
        , bMultisampleEnable(FALSE)
        , bAntialiasedLineEnable(FALSE)
        , forcedSampleCount(0)
        , conservativeRaster(D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF)
    {}
};

struct DepthStencilConfig
{
    BOOL bDepthEnable;
    BOOL bStencilEnable;
    D3D12_DEPTH_WRITE_MASK depthWriteMask;
    D3D12_COMPARISON_FUNC depthFunc;
    D3D12_DEPTH_STENCILOP_DESC frontFace;
    D3D12_DEPTH_STENCILOP_DESC backFace;

    DepthStencilConfig()
        : bDepthEnable(TRUE)
        , bStencilEnable(FALSE)
        , depthWriteMask(D3D12_DEPTH_WRITE_MASK_ALL)
        , depthFunc(D3D12_COMPARISON_FUNC_LESS)
    {
        frontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        frontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        frontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        frontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        backFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        backFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        backFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        backFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    }
};

class D3D12PipelineState
{
public:
    D3D12PipelineState();
    ~D3D12PipelineState();

    // 初始化管线状态
    HRESULT Initialize(
        ID3D12Device* pDevice,
        ID3D12RootSignature* pRootSignature,
        const D3D12_SHADER_BYTECODE& vertexShader,
        const D3D12_SHADER_BYTECODE& pixelShader,
        const D3D12_INPUT_LAYOUT_DESC& inputLayout,
        DXGI_FORMAT renderTargetFormat,
        DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D32_FLOAT);

    // 设置混合状态
    void SetBlendState(const BlendConfig& config);
    const BlendConfig& GetBlendState() const { return m_blendConfig; }

    // 设置光栅化状态
    void SetRasterizerState(const RasterizerConfig& config);
    const RasterizerConfig& GetRasterizerState() const { return m_rasterConfig; }

    // 设置深度模板状态
    void SetDepthStencilState(const DepthStencilConfig& config);
    const DepthStencilConfig& GetDepthStencilState() const { return m_depthStencilConfig; }

    // 设置图元拓扑
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);
    D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopology() const { return m_topology; }

    // 获取管线状态对象
    ID3D12PipelineState* GetPipelineState() const { return m_pPipelineState.Get(); }

    // 获取根签名
    ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature.Get(); }

    // 释放资源
    void Release();

private:
    HRESULT CreatePipelineState();

private:
    ComPtr<ID3D12Device> m_pDevice;
    ComPtr<ID3D12PipelineState> m_pPipelineState;
    ComPtr<ID3D12RootSignature> m_pRootSignature;

    BlendConfig m_blendConfig;
    RasterizerConfig m_rasterConfig;
    DepthStencilConfig m_depthStencilConfig;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topology;

    D3D12_SHADER_BYTECODE m_vertexShader;
    D3D12_SHADER_BYTECODE m_pixelShader;
    D3D12_INPUT_LAYOUT_DESC m_inputLayout;
    DXGI_FORMAT m_renderTargetFormat;
    DXGI_FORMAT m_depthStencilFormat;

    BOOL m_bInitialized;
};
