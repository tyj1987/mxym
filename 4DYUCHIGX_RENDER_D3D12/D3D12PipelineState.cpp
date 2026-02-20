// ============================================
// D3D12PipelineState.cpp
// DirectX 12 管线状态对象实现
// ============================================

// 必须首先包含预编译头
#include "D3D12PCH.h"

#include "D3D12PipelineState.h"
#include "D3D12Fixes.h"
#include "d3dx12.h"
#include <stdexcept>

D3D12PipelineState::D3D12PipelineState()
    : m_renderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
    , m_depthStencilFormat(DXGI_FORMAT_D32_FLOAT)
    , m_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
    , m_bInitialized(FALSE)
{
}

D3D12PipelineState::~D3D12PipelineState()
{
    Release();
}

HRESULT D3D12PipelineState::Initialize(
    ID3D12Device* pDevice,
    ID3D12RootSignature* pRootSignature,
    const D3D12_SHADER_BYTECODE& vertexShader,
    const D3D12_SHADER_BYTECODE& pixelShader,
    const D3D12_INPUT_LAYOUT_DESC& inputLayout,
    DXGI_FORMAT renderTargetFormat,
    DXGI_FORMAT depthStencilFormat)
{
    if (!pDevice || !pRootSignature)
        return E_INVALIDARG;

    m_pDevice = pDevice;
    m_pRootSignature = pRootSignature;
    m_vertexShader = vertexShader;
    m_pixelShader = pixelShader;
    m_inputLayout = inputLayout;
    m_renderTargetFormat = renderTargetFormat;
    m_depthStencilFormat = depthStencilFormat;

    HRESULT hr = CreatePipelineState();
    if (SUCCEEDED(hr))
    {
        m_bInitialized = TRUE;
    }

    return hr;
}

void D3D12PipelineState::SetBlendState(const BlendConfig& config)
{
    m_blendConfig = config;
    if (m_bInitialized)
    {
        CreatePipelineState();
    }
}

void D3D12PipelineState::SetRasterizerState(const RasterizerConfig& config)
{
    m_rasterConfig = config;
    if (m_bInitialized)
    {
        CreatePipelineState();
    }
}

void D3D12PipelineState::SetDepthStencilState(const DepthStencilConfig& config)
{
    m_depthStencilConfig = config;
    if (m_bInitialized)
    {
        CreatePipelineState();
    }
}

void D3D12PipelineState::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
{
    m_topology = topology;
    if (m_bInitialized)
    {
        CreatePipelineState();
    }
}

HRESULT D3D12PipelineState::CreatePipelineState()
{
    if (!m_pDevice)
        return E_INVALIDARG;

    // 设置光栅化状态
    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = m_rasterConfig.fillMode;
    rasterizerDesc.CullMode = m_rasterConfig.cullMode;
    rasterizerDesc.FrontCounterClockwise = m_rasterConfig.bFrontCounterClockwise;
    rasterizerDesc.DepthBias = m_rasterConfig.depthBias;
    rasterizerDesc.DepthBiasClamp = m_rasterConfig.depthBiasClamp;
    rasterizerDesc.SlopeScaledDepthBias = m_rasterConfig.slopedDepthBias;
    rasterizerDesc.DepthClipEnable = m_rasterConfig.bDepthClipEnable;
    rasterizerDesc.MultisampleEnable = m_rasterConfig.bMultisampleEnable;
    rasterizerDesc.AntialiasedLineEnable = m_rasterConfig.bAntialiasedLineEnable;
    rasterizerDesc.ForcedSampleCount = m_rasterConfig.forcedSampleCount;
    rasterizerDesc.ConservativeRaster = m_rasterConfig.conservativeRaster;

    // 设置混合状态
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = m_blendConfig.bAlphaToCoverageEnable;
    blendDesc.IndependentBlendEnable = m_blendConfig.bIndependentBlendEnable;
    for (int i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i] = m_blendConfig.renderTargetBlendDesc[i];
    }

    // 设置深度模板状态
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = m_depthStencilConfig.bDepthEnable;
    depthStencilDesc.StencilEnable = m_depthStencilConfig.bStencilEnable;
    depthStencilDesc.DepthWriteMask = m_depthStencilConfig.depthWriteMask;
    depthStencilDesc.DepthFunc = m_depthStencilConfig.depthFunc;
    depthStencilDesc.FrontFace.StencilFailOp = m_depthStencilConfig.frontFace.StencilFailOp;
    depthStencilDesc.FrontFace.StencilDepthFailOp = m_depthStencilConfig.frontFace.StencilDepthFailOp;
    depthStencilDesc.FrontFace.StencilPassOp = m_depthStencilConfig.frontFace.StencilPassOp;
    depthStencilDesc.FrontFace.StencilFunc = m_depthStencilConfig.frontFace.StencilFunc;
    depthStencilDesc.BackFace.StencilFailOp = m_depthStencilConfig.backFace.StencilFailOp;
    depthStencilDesc.BackFace.StencilDepthFailOp = m_depthStencilConfig.backFace.StencilDepthFailOp;
    depthStencilDesc.BackFace.StencilPassOp = m_depthStencilConfig.backFace.StencilPassOp;
    depthStencilDesc.BackFace.StencilFunc = m_depthStencilConfig.backFace.StencilFunc;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // 设置流输出（暂不使用）
    D3D12_STREAM_OUTPUT_DESC streamOutputDesc = {};

    // 创建管线状态描述
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = m_inputLayout;
    psoDesc.pRootSignature = m_pRootSignature.Get();
    psoDesc.VS = m_vertexShader;
    psoDesc.PS = m_pixelShader;
    psoDesc.GS = CD3DX12_SHADER_BYTECODE(nullptr, 0);
    psoDesc.DS = CD3DX12_SHADER_BYTECODE(nullptr, 0);
    psoDesc.HS = CD3DX12_SHADER_BYTECODE(nullptr, 0);
    psoDesc.StreamOutput = streamOutputDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_renderTargetFormat;
    psoDesc.DSVFormat = m_depthStencilFormat;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.PrimitiveTopologyType = m_topology;

    HRESULT hr = m_pDevice->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_pPipelineState));

    return hr;
}

void D3D12PipelineState::Release()
{
    m_pPipelineState.Reset();
    m_pRootSignature.Reset();
    m_pDevice.Reset();
    m_bInitialized = FALSE;
}
